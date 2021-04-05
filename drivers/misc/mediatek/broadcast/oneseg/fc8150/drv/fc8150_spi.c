/*****************************************************************************
 Copyright(c) 2012 FCI Inc. All Rights Reserved

 File name : fc8150_spi.c

 Description : fc8150 host interface

*******************************************************************************/
#include <linux/spi/spi.h>
#include <linux/slab.h>

#include "fci_types.h"
#include "fc8150_regs.h"
#include "fci_oal.h"

#include <linux/of_gpio.h>
#if !defined(CONFIG_ARCH_MT6582)    // MediaTek platform
#include <linux/gpio.h>
#else
#include <mach/mt_spi.h>
#endif //defined(CONFIG_ARCH_MT6582) // MediaTek platform
#include <linux/module.h>

#define SPI_BMODE           0x00
#define SPI_WMODE           0x10
#define SPI_LMODE           0x20
#define SPI_READ            0x40
#define SPI_WRITE           0x00
#define SPI_AINC            0x80
#define CHIPID              (0 << 3)

#define DRIVER_NAME "isdbt"

//#define QUP_GSBI_SPI_USE_DMOV

struct spi_device *fc8150_spi = NULL;

static u8 tx_data[10];

#ifdef QUP_GSBI_SPI_USE_DMOV
#define TX_RX_DATA_BUF_SIZE (8192)

static u8 *wdata_buf;
static u8 *rdata_buf;
static u8 wx_data_buf[TX_RX_DATA_BUF_SIZE+32] ={0,};
static u8 rx_data_buf[TX_RX_DATA_BUF_SIZE+32] = {0,};
#else
static u8 rdata_buf[8192]={0};
static u8 wdata_buf[8192]={0};
#endif

#define BUFFER_SIZE 4<<10
struct spi_message spi_msg;

struct spi_transfer spi_xfer;
u8*     tx_buf; //This needs to be DMA friendly buffer

static DEFINE_MUTEX(lock);

static int spi_test_transfer(struct spi_device *spi)
{
    spi_message_init(&spi_msg);
//    spi_xfer.tx_buf = tx_buf;
    spi_xfer.tx_buf =&wdata_buf[0];
    spi_xfer.len = BUFFER_SIZE;
    spi_xfer.bits_per_word = 8;
    spi_xfer.speed_hz = spi->max_speed_hz;

    spi_message_add_tail(&spi_xfer, &spi_msg);
    return spi_sync(spi, &spi_msg);
}

static int __init fc8150_spi_probe(struct spi_device *spi)
{
    s32 ret;
    int irq_gpio = -1;
    int irq;
    int cs;
    int cpha,cpol,cs_high;
    u32 max_speed;


    PRINTF(0, "fc8150_spi_probe\n");
    dev_err(&spi->dev, "[1seg]%s\n", __func__);    //tae00k.kang

   #if defined(CONFIG_ARCH_MT6582)  // MediaTek platform
   struct mt_chip_conf *mtk_spi_config = NULL;

    mtk_spi_config = ( struct mt_chip_conf * ) spi->controller_data;
    if(mtk_spi_config)
    {
        mtk_spi_config->setuptime = 3;
        mtk_spi_config->holdtime = 3;
        mtk_spi_config->high_time = 10;
        mtk_spi_config->low_time = 10;
        mtk_spi_config->cs_idletime = 2;
        mtk_spi_config->ulthgh_thrsh = 0;
        mtk_spi_config->cpol = 0;
        mtk_spi_config->cpha = 0;
        mtk_spi_config->rx_mlsb = 1;
        mtk_spi_config->tx_mlsb = 1;
        mtk_spi_config->tx_endian = 0;
        mtk_spi_config->rx_endian = 0;
        mtk_spi_config->com_mod = DMA_TRANSFER,
        mtk_spi_config->pause = 0;
        mtk_spi_config->finish_intr = 1;
        mtk_spi_config->deassert = 0;
        mtk_spi_config->ulthigh = 0;
        mtk_spi_config->tckdly = 0;
    }
    #endif

    //Once you have a spi_device structure you can do a transfer anytime
    spi->bits_per_word = 8;
    spi->mode =  SPI_MODE_0;
/*

    //spi->max_speed_hz =  10800000; //kernel\arch\arm\mach-msm\clock-8960.c -> clk_tbl_gsbi_qup
    //spi->max_speed_hz =  24000000;
    //spi->max_speed_hz =  8000000;
    spi->bits_per_word = 8;
    spi->mode =  SPI_MODE_0;
*/
    spi->max_speed_hz =  24000000;
    ret = 1;
    ret = spi_setup(spi);

    if (ret < 0)
        return ret;

    fc8150_spi = spi;

    return ret;
}

static int fc8150_spi_remove(struct spi_device *spi)
{

    return 0;
}

struct spi_device_id spi_dtv_id_table = {"dtv_spi", 0};

static struct spi_driver fc8150_spi_driver = {
	.driver = {
		.name = "dtv_spi",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe = fc8150_spi_probe,
	.remove= __exit_p(fc8150_spi_remove),
	.id_table = &spi_dtv_id_table,
};

static int fc8150_spi_write_then_read(struct spi_device *spi, u8 *txbuf, u16 tx_length, u8 *rxbuf, u16 rx_length)
{
    int res = 0;

    struct spi_message    message;
    struct spi_transfer    x;

    spi_message_init(&message);
    memset(&x, 0, sizeof x);

    spi_message_add_tail(&x, &message);

    memcpy(&wdata_buf[0], txbuf, tx_length);

    x.tx_buf=&wdata_buf[0];
    x.rx_buf=&rdata_buf[0];
    x.len = tx_length + rx_length;
    x.cs_change = 0; // 0 : CS is alternate per transfer, 1 : CS is alternate per 8/16/24/32 bit which is setted in spi_setup
    x.bits_per_word = 8;
    res = spi_sync(spi, &message);
    //PRINTF(0, "fc8150_spi_write_then_read x.len : %d res: %d\n", x.len, res);

    memcpy(rxbuf, x.rx_buf + tx_length, rx_length);

    return res;
}

static int spi_bulkread(HANDLE hDevice, u16 addr, u8 command, u8 *data, u16 length)
{
    int res;

    tx_data[0] = addr & 0xff;
    tx_data[1] = (addr >> 8) & 0xff;
    tx_data[2] = (command & 0xf0) | CHIPID | ((length >> 16) & 0x07);
    tx_data[3] = (length >> 8) & 0xff;
    tx_data[4] = length & 0xff;

    res = fc8150_spi_write_then_read(fc8150_spi, &tx_data[0], 5, data, length);

    if(res)
    {
        PRINTF(0, "fc8150_spi_bulkread fail : %d\n", res);
        return BBM_NOK;
    }

    return BBM_OK;
}

static int spi_bulkwrite(HANDLE hDevice, u16 addr, u8 command, u8* data, u16 length)
{
    int i;
    int res;

    tx_data[0] = addr & 0xff;
    tx_data[1] = (addr >> 8) & 0xff;
    tx_data[2] = (command & 0xf0) | CHIPID | ((length >> 16) & 0x07);
    tx_data[3] = (length >> 8) & 0xff;
    tx_data[4] = length & 0xff;

    for(i=0;i<length;i++)
    {
        tx_data[5+i] = data[i];
    }

    res = fc8150_spi_write_then_read(fc8150_spi, &tx_data[0], length+5, NULL, 0);

    if(res)
    {
        PRINTF(0, "fc8150_spi_bulkwrite fail : %d\n", res);
        return BBM_NOK;
    }

    return BBM_OK;
}

static int spi_dataread(HANDLE hDevice, u16 addr, u8 command, u8* data, u32 length)
{
    int res;

    tx_data[0] = addr & 0xff;
    tx_data[1] = (addr >> 8) & 0xff;
    tx_data[2] = (command & 0xf0) | CHIPID | ((length >> 16) & 0x07);
    tx_data[3] = (length >> 8) & 0xff;
    tx_data[4] = length & 0xff;

    res = fc8150_spi_write_then_read(fc8150_spi, &tx_data[0], 5, data, length);

    if(res)
    {
        PRINTF(0, "fc8150_spi_dataread fail : %d\n", res);
        return BBM_NOK;
    }

    return BBM_OK;
}

int fc8150_spi_init(HANDLE hDevice, u16 param1, u16 param2)
{
    int res = 0;

    PRINTF(0, "fc8150_spi_init : %d\n", res);

    res = spi_register_driver(&fc8150_spi_driver);

    if(res)
    {
        PRINTF(0, "fc8150_spi register fail : %d\n", res);
        return BBM_NOK;
    }

#ifdef QUP_GSBI_SPI_USE_DMOV
    wdata_buf = (u8*)(((u32)wx_data_buf +31)&~31);
    rdata_buf = (u8*)(((u32)rx_data_buf +31)&~31);
#endif

    return res;
}

int fc8150_spi_byteread(HANDLE hDevice, u16 addr, u8 *data)
{
    int res;
    u8 command = SPI_READ;

    mutex_lock(&lock);
    res = spi_bulkread(hDevice, addr, command, data, 1);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_wordread(HANDLE hDevice, u16 addr, u16 *data)
{
    int res;
    u8 command = SPI_READ | SPI_AINC;

    mutex_lock(&lock);
    res = spi_bulkread(hDevice, addr, command, (u8*)data, 2);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_longread(HANDLE hDevice, u16 addr, u32 *data)
{
    int res;
    u8 command = SPI_READ | SPI_AINC;

    mutex_lock(&lock);
    res = spi_bulkread(hDevice, addr, command, (u8*)data, 4);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_bulkread(HANDLE hDevice, u16 addr, u8 *data, u16 length)
{
    int res;
    u8 command = SPI_READ | SPI_AINC;

    mutex_lock(&lock);
    res = spi_bulkread(hDevice, addr, command, data, length);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_bytewrite(HANDLE hDevice, u16 addr, u8 data)
{
    int res;
    u8 command = SPI_WRITE;

    mutex_lock(&lock);
    res = spi_bulkwrite(hDevice, addr, command, (u8*)&data, 1);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_wordwrite(HANDLE hDevice, u16 addr, u16 data)
{
    int res;
    u8 command = SPI_WRITE | SPI_AINC;

    mutex_lock(&lock);
    res = spi_bulkwrite(hDevice, addr, command, (u8*)&data, 2);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_longwrite(HANDLE hDevice, u16 addr, u32 data)
{
    int res;
    u8 command = SPI_WRITE | SPI_AINC;

    mutex_lock(&lock);
    res = spi_bulkwrite(hDevice, addr, command, (u8*)&data, 4);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_bulkwrite(HANDLE hDevice, u16 addr, u8* data, u16 length)
{
    int res;
    u8 command = SPI_WRITE | SPI_AINC;

    mutex_lock(&lock);
    res = spi_bulkwrite(hDevice, addr, command, data, length);
    mutex_unlock(&lock);

    return res;
}

int fc8150_spi_dataread(HANDLE hDevice, u16 addr, u8* data, u32 length)
{
    int res = BBM_OK;
    u8 command = SPI_READ;
#if defined(CONFIG_ARCH_MT6582)    // MediaTek platform
    u32 rx_len = 0;
    u32 remain = length;


    mutex_lock(&lock);
    if ((length + 5) > 1024) {
        rx_len = 1024 * (length / 1024) - 5;
        res |= spi_dataread(hDevice, addr, command, data, rx_len);
        remain = length - rx_len;
        if ((remain + 5) > 1024) {
            res |= spi_dataread(hDevice, addr, command, &data[rx_len], 1024 - 5);
            remain -= (1024 - 5);
            rx_len += (1024 - 5);
        }
    }
    res |= spi_dataread(hDevice, addr, command, &data[rx_len], remain);
    PRINTF(NULL, "Spi Data Read!@!@ rx_len : %d, remain : %d \n", rx_len, remain);
    mutex_unlock(&lock);
#else
    mutex_lock(&lock);
    res = spi_dataread(hDevice, addr, command, data, length);
    mutex_unlock(&lock);
#endif
    return res;
}

int fc8150_spi_deinit(HANDLE hDevice)
{
    //spi_unregister_driver(&fc8150_spi_driver);
    return BBM_OK;
}