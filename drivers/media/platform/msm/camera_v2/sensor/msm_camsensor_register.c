/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "msm_sensor.h"

#define CDBG_MSM_SENSOR_REGISTER
#undef CDBG
#ifdef CDBG_MSM_SENSOR_REGISTER
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

static int camera_register_flag[2] = {0, 0};

int msm_camsensor_register(enum camb_position_t position)
{
	if(position == BACK_CAMERA_B)
		CDBG("hanjf %s, there was a back sensor registered", __func__);
	else if(position == FRONT_CAMERA_B)
		CDBG("hanjf %s, there was a front sensor registered", __func__);
	else
		pr_err("hanjf %s, there was a unknown sensor registered", __func__);
	
	camera_register_flag[position] = 1;
	return 0;
}

int msm_camsensor_is_registered(enum camb_position_t position)
{
	return camera_register_flag[position];
}