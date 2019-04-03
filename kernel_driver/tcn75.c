#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/regmap.h>

#define TCN75_REG_TEMP		0x00
#define TCN75_REG_MAX		0x03

enum tcn75_type {		/* keep sorted in alphabetical order */
	tcn75,
};

static const unsigned short normal_i2c[] = { 0x48, 0x49, 0x4a, 0x4b, 0x4c,
					0x4d, 0x4e, 0x4f, I2C_CLIENT_END };

struct tcn75_data {
	struct i2c_client	*client;
	struct regmap		*regmap;
};

static long tcn75_convertTemp(unsigned int val)
{
	unsigned char aa, bb;
	long temp;
	
	aa = (unsigned char)((val >> 8)&0xFF);
	bb = (unsigned char)(val & 0xFF);

	if( aa > 127 )
	{
		temp = ( (aa-128) * -1) * 10;
		if( bb == 128 )
		{
			temp -= 5;
		}
	} else
	{
		temp = aa * 10;
		if( bb == 128 )
		{
			temp += 5;
		}
	}

	return temp;
}

static int tcn75_read(struct device *dev, enum hwmon_sensor_types type,
		     u32 attr, int channel, long *val)
{
	struct tcn75_data *data = dev_get_drvdata(dev);
	int err;
	int reg;
	unsigned int regval;

	switch( type )
	{
		case hwmon_chip:
			switch( attr )
			{
				case hwmon_chip_update_interval:
					// TODO...
					break;
				default:
					break;
			}
			break;

		case hwmon_temp:
			switch( attr )
			{
				case hwmon_temp_input:
					reg = TCN75_REG_TEMP;
					break;
				case hwmon_temp_max:
				case hwmon_temp_max_hyst:
				default:
					return -EINVAL;
			}
			err = regmap_read(data->regmap, reg, &regval);
			if( err < 0 )
				return err;
			*val = tcn75_convertTemp(regval);
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

static int tcn75_write(struct device *dev, enum hwmon_sensor_types type,
	u32 attr, int channel, long temp)
{
	printk("tcn75: %s\n", __func__);

	// TODO...
	return 0;
}

static umode_t tcn75_is_visible(const void *data, enum hwmon_sensor_types type,
	u32 attr, int channel)
{
	printk("tcn75: %s\n", __func__);

	switch( type )
	{
		case hwmon_chip:
			switch (attr) {
				case hwmon_chip_update_interval:
					return S_IRUGO;
			}
			break;
		case hwmon_temp:
			switch (attr) {
				case hwmon_temp_input:
					return S_IRUGO;
				case hwmon_temp_max:
				case hwmon_temp_max_hyst:
					return S_IRUGO | S_IWUSR;
			}
			break;
		default:
			break;
	}

	return 0;
}

/* ------------------------------------------------------------------- */
/* chip donfiguration */
static const u32 tcn75_chip_config[] = {
	HWMON_C_REGISTER_TZ | HWMON_C_UPDATE_INTERVAL,
	0
};

static const struct hwmon_channel_info tcn75_chip = {
	.type = hwmon_chip,
	.config = tcn75_chip_config,
};

static const u32 tcn75_temp_config[] = {
	HWMON_T_INPUT | HWMON_T_MAX | HWMON_T_MAX_HYST,
	0
};

static const struct hwmon_channel_info tcn75_temp = {
	.type = hwmon_temp,
	.config = tcn75_temp_config,
};

static const struct hwmon_channel_info *tcn75_info[] = {
	&tcn75_chip,
	&tcn75_temp,
	NULL
};

static const struct hwmon_ops tcn75_hwmon_ops = {
	.is_visible = tcn75_is_visible,
	.read = tcn75_read,
	.write = tcn75_write
};

static const struct hwmon_chip_info tcn75_chip_info = {
	.ops = &tcn75_hwmon_ops,
	.info = tcn75_info
};

static const struct regmap_config tcn75_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,
	.max_register = TCN75_REG_MAX,
	//TODO...
};

static void tcn75_remove(void *data)
{
	printk("tcn75: %s\n", __func__);
	// TODO...
}

static int
tcn75_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct device *hwmon_dev;
	struct tcn75_data *data;
	int err;

	printk("tcn75: %s, %d\n", __func__, __LINE__);

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	data = devm_kzalloc(dev, sizeof(struct tcn75_data), GFP_KERNEL);
	if( !data )
	{
		return ENOMEM;
	}

	data->client = client;

	data->regmap = devm_regmap_init_i2c(client, &tcn75_regmap_config);
	if( IS_ERR(data->regmap) )
		return PTR_ERR(data->regmap);

	err = devm_add_action_or_reset(dev, tcn75_remove, data);
	if( err )
		return err;

	printk("tcn75: %s, %d\n", __func__, __LINE__);
	hwmon_dev = devm_hwmon_device_register_with_info(dev, client->name,
							 data, &tcn75_chip_info,
							 NULL);
	printk("tcn75: %s, %d\n", __func__, __LINE__);
	if (IS_ERR(hwmon_dev))
		return PTR_ERR(hwmon_dev);

	dev_info(dev, "%s: sensor '%s'\n", dev_name(hwmon_dev), client->name);

	printk("tcn75: %s, %d\n", __func__, __LINE__);

	return 0;
}

static const struct i2c_device_id tcn75_ids[] = {
	{ "tcn75", tcn75, },
	{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, tcn75_ids);

static const struct of_device_id tcn75_of_match[] = {
	{
		.compatible = "microchip,tcn75",
		.data = (void *)tcn75
	},
	{ },
};
MODULE_DEVICE_TABLE(of, lm75_of_match);

static int
tcn75_detect(struct i2c_client *new_client,
           struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = new_client->adapter;

	printk("tcn75: %s, %d\n", __func__, __LINE__);

	if( !i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA ||
	    I2C_FUNC_SMBUS_WORD_DATA))
	{
		return -ENODEV;
	}

	// TODO...

	printk("tcn75: %s, %d\n", __func__, __LINE__);
	return 0;
}

#ifdef CONFIG_PM
static int 
tcn75_suspend(struct device *dev)
{
	printk("tcn75: %s, %d\n", __func__, __LINE__);
	// TODO...
	return 0;
}

static int 
tcn75_resume(struct device *dev)
{
	printk("tcn75: %s, %d\n", __func__, __LINE__);
	// TODO...
	return 0;
}

static const struct dev_pm_ops tcn75_dev_pm_ops = {
	.suspend	= tcn75_suspend,
	.resume		= tcn75_resume,
};
#define TCN75_DEV_PM_OPS (&tcn75_dev_pm_ops)
#else
#define LM75_DEV_PM_OPS NULL
#endif /* end of CONFIG_PM */

static struct i2c_driver tcn75_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver		= {
		.name	= "tcn75",
		.of_match_table = of_match_ptr(tcn75_of_match),
		.pm	= TCN75_DEV_PM_OPS,
	},
	.probe		= tcn75_probe,
	.id_table	= tcn75_ids,
	.detect		= tcn75_detect,
	.address_list	= normal_i2c,
};

module_i2c_driver(tcn75_driver);

MODULE_AUTHOR("Patrick Lin <patrick.lin@microchip.com>");
MODULE_DESCRIPTION("TCN75 driver");
MODULE_LICENSE("GPL");

/* end of file */

