//#include <linux/spi/spi_gpio.h>
#define DRIVER_NAME     "as_spi_gpio"
#define SPI_MISO_GPIO   ((unsigned long)-1l)
#define SPI_MOSI_GPIO   9
#define SPI_SCK_GPIO    8
#define SPI_N_CHIPSEL   1
#include "../linux-3.10.49/drivers/spi/spi-gpio.c"