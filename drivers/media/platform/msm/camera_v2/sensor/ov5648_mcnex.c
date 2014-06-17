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

#define OV5648_MCNEX_SENSOR_NAME "ov5648_mcnex"
DEFINE_MSM_MUTEX(ov5648_mcnex_mut);

/* Added by yangze for camera sensor hardware_info 2013-08-30 begin */
#ifdef CONFIG_GET_HARDWARE_INFO
#include <mach/hardware_info.h>
static char *ov5648_mcnex_hardware_info="mcnex ov5648 BW901_5M_FF";
#endif
/* Added by yangze for camera sensor hardware_info 2013-08-30 end */

static struct msm_sensor_ctrl_t ov5648_mcnex_s_ctrl;

static struct msm_sensor_power_setting ov5648_mcnex_power_setting[] = {
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 15,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 15,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 30,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct v4l2_subdev_info ov5648_mcnex_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id ov5648_mcnex_i2c_id[] = {
	{OV5648_MCNEX_SENSOR_NAME,
		(kernel_ulong_t)&ov5648_mcnex_s_ctrl},
	{ }
};

static int32_t msm_ov5648_mcnex_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov5648_mcnex_s_ctrl);
}

static struct i2c_driver ov5648_mcnex_i2c_driver = {
	.id_table = ov5648_mcnex_i2c_id,
	.probe  = msm_ov5648_mcnex_i2c_probe,
	.driver = {
		.name = OV5648_MCNEX_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov5648_mcnex_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

// add by yangze for camera sensor otp func test (x825) 2013-08-21 begin
struct st_ov5648_mcnex_otp{
	int16_t module_integrator_id;
	int16_t lens_id;
	int16_t rg_ratio;
	int16_t bg_ratio;
	int16_t user_data[2];
	int16_t light_rg;
	int16_t light_bg;
};

struct st_ov5648_mcnex_otp st_ov5648_mcnex_default_otp={
	.module_integrator_id = 0x31,
	.lens_id = 0x10,
	.rg_ratio = 0x13b,
	.bg_ratio = 0xf9,
	.user_data[0] = 0,
	.user_data[1] = 0,	
	.light_rg = 0,
	.light_bg= 0,
};

// modified by yangze for ov5648 AWB typical value (x825) 2013-08-28 begin
#define BG_Ratio_Typical_ov5648_mcnex 0x0129
#define RG_Ratio_Typical_ov5648_mcnex 0x013E
// modified by yangze for ov5648 AWB typical value (x825) 2013-08-28 end

#define OTP_DEBUG_EN 0

static int16_t ov5648_mcnex_check_otp(struct msm_sensor_ctrl_t *s_ctrl, int16_t index){
	int16_t flag, i;
	int16_t rg, bg;

	if(index == 1){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xc0,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d85,0x00,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d86,0x0f,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(10);		
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x00,MSM_CAMERA_I2C_BYTE_DATA);				
		msleep(5);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d05,&flag,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d07,&rg,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d08,&bg,MSM_CAMERA_I2C_BYTE_DATA);
	}else if(index == 2){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xc0,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d85,0x00,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d86,0x0f,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(10);		
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x00,MSM_CAMERA_I2C_BYTE_DATA);				
		msleep(5);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0e,&flag,MSM_CAMERA_I2C_BYTE_DATA);

		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d10,&rg,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d11,&bg,MSM_CAMERA_I2C_BYTE_DATA);
	}else if(index == 3){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xc0,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d85,0x00,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d86,0x0f,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(10);		
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x00,MSM_CAMERA_I2C_BYTE_DATA);				
		msleep(5);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d17,&flag,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d19,&rg,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d1a,&bg,MSM_CAMERA_I2C_BYTE_DATA);
	}

	for(i=0;i<16;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d00+i,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: flag is %x, rg is %x, bg is %x.\n", __func__, flag, rg, bg);		
#endif

	if(flag == 0x31){
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index has valid data.\n", __func__);		
#endif				
		return 2;
	}else if(flag == 0){
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index is empty.\n", __func__);		
#endif			
		return 0;
	}else{
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index has invalid data.\n", __func__);			
#endif
		return 1;
	}
}

static int16_t ov5648_mcnex_read_otp(struct msm_sensor_ctrl_t *s_ctrl, int16_t index, struct st_ov5648_mcnex_otp *otp_ptr){
	int16_t i;
	int16_t temp1, temp2;

	if(index == 1){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xc0,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d85,0x00,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d86,0x0f,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);
		
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d05,&(otp_ptr->module_integrator_id),MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->module_integrator_id = otp_ptr->module_integrator_id &0x7f;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d06,&(otp_ptr->lens_id),MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0b,&temp1,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d07,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->rg_ratio = (temp2 << 2) + ((temp1 >> 6) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d08,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->bg_ratio = (temp2 << 2) + ((temp1 >> 4) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0c,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->light_rg = (temp2 << 2) + ((temp1 >> 2) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0d,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->light_bg = (temp2 << 2) + (temp1 & 0x03);

		for(i=0;i<2;i++){
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d09+i,&(otp_ptr->user_data[i]),MSM_CAMERA_I2C_BYTE_DATA);
		}
	}else if(index == 2){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xc0,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d85,0x00,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d86,0x0f,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);
		
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0e,&(otp_ptr->module_integrator_id),MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->module_integrator_id = otp_ptr->module_integrator_id &0x7f;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0f,&(otp_ptr->lens_id),MSM_CAMERA_I2C_BYTE_DATA);

		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xc0,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d85,0x10,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d86,0x1f,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);
		
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d04,&temp1,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d00,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->rg_ratio = (temp2 << 2) + ((temp1 >> 6) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d01,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->bg_ratio = (temp2 << 2) + ((temp1 >> 4) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d05,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->light_rg = (temp2 << 2) + ((temp1 >> 2) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d06,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->light_bg = (temp2 << 2) + (temp1 & 0x03);

		for(i=0;i<2;i++){
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d02+i,&(otp_ptr->user_data[i]),MSM_CAMERA_I2C_BYTE_DATA);
		}
	}else if(index == 3){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xc0,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d85,0x10,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d86,0x1f,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);

		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d07,&(otp_ptr->module_integrator_id),MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->module_integrator_id = otp_ptr->module_integrator_id &0x7f;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d08,&(otp_ptr->lens_id),MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0d,&temp1,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d09,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->rg_ratio = (temp2 << 2) + ((temp1 >> 6) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0a,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->bg_ratio = (temp2 << 2) + ((temp1 >> 4) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0e,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->light_rg = (temp2 << 2) + ((temp1 >> 2) & 0x03);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0f,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
		otp_ptr->light_bg = (temp2 << 2) + (temp1 & 0x03);

		for(i=0;i<2;i++){
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0b+i,&(otp_ptr->user_data[i]),MSM_CAMERA_I2C_BYTE_DATA);
		}
	}

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: module_integrator_id is %x.\n", __func__, otp_ptr->module_integrator_id);
	printk(KERN_ERR "%s: lens_id is %x.\n", __func__, otp_ptr->lens_id);
	printk(KERN_ERR "%s: rg_ratio is %x.\n", __func__, otp_ptr->rg_ratio);
	printk(KERN_ERR "%s: bg_ratio is %x.\n", __func__, otp_ptr->bg_ratio);
	printk(KERN_ERR "%s: light_rg is %x.\n", __func__, otp_ptr->light_rg);
	printk(KERN_ERR "%s: light_bg is %x.\n", __func__, otp_ptr->light_bg);
	for(i=0;i<2;i++){
		printk(KERN_ERR "%s: user_data[%d] is %x.\n", __func__, i, otp_ptr->user_data[i]);
	}	
#endif		

	for(i=0;i<16;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d00+i,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}

	return 0;
}

static int16_t ov5648_mcnex_update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl, int16_t R_gain, int16_t G_gain, int16_t B_gain){

#if OTP_DEBUG_EN
	int16_t temp;
	int16_t gain;
#endif

	if(R_gain  >0x400){
		//s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3400,R_gain,MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x5186,R_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x5187,R_gain&0x00ff,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x5186,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = temp << 8;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x5187,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = (gain & 0xff00) | temp;		
		printk(KERN_ERR "%s: R_gain is %x, read reg value is %x.\n", __func__, R_gain, gain);	
#endif
	}

	if(G_gain  >0x400){
		//s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3402,G_gain,MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x5188,G_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x5189,G_gain&0x00ff,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x5188,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = temp << 8;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x5189,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = (gain & 0xff00) | temp;		
		printk(KERN_ERR "%s: G_gain is %x, read reg value is %x.\n", __func__, G_gain, gain);	
#endif
	}

	if(B_gain  >0x400){
		//s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3404,B_gain,MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x518a,B_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x518b,B_gain&0x00ff,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x518a,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = temp << 8;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x518b,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = (gain & 0xff00) | temp;		
		printk(KERN_ERR "%s: B_gain is %x, read reg value is %x.\n", __func__, B_gain, gain);	
#endif
	}	
	
	return 0;
}

static void ov5648_mcnex_start_setting(struct msm_sensor_ctrl_t * s_ctrl){
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x01,MSM_CAMERA_I2C_BYTE_DATA);
}

static void ov5648_mcnex_stop_setting(struct msm_sensor_ctrl_t * s_ctrl){
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x00,MSM_CAMERA_I2C_BYTE_DATA);
}	

static int16_t ov5648_mcnex_update_otp_awb(struct msm_sensor_ctrl_t * s_ctrl){
	struct st_ov5648_mcnex_otp current_otp;
	int16_t i;
	int16_t otp_index;
	int16_t temp;
	int16_t R_gain;
	int16_t G_gain;
	int16_t B_gain;
	int16_t G_gain_R;
	int16_t G_gain_B;
	int16_t rg,bg;

	for(i=1;i<=3;i++){
		temp = ov5648_mcnex_check_otp(s_ctrl, i);
		if(temp == 2){
			otp_index = i;
			break;
		}
	}

	if(i>3){
		current_otp.module_integrator_id = st_ov5648_mcnex_default_otp.module_integrator_id;
		current_otp.lens_id = st_ov5648_mcnex_default_otp.lens_id;
		current_otp.rg_ratio = st_ov5648_mcnex_default_otp.rg_ratio;
		current_otp.bg_ratio = st_ov5648_mcnex_default_otp.bg_ratio;
		current_otp.user_data[0] = st_ov5648_mcnex_default_otp.user_data[0];
		current_otp.user_data[1] = st_ov5648_mcnex_default_otp.user_data[1];
		current_otp.light_rg = st_ov5648_mcnex_default_otp.light_rg;
		current_otp.light_bg= st_ov5648_mcnex_default_otp.light_bg;
	}else{
		ov5648_mcnex_read_otp(s_ctrl, otp_index, &current_otp);
	}

	if(current_otp.light_rg == 0){
		rg = current_otp.rg_ratio;
	}else{
		rg = current_otp.rg_ratio*((current_otp.light_rg+512)/1024);
	}

	if(current_otp.light_bg == 0){
		bg = current_otp.bg_ratio;
	}else{
		bg = current_otp.bg_ratio*((current_otp.light_bg+512)/1024);
	}

	if(bg<BG_Ratio_Typical_ov5648_mcnex){
		if(rg<RG_Ratio_Typical_ov5648_mcnex){
			G_gain = 0x400;
			B_gain = 0x400*BG_Ratio_Typical_ov5648_mcnex/bg;
			R_gain = 0x400*RG_Ratio_Typical_ov5648_mcnex/rg;
		}else{
			R_gain = 0x400;
			G_gain = 0x400*rg/RG_Ratio_Typical_ov5648_mcnex;
			B_gain = G_gain*BG_Ratio_Typical_ov5648_mcnex/bg;
		}	
	}else{
		if(rg<RG_Ratio_Typical_ov5648_mcnex){
			B_gain = 0x400;
			G_gain = 0x400*bg/BG_Ratio_Typical_ov5648_mcnex;
			R_gain = G_gain*RG_Ratio_Typical_ov5648_mcnex/rg;
		}else{
			G_gain_B = 0x400*bg/BG_Ratio_Typical_ov5648_mcnex;
			G_gain_R = 0x400*rg/RG_Ratio_Typical_ov5648_mcnex;

			if(G_gain_B > G_gain_R){
				B_gain = 0x400;
				G_gain = G_gain_B;
				R_gain = G_gain*RG_Ratio_Typical_ov5648_mcnex/rg;
			}else{
				R_gain = 0x400;
				G_gain = G_gain_R;
				B_gain = G_gain*BG_Ratio_Typical_ov5648_mcnex/bg;
			}
		}
	}

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: R_gain is %x, G_gain is %x, B_gain is %x.\n", __func__, R_gain, G_gain, B_gain);	
#endif	

	ov5648_mcnex_update_awb_gain(s_ctrl, R_gain, G_gain, B_gain);

	if(i>3){
		return 1;
	}else{
		return 0;
	}
}
// add by yangze for camera sensor otp func test (x825) 2013-08-21 end

// add by yangze for camera sensor otp func test (x825) 2013-08-19 begin
static void ov5648_mcnex_otp_init_setting(struct msm_sensor_ctrl_t *s_ctrl){

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: add by yangze for otp test.\n", __func__);
#endif	

	return;
}

static void ov5648_mcnex_update_otp(struct msm_sensor_ctrl_t *s_ctrl){
	
	// add by yangze for camera sensor otp func test (x825) 2013-08-21 begin
	int16_t rc;

#if OTP_DEBUG_EN	
	printk(KERN_ERR "%s: add by yangze for otp test.\n", __func__);
#endif

	ov5648_mcnex_start_setting(s_ctrl);

	rc = ov5648_mcnex_update_otp_awb(s_ctrl);
	if(rc == 0){
		printk(KERN_ERR "update_otp_wb: update success.\n");
	}else if(rc == 1){
		printk(KERN_ERR "update_otp_wb: no OTP, using default OTP data.\n");
	}

	ov5648_mcnex_stop_setting(s_ctrl);
	// add by yangze for camera sensor otp func (x825) 2013-08-21 end

	return;
}

static struct msm_sensor_fn_t ov5648_mcnex_sensor_func_tbl = {
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = msm_sensor_match_id,
	.sensor_otp_init_setting = ov5648_mcnex_otp_init_setting,
	.sensor_update_otp = ov5648_mcnex_update_otp,
};
// add by yangze for camera sensor otp func test (x825) 2013-08-19 end

static struct msm_sensor_ctrl_t ov5648_mcnex_s_ctrl = {
	.sensor_i2c_client = &ov5648_mcnex_sensor_i2c_client,
	.power_setting_array.power_setting = ov5648_mcnex_power_setting,
	.power_setting_array.size =
			ARRAY_SIZE(ov5648_mcnex_power_setting),
	.msm_sensor_mutex = &ov5648_mcnex_mut,
	.sensor_v4l2_subdev_info = ov5648_mcnex_subdev_info,
	.sensor_v4l2_subdev_info_size =
			ARRAY_SIZE(ov5648_mcnex_subdev_info),
	// add by yangze for camera sensor otp func test (x825) 2013-08-19
	.func_tbl = &ov5648_mcnex_sensor_func_tbl,				
};

static const struct of_device_id ov5648_mcnex_dt_match[] = {
	{
		.compatible = "qcom,ov5648_mcnex",
		.data = &ov5648_mcnex_s_ctrl
	},
	{}
};

MODULE_DEVICE_TABLE(of, ov5648_mcnex_dt_match);

static struct platform_driver ov5648_mcnex_platform_driver = {
	.driver = {
		.name = "qcom,ov5648_mcnex",
		.owner = THIS_MODULE,
		.of_match_table = ov5648_mcnex_dt_match,
	},
};

static int32_t ov5648_mcnex_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;

	/*Added Begain:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	if(msm_camsensor_is_registered(FRONT_CAMERA_B) == 1)
		return rc;
	/*Added End:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	
	match = of_match_device(ov5648_mcnex_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);

	// added by yangze for camera sensor hardware_info 2013-08-30 begin
	#ifdef CONFIG_GET_HARDWARE_INFO
	if(rc == 0){
		register_hardware_info(SUB_CAM, ov5648_mcnex_hardware_info);
	}
	#endif
	// added by yangze for camera sensor hardware_info 2013-08-30 end
	
	return rc;
}

static int __init ov5648_mcnex_init_module(void)
{
	int32_t rc = 0;

	rc = platform_driver_probe(&ov5648_mcnex_platform_driver,
		ov5648_mcnex_platform_probe);
	if (!rc)
		return rc;
	return i2c_add_driver(&ov5648_mcnex_i2c_driver);
}

static void __exit ov5648_mcnex_exit_module(void)
{
	if (ov5648_mcnex_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov5648_mcnex_s_ctrl);
		platform_driver_unregister(&ov5648_mcnex_platform_driver);
	} else
		i2c_del_driver(&ov5648_mcnex_i2c_driver);
	return;
}

module_init(ov5648_mcnex_init_module);
module_exit(ov5648_mcnex_exit_module);
MODULE_DESCRIPTION("ov5648_mcnex");
MODULE_LICENSE("GPL v2");
