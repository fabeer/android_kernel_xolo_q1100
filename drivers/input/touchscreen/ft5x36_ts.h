#ifndef __FOCALTECH_TS_H__
#define __FOCALTECH_TS_H__

/* -- dirver configure -- */
#define PRESS_MAX					0xFF
#define FT_PRESS					0x7F

#define FT_MAX_ID					0x0F
#define FT_TOUCH_STEP				6
#define FT_TOUCH_X_H_POS			3
#define FT_TOUCH_X_L_POS			4
#define FT_TOUCH_Y_H_POS			5
#define FT_TOUCH_Y_L_POS			6
#define FT_TOUCH_EVENT_POS			3
#define FT_TOUCH_ID_POS				5

#define FT_COORDS_ARR_SIZE	4
#define MAX_BUTTONS		4
#define FT_FW_NAME_MAX_LEN	50

#define FT_VTG_MIN_UV		2600000
#define FT_VTG_MAX_UV		3300000
#define FT_I2C_VTG_MIN_UV	1800000
#define FT_I2C_VTG_MAX_UV	1800000

#define FT_STARTUP_DLY		250
#define FT_RESET_DLY		20


int fts_i2c_Read(struct i2c_client *client, char *writebuf, int writelen, char *readbuf, int readlen);
int fts_i2c_Write(struct i2c_client *client, char *writebuf, int writelen);

/* The platform data for the Focaltech touchscreen driver */
struct fts_platform_data {
	u32 irqflags;
	u32 irq_gpio;
	u32 irq_gpio_flags;
	u32 reset_gpio;
	u32 reset_gpio_flags;
	u32 family_id;
	u32 x_max;
	u32 y_max;
	u32 x_min;
	u32 y_min;
	u32 panel_minx;
	u32 panel_miny;
	u32 panel_maxx;
	u32 panel_maxy;
	bool no_force_update;
	bool i2c_pull_up;
	int (*power_init) (bool);
	int (*power_on) (bool);
};
/******************************************************************************/
/*Chip Device Type*/
#define IC_FT5X06						0	/*x=2,3,4*/
#define IC_FT5606						1	/*ft5506/FT5606/FT5816*/
#define IC_FT5316						2	/*ft5x16*/
#define IC_FT6208						3  	/*ft6208*/
#define IC_FT6x06     					4	/*ft6206/FT6306*/
#define IC_FT5x06i     					5	/*ft5306i*/
#define IC_FT5x36     					6	/*ft5336/ft5436/FT5436i*/

#define DEVICE_IC_TYPE					IC_FT5x36

/*register address*/
#define FTS_REG_FW_VER					0xA6
#define FTS_REG_VENDOR_ID				0xA8
#define FTS_REG_POINT_RATE	                     0x88
#define FTS_READ_ID_REG		0x90
#define FTS_REG_PMODE		0xA5
#define FTS_PMODE_HIBERNATE	0x03



#if ( DEVICE_IC_TYPE == IC_FT5X06 )
  #define FTS_NAME						"FT5x06"
  #define CFG_MAX_TOUCH_POINTS 				5
  #define AUTO_CLB
#elif ( DEVICE_IC_TYPE == IC_FT5606 )
  #define FTS_NAME						"FT5606"
  #define CFG_MAX_TOUCH_POINTS 				5//10
  #define AUTO_CLB
#elif ( DEVICE_IC_TYPE == IC_FT5316 )
  #define FTS_NAME						"FT5316"
  #define CFG_MAX_TOUCH_POINTS 				5
  #define AUTO_CLB
#elif ( DEVICE_IC_TYPE == IC_FT6208 )
  #define FTS_NAME						"FT6208"
  #define CFG_MAX_TOUCH_POINTS 				2
  //#define AUTO_CLB
#elif ( DEVICE_IC_TYPE == IC_FT6x06 )
  #define FTS_NAME						"FT6x06"
  #define CFG_MAX_TOUCH_POINTS 				2
  //#define AUTO_CLB
#elif ( DEVICE_IC_TYPE == IC_FT5x06i )
  #define FTS_NAME						"FT5x06i"
  #define CFG_MAX_TOUCH_POINTS 				5
  #define    AUTO_CLB
#elif ( DEVICE_IC_TYPE == IC_FT5x36 )
  #define FTS_NAME						"FT5x36"
  #define CFG_MAX_TOUCH_POINTS 				5
  #define AUTO_CLB
#endif
#define POINT_READ_BUF					(3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)

//#define FTS_CTL_IIC
#define FTS_SYSFS_DEBUG
//#define FTS_APK_DEBUG
#define FTS_AUTO_UPDATE

#ifdef FTS_SYSFS_DEBUG
//#define FTS_APK_DEBUG
#define TPD_AUTO_UPGRADE				// if need upgrade CTP FW when POWER ON,pls enable this MACRO
#endif

#define FTS_DBG
#ifdef FTS_DBG
#define DBG(fmt, args...) 				printk("[FTS]" fmt, ## args)
#else
#define DBG(fmt, args...) 				do{}while(0)
#endif


#endif
