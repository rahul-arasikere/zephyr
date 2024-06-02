/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/drivers/hwinfo.h>

void z_arm_platform_init(void)
{
	uint32_t reset_cause;
	volatile struct hercules_syscon_1_regs *sys_regs_1 = (void *)DT_REG_ADDR(SYS1_NODE);
	__ASSERT_NO_MSG(hwinfo_get_reset_cause(&reset_cause) == 0);
	if ((reset_cause & RESET_POR) ^ (reset_cause & RESET_HARDWARE)) {
		/* Initialize L2RAM to avoid ECC errors */
		sys_regs_1->MINITGCR = 0xA; // Enable memory initialization
		sys_regs_1->MSINENA = 0x1;  // Initialize memory
		while (sys_regs_1->MSTCGSTAT & BIT(8))
			;                   // Await for initialization
		sys_regs_1->MINITGCR = 0x5; // Disable memory initialization
	}
	hwinfo_clear_reset_cause();
}
