/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/hwinfo.h>
#include <string.h>
#include <soc.h>

#define SYS_EXCEPTION (*(volatile uint32_t *)0xffffffe4U)
#define DEVICE_ID_REV (*(volatile uint32_t *)0xfffffff0U)

enum rm57lx_reset_bits {
	POWERON_RESET = 0x8000U,
	OSC_FAILURE_RESET = 0x4000U,
	WATCHDOG_RESET = 0x2000U,
	WATCHDOG2_RESET = 0x1000U,
	DEBUG_RESET = 0x0800U,
	INTERCONNECT_RESET = 0x0080U,
	CPU0_RESET = 0x0020U,
	SW_RESET = 0x0010U,
	EXT_RESET = 0x0008U,
	NO_RESET = 0x0000U
};

ssize_t z_impl_hwinfo_get_device_id(uint8_t *buffer, size_t length)
{
	uint32_t id = DEVICE_ID_REV;
	if (length > sizeof(id)) {
		length = sizeof(id);
	}
	memcpy(buffer, &id, length);
	return length;
}

int z_impl_hwinfo_get_reset_cause(uint32_t *cause)
{
	uint32_t flags = 0;
	if ((SYS_EXCEPTION & (uint32_t)POWERON_RESET) != 0U) {
		flags = RESET_POR;
	} else if ((SYS_EXCEPTION & (uint32_t)EXT_RESET) != 0U) {
		if ((SYS_EXCEPTION & (uint32_t)OSC_FAILURE_RESET) != 0U) {
			flags = RESET_CLOCK;
		} else if ((SYS_EXCEPTION &
			    ((uint32_t)WATCHDOG_RESET | (uint32_t)WATCHDOG2_RESET)) != 0U) {
			flags = RESET_WATCHDOG;
		} else if ((SYS_EXCEPTION & (uint32_t)SW_RESET) != 0U) {
			flags = RESET_SOFTWARE;
		} else {
			flags = RESET_HARDWARE;
		}
	} else if ((SYS_EXCEPTION & (uint32_t)DEBUG_RESET) != 0U) {
		flags = RESET_DEBUG;
	} else if ((SYS_EXCEPTION & (uint32_t)CPU0_RESET) != 0U) {
		flags = RESET_HARDWARE;
	}
	*cause = flags;
	return 0;
}

int z_impl_hwinfo_clear_reset_cause(void)
{
	SYS_EXCEPTION = (uint32_t)UINT32_MAX;
	return 0;
}

int z_impl_hwinfo_get_supported_reset_cause(uint32_t *supported)
{
	*supported = (RESET_POR | RESET_CLOCK | RESET_WATCHDOG | RESET_DEBUG | RESET_HARDWARE |
		      RESET_SOFTWARE);
	return 0;
}
