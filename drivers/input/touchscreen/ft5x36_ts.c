/* drivers/input/touchscreen/focaltech_ts.c
 *
 * FocalTech Serils IC TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include "ft5x36_ts.h"
#if defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>

#elif defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
/* Early-suspend level */
#define FT_SUSPEND_LEVEL 1
#endif

#ifdef FTS_CTL_IIC
#include "ft5x36_ctl.h"
#endif
#ifdef FTS_SYSFS_DEBUG
#include "ft5x36_ex.h"
#endif
/* Added by yanwenlong for increase flash hardware_info (general) 2013.8.29 begin */
#ifdef CONFIG_GET_HARDWARE_INFO
#include <mach/hardware_info.h>
static char tmp_panel_name[100];
#endif
/* Added by yanwenlong for increase flash hardware_info (general) 2013.8.29 end */
struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};
#if 0
struct fts_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	const struct fts_platform_data *pdata;
	struct regulator *vdd;
	struct regulator *vcc_i2c;
	char fw_name[FT_FW_NAME_MAX_LEN];
	bool loading_fw;
	u8 family_id;
	struct dentry *dir;
	u16 addr;
	bool suspended;
	char *ts_info;
#if defined(CONFIG_FB)
	struct notifier_block fb_notif;
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_suspend;
#endif
};
#endif
#if 1
struct fts_ts_data {
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	struct fts_platform_data *pdata;
	struct regulator *vdd;
	struct regulator *vcc_i2c;
	bool loading_fw;
	bool suspended;
#if defined(CONFIG_FB)
	struct notifier_block fb_notif;
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_suspend;
#endif
};
#endif
#define FTS_POINT_UP		0x01
#define FTS_POINT_DOWN		0x00
#define FTS_POINT_CONTACT	0x02


/*
*fts_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
struct kobject *ft5x36_virtual_key_properties_kobj;
static ssize_t
ft5x36_virtual_keys_register(struct kobject *kobj,
                            struct kobj_attribute *attr,
                            char *buf)
{
       return snprintf(buf, 200,
       __stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":150:1350:80:100"
       ":" __stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE)   ":400:1350:80:100"
       ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":600:1350:80:100"
       "\n");
}

static struct kobj_attribute ft5x36_virtual_keys_attr = {
       .attr = {
               .name = "virtualkeys.FT5x36",
               .mode = S_IRUGO,
       },
       .show = &ft5x36_virtual_keys_register,
};

static struct attribute *ft5x36_virtual_key_properties_attrs[] = {
       &ft5x36_virtual_keys_attr.attr,
       NULL,
};

static struct attribute_group ft5x36_virtual_key_properties_attr_group = {
       .attrs = ft5x36_virtual_key_properties_attrs,
};

int fts_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int fts_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);

	return ret;
}
#if 0
/*release the point*/
static void fts_ts_release(struct fts_ts_data *data)
{
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_sync(data->input_dev);
}
#endif
/*Read touch point information when the interrupt  is asserted.*/
static int fts_read_Touchdata(struct fts_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = fts_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	//event->touch_point = buf[2] & 0x0F;

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
	//for (i = 0; i < event->touch_point; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
	}
	
	event->pressure = FT_PRESS;

	return 0;
}

/*
*report the point information
*/
static void fts_report_value(struct fts_ts_data *data)
{
	struct ts_event *event = &data->event;
	//struct fts_platform_data *pdata = &data->pdata;
	int i = 0;
	//int up_point = 0;
	//int touch_point = 0;
	int fingerdown = 0;

	for (i = 0; i < event->touch_point; i++) {
		if (event->au8_touch_event[i] == 0 || event->au8_touch_event[i] == 2) {
			event->pressure = FT_PRESS;
			fingerdown++;
		} else {
			event->pressure = 0;
		}
		input_mt_slot(data->input_dev, event->au8_finger_id[i]);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,
						!!event->pressure);
		if (event->pressure == FT_PRESS) {
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
					event->pressure);
		}
	}
	//printk(KERN_INFO"----------------X=%d.Y=%d\n",event->au16_x[i],event->au16_y[i]);
	input_report_key(data->input_dev, BTN_TOUCH, !!fingerdown);
	input_sync(data->input_dev);

}

/*The FocalTech device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t fts_ts_interrupt(int irq, void *dev_id)
{
	struct fts_ts_data *fts_ts = dev_id;
	int ret = 0;
	disable_irq_nosync(fts_ts->irq);

	ret = fts_read_Touchdata(fts_ts);
	if (ret == 0)
		fts_report_value(fts_ts);

	enable_irq(fts_ts->irq);

	return IRQ_HANDLED;
}

#ifdef CONFIG_OF
static int ft5x36_get_dt_coords(struct device *dev, char *name,
				struct fts_platform_data *pdata)
{
	u32 coords[FT_COORDS_ARR_SIZE];
	struct property *prop;
	struct device_node *np = dev->of_node;
	int coords_size, rc;

	prop = of_find_property(np, name, NULL);
	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;

	coords_size = prop->length / sizeof(u32);
	if (coords_size != FT_COORDS_ARR_SIZE) {
		dev_err(dev, "invalid %s\n", name);
		return -EINVAL;
	}

	rc = of_property_read_u32_array(np, name, coords, coords_size);
	if (rc && (rc != -EINVAL)) {
		dev_err(dev, "Unable to read %s\n", name);
		return rc;
	}

	if (!strcmp(name, "focaltech,panel-coords")) {
		pdata->panel_minx = coords[0];
		pdata->panel_miny = coords[1];
		pdata->panel_maxx = coords[2];
		pdata->panel_maxy = coords[3];
	} else if (!strcmp(name, "focaltech,display-coords")) {
		pdata->x_min = coords[0];
		pdata->y_min = coords[1];
		pdata->x_max = coords[2];
		pdata->y_max = coords[3];
	} else {
		dev_err(dev, "unsupported property %s\n", name);
		return -EINVAL;
	}

	return 0;
}

static int ft5x36_parse_dt(struct device *dev,
			struct fts_platform_data *pdata)
{
	int rc;
	struct device_node *np = dev->of_node;
	struct property *prop;
	u32 temp_val, num_buttons;
	u32 button_map[MAX_BUTTONS];
	bool virthual_key_support;

	rc = ft5x36_get_dt_coords(dev, "focaltech,panel-coords", pdata);
	if (rc && (rc != -EINVAL))
		return rc;

	rc = ft5x36_get_dt_coords(dev, "focaltech,display-coords", pdata);
	if (rc)
		return rc;

	pdata->i2c_pull_up = of_property_read_bool(np,
						"focaltech,i2c-pull-up");

	pdata->no_force_update = of_property_read_bool(np,
						"focaltech,no-force-update");
	/* reset, irq gpio info */
	pdata->reset_gpio = of_get_named_gpio_flags(np, "focaltech,reset-gpio",
				0, &pdata->reset_gpio_flags);
	if (pdata->reset_gpio < 0)
		return pdata->reset_gpio;

	pdata->irq_gpio = of_get_named_gpio_flags(np, "focaltech,irq-gpio",
				0, &pdata->irq_gpio_flags);
	if (pdata->irq_gpio < 0)
		return pdata->irq_gpio;

	rc = of_property_read_u32(np, "focaltech,family-id", &temp_val);
	if (!rc)
		pdata->family_id = temp_val;
	else
		return rc;

	prop = of_find_property(np, "focaltech,button-map", NULL);
	if (prop) {
		num_buttons = prop->length / sizeof(temp_val);
		if (num_buttons > MAX_BUTTONS)
			return -EINVAL;

		rc = of_property_read_u32_array(np,
			"focaltech,button-map", button_map,
			num_buttons);
		if (rc) {
			dev_err(dev, "Unable to read key codes\n");
			return rc;
		}
	}

	virthual_key_support = of_property_read_bool(np,
                        "focaltech,virtual_key");
	if(virthual_key_support){
	      printk("%s,virtual key support...\n",__func__);
	      ft5x36_virtual_key_properties_kobj =
                       kobject_create_and_add("board_properties", NULL);
	      if(ft5x36_virtual_key_properties_kobj){
	      rc = sysfs_create_group(ft5x36_virtual_key_properties_kobj,
                               &ft5x36_virtual_key_properties_attr_group);
	      if(rc)
		    return rc;
	       }
	}	
	return 0;
}
#else
static int ft5x36_parse_dt(struct device *dev,
			struct fts_platform_data *pdata)
{
	return -ENODEV;
}
#endif

static int ft5x36_power_on(struct fts_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto power_off;

	rc = regulator_enable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd enable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_enable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c enable failed rc=%d\n", rc);
		regulator_disable(data->vdd);
	}

	return rc;

power_off:
	rc = regulator_disable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd disable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_disable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c disable failed rc=%d\n", rc);
		regulator_enable(data->vdd);
	}

	return rc;
}

static int ft5x36_power_init(struct fts_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto pwr_deinit;

	data->vdd = regulator_get(&data->client->dev, "vdd");
	if (IS_ERR(data->vdd)) {
		rc = PTR_ERR(data->vdd);
		dev_err(&data->client->dev,
			"Regulator get failed vdd rc=%d\n", rc);
		return rc;
	}

	if (regulator_count_voltages(data->vdd) > 0) {
		rc = regulator_set_voltage(data->vdd, FT_VTG_MIN_UV,
					   FT_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
				"Regulator set_vtg failed vdd rc=%d\n", rc);
			goto reg_vdd_put;
		}
	}

	data->vcc_i2c = regulator_get(&data->client->dev, "vcc_i2c");
	if (IS_ERR(data->vcc_i2c)) {
		rc = PTR_ERR(data->vcc_i2c);
		dev_err(&data->client->dev,
			"Regulator get failed vcc_i2c rc=%d\n", rc);
		goto reg_vdd_set_vtg;
	}

	if (regulator_count_voltages(data->vcc_i2c) > 0) {
		rc = regulator_set_voltage(data->vcc_i2c, FT_I2C_VTG_MIN_UV,
					   FT_I2C_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
			"Regulator set_vtg failed vcc_i2c rc=%d\n", rc);
			goto reg_vcc_i2c_put;
		}
	}
	return 0;

reg_vcc_i2c_put:
	regulator_put(data->vcc_i2c);
reg_vdd_set_vtg:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT_VTG_MAX_UV);
reg_vdd_put:
	regulator_put(data->vdd);
	return rc;

pwr_deinit:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT_VTG_MAX_UV);

	regulator_put(data->vdd);

	if (regulator_count_voltages(data->vcc_i2c) > 0)
		regulator_set_voltage(data->vcc_i2c, 0, FT_I2C_VTG_MAX_UV);

	regulator_put(data->vcc_i2c);
	return 0;
}
#ifdef CONFIG_PM
static int ft5x36_ts_suspend(struct device *dev)
{
	struct fts_ts_data *ft5x36_ts = dev_get_drvdata(dev);
	char txbuf[2], i;
	int err;

	if (ft5x36_ts->loading_fw) {
		dev_info(dev, "Firmware loading in process...\n");
		return 0;
	}

	if (ft5x36_ts->suspended) {
		dev_info(dev, "Already in suspend state\n");
		return 0;
	}

	disable_irq(ft5x36_ts->client->irq);

	/* release all touches */
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		input_mt_slot(ft5x36_ts->input_dev, i);
		input_mt_report_slot_state(ft5x36_ts->input_dev, MT_TOOL_FINGER, 0);
	}
	input_report_key(ft5x36_ts->input_dev, BTN_TOUCH, 0);
	input_sync(ft5x36_ts->input_dev);

	if (gpio_is_valid(ft5x36_ts->pdata->reset_gpio)) {
		txbuf[0] = FTS_REG_PMODE;
		txbuf[1] = FTS_PMODE_HIBERNATE;
		fts_i2c_Write(ft5x36_ts->client, txbuf, sizeof(txbuf));
	}

	if (ft5x36_ts->pdata->power_on) {
		err = ft5x36_ts->pdata->power_on(false);
		if (err) {
			dev_err(dev, "power off failed");
			goto pwr_off_fail;
		}
	} else {
		err = ft5x36_power_on(ft5x36_ts, false);
		if (err) {
			dev_err(dev, "power off failed");
			goto pwr_off_fail;
		}
	}

	ft5x36_ts->suspended = true;

	return 0;

pwr_off_fail:
	if (gpio_is_valid(ft5x36_ts->pdata->reset_gpio)) {
		gpio_set_value_cansleep(ft5x36_ts->pdata->reset_gpio, 0);
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(ft5x36_ts->pdata->reset_gpio, 1);
	}
	enable_irq(ft5x36_ts->client->irq);
	return err;
}

static int ft5x36_ts_resume(struct device *dev)
{
	struct fts_ts_data *ft5x36_ts = dev_get_drvdata(dev);
	int err;

	if (!ft5x36_ts->suspended) {
		dev_info(dev, "Already in awake state\n");
		return 0;
	}

	if (ft5x36_ts->pdata->power_on) {
		err = ft5x36_ts->pdata->power_on(true);
		if (err) {
			dev_err(dev, "power on failed");
			return err;
		}
	} else {
		err = ft5x36_power_on(ft5x36_ts, true);
		if (err) {
			dev_err(dev, "power on failed");
			return err;
		}
	}

	if (gpio_is_valid(ft5x36_ts->pdata->reset_gpio)) {
		gpio_set_value_cansleep(ft5x36_ts->pdata->reset_gpio, 0);
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(ft5x36_ts->pdata->reset_gpio, 1);
	}

	msleep(FT_STARTUP_DLY);

	enable_irq(ft5x36_ts->client->irq);

	ft5x36_ts->suspended = false;

	return 0;
}

static const struct dev_pm_ops ft5x36_ts_pm_ops = {
#if (!defined(CONFIG_FB) && !defined(CONFIG_HAS_EARLYSUSPEND))
	.suspend = ft5x36_ts_suspend,
	.resume = ft5x36_ts_resume,
#endif
};
#endif

#if defined(CONFIG_FB)
static int fb_notifier_callback(struct notifier_block *self,
				 unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;
	struct fts_ts_data *ft5x36_data =
		container_of(self, struct fts_ts_data, fb_notif);

	if (evdata && evdata->data && event == FB_EVENT_BLANK &&
			ft5x36_data && ft5x36_data->client) {
		blank = evdata->data;
		if (*blank == FB_BLANK_UNBLANK)
			ft5x36_ts_resume(&ft5x36_data->client->dev);
		else if (*blank == FB_BLANK_POWERDOWN)
			ft5x36_ts_suspend(&ft5x36_data->client->dev);
	}

	return 0;
}
#elif defined(CONFIG_HAS_EARLYSUSPEND)
static void ft5x36_ts_early_suspend(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct fts_ts_data,
						   early_suspend);

	ft5x36_ts_suspend(&data->client->dev);
}

static void ft5x36_ts_late_resume(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct fts_ts_data,
						   early_suspend);

	ft5x36_ts_resume(&data->client->dev);
}
#endif



static int fts_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct fts_platform_data *pdata =
	    (struct fts_platform_data *)client->dev.platform_data;
	struct fts_ts_data *fts_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;
	//char projectcode[32]; 

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct fts_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}

		err = ft5x36_parse_dt(&client->dev, pdata);
		if (err)
			return err;
	} else
		pdata = client->dev.platform_data;

	if (!pdata) {
		dev_err(&client->dev, "Invalid pdata\n");
		return -EINVAL;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		return err;
	}
	fts_ts = kzalloc(sizeof(struct fts_ts_data), GFP_KERNEL);

	if (!fts_ts) {
		err = -ENOMEM;
		return err;
	}
	
	i2c_set_clientdata(client, fts_ts);
	fts_ts->irq = client->irq;
	fts_ts->client = client;
	fts_ts->pdata = pdata;
	fts_ts->x_max = pdata->x_max - 1;
	fts_ts->y_max = pdata->y_max - 1;
	#if 0
	err = request_threaded_irq(client->irq, NULL, fts_ts_interrupt,
				   pdata->irqflags, client->dev.driver->name,
				   fts_ts);
	if (err < 0) {
		dev_err(&client->dev, "fts_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}
	disable_irq(client->irq);
	#endif
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto free_mem;
	}
	fts_ts->input_dev = input_dev;
	
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	input_mt_init_slots(input_dev, CFG_MAX_TOUCH_POINTS);
       input_set_abs_params(input_dev, ABS_MT_POSITION_X, pdata->x_min, pdata->x_max, 0, 0);
       input_set_abs_params(input_dev, ABS_MT_POSITION_Y, pdata->y_min, pdata->y_max, 0, 0);
       input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
       input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
       input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);

       input_set_capability(input_dev, EV_KEY, KEY_MENU); 
       input_set_capability(input_dev, EV_KEY, KEY_HOMEPAGE); 
       input_set_capability(input_dev, EV_KEY, KEY_BACK); 

	input_dev->name = FTS_NAME;
	input_dev->id.bustype = BUS_I2C;
	input_dev->id.product = 0x0101;	
	input_dev->id.vendor = 0x1010;	
	input_dev->id.version = 0x000a;	
	
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
			"fts_ts_probe: failed to register input device: %s\n",
			dev_name(&client->dev));
		goto free_inputdev;
	}
		
	if (pdata->power_init) {
		err = pdata->power_init(true);
		if (err) {
			dev_err(&client->dev, "power init failed");
			goto unreg_inputdev;
		}
	} else {
		err = ft5x36_power_init(fts_ts, true);
		if (err) {
			dev_err(&client->dev, "power init failed");
			goto unreg_inputdev;
		}
	}

	if (pdata->power_on) {
		err = pdata->power_on(true);
		if (err) {
			dev_err(&client->dev, "power on failed");
			goto pwr_deinit;
		}
	} else {
		err = ft5x36_power_on(fts_ts, true);
		if (err) {
			dev_err(&client->dev, "power on failed");
			goto pwr_deinit;
		}
	}

	if (gpio_is_valid(pdata->irq_gpio)) {
		
		err = gpio_request(pdata->irq_gpio, "ft5x36_irq_gpio");
		if (err) {
			dev_err(&client->dev, "irq gpio request failed");
			goto pwr_off;
		}
		err = gpio_direction_input(pdata->irq_gpio);
		if (err) {
			dev_err(&client->dev,
				"set_direction for irq gpio failed\n");
			goto free_irq_gpio;
		}
	}

	if (gpio_is_valid(pdata->reset_gpio)) {
		
		err = gpio_request(pdata->reset_gpio, "ft5x36_reset_gpio");
		if (err) {
			dev_err(&client->dev, "reset gpio request failed");
			goto free_irq_gpio;
		}

		err = gpio_direction_output(pdata->reset_gpio, 0);
		if (err) {
			dev_err(&client->dev,
				"set_direction for reset gpio failed\n");
			goto free_reset_gpio;
		}
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(fts_ts->pdata->reset_gpio, 1);
	}

	/*make sure CTP already finish startup process */
	msleep(150);

	/*get some register information */
	uc_reg_addr = FTS_REG_FW_VER;
	err = fts_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	if (err < 0) {
		dev_err(&client->dev, "version read failed");
		goto free_reset_gpio;
	}
	printk("[FT5x36] Firmware version = 0x%x\n", uc_reg_value);
       fts_ctpm_auto_upgrade(client); 
	err = request_threaded_irq(client->irq, NULL, fts_ts_interrupt,
				   pdata->irqflags, client->dev.driver->name,
				   fts_ts);
	if (err < 0) {
		dev_err(&client->dev, "fts_probe: request irq failed\n");
		goto free_reset_gpio;
	}
	disable_irq(client->irq);

#if defined(CONFIG_FB)
	fts_ts->fb_notif.notifier_call = fb_notifier_callback;

	err = fb_register_client(&fts_ts->fb_notif);

	if (err)
		dev_err(&client->dev, "Unable to register fb_notifier: %d\n",
			err);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	fts_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +
						    FT_SUSPEND_LEVEL;
	fts_ts->early_suspend.suspend = ft5x36_ts_early_suspend;
	fts_ts->early_suspend.resume = ft5x36_ts_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

    /* Added by yanwenlong for hardware_info. (general) 2013.8.29 begin */
    #if defined(CONFIG_GET_HARDWARE_INFO)
    strcpy(tmp_panel_name, FTS_NAME);
    register_hardware_info(CTP, tmp_panel_name);
    #endif
    /* Added by yanwenlong for hardware_info. (general) 2013.8.29 end */

#ifdef FTS_SYSFS_DEBUG
	fts_create_sysfs(client);
  #ifdef FTS_APK_DEBUG
	fts_create_apk_debug_channel(client);
  #endif
#endif

#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
		dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
				__func__);
#endif
	enable_irq(client->irq);
	return 0;
	
free_reset_gpio:
	if (gpio_is_valid(pdata->reset_gpio))
		gpio_free(pdata->reset_gpio);
free_irq_gpio:
	if (gpio_is_valid(pdata->irq_gpio))
		gpio_free(pdata->irq_gpio);
pwr_off:
	if (pdata->power_on)
		pdata->power_on(false);
	else
		ft5x36_power_on(fts_ts, false);
pwr_deinit:
	if (pdata->power_init)
		pdata->power_init(false);
	else
		ft5x36_power_init(fts_ts, false);
unreg_inputdev:
	input_unregister_device(input_dev);
	input_dev = NULL;

free_inputdev:
	input_free_device(input_dev);
	
free_mem:
	kfree(fts_ts);
	return err;
}
static int __devexit fts_ts_remove(struct i2c_client *client)
{
	struct fts_ts_data *fts_ts;
	fts_ts = i2c_get_clientdata(client);
	input_unregister_device(fts_ts->input_dev);
	#ifdef CONFIG_PM
	gpio_free(fts_ts->pdata->reset_gpio);
	#endif
#ifdef FTS_SYSFS_DEBUG
  #ifdef FTS_APK_DEBUG
	fts_release_apk_debug_channel();
  #endif
	fts_release_sysfs(client);
#endif
	#ifdef FTS_CTL_IIC
	ft_rw_iic_drv_exit();
	#endif
	free_irq(client->irq, fts_ts);
	kfree(fts_ts);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id fts_ts_id[] = {
	{FTS_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, fts_ts_id);

#ifdef CONFIG_OF
static struct of_device_id ft5x36_match_table[] = {
	{ .compatible = "focaltech,FT5x36",},
	{ },
};
#else
#define ft5x36_match_table NULL
#endif


static struct i2c_driver fts_ts_driver = {
	.probe 		= fts_ts_probe,
	.remove 	= __devexit_p(fts_ts_remove),
	.id_table 	= fts_ts_id,
	.driver = {
		   .name = FTS_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = ft5x36_match_table,
#ifdef CONFIG_PM
		   .pm = &ft5x36_ts_pm_ops,
#endif
		   },
};

static int __init fts_ts_init(void)
{
	int ret;
	ret = i2c_add_driver(&fts_ts_driver);
	if (ret) {
		printk(KERN_WARNING "Adding FTS driver failed " "(errno = %d)\n", ret);
	} else {
		pr_info("Successfully added driver %s\n",fts_ts_driver.driver.name);
	}
	return ret;
}

static void __exit fts_ts_exit(void)
{
	i2c_del_driver(&fts_ts_driver);
}

module_init(fts_ts_init);
module_exit(fts_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech TouchScreen driver");
MODULE_LICENSE("GPL");
