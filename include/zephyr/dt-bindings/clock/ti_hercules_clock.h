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

enum hercules_periph_quadrants {
	QUADRANT0 = (1 << 0),
	QUADRANT1 = (1 << 1),
	QUADRANT2 = (1 << 2),
	QUADRANT3 = (1 << 3),
};

enum hercules_periph_frames {
	PS0 = 0,
	PS1,
	PS2,
	PS3,
	PS4,
	PS5,
	PS6,
	PS7,
	PS8,
	PS9,
	PS10,
	PS11,
	PS12,
	PS13,
	PS14,
	PS15,
	PS16,
	PS17,
	PS18,
	PS19,
	PS20,
	PS21,
	PS22,
	PS23,
	PS24,
	PS25,
	PS26,
	PS27,
	PS28,
	PS29,
	PS30,
	PS31
};

enum hercules_priv_periph_frames {
	PPS0 = 0,
	PPS1,
	PPS2,
	PPS3,
	PPS4,
	PPS5,
	PPS6,
	PPS7
};

enum hercules_priv_ext_periph_frames {
	PPSE0 = 0,
	PPSE1,
	PPSE2,
	PPSE3,
	PPSE4,
	PPSE5,
	PPSE6,
	PPSE7,
	PPSE8,
	PPSE9,
	PPSE10,
	PPSE11,
	PPSE12,
	PPSE13,
	PPSE14,
	PPSE15,
	PPSE16,
	PPSE17,
	PPSE18,
	PPSE19,
	PPSE20,
	PPSE21,
	PPSE22,
	PPSE23,
	PPSE24,
	PPSE25,
	PPSE26,
	PPSE27,
	PPSE28,
	PPSE29,
	PPSE30,
	PPSE31
};

enum hercules_periph_memory {
	PCS0 = 0,
	PCS1,
	PCS2,
	PCS3,
	PCS4,
	PCS5,
	PCS6,
	PCS7,
	PCS8,
	PCS9,
	PCS10,
	PCS11,
	PCS12,
	PCS13,
	PCS14,
	PCS15,
	PCS16,
	PCS17,
	PCS18,
	PCS19,
	PCS20,
	PCS21,
	PCS22,
	PCS23,
	PCS24,
	PCS25,
	PCS26,
	PCS27,
	PCS28,
	PCS29,
	PCS30,
	PCS31,
	PCS32,
	PCS33,
	PCS34,
	PCS35,
	PCS36,
	PCS37,
	PCS38,
	PCS39,
	PCS40,
	PCS41,
	PCS42,
	PCS43,
	PCS44,
	PCS45,
	PCS46,
	PCS47,
	PCS48,
	PCS49,
	PCS50,
	PCS51,
	PCS52,
	PCS53,
	PCS54,
	PCS55,
	PCS56,
	PCS57,
	PCS58,
	PCS59,
	PCS60,
	PCS61,
	PCS62,
	PCS63
};

enum hercules_priv_periph_memory {
	PPCS0 = 0,
	PPCS1,
	PPCS2,
	PPCS3,
	PPCS4,
	PPCS5,
	PPCS6,
	PPCS7,
	PPCS8,
	PPCS9,
	PPCS10,
	PPCS11,
	PPCS12,
	PPCS13,
	PPCS14,
	PPCS15
};

enum hercules_master_ids {
	CPU0 = 0,
	CPU1 = 1, /* Lockstep CPU */
	DMA = 2,
	HTU1 = 3,
	HTU2 = 4,
	FTU = 5,
	DMM = 7,
	DAP = 9,
	EMAC = 10,
};

#endif /* INCLUDE_ZEPHYR_DT_BINDINGS_CLOCK_TI_HERCULES_CLOCK_H_ */
