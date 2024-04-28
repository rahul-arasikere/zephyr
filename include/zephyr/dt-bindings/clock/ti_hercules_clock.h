/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_
#define INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_

/** @enum hercules_clk_srcs maps the various clock sources to
 * the register bits in the CSDIS, CSDISSET and CSDISCLR registers.
 */
enum hercules_clk_srcs {
	OSCILLATOR = (1 << 0),
	PLL1 = (1 << 1),
	RESERVED = (1 << 2),
	EXTCLKIN = (1 << 3),
	LF_LPO = (1 << 4),
	HF_LPO = (1 << 5),
	PLL2 = (1 << 6),
	EXTCLKIN2 = (1 << 7),
};

#endif /* INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_ */
