/**
 * @file EQSP32_HTTPS_REST_API_Demo.ino
 * @brief HTTPS REST API for SCADA polling, with a built-in browser SCADA page, using EQSP32 over Ethernet.
 *
 * This example demonstrates how to:
 * - Serve the values of EQSP32 terminals as a JSON REST endpoint over HTTPS with a self-signed certificate.
 * - Let a SCADA system, an HMI or any HTTPS client poll that endpoint, for example every 5 seconds.
 * - Serve a SCADA page from the EQSP32 itself, so the values can be watched live in a browser
 *   without any SCADA software installed.
 * - Keep pulse counts as running totals that a client can read as often as it needs.
 *
 * Values served (tags):
 * - pulse_counter_1   Terminal 9, PCC mode: running total of rising edges.
 * - pulse_counter_2   Terminal 10, PCC mode: running total of rising edges.
 * - digital_input     Terminal 3, DIN mode: 0 or 1.
 * - analog_input_v    Terminal 1, AIN mode: voltage in V.
 * - uptime_s          Seconds since the EQSP32 started.
 *
 * Simulated values:
 * - Set USE_SIMULATED_VALUES to true to try the example without sensors or wiring.
 *   The tags then show simulated values: counters rising at 450 pulses per minute, an input that
 *   toggles every 10 seconds and a slowly changing voltage. No terminal mode is changed.
 * - The JSON field "simulated" tells the client which kind of values it receives.
 *
 * Connectivity Notes:
 * - Ethernet is started and managed by the EQSP32 library. Wi-Fi and Erqos IoT services are disabled.
 * - DHCP is used by default. Connect the EQSP32 and the PC to the same LAN.
 * - SCADA page:     https://<device-ip>/
 * - REST endpoint:  https://<device-ip>/api/status  (GET, returns JSON)
 * - The device IP address is printed on the Serial Monitor (115200 baud) when the server starts.
 *
 * Setup Instructions (one-time HTTPS certificate):
 * - Create your own private key and self-signed certificate with OpenSSL. Replace 192.168.1.3 with
 *   the IP address of your EQSP32. Use a fixed address, so that the certificate keeps matching:
 *     openssl req -x509 -newkey rsa:2048 -sha256 -nodes -days 365 -keyout server-key.pem -out server-cert.pem -subj "/CN=EQSP32" -addext "subjectAltName=IP:192.168.1.3"
 * - Paste the complete contents of server-cert.pem and server-key.pem into SERVER_CERT and SERVER_KEY below.
 * - Trust server-cert.pem on the PC (browser or operating system) and in the certificate store of the
 *   SCADA client. A self-signed certificate encrypts the traffic but is not trusted automatically.
 * - Keep the key private. Do not share a sketch that contains it.
 *
 * Hardware Setup:
 * - EQSP32 with Ethernet, connected to a router or switch with DHCP. USB power is enough for a first test.
 * - Optional: pulse signals on terminals 9 and 10, a switch between the EQSP32 5V output (or VIn) and
 *   terminal 3, and a voltage signal on terminal 1. Inputs that are not wired read 0.
 *
 * Requirements:
 * - Arduino ESP32 core 2.0.17 (ESP-IDF 4.4 HTTPS server).
 *
 * Features:
 * - The page requests the JSON every 5 seconds, one request at a time, and updates without reloading.
 * - Values are marked stale when a request fails, and updates resume by themselves.
 * - A boot ID in the JSON shows whether the EQSP32 restarted between two readings.
 * - Connections that do not complete the secure handshake within 3 seconds are closed,
 *   so one silent client cannot hold up the others.
 * - Up to 4 HTTPS connections at the same time (ESP-IDF default).
 *
 * @author Erqos Technologies
 * @date 2026-10-15
 */

#include <EQSP32.h>
#include <WiFi.h>
#include <esp_https_server.h>
#include <esp_heap_caps.h>
#include <esp_idf_version.h>
#include <esp_timer.h>
#include <lwip/sockets.h>
#include <inttypes.h>
#include <math.h>

#if ESP_IDF_VERSION_MAJOR != 4
#error "This example uses the ESP-IDF 4.4 HTTPS server of Arduino ESP32 core 2.0.x."
#endif


// ================== SETTINGS ==================
#define USE_SIMULATED_VALUES    false       // true: simulated values, no sensors or wiring needed

#define PIN_PULSE_COUNTER_1     EQ_PIN_9    // PCC, running total
#define PIN_PULSE_COUNTER_2     EQ_PIN_10   // PCC, running total
#define PIN_DIGITAL_INPUT       EQ_PIN_3    // DIN
#define PIN_ANALOG_INPUT        EQ_PIN_1    // AIN


// ====== Replace these placeholders with your own certificate and key ======
// Paste each file's complete contents, including its BEGIN and END lines.
static const char SERVER_CERT[] = R"PEM(
PASTE THE COMPLETE CONTENTS OF server-cert.pem HERE
)PEM";

static const char SERVER_KEY[] = R"PEM(
PASTE THE COMPLETE CONTENTS OF server-key.pem HERE
)PEM";
// ==========================================================================


// ================== EQSP32 & SERVER SETUP ==================
EQSP32 eqsp32;
EQTimer sampleTimer(1000);          // Read the tags once per second
EQTimer serverRetryTimer(1000);     // Try to start the HTTPS server once per second

static httpd_handle_t server = nullptr;
static String serverAddress;
static bool demoReady = false;
static char bootId[17];

// Latest tag values. loop() writes them; the HTTPS server task reads them.
struct TagValues {
    int32_t  pulseCounter1;
    int32_t  pulseCounter2;
    int      digitalInput;
    float    analogInputV;
    uint32_t uptimeS;
    uint32_t sampleSeq;
};
static TagValues tags = {};
static portMUX_TYPE tagsLock = portMUX_INITIALIZER_UNLOCKED;

static bool networkConnected();
static void startServer();
static void stopServer();
static void tlsReady(esp_https_server_user_cb_arg_t* info);
static void handshakeGuard(void*);


// ================== YOUR APPLICATION VALUES ==================
// Called once per second from loop(). To add a tag: read it here, add it to the JSON
// in statusHandler() and add a line to the TAGS list of the page.
static void readTags() {
    TagValues v;
    uint64_t uptimeMs = esp_timer_get_time() / 1000;

#if USE_SIMULATED_VALUES
    v.pulseCounter1 = (int32_t)((uptimeMs * 3 / 400) & 0x7FFFFFFF);    // 7.5 pulses per second
    v.pulseCounter2 = (int32_t)((uptimeMs * 3 / 800) & 0x7FFFFFFF);    // Half that rate
    v.digitalInput  = (int)((uptimeMs / 10000) % 2);                   // Toggles every 10 s
    v.analogInputV  = 5.0f + 4.0f * sinf((float)(uptimeMs % 60000) * 2.0f * (float)M_PI / 60000.0f);
#else
    v.pulseCounter1 = eqsp32.readPin(PIN_PULSE_COUNTER_1);    // Running total, reading does not clear it
    v.pulseCounter2 = eqsp32.readPin(PIN_PULSE_COUNTER_2);
    v.digitalInput  = eqsp32.readPin(PIN_DIGITAL_INPUT);
    v.analogInputV  = CONVERT_AIN(eqsp32.readPin(PIN_ANALOG_INPUT));
#endif
    v.uptimeS = (uint32_t)(uptimeMs / 1000);

    portENTER_CRITICAL(&tagsLock);
    v.sampleSeq = tags.sampleSeq + 1;
    tags = v;
    portEXIT_CRITICAL(&tagsLock);
}


// ================== APPLICATION SETUP AND LOOP ==================
void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting EQSP32 HTTPS REST API demo for SCADA ...");

    if (strstr(SERVER_CERT, "PASTE") || strstr(SERVER_KEY, "PASTE")) {
        Serial.println("Paste your certificate and key into SERVER_CERT and SERVER_KEY first (see Setup Instructions).");
        return;     // No plain HTTP fallback
    }
    snprintf(bootId, sizeof(bootId), "%08" PRIx32 "%08" PRIx32, esp_random(), esp_random());
    xTaskCreate(handshakeGuard, "tls_guard", 4096, nullptr, tskIDLE_PRIORITY + 2, nullptr);

    // -- Ethernet only: the library starts Ethernet, Wi-Fi and Erqos IoT services stay off
    WiFi.persistent(false);
    EQSP32Configs config;
    config.disableErqosIoT = true;
    config.disableNetSwitching = true;
    eqsp32.begin(config);
    WiFi.mode(WIFI_OFF);

#if !USE_SIMULATED_VALUES
    // -- Configure the tag terminals
    eqsp32.pinMode(PIN_PULSE_COUNTER_1, PCC);
    eqsp32.configPCC(PIN_PULSE_COUNTER_1, ON_RISING, false);  // Running total: reading does not clear it
    eqsp32.pinMode(PIN_PULSE_COUNTER_2, PCC);
    eqsp32.configPCC(PIN_PULSE_COUNTER_2, ON_RISING, false);
    eqsp32.pinMode(PIN_DIGITAL_INPUT, DIN);
    eqsp32.pinMode(PIN_ANALOG_INPUT, AIN);
    Serial.println("Tags are read from terminals 1, 3, 9 and 10.");
#else
    Serial.println("Tags show simulated values (USE_SIMULATED_VALUES is true).");
#endif

    readTags();
    sampleTimer.start();
    serverRetryTimer.start();
    demoReady = true;
    Serial.println("Waiting for Ethernet ...");
}

void loop() {
    if (!demoReady) {
        delay(100);
        return;
    }

    // -- Read the tags once per second
    if (sampleTimer.isElapsed(true)) {
        readTags();
    }

    // -- Stop the server when Ethernet is lost or the address changes, start it again when connected
    bool connected = networkConnected();
    if (server && (!connected || eqsp32.ethernetIP() != serverAddress)) {
        stopServer();
    }
    if (connected && !server && serverRetryTimer.isElapsed(true)) {
        startServer();
    }

    delay(10);
}


// ================== SCADA PAGE ==================
// Served from flash. The page reads /api/status every 5 seconds; TAGS lists what it shows.
static const char SCADA_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<link rel="icon" href="data:,"><title>EQSP32 SCADA</title>
<style>
:root{font-family:system-ui,sans-serif;color:#18303d;background:#e9eef1}
*{box-sizing:border-box}body{margin:0}
.bar{display:flex;justify-content:space-between;align-items:center;gap:16px;padding:14px 24px;background:#16323f;color:#fff}
.bar b{font-size:18px}.bar small{color:#a9c3cf;margin-left:10px}
.state{padding:6px 14px;border-radius:20px;font-weight:700;font-size:13px;background:#3c5966}
.state[data-s=live]{background:#1e8a5a}.state[data-s=stale]{background:#c47f12}
main{max-width:1060px;margin:auto;padding:20px 16px}
.sim{display:none;padding:10px 14px;margin-bottom:16px;border-radius:8px;background:#fff1cc;color:#6d4c0c;font-weight:600}
.tags{display:grid;grid-template-columns:repeat(auto-fill,minmax(190px,1fr));gap:14px}
.tag{background:#fff;border:1px solid #d3dde3;border-left:5px solid #1e8a5a;border-radius:8px;padding:14px 16px}
.tag.stale{border-left-color:#c47f12}.tag.none{border-left-color:#9aaab3}
.name{font-weight:650}.src{font-size:12px;color:#5d7582;margin-top:2px}
.val{font-size:32px;font-weight:650;margin-top:10px;font-variant-numeric:tabular-nums}.unit{font-size:14px;color:#5d7582;margin-left:6px}
.panels{display:grid;grid-template-columns:1fr 1fr;gap:14px;margin-top:14px}
.panel{background:#fff;border:1px solid #d3dde3;border-radius:8px;padding:14px 16px;min-width:0}
h2{font-size:15px;margin:0 0 8px}.row{display:flex;justify-content:space-between;gap:12px;padding:6px 0;border-bottom:1px solid #edf1f3}
.row:last-child{border:0}code{font-family:ui-monospace,monospace;overflow-wrap:anywhere}a{color:#1b6f94}
pre{white-space:pre-wrap;overflow-wrap:anywhere;font-size:12px;margin:8px 0 0}
@media(max-width:700px){.panels{grid-template-columns:1fr}.bar{padding:12px 16px}#updated{display:none}}
</style></head><body>
<div class="bar"><div><b>EQSP32</b><small>SCADA view &middot; HTTPS REST</small></div>
<div><span id="updated" class="src" style="color:#a9c3cf;margin-right:10px"></span><span id="state" class="state">CONNECTING</span></div></div>
<main>
<div id="sim" class="sim">SIMULATED VALUES &middot; set USE_SIMULATED_VALUES to false in the sketch to read the terminals</div>
<div id="tags" class="tags"></div>
<div class="panels">
<section class="panel"><h2>Device</h2>
<div class="row"><span>Uptime</span><span id="uptime">&mdash;</span></div>
<div class="row"><span>Boot ID</span><code id="boot">&mdash;</code></div>
<div class="row"><span>Free memory</span><span id="heap">&mdash;</span></div>
<div class="row"><span>Lowest free memory</span><span id="heapmin">&mdash;</span></div></section>
<section class="panel"><h2>Connection</h2>
<div class="row"><span>REST endpoint</span><a href="/api/status"><code>GET /api/status</code></a></div>
<div class="row"><span>Polling</span><span>every 5 s</span></div>
<div class="row"><span>Last request time</span><span id="latency">&mdash;</span></div>
<div class="row"><span>Requests OK / failed</span><span id="polls">0 / 0</span></div>
<div class="row"><span>Message</span><span id="msg">Waiting for the first reply</span></div></section>
</div>
<details class="panel" style="margin-top:14px"><summary>Latest JSON reply</summary><pre id="json">&mdash;</pre></details>
</main><script>
'use strict';
// One line per tag: JSON key, name, source, unit and how to show the value.
const TAGS = [
  {key:'pulse_counter_1', name:'Pulse counter 1', src:'Terminal 9 · PCC', unit:'pulses', show:v => v.toLocaleString()},
  {key:'pulse_counter_2', name:'Pulse counter 2', src:'Terminal 10 · PCC', unit:'pulses', show:v => v.toLocaleString()},
  {key:'digital_input', name:'Digital input', src:'Terminal 3 · DIN', unit:'', show:v => v ? 'ON' : 'OFF'},
  {key:'analog_input_v', name:'Analog input', src:'Terminal 1 · AIN', unit:'V', show:v => v.toFixed(2)},
];
const POLL_MS = 5000, DEADLINE_MS = 5000;
const el = id => document.getElementById(id);
let ok = 0, failed = 0, lastGoodAt = null, lastBoot = null;
el('tags').innerHTML = TAGS.map(t => '<div class="tag none" id="t_' + t.key + '"><div class="name">' + t.name +
  '</div><div class="src">' + t.src + '</div><div class="val"><span>—</span><span class="unit">' + t.unit + '</span></div></div>').join('');
function setState(s, text) { el('state').dataset.s = s; el('state').textContent = text; }
function markTags(cls) { for (const t of TAGS) el('t_' + t.key).className = 'tag ' + cls; }
function valid(d) {
  const keys = TAGS.map(t => t.key).concat(['uptime_s','sample_seq','heap_free_bytes','heap_min_free_bytes']);
  return d && d.device === 'EQSP32' && typeof d.simulated === 'boolean' && /^[0-9a-f]{16}$/.test(d.boot_id) &&
    keys.every(k => typeof d[k] === 'number' && isFinite(d[k]));
}
function checkAge() {
  if (lastGoodAt !== null && performance.now() - lastGoodAt > POLL_MS + DEADLINE_MS) { setState('stale','STALE'); markTags('stale'); }
}
async function poll() {
  const started = performance.now(), ctrl = new AbortController();
  const timer = setTimeout(() => ctrl.abort(), DEADLINE_MS);
  try {
    const r = await fetch('/api/status', {cache:'no-store', signal:ctrl.signal});
    if (!r.ok) throw new Error('HTTP ' + r.status);
    const d = await r.json();
    if (!valid(d)) throw new Error('unexpected JSON');
    const restarted = lastBoot !== null && lastBoot !== d.boot_id;
    lastBoot = d.boot_id; lastGoodAt = performance.now(); ++ok;
    for (const t of TAGS) el('t_' + t.key).querySelector('span').textContent = t.show(d[t.key]);
    markTags('');
    el('sim').style.display = d.simulated ? 'block' : 'none';
    el('uptime').textContent = d.uptime_s.toLocaleString() + ' s';
    el('boot').textContent = d.boot_id;
    el('heap').textContent = (d.heap_free_bytes / 1024).toFixed(1) + ' KiB';
    el('heapmin').textContent = (d.heap_min_free_bytes / 1024).toFixed(1) + ' KiB';
    el('latency').textContent = (performance.now() - started).toFixed(0) + ' ms';
    el('updated').textContent = 'Updated ' + new Date().toLocaleTimeString();
    el('json').textContent = JSON.stringify(d, null, 2);
    el('msg').textContent = restarted ? 'EQSP32 restarted (new boot ID)' : 'OK';
    setState('live','LIVE');
  } catch (e) {
    ++failed; setState('stale', lastGoodAt === null ? 'NO DATA' : 'STALE'); markTags(lastGoodAt === null ? 'none' : 'stale');
    el('msg').textContent = (e.name === 'AbortError' ? 'Timeout' : e.message) + ', retrying';
  } finally {
    clearTimeout(timer); el('polls').textContent = ok + ' / ' + failed; checkAge();
    setTimeout(poll, Math.max(100, POLL_MS - (performance.now() - started)));   // One request at a time
  }
}
setInterval(checkAge, 1000);
poll();
</script></body></html>)HTML";


// ================== REST API HANDLERS ==================
static esp_err_t pageHandler(httpd_req_t* request) {
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    return httpd_resp_send(request, SCADA_PAGE, sizeof(SCADA_PAGE) - 1);
}

static esp_err_t statusHandler(httpd_req_t* request) {
    TagValues v;
    portENTER_CRITICAL(&tagsLock);
    v = tags;
    portEXIT_CRITICAL(&tagsLock);

    char body[400];
    int size = snprintf(body, sizeof(body),
        "{\"device\":\"EQSP32\",\"boot_id\":\"%s\",\"sample_seq\":%" PRIu32 ",\"simulated\":%s,"
        "\"pulse_counter_1\":%" PRId32 ",\"pulse_counter_2\":%" PRId32 ",\"digital_input\":%d,"
        "\"analog_input_v\":%.3f,\"uptime_s\":%" PRIu32 ",\"heap_free_bytes\":%u,\"heap_min_free_bytes\":%u}",
        bootId, v.sampleSeq, USE_SIMULATED_VALUES ? "true" : "false",
        v.pulseCounter1, v.pulseCounter2, v.digitalInput, v.analogInputV, v.uptimeS,
        (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),
        (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    if (size < 0 || (size_t)size >= sizeof(body)) {
        return httpd_resp_send_err(request, HTTPD_500_INTERNAL_SERVER_ERROR, "Reply too long");
    }
    httpd_resp_set_type(request, "application/json");
    httpd_resp_set_hdr(request, "Cache-Control", "no-store");
    return httpd_resp_send(request, body, size);
}


// ================== HTTPS SERVER ==================
static bool networkConnected() {
    return eqsp32.getEthernetStatus() == EQ_ETH_CONNECTED && eqsp32.ethernetIP() != "0.0.0.0";
}

static void startServer() {
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();     // 4 connections, oldest closed first
    config.cacert_pem = (const uint8_t*)SERVER_CERT;            // In ESP-IDF 4.4 this is the server's own certificate
    config.cacert_len = sizeof(SERVER_CERT);
    config.prvtkey_pem = (const uint8_t*)SERVER_KEY;
    config.prvtkey_len = sizeof(SERVER_KEY);
    config.user_cb = tlsReady;
    if (httpd_ssl_start(&server, &config) != ESP_OK) {
        server = nullptr;
        Serial.println("HTTPS server could not start. Check SERVER_CERT and SERVER_KEY.");
        return;
    }

    httpd_uri_t page = {};
    page.uri = "/";
    page.method = HTTP_GET;
    page.handler = pageHandler;
    httpd_uri_t status = {};
    status.uri = "/api/status";
    status.method = HTTP_GET;
    status.handler = statusHandler;
    httpd_register_uri_handler(server, &page);
    httpd_register_uri_handler(server, &status);

    serverAddress = eqsp32.ethernetIP();
    Serial.printf("SCADA page:    https://%s/\n", serverAddress.c_str());
    Serial.printf("REST endpoint: https://%s/api/status\n", serverAddress.c_str());
}

static void stopServer() {
    httpd_ssl_stop(server);
    server = nullptr;
    Serial.println("HTTPS server stopped. Waiting for Ethernet ...");
}


// ================== HANDSHAKE GUARD ==================
// ESP-IDF 4.4 completes the TLS handshake inside the HTTPS server task. A client that opens a
// connection and sends nothing would hold up every other client. This task shuts down connections
// that have not completed the handshake after 3 s; the HTTPS server then closes and frees them.
static const unsigned long HANDSHAKE_LIMIT_MS = 3000;

struct HandshakeState {
    sockaddr_storage peer;
    socklen_t peerLength;
    bool ready;             // Handshake completed
    bool shutdownSent;
};
static HandshakeState handshakes[CONFIG_LWIP_MAX_SOCKETS] = {};
static EQTimer handshakeTimers[CONFIG_LWIP_MAX_SOCKETS];   // Used by the guard task only
static portMUX_TYPE handshakeLock = portMUX_INITIALIZER_UNLOCKED;

// True for a connected socket on the HTTPS port; returns the client's address.
static bool httpsPeer(int fd, sockaddr_storage& peer, socklen_t& length) {
    sockaddr_storage local = {};
    socklen_t localLength = sizeof(local);
    length = sizeof(peer);
    memset(&peer, 0, sizeof(peer));
    if (getsockname(fd, (sockaddr*)&local, &localLength) != 0 ||
        getpeername(fd, (sockaddr*)&peer, &length) != 0) return false;
    if (local.ss_family == AF_INET)  return ntohs(((sockaddr_in*)&local)->sin_port) == 443;
    if (local.ss_family == AF_INET6) return ntohs(((sockaddr_in6*)&local)->sin6_port) == 443;
    return false;
}

// Called by the HTTPS server when a handshake has completed.
static void tlsReady(esp_https_server_user_cb_arg_t* info) {
    int fd = -1;
    if (esp_tls_get_conn_sockfd(const_cast<esp_tls_t*>(info->tls), &fd) != ESP_OK) return;
    int slot = fd - LWIP_SOCKET_OFFSET;
    sockaddr_storage peer = {};
    socklen_t length;
    if (slot < 0 || slot >= CONFIG_LWIP_MAX_SOCKETS || !httpsPeer(fd, peer, length)) return;
    portENTER_CRITICAL(&handshakeLock);
    handshakes[slot].peer = peer;
    handshakes[slot].peerLength = length;
    handshakes[slot].ready = true;
    portEXIT_CRITICAL(&handshakeLock);
}

static void handshakeGuard(void*) {
    for (;;) {
        for (int slot = 0; slot < CONFIG_LWIP_MAX_SOCKETS; ++slot) {
            int fd = LWIP_SOCKET_OFFSET + slot;
            sockaddr_storage peer = {};
            socklen_t length;
            bool connected = httpsPeer(fd, peer, length);
            bool overdue = handshakeTimers[slot].isElapsed();
            bool newPeer = false;
            bool expire = false;

            portENTER_CRITICAL(&handshakeLock);
            HandshakeState& state = handshakes[slot];
            if (!connected) {
                state = {};
            } else if (state.peerLength != length || memcmp(&state.peer, &peer, length) != 0) {
                state = {};             // A new connection on this socket
                state.peer = peer;
                state.peerLength = length;
                newPeer = true;
            } else if (!state.ready && !state.shutdownSent && overdue) {
                state.shutdownSent = true;
                expire = true;
            }
            portEXIT_CRITICAL(&handshakeLock);

            if (!connected) {
                handshakeTimers[slot].stop();
            } else if (newPeer) {
                handshakeTimers[slot].reset(HANDSHAKE_LIMIT_MS);
                handshakeTimers[slot].start();
            } else if (expire) {
                // Socket numbers are reused: check it is still the same client before the shutdown.
                sockaddr_storage current = {};
                socklen_t currentLength;
                if (httpsPeer(fd, current, currentLength) && currentLength == length &&
                    memcmp(&current, &peer, length) == 0 && shutdown(fd, SHUT_RDWR) == 0) {
                    Serial.println("Closed a connection that did not complete the handshake within 3 s.");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
