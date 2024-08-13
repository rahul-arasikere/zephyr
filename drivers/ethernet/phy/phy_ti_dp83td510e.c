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
	const struct device *mdio;
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
	mdio_bus_enable(cfg->mdio);
	if (reg_addr < 0x20) {
		/* Direct access to the register */
		ret = mdio_read(cfg->mdio, cfg->addr, reg_addr, reg_value);
	} else if (reg_addr >= 0x1000 && reg_addr <= 0x18F8) {
		/* Access through MDIO_MMD_PMAPMD */
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, );
	} else if (reg_addr >= 0x3000 && reg_addr <= 0x38E7) {
		/* Access through MDIO_MMD_PCS*/
	} else if (reg_addr >= 0x200 && reg_addr <= 0x20F) {
		/* Access through MDIO_MMD_AN */
	} else if ((reg_addr >= 0x20 && reg_addr <= 0x130) || (reg_addr >= 0x300 && 0xE01)) {
		/* Access through MDIO_MMD_VENDOR_SPECIFIC2 */
	} else {
		ret = -EINVAL;
	}
	mdio_bus_disable(cfg->mdio);
	return ret;
}

static int phy_dp83td510e_reg_write(const struct device *dev, uint16_t reg_addr, uint16_t reg_value)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	mdio_bus_enable(cfg->mdio);
	mdio_bus_disable(cfg->mdio);
	return 0;
}

static int phy_dp83td510e_cfg_link(const struct device *dev, enum phy_link_speed adv_speeds)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	return 0;
}

static int phy_dp83td510e_get_link(const struct device *dev, struct phy_link_state *state)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	const struct ti_dp83td510e_config *data = dev->data;
	return 0;
}

static int phy_dp83td510e_link_cb_set(const struct device *dev, phy_callback_t cb, void *user_data)
{
	struct phy_link_state state;
	struct ti_dp83td510e_config *data = dev->data;
	data->cb = cb;
	data->cb_data = user_data;

	/**
	 * Immediately invoke the callback to notify the caller of the
	 * current link status.
	 */

	phy_dp83td510e_get_link(dev, &state);
	data->cb(data->dev, &state, data->cb_data);
	return 0;
}

static int phy_ti_dp83td510e_reset(const struct device *dev)
{
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios)
	const struct ti_dp83td510e_config *config = dev->config;
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */
	struct ti_dp83td510e_data *data = dev->data;
	int ret;

	ret = k_sem_take(&data->sem, K_FOREVER);
	if (ret) {
		LOG_ERR("Failed to take sem");
		return ret;
	}
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios)
	if (!config->reset_gpio.port) {
		goto skip_reset_gpio;
	}

	/* Start reset */
	ret = gpio_pin_set_dt(&config->reset_gpio, 0);
	if (ret) {
		goto done;
	}

	/* Wait for phy configuration*/
	k_busy_wait(500 * USEC_PER_MSEC);

	/* Reset over */
	ret = gpio_pin_set_dt(&config->reset_gpio, 1);
	goto done;
skip_reset_gpio:
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */
	ret = phy_dp83td510e_reg_write(dev, MII_BMCR, MII_BMCR_RESET);
	if (ret) {
		goto done;
	} /* Wait for phy configuration*/
	k_busy_wait(500 * USEC_PER_MSEC);
done:
	k_sem_give(&data->sem);
	return ret;
}

static void phy_ti_dp83td510e_mon_work_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct ti_dp83td510e_data *data =
		CONTAINER_OF(dwork, struct ti_dp83td510e_data, phy_monitor_work);
	const struct device *dev = data->dev;
	struct phy_link_state state = {};
	int ret;

	ret = phy_dp83td510e_get_link(dev, &state);

	if (ret == 0 && memcmp(&state, &data->state, sizeof(struct phy_link_state)) != 0) {
		k_sem_take(&data->sem, K_FOREVER);
		memcpy(&data->state, &state, sizeof(struct phy_link_state));
		k_sem_give(&data->sem);
		if (data->cb) {
			data->cb(dev, &data->state, data->cb_data);
		}
	}

	k_work_reschedule(&data->phy_monitor_work, K_MSEC(CONFIG_PHY_MONITOR_PERIOD));
}

static int phy_ti_dp83td510e_init(const struct device *dev)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	struct ti_dp83td510e_config *data = dev->data;
	int ret;
	k_sem_init(&data->sem, 1, 1);
	data->dev = dev;
	data->cb = NULL;
	/**
	 * If this is a *fixed* link then we don't need to communicate
	 * with a PHY. We set the link parameters as configured
	 * and set link state to up.
	 */
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios)
	if (cfg->reset_gpio.port) {
		ret = gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_ACTIVE);
		if (ret) {
			return ret;
		}
	}
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios)
	if (cfg->reset_gpio.port) {
		ret = gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_ACTIVE);
		if (ret) {
			return ret;
		}
	}
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */

	if (cfg->fixed) {
		/* Only one speed */
		data->state.speed = LINK_HALF_10BASE_T;
		data->state.is_up = true;
	} else {
		data->state.is_up = false;
		/* Reset the PHY */
		if (!cfg->no_reset) {
			ret = phy_ti_dp83td510e_reset();
			if (ret) {
				LOG_DBG("Failed to reset the PHY");
				return ret;
			}
		}

		ret = read_phy_id();

		ret = do_auto_negotiation();

		if (ret) {
			LOG_ERR("Failed to auto negotiate PHY (%d) ID: %d", cfg->addr, ret);
			return ret;
		}

		k_work_init_delayable(&data->phy_monitor_work, phy_ti_dp83td510e_mon_work_handler);
	}
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
	static struct ti_dp83td510e_config ti_dp83td510e_config_##n = {                            \
		.addr = DT_INST_REG_ADDR(n),                                                       \
		.no_reset = DT_INST_PROP(n, no_reset),                                             \
		.fixed = IS_FIXED_LINK(n),                                                         \
		.fixed_speed = DT_INST_ENUM_IDX_OR(n, fixed_link, 0),                              \
		.mdio = UTIL_AND(UTIL_NOT(IS_FIXED_LINK(n)), DEVICE_DT_GET(DT_INST_BUS(n)))};      \
	static struct ti_dp83td510e_data ti_dp83td510e_data_##n = {};                              \
	DEVICE_DT_INST_DEFINE(n, &phy_ti_dp83td510e_init, NULL, &ti_dp83td510e_data_##n,           \
			      &ti_dp83td510e_config_##n, POST_KERNEL, CONFIG_PHY_INIT_PRIORITY,    \
			      &phy_ti_dp83td510e_api);

DT_INST_FOREACH_STATUS_OKAY(TI_DP83TD510E_INIT);
