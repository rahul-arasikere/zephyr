/*
 * Copyright (c) 2023 Rahul Arasikere
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stm32_mdio

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <soc.h>
#include <zephyr/drivers/mdio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mdio_stm32, CONFIG_MDIO_LOG_LEVEL);

struct mdio_stm32_dev_data {
	struct k_sem sem;
};

struct mdio_stm32_dev_config {
	ETH_TypeDef *regs;
	const struct pinctrl_dev_config *pcfg;
	int protocol;
};

static int mdio_stm32_transfer(const struct device *dev, uint8_t prtadm, uint8_t devad, uint8_t rw, uint16_t data_in, uint16_t *data_out) {
	
}

static void mdio_stm32_bus_enable(const struct device *dev)
{
}

static void mdio_stm32_bus_disable(const struct device *dev)
{
}

static int mdio_stm32_read(const struct device *dev, uint8_t prtad, uint8_t devad, uint16_t *data)
{
}

static int mdio_stm32_write(const struct device *dev, uint8_t prtad, uint8_t devad, uint16_t data)
{
}

static int mdio_stm32_initialize(const struct device *dev)
{
	__ASSERT_NO_MSG(dev != NULL);
	__ASSERT_NO_MSG(dev->data != NULL);
	__ASSERT_NO_MSG(dev->config != NULL);
	const struct mdio_stm32_dev_config *const cfg = dev->config;
	struct mdio_stm32_dev_data *const data = dev->data;
	int retval;

	retval = k_sem_init(&data->sem, 1, 1);
	if (retval == 0) {
		retval = pinctrl_apply_state(cfg->pcfg, PINCTRL_STATE_DEFAULT);
	}
	return retval;
}

static const struct mdio_driver_api mdio_stm32_driver_api = {
	.read = mdio_stm32_read,
	.write = mdio_stm32_write,
	.bus_enable = mdio_stm32_bus_enable,
	.bus_disable = mdio_stm32_bus_disable,
};

#define MDIO_STM32_DEVICE(n)                                                                       \
	PINCTRL_DT_INST_DEFINE(n);                                                                 \
	static struc mdio_stm32_dev_data mdio_stm32_dev_data##n;                                   \
	static struc mdio_stm32_dev_config mdio_stm32_dev_config##n = {	\
		.regs = (ETH_TypeDef *)DT_REG_ADDR(DT_INST_PARENT(n)), \
		.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(n), \ 
		.protocol = DT_INST_ENUM_IDX(n, protocol), \
	};                          \
	DEVICE_DT_INST_DEFINE(n, &mdio_stm32_initialize, NULL, &mdio_stm32_dev_data##n,            \
			      &mdio_stm32_dev_config##n, POST_KERNEL, CONFIG_MDIO_INIT_PRIORITY,   \
			      &mdio_stm32_driver_api);