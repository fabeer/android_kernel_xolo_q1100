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
#define s5k3h7_SENSOR_NAME "s5k3h7"
DEFINE_MSM_MUTEX(s5k3h7_mut);

/* Added by yangze for camera sensor hardware_info 2013-08-30 begin */
#ifdef CONFIG_GET_HARDWARE_INFO
#include <mach/hardware_info.h>
static char *s5k3h7_hardware_info="truly s5k3h7 BT119_8M_AF";
#endif
/* Added by yangze for camera sensor hardware_info 2013-08-30 end */

static struct msm_sensor_ctrl_t s5k3h7_s_ctrl;

static struct msm_sensor_power_setting s5k3h7_power_setting[] = {
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

static struct v4l2_subdev_info s5k3h7_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SGRBG10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id s5k3h7_i2c_id[] = {
	{s5k3h7_SENSOR_NAME, (kernel_ulong_t)&s5k3h7_s_ctrl},
	{ }
};

static int32_t msm_s5k3h7_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &s5k3h7_s_ctrl);
}

static struct i2c_driver s5k3h7_i2c_driver = {
	.id_table = s5k3h7_i2c_id,
	.probe  = msm_s5k3h7_i2c_probe,
	.driver = {
		.name = s5k3h7_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client s5k3h7_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id s5k3h7_dt_match[] = {
	{.compatible = "qcom,s5k3h7", .data = &s5k3h7_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, s5k3h7_dt_match);

static struct platform_driver s5k3h7_platform_driver = {
	.driver = {
		.name = "qcom,s5k3h7",
		.owner = THIS_MODULE,
		.of_match_table = s5k3h7_dt_match,
	},
};

static int32_t s5k3h7_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	/*Added Begain:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	if(msm_camsensor_is_registered(BACK_CAMERA_B) == 1)
		return rc;
	/*Added End:By hanjianfeng to add interface of camera register for camera detect (x825)2013-07-26*/
	match = of_match_device(s5k3h7_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	
	// added by yangze for camera sensor hardware_info 2013-08-30 begin
	#ifdef CONFIG_GET_HARDWARE_INFO
	if(rc == 0){
		register_hardware_info(MAIN_CAM, s5k3h7_hardware_info);
	}
	#endif
	// added by yangze for camera sensor hardware_info 2013-08-30 end
	
	return rc;
}

static int __init s5k3h7_init_module(void)
{
	int32_t rc = 0;
	pr_info("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&s5k3h7_platform_driver,
		s5k3h7_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver(&s5k3h7_i2c_driver);
}

static void __exit s5k3h7_exit_module(void)
{
	pr_info("%s:%d\n", __func__, __LINE__);
	if (s5k3h7_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&s5k3h7_s_ctrl);
		platform_driver_unregister(&s5k3h7_platform_driver);
	} else
		i2c_del_driver(&s5k3h7_i2c_driver);
	return;
}

#if 0
struct st_s5k3h7_otp{
	uint16_t R_gain;
	uint16_t G_gain;
	uint16_t B_gain;
	uint8_t LSC_data[267];
};

static uint16_t s5k3h7_default_R_gain = 0x102;
static uint16_t s5k3h7_default_G_gain = 0x30e;
static uint16_t s5k3h7_default_B_gain = 0x100;

static int16_t s5k3h7_default_lenc[267] = {0};

#define OTP_DEBUG_EN 1

static int16_t s5k3h7_check_awb_page(struct msm_sensor_ctrl_t *s_ctrl){
	int16_t flag;

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a04,&flag,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN		
	printk(KERN_ERR "flag is %x.\n", flag);
#endif

	flag &= 0xc0;
	flag = flag >> 6;

	if(flag == 1){
		return 1;
	}else{
		return 0;
	}
}

static int16_t s5k3h7_read_awb_otp_data(struct msm_sensor_ctrl_t *s_ctrl, struct st_s5k3h7_otp *otp_ptr){
	int16_t rc = 1;
	uint16_t temp1 = 0,temp2=0,AWB_page=1;
	uint16_t C_AWB_R,C_AWB_B,C_AWB_GB,C_AWB_GR,C_AWB_G;
	uint16_t G_AWB_R,G_AWB_B,G_AWB_GB,G_AWB_GR,G_AWB_G;
	uint16_t R_Ratio = 0,B_Ratio=0,ct=128;
	uint16_t R_gain, G_gain, B_gain;
	
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x6010,0x0001,
		MSM_CAMERA_I2C_WORD_DATA);
	msleep(3);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x01,
		MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0xFCFC,0xD000,
		MSM_CAMERA_I2C_WORD_DATA);

	for(AWB_page=1;AWB_page<=4;AWB_page++){
		if(AWB_page==4)
		{
#if OTP_DEBUG_EN		
			printk(KERN_ERR "read AWB otp error.\n");
#endif
			return 0 ;
		}
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a02,AWB_page,
			MSM_CAMERA_I2C_BYTE_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a00,0x01,
			MSM_CAMERA_I2C_BYTE_DATA);

		msleep(2);
		rc=s5k3h7_check_awb_page(s_ctrl);	
		if(rc)
			break;
#if OTP_DEBUG_EN		
		printk(KERN_ERR "AWB_otp valid page=%d.\n",AWB_page);
#endif
	}

	msleep(2);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a05,&temp1,
		MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a06,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	C_AWB_R = ((temp1 & 0x3)<<8)+temp2;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a07,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	C_AWB_B = ((temp1 & 0xC)<<6)+temp2;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a08,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	C_AWB_GR = ((temp1 & 0x30)<<4)+temp2;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a09,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	C_AWB_GB = ((temp1 & 0xC0)<<2)+temp2;

	C_AWB_G = (C_AWB_GR+C_AWB_GB)/2;

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a0a,&temp1,
		MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a0b,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	G_AWB_R = ((temp1 & 0x3)<<8)+temp2;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a0c,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	G_AWB_B = ((temp1 & 0xC)<<6)+temp2;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a0d,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	G_AWB_GR = ((temp1 & 0x30)<<4)+temp2;
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a0e,&temp2,
		MSM_CAMERA_I2C_BYTE_DATA);
	G_AWB_GB = ((temp1 & 0xC0)<<2)+temp2;

	G_AWB_G = (G_AWB_GR+G_AWB_GB)/2;

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s.awb.C_AWB_R=%d\n",__func__,C_AWB_R);
	printk(KERN_ERR "%s.awb.C_AWB_B=%d\n",__func__,C_AWB_B);
	printk(KERN_ERR "%s.awb.C_AWB_GR=%d\n",__func__,C_AWB_GR);
	printk(KERN_ERR "%s.awb.C_AWB_GB=%d\n",__func__,C_AWB_GB);
	printk(KERN_ERR "%s.awb.G_AWB_R=%d\n",__func__,G_AWB_R);
	printk(KERN_ERR "%s.awb.G_AWB_B=%d\n",__func__,G_AWB_B);
	printk(KERN_ERR "%s.awb.G_AWB_GR=%d\n",__func__,G_AWB_GR);
	printk(KERN_ERR "%s.awb.G_AWB_GB=%d\n",__func__,G_AWB_GB);
#endif

	R_Ratio =(G_AWB_G*C_AWB_R*ct)/(G_AWB_R*C_AWB_G);
	B_Ratio =(G_AWB_G*C_AWB_B*ct)/(G_AWB_B*C_AWB_G);

#if OTP_DEBUG_EN
	printk(KERN_ERR "R_Ratio=%d,B_Ratio=%d\n",R_Ratio,B_Ratio);
#endif

    if (R_Ratio >= ct)
   	{
   	 if(B_Ratio >= ct)
   	 	{
   	 	 G_gain = 0x100;
		 B_gain = 0x100 * B_Ratio/ct;
		 R_gain = 0x100 * R_Ratio/ct;
   	 	}
	 else
	 	{
	 	B_gain = 0x100;
		G_gain = 0x100*ct/B_Ratio;
		R_gain = 0x100 * R_Ratio/B_Ratio;
	 	}
    }
	else
	{
	  if(B_Ratio >=ct )
	  	{
	  	 R_gain = 0x100;
		 G_gain = 0x100*ct/R_Ratio;
		 B_gain = 0x100*B_Ratio/R_Ratio;
	  	}
	  else
	  	{
	  	if(R_Ratio >= B_Ratio)
	  		{
	  		B_gain = 0x100;
			G_gain = 0x100*ct /B_Ratio;
			R_gain = 0x100 * R_Ratio/B_Ratio;
	  		}
		else
			{
			B_gain = 0x100;
			G_gain = 0x100 *ct/R_Ratio;
			R_gain = 0x100 * B_Ratio/R_Ratio;
			}
	  	}
	}
   otp_ptr->R_gain = R_gain;
   otp_ptr->B_gain = B_gain;
   otp_ptr->G_gain = G_gain;

 return rc;
}

static int16_t s5k3h7_update_awb_otp_data(struct msm_sensor_ctrl_t *s_ctrl){
	int16_t rc = 0;
	struct st_s5k3h7_otp current_otp;

	rc = s5k3h7_read_awb_otp_data(s_ctrl, &current_otp);
	if(rc == 0){

#if OTP_DEBUG_EN		
		printk(KERN_ERR "using default awb OTP data.\n");
#endif
		
		current_otp.R_gain = s5k3h7_default_R_gain;
		current_otp.G_gain = s5k3h7_default_G_gain;
		current_otp.B_gain = s5k3h7_default_B_gain;
	}

#if OTP_DEBUG_EN		
	printk(KERN_ERR "R_gain is %x, B_gain is %x, G_gain is %x.\n",current_otp.R_gain,current_otp.B_gain,current_otp.G_gain);
#endif	

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0210,current_otp.R_gain,MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0212,current_otp.B_gain,MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0214,current_otp.G_gain,MSM_CAMERA_I2C_WORD_DATA);	
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x020e,current_otp.G_gain,MSM_CAMERA_I2C_WORD_DATA);

	return 1;
}

static int16_t s5k3h7_check_Lens_otp_group(struct msm_sensor_ctrl_t *s_ctrl){
	uint16_t temp=0;
	int16_t valid_group = 0;

	 s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x301E,0x05,MSM_CAMERA_I2C_BYTE_DATA);
	 s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x323C,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	 s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x323D,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	 s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x10,MSM_CAMERA_I2C_BYTE_DATA);

	 /*page 0*/
	 s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a02,0x00,MSM_CAMERA_I2C_BYTE_DATA);
	 s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a00,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	 /*read 0x0a33*/
	 s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a33,&temp,MSM_CAMERA_I2C_BYTE_DATA);

#if OTP_DEBUG_EN	 
	printk(KERN_ERR "read OTP : temp : %d\n",temp);
#endif

	 if((temp&0x1)==1)
	 	valid_group = 0;
	 else if((temp&0x2)==1)
	 	valid_group = 1;
	 else
	 	printk(KERN_ERR "read OTP LENS faild\n");

	return valid_group;
}

static int16_t s5k3h7_read_Lens_otp_group(struct msm_sensor_ctrl_t *s_ctrl, struct st_s5k3h7_otp *otp_ptr)
{
	uint16_t temp=0,i=0,lens_addr=0x0a04;
	uint16_t valid_group_page = 0,FLG=0;
	uint16_t reg_0a1e = 0,reg_0a1f;

	valid_group_page = s5k3h7_check_Lens_otp_group(s_ctrl)+1;

#if OTP_DEBUG_EN	
	printk(KERN_ERR "valid_group_page=%d\n",valid_group_page);
#endif

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a02,valid_group_page,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a00,0x01,MSM_CAMERA_I2C_BYTE_DATA);

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a1e,&reg_0a1e,MSM_CAMERA_I2C_BYTE_DATA);
#if OTP_DEBUG_EN		
	printk(KERN_ERR "%s.reg_0a1e=%d\n",__func__,reg_0a1e);
#endif
	if((reg_0a1e&0x3)==0x3)
		return 0;
	else if((reg_0a1e&0x3)==1)
		printk(KERN_ERR "start read lens otp data \n");
	//read otp
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,0x0a1f,&reg_0a1f,MSM_CAMERA_I2C_BYTE_DATA);
#if OTP_DEBUG_EN	
	printk(KERN_ERR "%s.reg_0x0a1f=%d\n",__func__,reg_0a1e);
#endif

	(otp_ptr->LSC_data)[265] = 0x78 & reg_0a1e;//bit[6:3]
	(otp_ptr->LSC_data)[265] = (otp_ptr->LSC_data)[265]>>3;
	(otp_ptr->LSC_data)[266] = reg_0a1f;
	if(valid_group_page==1)
		lens_addr=0x0a04;
	else
		lens_addr=0x0a0d;
	FLG=valid_group_page*4;

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a02,FLG,MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a00,0x01,MSM_CAMERA_I2C_BYTE_DATA);
	msleep(2);
   	for(i = 0;i<265;i++)
	{
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,lens_addr++,&temp,MSM_CAMERA_I2C_BYTE_DATA);
		(otp_ptr->LSC_data)[i] = temp;
#if OTP_DEBUG_EN		
		printk(KERN_ERR "sunny_LSC_data[%d]=0x%x\n",i,temp);
#endif
		if(lens_addr == 0x0A44)
		{
			lens_addr = 0x0A04;
			FLG++;
#if OTP_DEBUG_EN		
			printk(KERN_ERR "sunny_LSC_read page=%d\n",FLG);
#endif
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a02,FLG,MSM_CAMERA_I2C_BYTE_DATA);
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0a00,0x01,MSM_CAMERA_I2C_BYTE_DATA);
			usleep_range(100,200);
		}
	}

	return 1;

}


static int16_t s5k3h7_update_Lens_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t startAddr=0x2d90,tempLSC=0,i=0;
	struct st_s5k3h7_otp current_otp;
	int16_t rc;

	rc = s5k3h7_read_Lens_otp_group(s_ctrl, &current_otp);
	if(rc == 0){

#if OTP_DEBUG_EN		
		printk(KERN_ERR "using default lens OTP data.\n");
#endif
		for(i = 1;i<265;i++){
			current_otp.LSC_data[i] = s5k3h7_default_lenc[i];
		}		
	}

	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0b01,(current_otp.LSC_data)[266],MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x3237,(current_otp.LSC_data)[265],MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x6028,0x7000,MSM_CAMERA_I2C_WORD_DATA);

	for(i = 1;i<265;i++)
	{
		if(startAddr == 0x2db8)
		{
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x602a,startAddr,MSM_CAMERA_I2C_WORD_DATA);
			s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x6f12,0x1c00,MSM_CAMERA_I2C_WORD_DATA);

			startAddr += 2;
			i--;
			continue;
		}
		tempLSC = (current_otp.LSC_data)[i];
		i++;
		tempLSC = tempLSC+(((current_otp.LSC_data)[i]<<8)&0xFF00);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x602a,startAddr,MSM_CAMERA_I2C_WORD_DATA);
		s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x6f12,tempLSC,MSM_CAMERA_I2C_WORD_DATA);
#if OTP_DEBUG_EN
		printk(KERN_ERR "LENS_OTP_test Addr=0x%x,tempLSC=0x%x\n",startAddr,tempLSC);
#endif
		startAddr += 2;
	}
#if OTP_DEBUG_EN
	printk(KERN_ERR "(p_otp->LSC_data)[265]=%d,(p_otp->LSC_data)[266]=%d\n",(current_otp.LSC_data)[265],(current_otp.LSC_data)[266]);
#endif
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x6028,0xd000,MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0b00,0x01,MSM_CAMERA_I2C_BYTE_DATA);//shading on

	return 1;
}

static void s5k3h7_start_setting(struct msm_sensor_ctrl_t * s_ctrl){
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x01,MSM_CAMERA_I2C_BYTE_DATA);
}

static void s5k3h7_stop_setting(struct msm_sensor_ctrl_t * s_ctrl){
	s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,0x0100,0x00,MSM_CAMERA_I2C_BYTE_DATA);
}	
// add by yangze for camera sensor otp func (x825) 2013-08-20 end

// add by yangze for camera sensor otp func test (x825) 2013-08-19 begin
static void s5k3h7_otp_init_setting(struct msm_sensor_ctrl_t *s_ctrl){

#if OTP_DEBUG_EN
	printk(KERN_ERR "%s: add by yangze for otp test.\n", __func__);
#endif

	return;
}

static void s5k3h7_update_otp(struct msm_sensor_ctrl_t *s_ctrl){

	// add by yangze for camera sensor otp func test (x825) 2013-08-20 begin
#if OTP_DEBUG_EN	
	printk(KERN_ERR "%s: add by yangze for otp test.\n", __func__);
#endif

	//s5k3h7_stop_setting(s_ctrl);

	s5k3h7_update_awb_otp_data(s_ctrl);
	//s5k3h7_update_Lens_otp(s_ctrl);

	//s5k3h7_start_setting(s_ctrl);
	// add by yangze for camera sensor otp func (x825) 2013-08-20 end
	
	return;
}
#endif

static struct msm_sensor_fn_t s5k3h7_sensor_func_tbl = {
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id = msm_sensor_match_id,
#if 0	
	.sensor_otp_init_setting = s5k3h7_otp_init_setting,
	.sensor_update_otp = s5k3h7_update_otp,
#else
	.sensor_otp_init_setting = NULL,
	.sensor_update_otp = NULL,	
#endif	
};

static struct msm_sensor_ctrl_t s5k3h7_s_ctrl = {
	.sensor_i2c_client = &s5k3h7_sensor_i2c_client,
	.power_setting_array.power_setting = s5k3h7_power_setting,
	.power_setting_array.size = ARRAY_SIZE(s5k3h7_power_setting),
	.msm_sensor_mutex = &s5k3h7_mut,
	.sensor_v4l2_subdev_info = s5k3h7_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(s5k3h7_subdev_info),
	.func_tbl = &s5k3h7_sensor_func_tbl,
};

module_init(s5k3h7_init_module);
module_exit(s5k3h7_exit_module);
MODULE_DESCRIPTION("s5k3h7");
MODULE_LICENSE("GPL v2");
