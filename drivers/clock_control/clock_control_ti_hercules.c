/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_hercules_gcm

#include <soc.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/dt-bindings/clock/ti_hercules_clock.h>

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

static int ti_hercules_gcm_clock_init(const struct device *dev) {
    return 0;
}

static const struct clock_control_driver_api ti_hercules_gcm_clock_api = {
	.on = ti_hercules_gcm_clock_on,
	.off = ti_hercules_gcm_clock_off,
	.get_status = ti_hercules_gcm_clock_get_status,
	.get_rate = ti_hercules_gcm_clock_get_rate,
	.configure = ti_hercules_gcm_clock_configure};
