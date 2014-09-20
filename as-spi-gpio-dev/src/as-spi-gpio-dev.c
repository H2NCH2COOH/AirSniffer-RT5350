#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>

#define DRV_NAME	"as-spi-gpio-dev"
#define DRV_DESC	"AirSniffer GPIO-based SPI device"
#define DRV_VERSION	"0.1"

#define PFX		DRV_NAME ": "

#define DEVID		25
#define SPI_MODE	0
#define MAX_FREQ	1000000

static struct platform_device *device=NULL;

static void as_spi_gpio_dev_cleanup(void)
{
	(device)? platform_device_unregister(device):NULL;
}

static int __init as_spi_gpio_dev_probe(void)
{
	struct platform_device *pdev;
	int err;
	struct spi_master *master;
	struct spi_device *slave;
	struct spi_board_info slave_info;

	pdev=platform_device_alloc("as_spi_gpio",DEVID);
	if(!pdev)
	{
		err=-ENOMEM;
		goto err;
	}

	err=platform_device_add(pdev);
	if(err)
	{
		printk(KERN_ERR PFX "platform_device_add failed with return code %d\n",err);
		platform_device_put(pdev);
		goto err;
	}

	memset(&slave_info,0,sizeof(slave_info));
	strcpy(slave_info.modalias,"spidev");
	slave_info.controller_data=(void*)SPI_GPIO_NO_CHIPSELECT;
	slave_info.max_speed_hz=MAX_FREQ;
	slave_info.chip_select=0;
	slave_info.mode=SPI_MODE;

	master=spi_busnum_to_master(DEVID);
	if(!master)
	{
		printk(KERN_ERR PFX "unable to get master for bus %d\n",DEVID);
		err=-EINVAL;
		goto err_unregister;
	}

	slave=spi_new_device(master,&slave_info);
	spi_master_put(master);
	if(!slave)
	{
		printk(KERN_ERR PFX "unable to create slave %d for bus %d\n",0,DEVID);
		/* Will most likely fail due to unsupported mode bits */
		err=-EINVAL;
		goto err_unregister;
	}

	device=pdev;

	return 0;

err_unregister:
	platform_device_unregister(pdev);
err:
	as_spi_gpio_dev_cleanup();
	return err;
}

#ifdef MODULE
static int __init as_spi_gpio_dev_init(void)
{
	return as_spi_gpio_dev_probe();
}
module_init(as_spi_gpio_dev_init);

static void __exit as_spi_gpio_dev_exit(void)
{
	as_spi_gpio_dev_cleanup();
}
module_exit(as_spi_gpio_dev_exit);
#else
subsys_initcall(spi_gpio_custom_probe);
#endif /* MODULE */

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("WZY <wzypublic@gmail.com>");
MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
