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
	OSCILLATOR = 0,
	PLL1 = 1,
	RESERVED = 2,
	EXTCLKIN = 3,
	LF_LPO = 4,
	HF_LPO = 5,
	PLL2 = 6,
	EXTCLKIN2 = 7,
	VCLK, /* Any other value usually defaults to VCLK as the source.*/
};

enum hercules_clk_domains {
	GCLK1 = (1 << 0),
	HCLK = (1 << 1),
	VCLK_PERIPH = (1 << 2),
	VCLK2 = (1 << 3),
	VCLKA = (0b11 << 4),
	RTICLK1 = (1 << 6),
	VCLK3 = (1 << 8),
	VCLKA4 = (1 << 11),
};

#endif /* INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_ */
