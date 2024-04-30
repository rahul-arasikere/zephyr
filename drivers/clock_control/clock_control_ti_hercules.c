/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_hercules_gcm

#include <soc.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/dt-bindings/clock/ti_hercules_clock.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>

#define CLOCKS_NODE       DT_NODELABEL(clocks)
#define OSCIN_CLOCK_NODE  DT_CHILD(CLOCKS_NODE, oscin)
#define EXT_CLKIN1_NODE   DT_CHILD(CLOCKS_NODE, ext_clkin1)
#define EXT_CLKIN2_NODE   DT_CHILD(CLOCKS_NODE, ext_clkin2)
#define PLL1_NODE         DT_CHILD(CLOCKS_NODE, pll1)
#define PLL2_NODE         DT_CHILD(CLOCKS_NODE, pll2)
#define LF_LPO_CLOCK_NODE DT_CHILD(CLOCKS_NODE, lf_lpo)
#define HF_LPO_CLOCK_NODE DT_CHILD(CLOCKS_NODE, hf_lpo)

/* Helper Macro Functions */
#define z_plldiv(x)    DT_PROP_BY_IDX(x, r)
#define z_refclkdiv(x) DT_PROP_BY_IDX(x, nr)
#define z_pllmul(x)    DT_PROP_BY_IDX(x, nf)
#define z_odpll(x)     DT_PROP_BY_IDX(x, od)

/* Check whether frequency modulation is enabled. */
#define FMENA DT_PROP_BY_IDX(CLOCKS_NODE, enable_freq_modulation)

#define z_pll1_mulmod()          DT_PROP_BY_IDX(PLL1_NODE, mulmod)
#define z_pll1_spreadingrate()   DT_PROP_BY_IDX(PLL1_NODE, ns)
#define z_pll1_spreadingamount() DT_PROP_BY_IDX(PLL1_NODE, nv)

#define FBSLIP  BIT(9)
#define RFSLIP  BIT(8)
#define OSCFAIL BIT(0)

struct ti_hercules_gcm_clock_config {
	enum hercules_clk_srcs source;
	enum hercules_clk_domains domain;
};

struct ti_hercules_gcm_clock_data {
};

static int ti_hercules_gcm_clock_on(const struct device *dev, clock_control_subsys_t sys)
{
	return 0;
}

static int ti_hercules_gcm_clock_off(const struct device *dev, clock_control_subsys_t sys)
{
	return 0;
}

static enum clock_control_status ti_hercules_gcm_clock_get_status(const struct device *dev,
								  clock_control_subsys_t sys)
{
	return CLOCK_CONTROL_STATUS_ON;
}

static int ti_hercules_gcm_clock_get_rate(const struct device *dev, clock_control_subsys_t sys,
					  uint32_t *rate)
{
	*rate = 0;
	return 0;
}

static int ti_hercules_gcm_clock_configure(const struct device *dev, clock_control_subsys_t sys,
					   void *data)
{
	return 0;
}

#define SYS1_NODE DT_NODELABEL(sys1)
#define SYS2_NODE DT_NODELABEL(sys2)
#define PCR1_NODE DT_NODELABEL(pcr1)
#define PCR2_NODE DT_NODELABEL(pcr2)
#define PCR3_NODE DT_NODELABEL(pcr3)

static int ti_hercules_gcm_clock_init(const struct device *dev)
{
	volatile struct hercules_syscon_1_regs *sys_regs_1 = DT_REG_ADDR(SYS1_NODE);
	volatile struct hercules_syscon_2_regs *sys_regs_2 = DT_REG_ADDR(SYS2_NODE);

	/* Configure PLL control registers and enable PLLs.
	 * The PLL takes (127 + 1024 * NR) oscillator cycles to acquire lock.
	 * This initialization sequence performs all the tasks that are not
	 * required to be done at full application speed while the PLL locks.
	 */
	sys_regs_1->CSDISSET = BIT(PLL1) | BIT(PLL2);
	while (sys_regs_1->CSDIS & (BIT(PLL1) | BIT(PLL2)) != (BIT(PLL1) | BIT(PLL2))) {
		/*nop*/;
	}

	/* Clear Global Status Flags */
	sys_regs_1->GBLSTAT = FBSLIP | RFSLIP | OSCFAIL;

#if IS_ENABLED(PLL1_NODE)
	uint32_t pll1_conf = 0;
	pll1_conf |= DT_PROP_BY_IDX(PLL1_NODE, reset_on_pll_slip) << 31;
#if DT_PROP_BY_IDX(PLL1_NODE, bypass_on_pll_slip)
	/* Enable Bypass on Slip*/
	pll1_conf |= 0b11 << 29;
#else
	/* Disable Bypass on Slip */
	pll1_conf |= 0x2 << 29;
#endif /* Bypass-On-PLL-Slip */
	BUILD_ASSERT(IN_RANGE(z_plldiv(PLL1_NODE), 1, 32), "R out of range! (1 -32)");
	pll1_conf |= (z_plldiv(PLL1_NODE) - 1) << 24;
	pll1_conf |= DT_PROP_BY_IDX(PLL1_NODE, reset_on_oscillator_fail) << 23;
	BUILD_ASSERT(IN_RANGE(z_refclkdiv(PLL1_NODE), 1, 64), "NR out of range! (1 - 64)");
	pll1_conf |= (z_refclkdiv(PLL1_NODE) - 1) << 16;
	BUILD_ASSERT(IN_RANGE(z_pllmul(PLL1_NODE), 1, 256), "NF out of range! (1 - 256)");
	pll1_conf |= (z_pllmul(PLL1_NODE) - 1) << 8;

	sys_regs_1->PLLCTL1 = pll1_conf;

	uint32_t pll2_conf = 0;
#if FMENA
	pll2_conf |= BIT(31);
	BUILD_ASSERT(IN_RANGE(z_pll1_spreadingrate(), 1, 512), "NS out of range! (1-512)");
	pll2_conf |= (z_pll1_spreadingrate() - 1) << 22;
	BUILD_ASSERT(IN_RANGE(z_pll1_mulmod(), 8, 511), "MULMOD out of range! (8 - 511)");
	pll2_conf |= z_pll1_mulmod() << 12;
	BUILD_ASSERT(IN_RANGE(z_pll1_spreadingamount(), 1, 512), "NV out of range! (1 - 512)");
	pll2_conf |= (z_pll1_spreadingamount() - 1);
#endif /* FMENA */
	BUILD_ASSERT(IN_RANGE(z_odpll(PLL1_NODE), 1, 8), "OD out of range! (1 - 8)");
	pll2_conf |= (z_odpll(PLL1_NODE) - 1) << 9;
	sys_regs_1->PLLCTL2 = pll2_conf;
#endif /* PLL1_NODE */

#if IS_ENABLED(PLL2_NODE)
	uint32_t pll3_conf = 0;
	BUILD_ASSERT(IN_RANGE(z_odpll(PLL2_NODE), 1, 8), "OD out of range! (1 - 8)");
	pll3_conf |= (z_odpll(PLL2_NODE) - 1) << 29;
	BUILD_ASSERT(IN_RANGE(z_plldiv(PLL2_NODE), 1, 32), "R out of range! (1 -32)");
	pll3_conf |= (z_plldiv(PLL2_NODE) - 1) << 24;
	BUILD_ASSERT(IN_RANGE(z_refclkdiv(PLL2_NODE), 1, 64), "NR out of range! (1 - 64)");
	pll3_conf |= (z_refclkdiv(PLL2_NODE) - 1) << 16;
	BUILD_ASSERT(IN_RANGE(z_pllmul(PLL2_NODE), 1, 256), "NF out of range! (1 - 256)");
	pll3_conf |= (z_pllmul(PLL2_NODE) - 1) << 8;
	sys_regs_2->PLLCTL3 = pll3_conf;
#endif /* PLL2_NODE */

#if IS_ENABLED(OSCIN_CLOCK_NODE)
	sys_regs_1->CSDIS |= BIT(OSCILLATOR);
#endif
#if IS_ENABLED(PLL1_NODE)
	sys_regs_1->CSDIS |= BIT(PLL1);
#endif
#if IS_ENABLED(EXT_CLKIN1_NODE)
	sys_regs_1->CSDIS |= BIT(EXTCLKIN);
#endif
#if IS_ENABLED(LF_LPO_CLOCK_NODE)
	sys_regs_1->CSDIS |= BIT(LF_LPO);
#endif
#if IS_ENABLED(HF_LPO_CLOCK_NODE)
	sys_regs_1->CSDIS |= BIT(HF_LPO);
#endif
#if IS_ENABLED(PLL2_NODE)
	sys_regs_1->CSDIS |= BIT(PLL2);
#endif
#if IS_ENABLED(EXT_CLKIN2_NODE)
	sys_regs_1->CSDIS |= BIT(EXTCLKIN2);
#endif
	return 0;
}

static const struct clock_control_driver_api ti_hercules_gcm_clock_api = {
	.on = ti_hercules_gcm_clock_on,
	.off = ti_hercules_gcm_clock_off,
	.get_status = ti_hercules_gcm_clock_get_status,
	.get_rate = ti_hercules_gcm_clock_get_rate,
	.configure = ti_hercules_gcm_clock_configure};
