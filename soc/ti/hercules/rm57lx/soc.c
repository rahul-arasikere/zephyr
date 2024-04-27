/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/drivers/hwinfo.h>


void z_arm_platform_init(void) {
    uint32_t reset_cause;
    __ASSERT_NO_MSG(hwinfo_get_reset_cause(&reset_cause) == 0);
    if((reset_cause & RESET_POR) ^ (reset_cause & RESET_HARDWARE) ) {
        /* Initialize L2RAM to avoid ECC errors */

    }
    hwinfo_clear_reset_cause();
}
