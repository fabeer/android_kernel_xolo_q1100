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

#define OV12830_MCNEX_SENSOR_NAME "ov12830_mcnex"
DEFINE_MSM_MUTEX(ov12830_mcnex_mut);

/* Added by yangze for camera sensor hardware_info 2013-08-30 begin */
#ifdef CONFIG_GET_HARDWARE_INFO
#include <mach/hardware_info.h>
static char *ov12830_mcnex_hardware_info="mcnex ov12830 BW901_13M_AF";
#endif
/* Added by yangze for camera sensor hardware_info 2013-08-30 end */

static struct msm_sensor_ctrl_t ov12830_mcnex_s_ctrl;

static struct msm_sensor_power_setting ov12830_mcnex_power_setting[] = {
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VAF,
		.config_val = 0,
		.delay = 15,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 15,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 40,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 40,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 40,
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

static struct v4l2_subdev_info ov12830_mcnex_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id ov12830_mcnex_i2c_id[] = {
	{OV12830_MCNEX_SENSOR_NAME,
	(kernel_ulong_t)&ov12830_mcnex_s_ctrl},
	{ }
};

static int32_t msm_ov12830_mcnex_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov12830_mcnex_s_ctrl);
}

static struct i2c_driver ov12830_mcnex_i2c_driver = {
	.id_table = ov12830_mcnex_i2c_id,
	.probe  = msm_ov12830_mcnex_i2c_probe,
	.driver = {
		.name = OV12830_MCNEX_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov12830_mcnex_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id ov12830_mcnex_dt_match[] = {
	{.compatible = "qcom,ov12830_mcnex",
	.data = &ov12830_mcnex_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, ov12830_mcnex_dt_match);

static struct platform_driver ov12830_mcnex_platform_driver = {
	.driver = {
		.name = "qcom,ov12830_mcnex",
		.owner = THIS_MODULE,
		.of_match_table = ov12830_mcnex_dt_match,
	},
};

static int32_t ov12830_mcnex_platform_probe
	(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	
	/*Added Begain:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	if(msm_camsensor_is_registered(BACK_CAMERA_B) == 1)
		return rc;
	/*Added End:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	
	match = of_match_device(ov12830_mcnex_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);

	// added by yangze for camera sensor hardware_info 2013-08-30 begin
	#ifdef CONFIG_GET_HARDWARE_INFO
	if(rc == 0){
		register_hardware_info(MAIN_CAM, ov12830_mcnex_hardware_info);
	}
	#endif
	// added by yangze for camera sensor hardware_info 2013-08-30 end
	
	return rc;
}

static int __init ov12830_mcnex_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&ov12830_mcnex_platform_driver,
		ov12830_mcnex_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&ov12830_mcnex_i2c_driver);
}

static void __exit ov12830_mcnex_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (ov12830_mcnex_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov12830_mcnex_s_ctrl);
		platform_driver_unregister
			(&ov12830_mcnex_platform_driver);
	} else
		i2c_del_driver(&ov12830_mcnex_i2c_driver);
	return;
}

// add by yangze for camera sensor otp func (x825) 2013-08-20 begin
struct st_ov12830_mcnex_otp{
	int16_t module_integrator_id;
	int16_t lens_id;
	int16_t production_year;
	int16_t production_month;
	int16_t production_day;
	int16_t rg_ratio;
	int16_t bg_ratio;
	int16_t light_rg;
	int16_t light_bg;
	int16_t user_data[5];
	int16_t lenc[62];
};

static int16_t ov12830_mcnex_default_rg_ratio = 0x146;
static int16_t ov12830_mcnex_default_bg_ratio = 0x146;
static int16_t ov12830_mcnex_default_light_rg = 0x201;
static int16_t ov12830_mcnex_default_light_bg= 0x201;

static int16_t ov12830_mcnex_default_lenc[62] = {
	0x1f,0x16,0x14,0x14,0x16,0x21,0x0f,0x09,
	0x06,0x06,0x0a,0x0f,0x0a,0x03,0,0,
	0x03,0x0a,0x0a,0x03,0,0,0x04,0x0b,
	0x11,0x0b,0x07,0x08,0x0c,0x12,0x26,0x1a,
	0x18,0x18,0x1b,0x28,0x06,0x23,0x02,0x24,
	0x03,0x13,0x13,0x14,0x12,0x33,0x01,0x23,
	0x23,0x23,0x20,0x12,0x12,0x22,0x11,0x31,
	0x11,0x52,0x50,0x51,0x10,0xed
};

// modified by yangze for ov12830 AWB typical value (x825) 2013-08-28 begin
#define BG_Ratio_Typical_ov12830_mcnex 0x0151
#define RG_Ratio_Typical_ov12830_mcnex 0x0136
// modified by yangze for ov12830 AWB typical value (x825) 2013-08-28 end

#define OTP_DEBUG_EN 0

static int16_t ov12830_mcnex_check_otp_wb(struct msm_sensor_ctrl_t *s_ctrl, int16_t index){
	int16_t flag, i;
	int16_t bank;
	int16_t address;

	bank = 0xc0 | index;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,bank,MSM_CAMERA_I2C_BYTE_DATA);
	
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(5);

	address = 0x3d00;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address,&flag,MSM_CAMERA_I2C_BYTE_DATA);
	flag = flag & 0xc0;

	for(i=0;i<16;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d00+i,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}

	if(flag == 0x00){
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index is empty.\n", __func__);		
#endif
		return 0;
	}else if(flag & 0x80){
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index has invalid data.\n", __func__);		
#endif	
		return 1;
	}else{
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index has valid data.\n", __func__);		
#endif		
		return 2;
	}
}

static int16_t ov12830_mcnex_check_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl, int16_t index){
	int16_t flag, i;
	int16_t bank;
	int16_t address;

	bank = 0xc0 | (index*4);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,bank,MSM_CAMERA_I2C_BYTE_DATA);
	
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(5);

	address = 0x3d00;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address,&flag,MSM_CAMERA_I2C_BYTE_DATA);
	flag = flag & 0xc0;

	for(i=0;i<16;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d00+i,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}

	if(flag == 0x00){
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index is empty.\n", __func__);		
#endif
		return 0;
	}else if(flag & 0x80){
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index has invalid data.\n", __func__);		
#endif	
		return 1;
	}else{
#if OTP_DEBUG_EN
		printk(KERN_ERR "%s: group index has valid data.\n", __func__);		
#endif		
		return 2;
	}
}

static int16_t ov12830_mcnex_read_otp_wb(struct msm_sensor_ctrl_t *s_ctrl, int16_t index, struct st_ov12830_mcnex_otp *otp_ptr){
	int16_t i;
	int16_t bank;
	int16_t address;
	int16_t temp1, temp2;

	bank = 0xc0 | index;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,bank,MSM_CAMERA_I2C_BYTE_DATA);
	
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(5);

	address = 0x3d00;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+1,&(otp_ptr->module_integrator_id),MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+2,&(otp_ptr->lens_id),MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+3,&(otp_ptr->production_year),MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+4,&(otp_ptr->production_month),MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+5,&(otp_ptr->production_day),MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+10,&temp1,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+6,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
	otp_ptr->rg_ratio = (temp2 << 2) + ((temp1 >> 6) & 0x03);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+7,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
	otp_ptr->bg_ratio = (temp2 << 2) + ((temp1 >> 4) & 0x03);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+8,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
	otp_ptr->light_rg = (temp2 << 2) + ((temp1 >> 2) & 0x03);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address+9,&temp2,MSM_CAMERA_I2C_BYTE_DATA);
	otp_ptr->light_bg = (temp2 << 2) + (temp1 & 0x03);

	for(i=0;i<5;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0b+i,&(otp_ptr->user_data[i]),MSM_CAMERA_I2C_BYTE_DATA);
	}

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: module_integrator_id is %x.\n", __func__, otp_ptr->module_integrator_id);
	printk(KERN_ERR "%s: lens_id is %x.\n", __func__, otp_ptr->lens_id);
	printk(KERN_ERR "%s: production_year is %x.\n", __func__, otp_ptr->production_year);
	printk(KERN_ERR "%s: production_month is %x.\n", __func__, otp_ptr->production_month);
	printk(KERN_ERR "%s: production_day is %x.\n", __func__, otp_ptr->production_day);
	printk(KERN_ERR "%s: rg_ratio is %x.\n", __func__, otp_ptr->rg_ratio);
	printk(KERN_ERR "%s: bg_ratio is %x.\n", __func__, otp_ptr->bg_ratio);
	printk(KERN_ERR "%s: light_rg is %x.\n", __func__, otp_ptr->light_rg);
	printk(KERN_ERR "%s: light_bg is %x.\n", __func__, otp_ptr->light_bg);
	for(i=0;i<5;i++){
		printk(KERN_ERR "%s: user_data[%d] is %x.\n", __func__, i, otp_ptr->user_data[i]);
	}	
#endif		

	for(i=0;i<16;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d00+i,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}

	return 0;
}

static int16_t ov12830_mcnex_read_otp_lenc(struct msm_sensor_ctrl_t *s_ctrl, int16_t index, struct st_ov12830_mcnex_otp *otp_ptr){
	int16_t i,j,k,l=0;
	int16_t bank;
	int16_t address;

	for(i=0;i<16;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d00+i,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	}	

	bank = 0xc0 | (index*4);

	for(i=0;i<4;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,bank+i,MSM_CAMERA_I2C_BYTE_DATA);
	
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
		msleep(5);

		if(i == 0){
			address = 0x3d01;
			k = 15;
		}else if(i == 3){
			address = 0x3d00;
			k = 15;
		}else{
			address = 0x3d00;
			k = 16;
		}

		for(j=0;j<k;j++){
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,address,&(otp_ptr->lenc[l]),MSM_CAMERA_I2C_BYTE_DATA);
			address++;
			l++;
		}

		for(j=0;j<16;j++){
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d00+j,0x00,MSM_CAMERA_I2C_BYTE_DATA);
		}
	}

#if OTP_DEBUG_EN
	for(i=0;i<62;i++){
		printk(KERN_ERR "%s: lenc[%d] is %x.\n", __func__, i, otp_ptr->lenc[i]);
	}	
#endif

	return 0;
}

static int16_t ov12830_mcnex_update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl, int16_t R_gain, int16_t G_gain, int16_t B_gain){

#if OTP_DEBUG_EN
	int16_t temp;
	int16_t gain;
#endif

	if(R_gain  >0x400){
		//s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3400,R_gain,MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3400,R_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3401,R_gain&0x00ff,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3400,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = temp << 8;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3401,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = (gain & 0xff00) | temp;		
		printk(KERN_ERR "%s: R_gain is %x, read reg value is %x.\n", __func__, R_gain, gain);	
#endif
	}

	if(G_gain  >0x400){
		//s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3402,G_gain,MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3402,G_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3403,G_gain&0x00ff,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3402,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = temp << 8;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3403,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = (gain & 0xff00) | temp;		
		printk(KERN_ERR "%s: G_gain is %x, read reg value is %x.\n", __func__, G_gain, gain);	
#endif
	}

	if(B_gain  >0x400){
		//s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3404,B_gain,MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3404,B_gain>>8,MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3405,B_gain&0x00ff,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3404,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = temp << 8;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3405,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		gain = (gain & 0xff00) | temp;		
		printk(KERN_ERR "%s: B_gain is %x, read reg value is %x.\n", __func__, B_gain, gain);	
#endif
	}	
	
	return 0;
}

static int16_t ov12830_mcnex_update_lenc(struct msm_sensor_ctrl_t *s_ctrl, struct st_ov12830_mcnex_otp *otp_ptr){
	int16_t i,temp;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x5000,&temp,MSM_CAMERA_I2C_BYTE_DATA);
	temp = temp | 0x80;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x5000,temp,MSM_CAMERA_I2C_BYTE_DATA);

	for(i=0;i<62;i++){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x5800+i,otp_ptr->lenc[i],MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x5800+i,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		printk(KERN_ERR "%s: lenc[%d] is %x, read reg value is %x.\n", __func__, i, otp_ptr->lenc[i], temp);	
#endif
	}

	return 0;
}

static int16_t ov12830_mcnex_update_otp_wb(struct msm_sensor_ctrl_t * s_ctrl){
	struct st_ov12830_mcnex_otp current_otp;
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
		temp = ov12830_mcnex_check_otp_wb(s_ctrl, i);
		if(temp == 2){
			otp_index = i;
			break;
		}
	}

	if(i>3){
		current_otp.rg_ratio = ov12830_mcnex_default_rg_ratio;
		current_otp.bg_ratio = ov12830_mcnex_default_bg_ratio;
		current_otp.light_rg = ov12830_mcnex_default_light_rg;
		current_otp.light_bg= ov12830_mcnex_default_light_bg;
	}else{
		ov12830_mcnex_read_otp_wb(s_ctrl, otp_index, &current_otp);
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

	if(bg<BG_Ratio_Typical_ov12830_mcnex){
		if(rg<RG_Ratio_Typical_ov12830_mcnex){
			G_gain = 0x400;
			B_gain = 0x400*BG_Ratio_Typical_ov12830_mcnex/bg;
			R_gain = 0x400*RG_Ratio_Typical_ov12830_mcnex/rg;
		}else{
			R_gain = 0x400;
			G_gain = 0x400*rg/RG_Ratio_Typical_ov12830_mcnex;
			B_gain = G_gain*BG_Ratio_Typical_ov12830_mcnex/bg;
		}	
	}else{
		if(rg<RG_Ratio_Typical_ov12830_mcnex){
			B_gain = 0x400;
			G_gain = 0x400*bg/BG_Ratio_Typical_ov12830_mcnex;
			R_gain = G_gain*RG_Ratio_Typical_ov12830_mcnex/rg;
		}else{
			G_gain_B = 0x400*bg/BG_Ratio_Typical_ov12830_mcnex;
			G_gain_R = 0x400*rg/RG_Ratio_Typical_ov12830_mcnex;

			if(G_gain_B > G_gain_R){
				B_gain = 0x400;
				G_gain = G_gain_B;
				R_gain = G_gain*RG_Ratio_Typical_ov12830_mcnex/rg;
			}else{
				R_gain = 0x400;
				G_gain = G_gain_R;
				B_gain = G_gain*BG_Ratio_Typical_ov12830_mcnex/bg;
			}
		}
	}

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: R_gain is %x, G_gain is %x, B_gain is %x.\n", __func__, R_gain, G_gain, B_gain);	
#endif	

	ov12830_mcnex_update_awb_gain(s_ctrl, R_gain, G_gain, B_gain);

	if(i>3){
		return 1;
	}else{
		return 0;
	}
}

static int16_t ov12830_mcnex_update_otp_lenc(struct msm_sensor_ctrl_t * s_ctrl){
	struct st_ov12830_mcnex_otp current_otp;
	int16_t i,j;
	int16_t otp_index;
	int16_t temp;

	for(i=1;i<=3;i++){
		temp = ov12830_mcnex_check_otp_lenc(s_ctrl, i);
		if(temp == 2){
			otp_index = i;
			break;
		}
	}

	if(i>3){
		for(j=0;j<62;j++){
			current_otp.lenc[i] = ov12830_mcnex_default_lenc[i];
		}
	}else{
		ov12830_mcnex_read_otp_lenc(s_ctrl, otp_index, &current_otp);
	}

	ov12830_mcnex_update_lenc(s_ctrl, &current_otp);

	if(i>3){
		return 1;
	}else{
		return 0;
	}
}

static int16_t ov12830_mcnex_update_blc_ratio(struct msm_sensor_ctrl_t * s_ctrl){
	int16_t k;
	int16_t temp;
	
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d84,0xdf,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3d81,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(5);

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0b,&k,MSM_CAMERA_I2C_BYTE_DATA);
	
#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: reg 0x3d0b is %x.\n", __func__, k);	
#endif		

	if(k != 0){
		if(k>=0x15 && k<=0x40){
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4008,&temp,MSM_CAMERA_I2C_BYTE_DATA);
			temp &= 0xfb;
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4008,temp,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4008,&temp,MSM_CAMERA_I2C_BYTE_DATA);
			printk(KERN_ERR "%s: reg 0x4008 is %x.\n", __func__, temp);	
#endif	

			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4000,&temp,MSM_CAMERA_I2C_BYTE_DATA);
			temp &= 0xf7;
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4000,temp,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4000,&temp,MSM_CAMERA_I2C_BYTE_DATA);
			printk(KERN_ERR "%s: reg 0x4000 is %x.\n", __func__, temp);	
#endif
			return 2;
		}
	}

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x3d0a,&k,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: reg 0x3d0a is %x.\n", __func__, k);	
#endif
	
	if(k>=0x10 && k<=0x40){
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4006,k,MSM_CAMERA_I2C_BYTE_DATA);
		
#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4006,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		printk(KERN_ERR "%s: reg 0x4006 is %x.\n", __func__, temp);	
#endif		

		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4008,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		temp &= 0xfb;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4008,temp,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4008,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		printk(KERN_ERR "%s: reg 0x4008 is %x.\n", __func__, temp);	
#endif		

		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4000,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		temp |= 0x08;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4000,temp,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4000,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		printk(KERN_ERR "%s: reg 0x4000 is %x.\n", __func__, temp);	
#endif	

		return 1;
	}else{
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4006,0x20,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4006,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		printk(KERN_ERR "%s: reg 0x4006 is %x.\n", __func__, temp);	
#endif			

		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4008,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		temp &= 0xfb;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4008,temp,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4008,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		printk(KERN_ERR "%s: reg 0x4008 is %x.\n", __func__, temp);	
#endif		

		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4000,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		temp |= 0x08;
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x4000,temp,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x4000,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		printk(KERN_ERR "%s: reg 0x4000 is %x.\n", __func__, temp);	
#endif	
		return 0;
	}
}	

static void ov12830_mcnex_start_setting(struct msm_sensor_ctrl_t * s_ctrl){
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x01,MSM_CAMERA_I2C_BYTE_DATA);
}

static void ov12830_mcnex_stop_setting(struct msm_sensor_ctrl_t * s_ctrl){
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x00,MSM_CAMERA_I2C_BYTE_DATA);
}	
// add by yangze for camera sensor otp func (x825) 2013-08-20 end

// add by yangze for camera sensor otp func test (x825) 2013-08-19 begin
static void ov12830_mcnex_otp_init_setting(struct msm_sensor_ctrl_t *s_ctrl){

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: add by yangze for otp test.\n", __func__);
#endif

	return;
}

static void ov12830_mcnex_update_otp(struct msm_sensor_ctrl_t *s_ctrl){

	// add by yangze for camera sensor otp func test (x825) 2013-08-20 begin
	int16_t rc;

#if OTP_DEBUG_EN	
	printk(KERN_ERR "%s: add by yangze for otp test.\n", __func__);
#endif

	ov12830_mcnex_start_setting(s_ctrl);

	rc = ov12830_mcnex_update_otp_wb(s_ctrl);
	if(rc == 0){
		printk(KERN_ERR "update_otp_wb: update success.\n");
	}else if(rc == 1){
		printk(KERN_ERR "update_otp_wb: no OTP, using default OTP data.\n");
		//return;
	}

	rc = ov12830_mcnex_update_otp_lenc(s_ctrl);
	if(rc == 0){
		printk(KERN_ERR "update_otp_lenc: update success.\n");
	}else if(rc == 1){
		printk(KERN_ERR "update_otp_lenc: no OTP, using default OTP data.\n");
		//return;
	}
	
	rc = ov12830_mcnex_update_blc_ratio(s_ctrl);
	if(rc == 2){
		printk(KERN_ERR "update_blc_ratio: use CP data from REG3D0A.\n");
	}else if(rc == 1){
		printk(KERN_ERR "update_blc_ratio: use Module data from REG3D0A.\n");
	}else if(rc == 0){
		printk(KERN_ERR "update_blc_ratio: data error.\n");
	}

	ov12830_mcnex_stop_setting(s_ctrl);
	// add by yangze for camera sensor otp func (x825) 2013-08-20 end

	return;
}

static struct msm_sensor_fn_t ov12830_mcnex_sensor_func_tbl = {
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = msm_sensor_match_id,
	.sensor_otp_init_setting = ov12830_mcnex_otp_init_setting,
	.sensor_update_otp = ov12830_mcnex_update_otp,
};
// add by yangze for camera sensor otp func test (x825) 2013-08-19 end
static struct msm_sensor_ctrl_t ov12830_mcnex_s_ctrl = {
	.sensor_i2c_client = &ov12830_mcnex_sensor_i2c_client,
	.power_setting_array.power_setting = ov12830_mcnex_power_setting,
	.power_setting_array.size =
		ARRAY_SIZE(ov12830_mcnex_power_setting),
	.msm_sensor_mutex = &ov12830_mcnex_mut,
	.sensor_v4l2_subdev_info = ov12830_mcnex_subdev_info,
	.sensor_v4l2_subdev_info_size =
		ARRAY_SIZE(ov12830_mcnex_subdev_info),
	// add by yangze for camera sensor otp func test (x825) 2013-08-19
	.func_tbl = &ov12830_mcnex_sensor_func_tbl,
};

module_init(ov12830_mcnex_init_module);
module_exit(ov12830_mcnex_exit_module);
MODULE_DESCRIPTION("ov12830_mcnex");
MODULE_LICENSE("GPL v2");
