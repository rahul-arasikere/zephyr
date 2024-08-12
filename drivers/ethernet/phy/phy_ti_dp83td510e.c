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

static inline int reg_read(const struct device *dev, uint16_t reg_addr, uint16_t *reg_value)
{
	return 0;
}

static inline int reg_write(const struct device *dev, uint16_t reg_addr, uint16_t reg_value)
{
	return 0;
}

static int get_id(const struct device *dev, uint32_t *phy_id)
{
	uint16_t value;

	if (reg_read(dev, MII_PHYID1R, &value) < 0) {
		return -EIO;
	}

	*phy_id = value << 16;

	if (reg_read(dev, MII_PHYID2R, &value) < 0) {
		return -EIO;
	}

	*phy_id |= value;

	return 0;
}

static int update_link_state(const struct device *dev)
{
	return 0;
}
