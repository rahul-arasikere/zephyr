/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_dp83td510e

#include <zephyr/kernel.h>
#include <zephyr/net/phy.h>
#include <zephyr/net/mii.h>
#include <zephyr/net/mdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/mdio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(dp83td510e, CONFIG_PHY_LOG_LEVEL);

#define IS_FIXED_LINK(n)    DT_INST_NODE_HAS_PROP(n, fixed_link)
#define USE_INTERRUPT(n)    DT_INST_NODE_HAS_PROP(n, use_pwdn_as_interrupt)
#define USE_RMII_REV_1_2(n) DT_INST_NODE_HAS_PROP(n, use_rmii_rev_1_2)
#define USE_SLOW_MODE(n)    DT_INST_NODE_HAS_PROP(n, use_slow_mode)
#define MII_INVALID_PHY_ID  UINT32_MAX

#define PHY_STS               0x0010U
#define PHY_STS_LINK_UP       BIT(0)
#define PHY_STS_MII_INTERRUPT BIT(7)
#define GEN_CFG               0x0011U
#define INT_REG1              0x0012U
#define INT_REG2              0x0013U
#define BISCR_REG             0x0016U
#define MAC_CFG_1             0x0017U
#define MAC_CFG_1_SLOW_MODE   BIT(6)
#define MAC_CFG_1_RMII_REV    BIT(4)
#define MAC_CFG_2             0x0018U
#define CTRL_REG              0x001FU

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(mac_connection_type)

enum phy_mii_mode {
	MII_MODE_MII,
	MII_MODE_RMII_MASTER,
	MII_MODE_RMII_SLAVE,
	MII_MODE_RGMII,
	MII_MODE_RMII_MASTER_LOW_POWER,
	MII_MODE_RMII_EXTENDER,
};

#endif

struct ti_dp83td510e_config {
	uint8_t addr;
	bool no_reset;
	bool fixed;
	bool use_interrupt;
	bool use_rmii_rev_1_2;
	int fixed_speed;
	const struct device *mdio;
#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios)
	const struct gpio_dt_spec reset_gpio;
#endif

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(int_pwdn_gpios)
	const struct gpio_dt_spec int_pwdn_gpio;
#endif

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(mac_connection_type)
	enum phy_mii_mode mac_mode;
#endif
};

struct ti_dp83td510e_data {
	const struct device *dev;
	struct phy_link_state state;
	phy_callback_t cb;
	void *cb_data;
	struct k_sem sem;
	struct k_work_delayable phy_monitor_work;
	struct gpio_callback interrupt_gpio_cb;
};

static int reg_read(const struct device *dev, uint16_t reg_addr, uint16_t *reg_value)
{
	int ret = 0;
	const struct ti_dp83td510e_config *cfg = dev->config;
	mdio_bus_enable(cfg->mdio);
	if (reg_addr < 0x20) {
		/* Direct access to the register */
		ret = mdio_read(cfg->mdio, cfg->addr, reg_addr, reg_value);
	} else if (reg_addr >= 0x1000 && reg_addr <= 0x18F8) {
		/* Access through MDIO_MMD_PMAPMD */
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_PMAPMD);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_PMAPMD);
		ret |= mdio_read(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);

	} else if (reg_addr >= 0x3000 && reg_addr <= 0x38E7) {
		/* Access through MDIO_MMD_PCS*/
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_PCS);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_PCS);
		ret = mdio_read(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);
	} else if (reg_addr >= 0x200 && reg_addr <= 0x20F) {
		/* Access through MDIO_MMD_AN */
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_AN);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_AN);
		ret |= mdio_read(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);
	} else if ((reg_addr >= 0x20 && reg_addr <= 0x130) || (reg_addr >= 0x300 && 0xE01)) {
		/* Access through MDIO_MMD_VENDOR_SPECIFIC2 */
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_VENDOR_SPECIFIC2);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_VENDOR_SPECIFIC2);
		ret |= mdio_read(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);
	} else {
		ret = -EINVAL;
	}
	mdio_bus_disable(cfg->mdio);
	return ret;
}

static int dp83td510e_read(const struct device *dev, uint16_t reg_addr, uint32_t *data)
{
	return reg_read(dev, reg_addr, (uint16_t *)data);
}

static int reg_write(const struct device *dev, uint16_t reg_addr, uint16_t reg_value)
{
	int ret = 0;
	const struct ti_dp83td510e_config *cfg = dev->config;
	mdio_bus_enable(cfg->mdio);
	if (reg_addr < 0x20) {
		/* Direct access to the register */
		ret = mdio_write(cfg->mdio, cfg->addr, reg_addr, reg_value);
	} else if (reg_addr >= 0x1000 && reg_addr <= 0x18F8) {
		/* Access through MDIO_MMD_PMAPMD */
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_PMAPMD);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_PMAPMD);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);

	} else if (reg_addr >= 0x3000 && reg_addr <= 0x38E7) {
		/* Access through MDIO_MMD_PCS*/
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_PCS);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_PCS);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);
	} else if (reg_addr >= 0x200 && reg_addr <= 0x20F) {
		/* Access through MDIO_MMD_AN */
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_AN);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_AN);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);
	} else if ((reg_addr >= 0x20 && reg_addr <= 0x130) || (reg_addr >= 0x300 && 0xE01)) {
		/* Access through MDIO_MMD_VENDOR_SPECIFIC2 */
		ret = mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR, MDIO_MMD_VENDOR_SPECIFIC2);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_addr);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_ACR,
				  MII_MMD_ACR_DATA_NO_POS_INC | MDIO_MMD_VENDOR_SPECIFIC2);
		ret |= mdio_write(cfg->mdio, cfg->addr, MII_MMD_AADR, reg_value);
	} else {
		ret = -EINVAL;
	}
	mdio_bus_disable(cfg->mdio);
	return ret;
}

static int dp83td510e_write(const struct device *dev, uint16_t reg_addr, uint32_t data)
{
	return reg_write(dev, reg_addr, (uint16_t)data);
}

static int cfg_link(const struct device *dev, enum phy_link_speed adv_speeds)
{
	int ret = 0;
	const struct ti_dp83td510e_config *cfg = dev->config;
	uint16_t mac_cfg_1_reg = 0;
	reg_read(dev, MAC_CFG_1, &mac_cfg_1_reg);

	/**
	 * Not sure which MAC peripherals support RMII rev 1.2,
	 * which is enabled by default.
	 * For example the STM32 chip expects the behavior or RMII rev 1.
	 */
	if (!cfg->use_rmii_rev_1_2) {
		mac_cfg_1_reg |= MAC_CFG_1_RMII_REV;
	} else {
		mac_cfg_1_reg ^= MAC_CFG_1_RMII_REV;
	}

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(mac_connection_type)
	switch (cfg->mac_mode) {
	}
	/**
	 * The behaviour of the slow mode bit differs from the datasheet.
	 * This bit needs to be set to operate in 50 Mhz mode.
	 */
	if (!cfg->use_slow_mode) {
		mac_cfg_1_reg |= MAC_CFG_1_SLOW_MODE;
	} else {
		mac_cfg_1_reg ^= MAC_CFG_1_SLOW_MODE;
	}
	reg_write(dev, MAC_CFG_1, mac_cfg_1_reg);
	return ret;

#endif
}

static int get_link_state(const struct device *dev, struct phy_link_state *state)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	struct ti_dp83td510e_data *data = dev->data;
	bool link_up;
	uint16_t phy_sts = 0;
	uint16_t an_ctrl = 0;
	uint16_t an_stat = 0;
	uint32_t timeout = CONFIG_PHY_AUTONEG_TIMEOUT_MS / 100;

	if (reg_read(dev, PHY_STS, &phy_sts) < 0) {
		return -EIO;
	}
	link_up = phy_sts & PHY_STS_LINK_UP;

	if (link_up == data->state.is_up) {
		return -EAGAIN;
	}
	state->is_up = link_up;

	/* If link is down, there is nothing more to be done */
	if (state->is_up == false) {
		LOG_INF("PHY (%d) Link state transition to down", cfg->addr);
		return 0;
	}

	/* Restart the Auto-Negotiation Process. */
	if (reg_read(dev, MDIO_AN_T1_CTRL, &an_ctrl) < 0) {
		return -EIO;
	}

	an_ctrl |= MDIO_AN_T1_CTRL_RESTART;

	if (reg_write(dev, MDIO_AN_T1_CTRL, an_ctrl) < 0) {
		return -EIO;
	}
	LOG_DBG("PHY (%d) Starting APL PHY auto-negotiate sequence", cfg->addr);

	do {
		if (timeout-- == 0U) {
			LOG_ERR("PHY (%d) auto-negotiate timed out", cfg->addr);
			return -ETIMEDOUT;
		}

		k_sleep(K_MSEC(100));

		if (reg_read(dev, MDIO_AN_T1_STAT, &an_stat) < 0) {
			return -EIO;
		}
	} while (!(an_stat & MDIO_AN_T1_STAT_COMPLETE));
	LOG_DBG("PHY (%d) Auto-negotiation complete", cfg->addr);

	if (an_stat & MDIO_AN_T1_STAT_LINK_STATUS) {
		state->speed = LINK_FULL_10BASE_T;
		LOG_INF("PHY (%d) link transition to up state", cfg->addr);
		return 0;
	}

	LOG_ERR("PHY (%d) failed to establish link", cfg->addr);
	if (an_stat & MDIO_AN_T1_STAT_REMOTE_FAULT) {
		LOG_ERR("PHY (%d) Remote fault occured during auto-negotiation", cfg->addr);
	}
	if (!(an_stat & MDIO_AN_T1_STAT_PAGE_RX)) {
		LOG_ERR("PHY (%d) Did not recieve remote page during auto-negotiation", cfg->addr);
	}
	if (!(an_stat & MDIO_AN_T1_STAT_ABLE)) {
		LOG_ERR("PHY (%d) is not capable of auto-negotiation", cfg->addr);
	}
	return -EIO;
}

static int link_cb_set(const struct device *dev, phy_callback_t cb, void *user_data)
{
	struct phy_link_state state;
	struct ti_dp83td510e_data *data = dev->data;
	data->cb = cb;
	data->cb_data = user_data;

	/**
	 * Immediately invoke the callback to notify the caller of the
	 * current link status.
	 */

	get_link_state(dev, &state);
	data->cb(data->dev, &state, data->cb_data);
	return 0;
}

static int reset_phy(const struct device *dev)
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
	ret = gpio_pin_set_dt(&config->reset_gpio, 1);
	if (ret) {
		goto done;
	}

	/* Wait for phy configuration*/
	k_busy_wait(500 * USEC_PER_MSEC);

	/* Reset over */
	ret = gpio_pin_set_dt(&config->reset_gpio, 0);
	goto done;
skip_reset_gpio:
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */
	ret = reg_write(dev, MII_BMCR, MII_BMCR_RESET);
	if (ret) {
		goto done;
	} /* Wait for phy configuration*/
	k_busy_wait(500 * USEC_PER_MSEC);
done:
	/* 1050 ns from reset release to strap latching. */
	k_busy_wait(5 * USEC_PER_MSEC);
	k_sem_give(&data->sem);
	return ret;
}

static void monitor_work_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct ti_dp83td510e_data *data =
		CONTAINER_OF(dwork, struct ti_dp83td510e_data, phy_monitor_work);
	const struct device *dev = data->dev;
	struct phy_link_state state = {};
	int ret;

	ret = get_link_state(dev, &state);

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

static void phy_ti_dp83td510e_interrupt(const struct device *port, struct gpio_callback *cb,
					gpio_port_pins_t pins)
{
	ARG_UNUSED(pins);
	struct ti_dp83td510e_data *data = CONTAINER_OF(cb, struct ti_dp83td510e_data, interrupt_gpio_cb);
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

static int phy_ti_dp83td510e_init(const struct device *dev)
{
	const struct ti_dp83td510e_config *cfg = dev->config;
	struct ti_dp83td510e_data *data = dev->data;
	int ret;
	uint32_t phy_id;
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
		ret = gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_INACTIVE);
		if (ret) {
			return ret;
		}
	}
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(int_pwdn_gpios)
	if (cfg->int_pwdn_gpio.port) {
		ret = gpio_pin_configure_dt(&cfg->int_pwdn_gpio, GPIO_OUTPUT_INACTIVE);
		if (ret) {
			return ret;
		}
	}
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(int_pwdn_gpios) */
	if (cfg->fixed) {
		const static int speed_to_phy_link_speed[] = {
			LINK_HALF_10BASE_T,  LINK_FULL_10BASE_T,   LINK_HALF_100BASE_T,
			LINK_FULL_100BASE_T, LINK_HALF_1000BASE_T, LINK_FULL_1000BASE_T,
		};
		/* Only one speed */
		data->state.speed = speed_to_phy_link_speed[cfg->fixed_speed];
		data->state.is_up = true;
	} else {
		data->state.is_up = false;
		/* Reset the PHY */
		if (!cfg->no_reset) {
			ret = reset_phy(dev);
			if (ret) {
				LOG_DBG("Failed to reset the PHY");
				return ret;
			}
		}

		if (get_id(dev, &phy_id) == 0) {
			if (phy_id == MII_INVALID_PHY_ID) {
				LOG_ERR("No PHY found at address %d", cfg->addr);

				return -EINVAL;
			}

			LOG_INF("PHY (%d) ID %X", cfg->addr, phy_id);
		}

		cfg_link(dev, LINK_HALF_10BASE_T | LINK_FULL_10BASE_T);

		if (ret) {
			LOG_ERR("Failed to auto negotiate PHY (%d) ID: %d", cfg->addr, ret);
			return ret;
		}

		k_work_init_delayable(&data->phy_monitor_work, monitor_work_handler);

		monitor_work_handler(&data->phy_monitor_work.work);
	}
	return 0;
}

static const struct ethphy_driver_api phy_ti_dp83td510e_api = {
	.get_link = get_link_state,
	.cfg_link = cfg_link,
	.link_cb_set = link_cb_set,
	.read = dp83td510e_read,
	.write = dp83td510e_write,
};

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(mac_connection_type)
#define MAC_MODE(n)                                                                                \
	.mac_mode = _CONCAT(MII_MODE_, DT_INST_STRING_UPPER_TOKEN(n, mac_connection_type))
#else
#define MAC_MODE(n)
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios)
#define RESET_GPIO(n) .reset_gpio = GPIO_DT_SPEC_INST_GET_OR(n, reset_gpios, {0})
#else
#define RESET_GPIO(n)
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(reset_gpios) */

#if DT_ANY_INST_HAS_PROP_STATUS_OKAY(int_pwdn_gpios)
#define PWDN_INT_GPIO(n) .int_pwdn_gpio = GPIO_DT_SPEC_INST_GET_OR(n, int_pwdn_gpios, {0})
#else
#define PWDN_INT_GPIO(n)
#endif /* DT_ANY_INST_HAS_PROP_STATUS_OKAY(int_pwdn_gpios) */

#define TI_DP83TD510E_INIT(n)                                                                      \
	static struct ti_dp83td510e_config ti_dp83td510e_config_##n = {                            \
		.addr = DT_INST_REG_ADDR(n),                                                       \
		.no_reset = DT_INST_PROP(n, no_reset),                                             \
		.fixed = IS_FIXED_LINK(n),                                                         \
		.fixed_speed = DT_INST_ENUM_IDX_OR(n, fixed_link, 0),                              \
		.use_interrupt = USE_INTERRUPT(n),                                                 \
		.use_rmii_rev_1_2 = USE_RMII_REV_1_2(n),                                           \
		.mdio = UTIL_AND(UTIL_NOT(IS_FIXED_LINK(n)), DEVICE_DT_GET(DT_INST_BUS(n))),       \
		UTIL_AND(UTIL_NOT(IS_FIXED_LINK(n)), MAC_MODE(n)),                                 \
		RESET_GPIO(n),                                                                     \
		PWDN_INT_GPIO(n)};                                                                 \
	static struct ti_dp83td510e_data ti_dp83td510e_data_##n = {};                              \
	DEVICE_DT_INST_DEFINE(n, &phy_ti_dp83td510e_init, NULL, &ti_dp83td510e_data_##n,           \
			      &ti_dp83td510e_config_##n, POST_KERNEL, CONFIG_PHY_INIT_PRIORITY,    \
			      &phy_ti_dp83td510e_api);

DT_INST_FOREACH_STATUS_OKAY(TI_DP83TD510E_INIT);
