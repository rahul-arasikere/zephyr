/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_dp83td510e

#include <zephyr/kernel.h>
#include <zephyr/net/phy.h>
#include <zephyr/net/mii.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/mdio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(phy_ti_dp83td510e, CONFIG_PHY_LOG_LEVEL);

struct ti_dp83td510e_config {
	uint8_t addr;
	bool no_reset;
	bool fixed;
	int fixed_speed;
	const struct device *mdio_bus;
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios)
	const struct gpio_dt_spec reset_gpio;
#endif
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(pwdn_gpios)
	const struct gpio_dt_spec pwdn_gpio;
#endif
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(int_gpios)
	const struct gpio_dt_spec interrupt_gpio;
#endif
};

struct ti_dp83td510e_data {
	const struct device *dev;
	struct phy_link_state state;
	phy_callback_t cb;
	void *cb_data;
	struct k_mutex mutex;
	struct k_work_delayable phy_monitor_work;
};

static int phy_dp83td510e_set_mmd(const struct device *dev, uint8_t mmd, uint8_t mode,
				  uint16_t reg_addr)
{
	return 0;
}

static int phy_dp83td510e_reg_read(const struct device *dev, uint16_t reg_addr, uint16_t *reg_value)
{
	int ret = 0;
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	mdio_bus_enable(cfg->mdio_bus);
	if (reg_addr < 0x20) {
		/* Direct access to the register */
		ret = mdio_read(cfg->mdio_bus, cfg->addr, reg_addr, reg_value);
	} else if (reg_addr >= 0x1000 && reg_addr <= 0x18F8) {
		/* Access through MDIO_MMD_PMAPMD */
		ret = mdio_write(cfg->mdio_bus, cfg->addr, MII_MMD_ACR, );
	} else if (reg_addr >= 0x3000 && reg_addr <= 0x38E7) {
		/* Access through MDIO_MMD_PCS*/
	} else if (reg_addr >= 0x200 && reg_addr <= 0x20F) {
		/* Access through MDIO_MMD_AN */
	} else if ((reg_addr >= 0x20 && reg_addr <= 0x130) || (reg_addr >= 0x300 && 0xE01)) {
		/* Access through MDIO_MMD_VENDOR_SPECIFIC2 */
	} else {
		ret = -EINVAL;
	}
	mdio_bus_disable(cfg->mdio_bus);
	return ret;
}

static int phy_dp83td510e_reg_write(const struct device *dev, uint16_t reg_addr, uint16_t reg_value)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	mdio_bus_enable(cfg->mdio_bus);
	mdio_bus_disable(cfg->mdio_bus);
	return 0;
}

static int phy_dp83td510e_cfg_link(const struct device *dev, enum phy_link_speed adv_speeds)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	return 0;
}

static int phy_dp83td510e_get_link(const struct device *dev)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	return 0;
}

static int phy_dp83td510e_link_cb_set(const struct device *dev, phy_callback_t cb, void *user_data)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	return 0;
}

static int phy_ti_dp83td510e_init(const struct device *dev)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	return 0;
}

static const struct ethphy_driver_api phy_ti_dp83td510e_api = {
	.get_link = phy_dp83td510e_get_link,
	.cfg_link = phy_dp83td510e_cfg_link,
	.link_cb_set = phy_dp83td510e_link_cb_set,
	.read = phy_dp83td510e_reg_read,
	.write = phy_dp83td510e_reg_write,
};

#define TI_DP83TD510E_INIT(n)                                                                      \
	static struct ti_dp83td510e_config ti_dp83td510e_config_##n = {};                          \
	static struct ti_dp83td510e_data ti_dp83td510e_data_##n = {};                              \
	DEVICE_DT_INST_DEFINE(n, &phy_ti_dp83td510e_init, NULL, &ti_dp83td510e_data_##n,           \
			      &ti_dp83td510e_config_##n, POST_KERNEL, CONFIG_PHY_INIT_PRIORITY,    \
			      &phy_ti_dp83td510e_api);

DT_INST_FOREACH_STATUS_OKAY(TI_DP83TD510E_INIT);
