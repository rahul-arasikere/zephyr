/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>

static int ti_rm57lx_init(void)
{
    return 0;
}

SYS_INIT(ti_rm57lx_init, PRE_KERNEL_1, 0);
