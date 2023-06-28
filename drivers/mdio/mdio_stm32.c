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

#include <zephyr/drivers/pinctrl.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mdio_stm32, CONFIG_MDIO_LOG_LEVEL);

#define STM32_SET_PHY_DEV_ADDR(addr) ((addr << ETH_MACMIIAR_PA_Pos) & ETH_MACMIIAR_PA_Msk)
#define STM32_SET_PHY_REG_ADDR(addr) ((addr << ETH_MACMIIAR_MR_Pos) & ETH_MACMIIAR_MR_Msk)

struct mdio_stm32_dev_data {
	struct k_sem sem;
};

struct mdio_stm32_dev_config {
	ETH_TypeDef *regs;
	const struct pinctrl_dev_config *pcfg;
	int protocol;
};

static void mdio_stm32_bus_enable(const struct device *dev)
{
	/* Does nothing */
}

static void mdio_stm32_bus_disable(const struct device *dev)
{
	/* Does nothing */
}

static int mdio_stm32_read(const struct device *dev, uint8_t prtad, uint8_t devad, uint16_t *data)
{
	struct mdio_stm32_dev_config const *cfg = dev->config;
	struct mdio_stm32_dev_data *dev_data = dev->data;
	int timeout = 50;

	k_sem_take(&dev_data->sem, K_FOREVER);

	uint32_t tmpreg1 = cfg->regs->MACMIIAR;
	if (cfg->protocol == CLAUSE_22) {
		tmpreg1 &= ~ETH_MACMIIAR_CR_MASK;          /* Preserve clock bits */
		tmpreg1 |= STM32_SET_PHY_DEV_ADDR(prtad)   /* Set PHY Device Address*/
			   | STM32_SET_PHY_REG_ADDR(devad); /* Set PHY Register Address*/
		tmpreg1 &= ~ETH_MACMIIAR_MR;               /* Set Read Mode */
		tmpreg1 |= ETH_MACMIIAR_MB;              /* Set the busy bit */ 
	} else {
		/* We might have to manually bit bang the other frame types */
		LOG_ERR("Unsupported protocol");
	}

	cfg->regs->MACMIIAR = tmpreg1;

	while ((cfg->regs->MACMIIAR & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB) {
		if (timeout-- == 0) {
			LOG_ERR("Read operation timed out %s", dev->name);
			k_sem_give(&dev_data->sem);
			return -ETIMEDOUT;
		}
		k_sleep(K_MSEC(5));
	}

	/* Move the data out of the PHY register. */
	*data = ((cfg->regs->MACMIIDR >> ETH_MACMIIDR_MD_Pos) & ETH_MACMIIDR_MD_Msk);

	k_sem_give(&dev_data->sem);
	return 0;
}

static int mdio_stm32_write(const struct device *dev, uint8_t prtad, uint8_t devad, uint16_t data)
{
	struct mdio_stm32_dev_config const *cfg = dev->config;
	struct mdio_stm32_dev_data *dev_data = dev->data;
	int timeout = 50;

	k_sem_take(&dev_data->sem, K_FOREVER);

	uint32_t tmpreg1 = cfg->regs->MACMIIAR;
	if (cfg->protocol == CLAUSE_22) {
		tmpreg1 &= ~ETH_MACMIIAR_CR_MASK;          /* Preserve clock bits */
		tmpreg1 |= STM32_SET_PHY_DEV_ADDR(prtad)   /* Set PHY Device Address*/
			   | STM32_SET_PHY_REG_ADDR(devad) /* Set PHY Register Address*/
			   | ETH_MACMIIAR_MW               /* Set Write Mode */
			   | ETH_MACMIIAR_MB;              /* Set the busy bit */
	} else {
		/* We might have to manually bit bang the other frame types */
		LOG_ERR("Unsupported protocol");
	}

	/* Set the data to write */
	cfg->regs->MACMIIDR = (data & ETH_MACMIIDR_MD_Msk) << ETH_MACMIIDR_MD_Pos;
	cfg->regs->MACMIIAR = tmpreg1;

	while ((cfg->regs->MACMIIAR & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB) {
		if (timeout-- == 0) {
			LOG_ERR("Write operation timed out %s", dev->name);
			k_sem_give(&dev_data->sem);
			return -ETIMEDOUT;
		}
		k_sleep(K_MSEC(5));
	}

	k_sem_give(&dev_data->sem);
	return 0;
}

static int mdio_stm32_initialize(const struct device *dev)
{
	__ASSERT_NO_MSG(dev != NULL);
	__ASSERT_NO_MSG(dev->data != NULL);
	__ASSERT_NO_MSG(dev->config != NULL);
	struct mdio_stm32_dev_config const *cfg = dev->config;
	struct mdio_stm32_dev_data *data = dev->data;
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
	static struct mdio_stm32_dev_data mdio_stm32_dev_data##n;                                  \
	static struct mdio_stm32_dev_config mdio_stm32_dev_config##n = {                           \
		.regs = (ETH_TypeDef *)DT_REG_ADDR(DT_INST_PARENT(n)),                             \
		.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(n),                                         \
		.protocol = DT_INST_ENUM_IDX(n, protocol),                                         \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(n, &mdio_stm32_initialize, NULL, &mdio_stm32_dev_data##n,            \
			      &mdio_stm32_dev_config##n, POST_KERNEL, CONFIG_MDIO_INIT_PRIORITY,   \
			      &mdio_stm32_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MDIO_STM32_DEVICE)
