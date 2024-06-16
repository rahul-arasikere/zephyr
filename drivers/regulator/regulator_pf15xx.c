/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT nxp_pf15xx_regulator

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/linear_range.h>

enum pf15xx_pmic_source {
    PF15xx_PMIC_SOURCE_BUCK1,
    PF15xx_PMIC_SOURCE_BUCK2,
    PF15xx_PMIC_SOURCE_BUCK3,
    PF15xx_PMIC_SOURCE_LDO1,
    PF15xx_PMIC_SOURCE_LDO2,
    PF15xx_PMIC_SOURCE_LDO3,
};

enum pf15xx_buck_current_limits {
    CURRENT_LIMIT_1_0A,
    CURRENT_LIMIT_1_2A,
    CURRENT_LIMIT_1_5A,
    CURRENT_LIMIT_2_0A,
};

static const struct linear_range ldo13_range = LINEAR_RANGE_INIT(75000, 5000, 0, 0x1f);
static const struct linear_range buck12_range = LINEAR_RANGE_INIT(60000, 1250, 0, 0x3f);
static const struct linear_range buck_3_ldo2_range = LINEAR_RANGE_INIT(180000, 10000, 0, 0xf);

static int regulator_pf15xx_power_off(const struct device *dev) {
    return 0;
}

static int regulator_pf15xx_set_dvs_state(const struct device *dev, regulator_dvs_state_t state) {
    return 0;
}

static const regulator_parent_driver_api parent_api = {
    .dvs_state_set = regulator_pf15xx_set_dvs_state,
    .ship_mode = regulator_pf15xx_power_off,
};

static int regulator_pf15xx_enable(const struct device *dev) {
    return 0;
}

static int regulator_pf15xx_disable(const struct device *dev) {
    return 0;
}

static int regulator_pf15xx_set_mode(const struct device *dev, regulator_mode_t mode) {
    return 0;
}

static int regulator_pf15xx_count_voltages(const struct device *dev) {
    return 0;
}

static int regulator_pf15xx_list_voltages(const struct device *dev, unsigned int idx, int32_t * volt_uv) {
    return 0;
}

static int regulator_pf15xx_set_voltage(const struct device *dev, int32_t min_uv, int32_t max_ux) {
    return 0;
}

static int regulator_pf15xx_get_voltage(const struct device *dev, int32_t *volt_uv) {
    return 0;
}

static unsigned int regulator_pf15xx_count_current_limits(const struct device *dev) {
    return 0;
}

static int regulator_pf15xx_list_current_limits(const struct device *dev, unsigned int idx, int32_t *current_ua) {
    return 0;
}

static int regulator_pf15xx_set_current_limit(const struct device *dev, int32_t min_ua, int32_t max_ua) {
    return 0;
}

static const regulator_driver_api api = {
    .enable = regulator_pf15xx_enable,
    .disable = regulator_pf15xx_disable,
    .set_mode = regulator_pf15xx_set_mode,
    .count_voltages = regulator_pf15xx_count_voltages,
    .list_voltage = regulator_pf15xx_list_voltage,
    .set_voltage = regulator_pf15xx_set_voltage,
    .get_voltage = regulator_pf15xx_get_voltage,
    .count_current_limits = regulator_pf15xx_count_current_limits,
    .list_current_limit = regulator_pf15xx_list_current_limit,
    .set_current_limit = regulator_pf15xx_set_current_limit,
};
