/*
 * Copyright (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT nxp_pf15xx

#include <errno.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nxp_pf15xx, CONFIG_MFD_LOG_LEVEL);

#define NXP_PF15XX_DEV_ID_ADDR 0x0
#define NXP_PF1500_DEV_ID_VAL  0
#define NXP_PF1550_DEV_ID_VAL  BIT(2)
#define NXP_PF15XX_FAMILY_VAL  GENMASK(7, 3)

struct mfd_pf15xx_config {
	struct i2c_dt_spec bus;
	struct gpio_dt_spec standby_pin;
};

static int mfd_pf15xx_init(const struct device *dev)
{
	const struct mfd_pf15xx_config *config = dev->config;
	uint8_t val;
	int ret;

	if (!i2c_is_ready_dt(&config->bus)) {
		return -ENODEV;
	}

	ret = i2c_reg_read_byte_dt(&config->bus, NXP_PF15XX_DEV_ID_ADDR, &val);
	if (ret < 0) {
		return ret;
	}

	if ((val & NXP_PF15XX_FAMILY_VAL) != 0) {
		if ((val & NXP_PF1500_DEV_ID_VAL) != 0) {
			LOG_INF("pf1500 chip found");
		} else if ((val & NXP_PF1550_DEV_ID_VAL) != 0) {
			LOG_INF("pf1550 chip found");
		}
	} else {
		LOG_ERR("no chip found!");
		return -ENODEV;
	}

	return 0;
}

#define MFD_NXP_PF15XX_DEFINE(inst)                                                                \
	static const struct mfd_pf15xx_config mfd_pf15xx_config##inst = {                          \
		.bus = I2C_DT_SPEC_INST_GET(inst),                                                 \
		.standby_pin = GPIO_DT_SPEC_GET_OR(inst, standby_pin, {0}),                        \
	};                                                                                         \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, mfd_pf15xx_init, NULL, NULL, &mfd_pf15xx_config##inst,         \
			      POST_KERNEL, CONFIG_MFD_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(MFD_NXP_PF15XX_DEFINE);
