/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "msm_sensor.h"
#include "msm_camsensor_register.h" //Added By hanjianfeng to add interface of camera register for camera detect 2013-07-26

#define HI841_QTECH_SENSOR_NAME "hi841_qtech"
DEFINE_MSM_MUTEX(hi841_qtech_mut);

#ifdef CONFIG_GET_HARDWARE_INFO
#include <mach/hardware_info.h>
static char *hi841_qtech_hardware_info="qtech hi841 FH841AD_QTECH";
#endif

static struct msm_sensor_ctrl_t hi841_qtech_s_ctrl;

static struct msm_sensor_power_setting hi841_qtech_power_setting[] = {
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VAF,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

int32_t hi841_qtech_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t id = 0x10;
	int32_t rc = -1;
	int pin_cameraid_value = 0; //add by hanjianfeng to check pin value of camera id 2013-07-25

  rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,
			0x8408,0x0a,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(1);
  rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,
			0x0103,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(1);
  rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,
			0x0103,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(1);
  rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,
			0x8400,0x03,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(1);

	printk("hi841_qtech_sensor_match_id id_reg : 0x%x,%d\n",s_ctrl->sensordata->slave_info->sensor_id_reg_addr,rc);
	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,
			s_ctrl->sensordata->slave_info->sensor_id_reg_addr,
			&id, MSM_CAMERA_I2C_WORD_DATA);
	printk("hi841_qtech_sensor_match_id readid 0x%x : 0x%x\n", 0x0000, id);

	if (rc < 0)
	{
		printk("%s: read id failed\n", __func__);
		return rc;
	}

	if (id != s_ctrl->sensordata->slave_info->sensor_id)
	{
		return -ENODEV;
	}
	/*Added Begain:By hanjianfeng to add check pin value of camera id  (x825)2013-07-25*/
	pin_cameraid_value = gpio_get_value(s_ctrl->sensordata->gpio_conf->gpio_num_info->gpio_num[SENSOR_GPIO_ID]);
	printk("hanjf read gpio camera id value :%d\n", pin_cameraid_value);
	printk("hanjf want dts camera id value :%d\n", s_ctrl->sensordata->gpio_cameraid_value);
	if(pin_cameraid_value != s_ctrl->sensordata->gpio_cameraid_value)
	{
		printk("hanjf msm_sensor_match_id  pin camerid doesnot match\n");
		return -ENODEV;
	}
	/*Added End:By hanjianfeng to add check pin value of camera id  (x825)2013-07-25*/
	printk("hi841_qtech_sensor_match_id readid ok, success\n");

	return rc;
}


static struct msm_sensor_fn_t hi841_qtech_sensor_fn_t = {
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = hi841_qtech_sensor_match_id,
};

static struct v4l2_subdev_info hi841_qtech_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SGRBG10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id hi841_qtech_i2c_id[] = {
	{HI841_QTECH_SENSOR_NAME, (kernel_ulong_t)&hi841_qtech_s_ctrl},
	{ }
};

static int32_t msm_hi841_qtech_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &hi841_qtech_s_ctrl);
}

static struct i2c_driver hi841_qtech_i2c_driver = {
	.id_table = hi841_qtech_i2c_id,
	.probe  = msm_hi841_qtech_i2c_probe,
	.driver = {
		.name = HI841_QTECH_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client hi841_qtech_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id hi841_qtech_dt_match[] = {
	{.compatible = "qcom,hi841_qtech", .data = &hi841_qtech_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, hi841_qtech_dt_match);

static struct platform_driver hi841_qtech_platform_driver = {
	.driver = {
		.name = "qcom,hi841_qtech",
		.owner = THIS_MODULE,
		.of_match_table = hi841_qtech_dt_match,
	},
};

static int32_t hi841_qtech_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	
	/*Added Begain:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	if(msm_camsensor_is_registered(BACK_CAMERA_B) == 1)
		return rc;
	/*Added End:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	match = of_match_device(hi841_qtech_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	#ifdef CONFIG_GET_HARDWARE_INFO
	if(rc == 0){
		register_hardware_info(MAIN_CAM, hi841_qtech_hardware_info);
	}
	#endif
	return rc;
}

static int __init hi841_qtech_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&hi841_qtech_platform_driver,
		hi841_qtech_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&hi841_qtech_i2c_driver);
}

static void __exit hi841_qtech_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (hi841_qtech_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&hi841_qtech_s_ctrl);
		platform_driver_unregister(&hi841_qtech_platform_driver);
	} else
		i2c_del_driver(&hi841_qtech_i2c_driver);
	return;
}

static struct msm_sensor_ctrl_t hi841_qtech_s_ctrl = {
	.sensor_i2c_client = &hi841_qtech_sensor_i2c_client,
	.power_setting_array.power_setting = hi841_qtech_power_setting,
	.power_setting_array.size = ARRAY_SIZE(hi841_qtech_power_setting),
	.func_tbl = &hi841_qtech_sensor_fn_t,
	.msm_sensor_mutex = &hi841_qtech_mut,
	.sensor_v4l2_subdev_info = hi841_qtech_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(hi841_qtech_subdev_info),
};

module_init(hi841_qtech_init_module);
module_exit(hi841_qtech_exit_module);
MODULE_DESCRIPTION("hi841_qtech");
MODULE_LICENSE("GPL v2");