#ifndef EQSP32_BUILD_CONFIG_H
#define EQSP32_BUILD_CONFIG_H

/*
 * Prepared configuration for the precompiled EQSP32 (normal) package.
 * This file must be distributed only with the matching normal archive.
 * Package consumers select the library and do not define build-mode flags.
 */
#if defined(EQSP32_LITE)
    #error "EQSP32_LITE is producer-only. Remove the flag and select the EQSP32 or EQSP32_Lite package."
#endif

#define EQSP32_PRECOMPILED_PACKAGE 1
#define EQSP32_BUILD_VARIANT_LITE 0
#define EQSP32_IOT_ENABLED 1
#define EQSP32_LIBRARY_VARIANT_NAME "EQSP32"

#endif
