/*
 * Copyright (C) 2012-2014 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/i2c.h>
#include <linux/i2c/pca953x.h>
#include <linux/ata.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/regulator/consumer.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include <linux/ipu.h>
#include <linux/mxcfb.h>
#include <linux/pwm_backlight.h>
#include <linux/fec.h>
#include <linux/memblock.h>
#include <linux/gpio.h>
#include <linux/ion.h>
#include <linux/etherdevice.h>
#include <linux/power/sabresd_battery.h>
#include <linux/regulator/anatop-regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/max17135.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/mfd/wm8994/gpio.h>
#ifdef CONFIG_SND_SOC_WM8962
#include <sound/wm8962.h>
#endif
#ifdef CONFIG_SND_SOC_WM8960
#include <sound/wm8960.h>
#endif
#include <linux/mfd/mxc-hdmi-core.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/mxc_dvfs.h>
#include <mach/memory.h>
#include <mach/iomux-mx6q.h>
#include <mach/imx-uart.h>
#include <mach/viv_gpu.h>
#include <mach/ahci_sata.h>
#include <mach/ipu-v3.h>
#include <mach/mxc_hdmi.h>
#include <mach/mxc_asrc.h>
#include <mach/mipi_dsi.h>
#include <mach/mxc_ir.h>

#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include "usb.h"
#include "devices-imx6q.h"
#include "crm_regs.h"
#include "cpu_op-mx6.h"
#include "board-mx6q_topeet.h"
#include "board-mx6dl_topeet.h"
#include <mach/imx_rfkill.h>

#define TOPEET_USR_DEF_GRN_LED	IMX_GPIO_NR(1, 1)
#define TOPEET_BT_RESET	IMX_GPIO_NR(1, 2)
#define TOPEET_USR_DEF_RED_LED	IMX_GPIO_NR(1, 2)
#define TOPEET_VOLUME_UP	IMX_GPIO_NR(7, 13)//IMX_GPIO_NR(1, 9)
#define TOPEET_VOLUME_DN	IMX_GPIO_NR(1, 9)//IMX_GPIO_NR(7, 13)
#define TOPEET_MICROPHONE_DET	IMX_GPIO_NR(3, 31)
#define TOPEET_CSI0_PWN	IMX_GPIO_NR(1, 16)
#define TOPEET_CSI0_RST	IMX_GPIO_NR(1, 17)
//#define TOPEET_ACCL_INT	IMX_GPIO_NR(1, 18)
#define TOPEET_MIPICSI_PWN	IMX_GPIO_NR(1, 19)
#define TOPEET_MIPICSI_RST	IMX_GPIO_NR(1, 20)
#define TOPEET_RGMII_RST	IMX_GPIO_NR(1, 25)
#define TOPEET_RGMII_INT	IMX_GPIO_NR(1, 26)
#define TOPEET_CHARGE_UOK_B	IMX_GPIO_NR(1, 27)
#define TOPEET_USBH1_PWR_EN	IMX_GPIO_NR(1, 29)
#define TOPEET_DISP0_PWR_EN	IMX_GPIO_NR(1, 30)

/* add by cym 20170228 */
#define HS0038_GPIO_IRQ         IMX_GPIO_NR(7, 12)
/* end add */

#define TOPEET_SD3_CD		IMX_GPIO_NR(2, 0)
#define TOPEET_SD3_WP		IMX_GPIO_NR(2, 1)
#define TOPEET_SD2_CD		IMX_GPIO_NR(1, 4)//cym IMX_GPIO_NR(2, 2)
#define TOPEET_SD2_WP		IMX_GPIO_NR(2, 3)
#define TOPEET_CHARGE_DOK_B	IMX_GPIO_NR(2, 24)
#define TOPEET_GPS_RESET	IMX_GPIO_NR(2, 28)
#define TOPEET_SENSOR_EN	IMX_GPIO_NR(2, 31)

#define TOPEET_GPS_EN	IMX_GPIO_NR(3, 0)
#define TOPEET_DISP0_RST_B	IMX_GPIO_NR(3, 8)
#define TOPEET_ALS_INT		IMX_GPIO_NR(3, 9)
#define TOPEET_CHARGE_CHG_2_B	IMX_GPIO_NR(3, 13)
#define TOPEET_CHARGE_FLT_2_B	IMX_GPIO_NR(3, 14)
#define TOPEET_BAR0_INT	IMX_GPIO_NR(3, 15)
#define TOPEET_eCOMPASS_INT	IMX_GPIO_NR(3, 16)
#define TOPEET_GPS_PPS		IMX_GPIO_NR(3, 18)
#define TOPEET_ACCL_INT	    IMX_GPIO_NR(3, 19) //caesar.
#define TOPEET_PCIE_PWR_EN	IMX_GPIO_NR(3, 17)
#define TOPEET_USB_OTG_PWR	IMX_GPIO_NR(3, 22)
#define TOPEET_USB_H1_PWR	IMX_GPIO_NR(1, 29)
#define TOPEET_CHARGE_CHG_1_B	IMX_GPIO_NR(3, 2)//cym IMX_GPIO_NR(3, 23)
#define TOPEET_TS_INT		IMX_GPIO_NR(3, 26)
#define TOPEET_DISP0_RD	IMX_GPIO_NR(3, 28)
#define TOPEET_POWER_OFF	IMX_GPIO_NR(2, 2)//IMX_GPIO_NR(3, 29)
#define TOPEET_BACK        	IMX_GPIO_NR(6, 9)

#define TOPEET_CAN1_STBY	IMX_GPIO_NR(4, 5)
#define TOPEET_CAN2_STBY	IMX_GPIO_NR(3, 1)
#define TOPEET_ECSPI1_CS0  IMX_GPIO_NR(4, 9)
#define TOPEET_CODEC_PWR_EN	IMX_GPIO_NR(4, 10)
#define TOPEET_HDMI_CEC_IN	IMX_GPIO_NR(4, 11)
#define TOPEET_PCIE_DIS_B	IMX_GPIO_NR(4, 14)

/* add by cym 20170809 */
#define TOPEET_ECSPI2_CS0  IMX_GPIO_NR(2, 27)
/* end add */

#define TOPEET_DI0_D0_CS	IMX_GPIO_NR(5, 0)
#define TOPEET_CHARGE_FLT_1_B	IMX_GPIO_NR(5, 2)
#define TOPEET_PCIE_WAKE_B	IMX_GPIO_NR(5, 20)

#define TOPEET_CAP_TCH_INT1	IMX_GPIO_NR(6, 7)
#define TOPEET_CAP_TCH_INT0	IMX_GPIO_NR(6, 8)
#define TOPEET_DISP_RST_B	IMX_GPIO_NR(6, 11)
#define TOPEET_DISP_PWR_EN	IMX_GPIO_NR(6, 14)
#define TOPEET_CABC_EN0	IMX_GPIO_NR(6, 15)
#define TOPEET_CABC_EN1	IMX_GPIO_NR(6, 16)
#define TOPEET_AUX_3V15_EN	IMX_GPIO_NR(6, 9)
#define TOPEET_DISP0_WR_REVB	IMX_GPIO_NR(6, 9)
#define TOPEET_AUX_5V_EN	IMX_GPIO_NR(6, 10)
#define TOPEET_DI1_D0_CS	IMX_GPIO_NR(6, 31)

#define TOPEET_HEADPHONE_DET	IMX_GPIO_NR(3, 31)//cym IMX_GPIO_NR(7, 8)
#define TOPEET_PCIE_RST_B_REVB	IMX_GPIO_NR(7, 8)
#if 0	//remove by cym 
#define TOPEET_PMIC_INT_B	IMX_GPIO_NR(7, 13)
#define TOPEET_PFUZE_INT	IMX_GPIO_NR(7, 13)
#endif/

#define TOPEET_EPDC_SDDO_0	IMX_GPIO_NR(2, 22)
#define TOPEET_EPDC_SDDO_1	IMX_GPIO_NR(3, 10)
#define TOPEET_EPDC_SDDO_2	IMX_GPIO_NR(3, 12)
#define TOPEET_EPDC_SDDO_3	IMX_GPIO_NR(3, 11)
#define TOPEET_EPDC_SDDO_4	IMX_GPIO_NR(2, 27)
#define TOPEET_EPDC_SDDO_5	IMX_GPIO_NR(2, 30)
#define TOPEET_EPDC_SDDO_6	IMX_GPIO_NR(2, 23)
#define TOPEET_EPDC_SDDO_7	IMX_GPIO_NR(2, 26)
#define TOPEET_EPDC_SDDO_8	IMX_GPIO_NR(2, 24)
#define TOPEET_EPDC_SDDO_9	IMX_GPIO_NR(3, 15)
#define TOPEET_EPDC_SDDO_10	IMX_GPIO_NR(3, 16)
#define TOPEET_EPDC_SDDO_11	IMX_GPIO_NR(3, 23)
#define TOPEET_EPDC_SDDO_12	IMX_GPIO_NR(3, 19)
#define TOPEET_EPDC_SDDO_13	IMX_GPIO_NR(3, 13)
#define TOPEET_EPDC_SDDO_14	IMX_GPIO_NR(3, 14)
#define TOPEET_EPDC_SDDO_15	IMX_GPIO_NR(5, 2)
#define TOPEET_EPDC_GDCLK	IMX_GPIO_NR(2, 17)
#define TOPEET_EPDC_GDSP	IMX_GPIO_NR(2, 16)
#define TOPEET_EPDC_GDOE	IMX_GPIO_NR(6, 6)
#define TOPEET_EPDC_GDRL	IMX_GPIO_NR(5, 4)
#define TOPEET_EPDC_SDCLK	IMX_GPIO_NR(3, 29)//cym IMX_GPIO_NR(3, 31)
#define TOPEET_EPDC_SDOEZ	IMX_GPIO_NR(3, 30)
#define TOPEET_EPDC_SDOED	IMX_GPIO_NR(3, 26)
#define TOPEET_EPDC_SDOE	IMX_GPIO_NR(3, 27)
//#define TOPEET_EPDC_SDLE	IMX_GPIO_NR(3, 1)
#define TOPEET_EPDC_SDCLKN	IMX_GPIO_NR(3, 0)
#define TOPEET_EPDC_SDSHR	IMX_GPIO_NR(2, 29)
#define TOPEET_EPDC_PWRCOM	IMX_GPIO_NR(2, 28)
#define TOPEET_EPDC_PWRSTAT	IMX_GPIO_NR(2, 21)
#define TOPEET_EPDC_PWRCTRL0	IMX_GPIO_NR(2, 20)
#define TOPEET_EPDC_PWRCTRL1	IMX_GPIO_NR(2, 19)
#define TOPEET_EPDC_PWRCTRL2	IMX_GPIO_NR(2, 18)
#define TOPEET_EPDC_PWRCTRL3	IMX_GPIO_NR(3, 28)
#define TOPEET_EPDC_BDR0	IMX_GPIO_NR(3, 2)
#define TOPEET_EPDC_BDR1	IMX_GPIO_NR(3, 3)
#define TOPEET_EPDC_SDCE0	IMX_GPIO_NR(3, 4)
#define TOPEET_EPDC_SDCE1	IMX_GPIO_NR(3, 5)
#define TOPEET_EPDC_SDCE2	IMX_GPIO_NR(3, 6)
#define TOPEET_EPDC_SDCE3	IMX_GPIO_NR(3, 7)
#define TOPEET_EPDC_SDCE4	IMX_GPIO_NR(3, 8)
#define TOPEET_EPDC_PMIC_WAKE	IMX_GPIO_NR(3, 20)
#define TOPEET_EPDC_PMIC_INT	IMX_GPIO_NR(2, 25)
#define TOPEET_EPDC_VCOM	IMX_GPIO_NR(3, 17)
#define TOPEET_CHARGE_NOW	IMX_GPIO_NR(1, 2)
#define TOPEET_CHARGE_DONE	IMX_GPIO_NR(1, 1)
#define TOPEET_ELAN_CE		IMX_GPIO_NR(2, 18)
#define TOPEET_ELAN_RST	IMX_GPIO_NR(3, 8)
#define TOPEET_ELAN_INT	IMX_GPIO_NR(3, 28)

/* add by cym 20160712 */
#define CFG_IO_MT6620_CD_PIN		IMX_GPIO_NR(2, 0)//IMX_GPIO_NR(3, 2)
#define CFG_IO_MT6620_TRIGGER_PIN	IMX_GPIO_NR(3, 23)//cym IMX_GPIO_NR(3, 2)//IMX_GPIO_NR(2, 0)
#define CFG_IO_MT6620_POWER_PIN		IMX_GPIO_NR(6, 18)
#define CFG_IO_MT6620_SYSRST_PIN	IMX_GPIO_NR(6, 17)
#define CFG_IO_MT6620_BGF_INT_PIN	IMX_GPIO_NR(3, 21)//cym IMX_GPIO_NR(3, 31)//cym IMX_GPIO_NR(3, 8)
#define CFG_IO_MT6620_WIFI_INT_PIN	IMX_GPIO_NR(3, 30)//cym IMX_GPIO_NR(3, 10)

/* end add */

#define MX6_ENET_IRQ		IMX_GPIO_NR(1, 6)
#define IOMUX_OBSRV_MUX1_OFFSET	0x3c
#define OBSRV_MUX1_MASK			0x3f
#define OBSRV_MUX1_ENET_IRQ		0x9

static struct clk *sata_clk;
static struct clk *clko;
#ifdef CONFIG_SENSORS_MMA8X5X
static int mma8x5x_position;
#endif
static int mpu6050_position;
static int mag3110_position = 1;
#ifdef CONFIG_TOUCHSCREEN_MAX11801
static int max11801_mode = 1;
#endif
static int caam_enabled;
static int uart5_enabled;

extern char *gp_reg_id;
extern char *soc_reg_id;
extern char *pu_reg_id;
extern int epdc_enabled;
extern bool enet_to_gpio_6;

static int max17135_regulator_init(struct max17135 *max17135);

static const struct esdhc_platform_data mx6q_topeet_sd2_data __initconst = {
        .cd_gpio = TOPEET_SD2_CD,
        //cym .wp_gpio = TOPEET_SD2_WP,
	.keep_power_at_suspend = 1,
	//cym .support_8bit = 1,
	.delay_line = 0,
	.cd_type = ESDHC_CD_CONTROLLER,
	.runtime_pm = 1,
};

static const struct esdhc_platform_data mx6q_topeet_sd3_data __initconst = {
        .cd_gpio = TOPEET_SD3_CD,
        .wp_gpio = TOPEET_SD3_WP,
	.keep_power_at_suspend = 1,
	.support_8bit = 0,//1,
	.delay_line = 0,
	.cd_type = ESDHC_CD_GPIO,//ESDHC_CD_CONTROLLER,
	.runtime_pm = 1,
};

static const struct esdhc_platform_data mx6q_topeet_sd4_data __initconst = {
	.always_present = 1,
	.keep_power_at_suspend = 1,
	.support_8bit = 1,
	.delay_line = 0,
	.cd_type = ESDHC_CD_PERMANENT,
};

static const struct anatop_thermal_platform_data
        mx6q_topeet_anatop_thermal_data __initconst = {
		.name = "anatop_thermal",
};

static const struct imxuart_platform_data mx6q_sd_uart5_data __initconst = {
	.flags      = IMXUART_HAVE_RTSCTS,
	.dma_req_rx = MX6Q_DMA_REQ_UART5_RX,
	.dma_req_tx = MX6Q_DMA_REQ_UART5_TX,
};

static inline void mx6q_topeet_init_uart(void)
{
	imx6q_add_imx_uart(0, NULL);

	imx6q_add_imx_uart(1, NULL);

	imx6q_add_imx_uart(2, NULL);
}

static int mx6q_topeet_fec_phy_init(struct phy_device *phydev)
{
#if 0
	unsigned short val;

	/* Ar8031 phy SmartEEE feature cause link status generates glitch,
	 * which cause ethernet link down/up issue, so disable SmartEEE
	 */
	phy_write(phydev, 0xd, 0x3);
	phy_write(phydev, 0xe, 0x805d);
	phy_write(phydev, 0xd, 0x4003);
	val = phy_read(phydev, 0xe);
	val &= ~(0x1 << 8);
	phy_write(phydev, 0xe, val);

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, 0xd, 0x7);
	phy_write(phydev, 0xe, 0x8016);
	phy_write(phydev, 0xd, 0x4007);
	val = phy_read(phydev, 0xe);

	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, 0xe, val);

	/* Introduce tx clock delay */
	phy_write(phydev, 0x1d, 0x5);
	val = phy_read(phydev, 0x1e);
	val |= 0x0100;
	phy_write(phydev, 0x1e, val);

	/*check phy power*/
	val = phy_read(phydev, 0x0);

	if (val & BMCR_PDOWN)
		phy_write(phydev, 0x0, (val & ~BMCR_PDOWN));
#endif
	return 0;
}

static struct fec_platform_data fec_data __initdata = {
        .init = mx6q_topeet_fec_phy_init,
	.phy = PHY_INTERFACE_MODE_RGMII,
	.gpio_irq = MX6_ENET_IRQ,
};

static int mx6q_topeet_spi_cs[] = {
        TOPEET_ECSPI1_CS0,
};

static const struct spi_imx_master mx6q_topeet_spi_data __initconst = {
        .chipselect     = mx6q_topeet_spi_cs,
        .num_chipselect = ARRAY_SIZE(mx6q_topeet_spi_cs),
};

/* add by cym 20170809 */
static int mx6q_topeet_spi2_cs[] = {
        TOPEET_ECSPI2_CS0,
};

static const struct spi_imx_master mx6q_topeet_spi2_data __initconst = {
        .chipselect     = mx6q_topeet_spi2_cs,
        .num_chipselect = ARRAY_SIZE(mx6q_topeet_spi2_cs),
};
/* end add */

#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
static struct mtd_partition imx6_topeet_spi_nor_partitions[] = {
	{
	 .name = "bootloader",
	 .offset = 0,
	 .size = 0x00100000,
	},
	{
	 .name = "kernel",
	 .offset = MTDPART_OFS_APPEND,
	 .size = MTDPART_SIZ_FULL,
	},
};

static struct flash_platform_data imx6_topeet__spi_flash_data = {
	.name = "m25p80",
        .parts = imx6_topeet_spi_nor_partitions,
        .nr_parts = ARRAY_SIZE(imx6_topeet_spi_nor_partitions),
	.type = "sst25vf016b",
};
#endif

static struct spi_board_info imx6_topeet_spi_nor_device[] __initdata = {
#if defined(CONFIG_MTD_M25P80)
	{
		.modalias = "m25p80",
		.max_speed_hz = 20000000, /* max spi clock (SCK) speed in HZ */
		.bus_num = 0,
		.chip_select = 0,
                .platform_data = &imx6_topeet__spi_flash_data,
	},
#endif
};

/* add by cym 20170723 */
#if defined(CONFIG_SPI_RC522) || defined(CONFIG_SPI_RC522_MODULE)
static struct spi_board_info rc522_plat_board[] __initdata = {
        [0] = {
        .modalias        = "rc522",
        .max_speed_hz = 10000000,//20000000,//4000000, /*4MHZ*/
        .bus_num         = 1,//0,
        //.controller_data = &spi2_info,
        .chip_select     = 0,
    },
};
#endif
/* end add */

static void spi_device_init(void)
{
        spi_register_board_info(imx6_topeet_spi_nor_device,
                                ARRAY_SIZE(imx6_topeet_spi_nor_device));

	/* add by cym 20170723 */
#if defined(CONFIG_SPI_RC522) || defined(CONFIG_SPI_RC522_MODULE)
	spi_register_board_info(rc522_plat_board,
                                ARRAY_SIZE(rc522_plat_board));
#endif
	/* end add */
}

static struct imx_ssi_platform_data mx6_topeet_ssi_pdata = {
	.flags = IMX_SSI_DMA | IMX_SSI_SYN,
};

static struct platform_device mx6_topeet_audio_wm8958_device = {
	.name = "imx-wm8958",
};

static struct mxc_audio_platform_data wm8958_data = {
	.ssi_num = 1,
	.src_port = 2,
	.ext_port = 3,
        .hp_gpio = TOPEET_HEADPHONE_DET,
	.hp_active_low = 1,
};

static struct wm8994_pdata wm8958_config_data = {
	.gpio_defaults = {
		[0] = WM8994_GP_FN_GPIO | WM8994_GPN_DB,
		[1] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[2] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[3] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[4] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[5] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[7] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[8] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[9] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
		[10] = WM8994_GP_FN_GPIO | WM8994_GPN_DB | WM8994_GPN_PD,
	},
};

static int mxc_wm8958_init(void)
{
	struct clk *clko;
	int rate;

	clko = clk_get(NULL, "clko_clk");
	if (IS_ERR(clko)) {
		pr_err("can't get CLKO clock.\n");
		return PTR_ERR(clko);
	}
	/* both audio codec and comera use CLKO clk*/
	rate = clk_round_rate(clko, 24000000);

	wm8958_data.sysclk = rate;
	clk_set_rate(clko, rate);

	/* enable wm8958 4.2v power supply */
        gpio_request(TOPEET_CODEC_PWR_EN, "aud_4v2");
        gpio_direction_output(TOPEET_CODEC_PWR_EN, 1);
	msleep(1);
        gpio_set_value(TOPEET_CODEC_PWR_EN, 1);

	return 0;
}

#ifdef CONFIG_SND_SOC_WM8960
static struct platform_device mx6_topeet_audio_wm8960_device =
{
        .name = "imx-wm8960",
};

static struct mxc_audio_platform_data wm8960_data;

static int wm8960_clk_enable(int enable)
{
        if (enable)
                clk_enable(clko);
        else
                clk_disable(clko);

        return 0;
}

static int mxc_wm8960_init(void)
{
        int rate;

        clko = clk_get(NULL, "clko_clk");
        if (IS_ERR(clko))
        {
                pr_err("can't get CLKO clock.\n");
                return PTR_ERR(clko);
        }
        /* both audio codec and comera use CLKO clk*/
        rate = clk_round_rate(clko, 24000000);
        clk_set_rate(clko, rate);

        wm8960_data.sysclk = rate;

        return 0;
}

static struct wm8960_data wm8960_config_data =
{
        .gpio_init = {
                [2] = WM8960_GPIO_FN_DMICCLK,
                [4] = 0x8000 | WM8960_GPIO_FN_DMICDAT,
        },
};

static struct mxc_audio_platform_data wm8960_data =
{
        .ssi_num = 1,
        .src_port = 2,
        .ext_port = 3,
        .hp_gpio = TOPEET_HEADPHONE_DET,
        .hp_active_low = 1,
        .mic_gpio = TOPEET_MICROPHONE_DET,
        .mic_active_low = 1,
        .init = mxc_wm8960_init,
        .clock_enable = wm8960_clk_enable,
};

static struct regulator_consumer_supply topeet_vwm8960_consumers[] =
{
        REGULATOR_SUPPLY("SPKVDD1", "1-001a"),
        REGULATOR_SUPPLY("SPKVDD2", "1-001a"),
};

static struct regulator_init_data topeet_vwm8960_init =
{
        .constraints = {
                .name = "SPKVDD",
                .valid_ops_mask =  REGULATOR_CHANGE_STATUS,
                .boot_on = 1,
        },
        .num_consumer_supplies = ARRAY_SIZE(topeet_vwm8960_consumers),
        .consumer_supplies = topeet_vwm8960_consumers,
};

static struct fixed_voltage_config topeet_vwm8960_reg_config =
{
        .supply_name    = "SPKVDD",
        .microvolts     = 4200000,
        .gpio           = TOPEET_CODEC_PWR_EN,
        .enable_high    = 1,
        .enabled_at_boot = 1,
        .init_data      = &topeet_vwm8960_init,
};

static struct platform_device topeet_vwm8960_reg_devices =
{
        .name   = "reg-fixed-voltage",
        .id     = 4,
        .dev    = {
                .platform_data = &topeet_vwm8960_reg_config,
        },
};

#endif

#ifdef CONFIG_SND_SOC_WM8962
static struct platform_device mx6_topeet_audio_wm8962_device = {
	.name = "imx-wm8962",
};

static struct mxc_audio_platform_data wm8962_data;

static int wm8962_clk_enable(int enable)
{
	if (enable)
		clk_enable(clko);
	else
		clk_disable(clko);

	return 0;
}

static int mxc_wm8962_init(void)
{
	int rate;

	clko = clk_get(NULL, "clko_clk");
	if (IS_ERR(clko)) {
		pr_err("can't get CLKO clock.\n");
		return PTR_ERR(clko);
	}
	/* both audio codec and comera use CLKO clk*/
	rate = clk_round_rate(clko, 24000000);
	clk_set_rate(clko, rate);

	wm8962_data.sysclk = rate;

	return 0;
}

static struct wm8962_pdata wm8962_config_data = {
	.gpio_init = {
		[2] = WM8962_GPIO_FN_DMICCLK,
		[4] = 0x8000 | WM8962_GPIO_FN_DMICDAT,
	},
	.clock_enable = wm8962_clk_enable,
};

static struct mxc_audio_platform_data wm8962_data = {
	.ssi_num = 1,
	.src_port = 2,
	.ext_port = 3,
        .hp_gpio = TOPEET_HEADPHONE_DET,
	.hp_active_low = 1,
        .mic_gpio = TOPEET_MICROPHONE_DET,
	.mic_active_low = 1,
	.init = mxc_wm8962_init,
	.clock_enable = wm8962_clk_enable,
};

static struct regulator_consumer_supply topeet_vwm8962_consumers[] = {
	REGULATOR_SUPPLY("SPKVDD1", "0-001a"),
	REGULATOR_SUPPLY("SPKVDD2", "0-001a"),
};

static struct regulator_init_data topeet_vwm8962_init = {
	.constraints = {
		.name = "SPKVDD",
		.valid_ops_mask =  REGULATOR_CHANGE_STATUS,
		.boot_on = 1,
	},
        .num_consumer_supplies = ARRAY_SIZE(topeet_vwm8962_consumers),
        .consumer_supplies = topeet_vwm8962_consumers,
};

static struct fixed_voltage_config topeet_vwm8962_reg_config = {
	.supply_name	= "SPKVDD",
	.microvolts		= 4200000,
        .gpio			= TOPEET_CODEC_PWR_EN,
	.enable_high	= 1,
	.enabled_at_boot = 1,
        .init_data		= &topeet_vwm8962_init,
};

static struct platform_device topeet_vwm8962_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id		= 4,
	.dev	= {
                .platform_data = &topeet_vwm8962_reg_config,
	},
};
#endif

/* add by cym 20170228 */
#if defined(CONFIG_KEYBOARD_HS0038) || defined(CONFIG_KEYBOARD_HS0038_MODULE)
static struct gpio_keys_button hs0038[] =
{
        {
                .gpio       = HS0038_GPIO_IRQ,/* 4(- + backspace enter) */
                .desc       = "hs0038",
        },
};

static struct gpio_keys_platform_data hs0038_data =
{
        .buttons    = hs0038,
        .nbuttons   = ARRAY_SIZE(hs0038),
};

static struct platform_device hs0038_device =
{
        .name       = "hs0038",
        .id     = -1,
        .dev        = {
                .platform_data  = &hs0038_data,
        }
};
#endif
/* end add */

static void mx6q_csi0_cam_powerdown(int powerdown)
{
	if (powerdown)
                gpio_set_value(TOPEET_CSI0_PWN, 1);
	else
                gpio_set_value(TOPEET_CSI0_PWN, 0);

	msleep(2);
}

static void mx6q_csi0_io_init(void)
{
	if (cpu_is_mx6q())
                mxc_iomux_v3_setup_multiple_pads(mx6q_topeet_csi0_sensor_pads,
                        ARRAY_SIZE(mx6q_topeet_csi0_sensor_pads));
	else if (cpu_is_mx6dl())
                mxc_iomux_v3_setup_multiple_pads(mx6dl_topeet_csi0_sensor_pads,
                        ARRAY_SIZE(mx6dl_topeet_csi0_sensor_pads));

	/* Camera reset */
        gpio_request(TOPEET_CSI0_RST, "cam-reset");
        gpio_direction_output(TOPEET_CSI0_RST, 1);

	/* Camera power down */
        gpio_request(TOPEET_CSI0_PWN, "cam-pwdn");
        gpio_direction_output(TOPEET_CSI0_PWN, 1);
	msleep(5);
        gpio_set_value(TOPEET_CSI0_PWN, 0);
	msleep(5);
        gpio_set_value(TOPEET_CSI0_RST, 0);
	msleep(1);
        gpio_set_value(TOPEET_CSI0_RST, 1);
	msleep(5);
        gpio_set_value(TOPEET_CSI0_PWN, 1);

	/* For MX6Q:
	 * GPR1 bit19 and bit20 meaning:
	 * Bit19:       0 - Enable mipi to IPU1 CSI0
	 *                      virtual channel is fixed to 0
	 *              1 - Enable parallel interface to IPU1 CSI0
	 * Bit20:       0 - Enable mipi to IPU2 CSI1
	 *                      virtual channel is fixed to 3
	 *              1 - Enable parallel interface to IPU2 CSI1
	 * IPU1 CSI1 directly connect to mipi csi2,
	 *      virtual channel is fixed to 1
	 * IPU2 CSI0 directly connect to mipi csi2,
	 *      virtual channel is fixed to 2
	 *
	 * For MX6DL:
	 * GPR13 bit 0-2 IPU_CSI0_MUX
	 *   000 MIPI_CSI0
	 *   100 IPU CSI0
	 */
	if (cpu_is_mx6q())
		mxc_iomux_set_gpr_register(1, 19, 1, 1);
	else if (cpu_is_mx6dl())
		mxc_iomux_set_gpr_register(13, 0, 3, 4);
}

static struct fsl_mxc_camera_platform_data camera_data = {
	.mclk = 24000000,
	.mclk_source = 0,
	.csi = 0,
	.io_init = mx6q_csi0_io_init,
	.pwdn = mx6q_csi0_cam_powerdown,
};

static void mx6q_mipi_powerdown(int powerdown)
{
	if (powerdown)
                gpio_set_value(TOPEET_MIPICSI_PWN, 1);
	else
                gpio_set_value(TOPEET_MIPICSI_PWN, 0);

	msleep(2);
}

static void mx6q_mipi_sensor_io_init(void)
{
	if (cpu_is_mx6q())
                mxc_iomux_v3_setup_multiple_pads(mx6q_topeet_mipi_sensor_pads,
                        ARRAY_SIZE(mx6q_topeet_mipi_sensor_pads));
	else if (cpu_is_mx6dl())
                mxc_iomux_v3_setup_multiple_pads(mx6dl_topeet_mipi_sensor_pads,
                        ARRAY_SIZE(mx6dl_topeet_mipi_sensor_pads));

	/* Camera reset */
        gpio_request(TOPEET_MIPICSI_RST, "cam-reset");
        gpio_direction_output(TOPEET_MIPICSI_RST, 1);

	/* Camera power down */
        gpio_request(TOPEET_MIPICSI_PWN, "cam-pwdn");
        gpio_direction_output(TOPEET_MIPICSI_PWN, 1);
	msleep(5);
        gpio_set_value(TOPEET_MIPICSI_PWN, 0);
	msleep(5);
        gpio_set_value(TOPEET_MIPICSI_RST, 0);
	msleep(1);
        gpio_set_value(TOPEET_MIPICSI_RST, 1);
	msleep(5);
        gpio_set_value(TOPEET_MIPICSI_PWN, 1);

	/*for mx6dl, mipi virtual channel 1 connect to csi 1*/
	if (cpu_is_mx6dl())
		mxc_iomux_set_gpr_register(13, 3, 3, 1);
}

static struct fsl_mxc_camera_platform_data mipi_csi2_data = {
	.mclk = 24000000,
	.mclk_source = 0,
	.csi = 1,
	.io_init = mx6q_mipi_sensor_io_init,
	.pwdn = mx6q_mipi_powerdown,
};

#define mV_to_uV(mV) (mV * 1000)
#define uV_to_mV(uV) (uV / 1000)
#define V_to_uV(V) (mV_to_uV(V * 1000))
#define uV_to_V(uV) (uV_to_mV(uV) / 1000)

static struct regulator_consumer_supply display_consumers[] = {
	{
		/* MAX17135 */
		.supply = "DISPLAY",
	},
};

static struct regulator_consumer_supply vcom_consumers[] = {
	{
		/* MAX17135 */
		.supply = "VCOM",
	},
};

static struct regulator_consumer_supply v3p3_consumers[] = {
	{
		/* MAX17135 */
		.supply = "V3P3",
	},
};

static struct regulator_init_data max17135_init_data[] = {
	{
		.constraints = {
			.name = "DISPLAY",
			.valid_ops_mask =  REGULATOR_CHANGE_STATUS,
		},
		.num_consumer_supplies = ARRAY_SIZE(display_consumers),
		.consumer_supplies = display_consumers,
	}, {
		.constraints = {
			.name = "GVDD",
			.min_uV = V_to_uV(20),
			.max_uV = V_to_uV(20),
		},
	}, {
		.constraints = {
			.name = "GVEE",
			.min_uV = V_to_uV(-22),
			.max_uV = V_to_uV(-22),
		},
	}, {
		.constraints = {
			.name = "HVINN",
			.min_uV = V_to_uV(-22),
			.max_uV = V_to_uV(-22),
		},
	}, {
		.constraints = {
			.name = "HVINP",
			.min_uV = V_to_uV(20),
			.max_uV = V_to_uV(20),
		},
	}, {
		.constraints = {
			.name = "VCOM",
			.min_uV = mV_to_uV(-4325),
			.max_uV = mV_to_uV(-500),
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		},
		.num_consumer_supplies = ARRAY_SIZE(vcom_consumers),
		.consumer_supplies = vcom_consumers,
	}, {
		.constraints = {
			.name = "VNEG",
			.min_uV = V_to_uV(-15),
			.max_uV = V_to_uV(-15),
		},
	}, {
		.constraints = {
			.name = "VPOS",
			.min_uV = V_to_uV(15),
			.max_uV = V_to_uV(15),
		},
	}, {
		.constraints = {
			.name = "V3P3",
			.valid_ops_mask =  REGULATOR_CHANGE_STATUS,
		},
		.num_consumer_supplies = ARRAY_SIZE(v3p3_consumers),
		.consumer_supplies = v3p3_consumers,
	},
};

static struct platform_device max17135_sensor_device = {
	.name = "max17135_sensor",
	.id = 0,
};

static struct max17135_platform_data max17135_pdata __initdata = {
	.vneg_pwrup = 1,
	.gvee_pwrup = 1,
	.vpos_pwrup = 2,
	.gvdd_pwrup = 1,
	.gvdd_pwrdn = 1,
	.vpos_pwrdn = 2,
	.gvee_pwrdn = 1,
	.vneg_pwrdn = 1,
        .gpio_pmic_pwrgood = TOPEET_EPDC_PWRSTAT,
        .gpio_pmic_vcom_ctrl = TOPEET_EPDC_VCOM,
        .gpio_pmic_wakeup = TOPEET_EPDC_PMIC_WAKE,
        .gpio_pmic_v3p3 = TOPEET_EPDC_PWRCTRL0,
        .gpio_pmic_intr = TOPEET_EPDC_PMIC_INT,
	.regulator_init = max17135_init_data,
	.init = max17135_regulator_init,
};

static int __init max17135_regulator_init(struct max17135 *max17135)
{
	struct max17135_platform_data *pdata = &max17135_pdata;
	int i, ret;

	if (!epdc_enabled) {
		printk(KERN_DEBUG
			"max17135_regulator_init abort: EPDC not enabled\n");
		return 0;
	}

	max17135->gvee_pwrup = pdata->gvee_pwrup;
	max17135->vneg_pwrup = pdata->vneg_pwrup;
	max17135->vpos_pwrup = pdata->vpos_pwrup;
	max17135->gvdd_pwrup = pdata->gvdd_pwrup;
	max17135->gvdd_pwrdn = pdata->gvdd_pwrdn;
	max17135->vpos_pwrdn = pdata->vpos_pwrdn;
	max17135->vneg_pwrdn = pdata->vneg_pwrdn;
	max17135->gvee_pwrdn = pdata->gvee_pwrdn;

	max17135->max_wait = pdata->vpos_pwrup + pdata->vneg_pwrup +
		pdata->gvdd_pwrup + pdata->gvee_pwrup;

	max17135->gpio_pmic_pwrgood = pdata->gpio_pmic_pwrgood;
	max17135->gpio_pmic_vcom_ctrl = pdata->gpio_pmic_vcom_ctrl;
	max17135->gpio_pmic_wakeup = pdata->gpio_pmic_wakeup;
	max17135->gpio_pmic_v3p3 = pdata->gpio_pmic_v3p3;
	max17135->gpio_pmic_intr = pdata->gpio_pmic_intr;

	gpio_request(max17135->gpio_pmic_wakeup, "epdc-pmic-wake");
	gpio_direction_output(max17135->gpio_pmic_wakeup, 0);

	gpio_request(max17135->gpio_pmic_vcom_ctrl, "epdc-vcom");
	gpio_direction_output(max17135->gpio_pmic_vcom_ctrl, 0);

	gpio_request(max17135->gpio_pmic_v3p3, "epdc-v3p3");
	gpio_direction_output(max17135->gpio_pmic_v3p3, 0);

	gpio_request(max17135->gpio_pmic_intr, "epdc-pmic-int");
	gpio_direction_input(max17135->gpio_pmic_intr);

	gpio_request(max17135->gpio_pmic_pwrgood, "epdc-pwrstat");
	gpio_direction_input(max17135->gpio_pmic_pwrgood);

	max17135->vcom_setup = false;
	max17135->init_done = false;

	for (i = 0; i < MAX17135_NUM_REGULATORS; i++) {
		ret = max17135_register_regulator(max17135, i,
			&pdata->regulator_init[i]);
		if (ret != 0) {
			printk(KERN_ERR"max17135 regulator init failed: %d\n",
				ret);
			return ret;
		}
	}

	/*
	 * TODO: We cannot enable full constraints for now, since
	 * it results in the PFUZE regulators being disabled
	 * at the end of boot, which disables critical regulators.
	 */
	/*regulator_has_full_constraints();*/

	return 0;
}

static struct imxi2c_platform_data mx6q_topeet_i2c_data = {
	.bitrate = 100000,
};

static struct fsl_mxc_lightsensor_platform_data ls_data = {
	.rext = 499,	/* calibration: 499K->700K */
};

/* add by cym 20161018 */
#ifdef CONFIG_TOUCHSCREEN_TSC2007
#include <linux/i2c/tsc2007.h>

#define GPIO_TSC_PORT (IMX_GPIO_NR(3, 9))//((PAD_GPIO_C + 26))
static int ts_get_pendown_state(void)
{
        int val;

        val = gpio_get_value(GPIO_TSC_PORT);

        return !val;
}

static int ts_init(void)
{
        //gpio_to_irq(GPIO_TSC_PORT);

        return 0;
}

static struct tsc2007_platform_data tsc2007_info = {
        .model                  = 2007,
        .x_plate_ohms           = 180,
        .get_pendown_state      = ts_get_pendown_state,
        .init_platform_hw       = ts_init,
};

#endif
/* end add */

#ifdef CONFIG_TOUCHSCREEN_FT5X0X
#include <mach/ft5x0x_touch.h>
static struct ft5x0x_i2c_platform_data ft5x0x_pdata1 = {
        .gpio_irq               = IMX_GPIO_NR(6, 8),
        //.irq_cfg                = S3C_GPIO_SFN(0xf),
        .screen_max_x   = 768,
        .screen_max_y   = 1024,
        .pressure_max   = 255,
};
static struct ft5x0x_i2c_platform_data ft5x0x_pdata2 = {
        .gpio_irq               = IMX_GPIO_NR(6, 7),
        //.irq_cfg                = S3C_GPIO_SFN(0xf),
        .screen_max_x   = 768,
        .screen_max_y   = 1024,
        .pressure_max   = 255,
};
#endif

static struct i2c_board_info mxc_i2c0_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("wm89**", 0x1a),
	},
	{
		I2C_BOARD_INFO("ov564x", 0x3c),
		.platform_data = (void *)&camera_data,
	},
#ifdef CONFIG_SENSORS_MMA8X5X
	{
		I2C_BOARD_INFO("mma8x5x", 0x1c),
                .irq =	gpio_to_irq(TOPEET_ACCL_INT),
		.platform_data = (void *)&mma8x5x_position,
	},
#endif
};

static struct i2c_board_info mxc_i2c1_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("mxc_hdmi_i2c", 0x50),
	},
#ifdef CONFIG_TOUCHSCREEN_MAX11801
	{
		I2C_BOARD_INFO("max11801", 0x48),
		.platform_data = (void *)&max11801_mode,
                .irq = gpio_to_irq(TOPEET_TS_INT),
	},
#endif
#ifdef CONFIG_SND_SOC_WM8960
	{
                I2C_BOARD_INFO("wm8960", 0x1a),
                .platform_data = (void *) &wm8960_config_data,
        },
#endif

#ifdef CONFIG_TOUCHSCREEN_FT5X0X
        {
                I2C_BOARD_INFO("ft5x0x_ts", 0x70>>1),
                .irq = gpio_to_irq(IMX_GPIO_NR(6, 8)),
                .platform_data = &ft5x0x_pdata1,
        },
#endif

};

static struct i2c_board_info mxc_i2c2_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("mpu6050", 0x68),
        .irq =	gpio_to_irq(TOPEET_ACCL_INT),
		.platform_data = (void *)&mpu6050_position,
	},
#if 0
	{
		I2C_BOARD_INFO("max17135", 0x48),
		.platform_data = &max17135_pdata,
	},
#endif
#if 0
	{
		I2C_BOARD_INFO("egalax_ts", 0x4),
                .irq = gpio_to_irq(TOPEET_CAP_TCH_INT1),
	},
	{
		I2C_BOARD_INFO("mag3110", 0x0e),
                .irq = gpio_to_irq(TOPEET_eCOMPASS_INT),
		.platform_data = (void *)&mag3110_position,
	},
	{
		I2C_BOARD_INFO("isl29023", 0x44),
                .irq  = gpio_to_irq(TOPEET_ALS_INT),
		.platform_data = &ls_data,
	}, {
		I2C_BOARD_INFO("elan-touch", 0x10),
                .irq = gpio_to_irq(TOPEET_ELAN_INT),
	},
#endif
/* add by cym 20161219 */
#if 0 //caesar
	{
                I2C_BOARD_INFO("mag3110", 0x0e),
                .irq = gpio_to_irq(TOPEET_eCOMPASS_INT),
                .platform_data = (void *)&mag3110_position,
        },
#endif
/* end add */
	{
		I2C_BOARD_INFO("mxc_ldb_i2c", 0x50),
		.platform_data = (void *)1,	/* lvds port1 */
	},


#ifdef CONFIG_TOUCHSCREEN_FT5X0X
        {
                I2C_BOARD_INFO("ft5x0x_ts", 0x70>>1),
                .irq = gpio_to_irq(IMX_GPIO_NR(6, 7)),
                .platform_data = &ft5x0x_pdata2,
        },
#endif

/* add by cym 20161018 */
#ifdef CONFIG_TOUCHSCREEN_TSC2007
	{
                I2C_BOARD_INFO("tsc2007", 0x48),
                .irq = gpio_to_irq(IMX_GPIO_NR(3, 9)),
                .platform_data = &tsc2007_info,
        },
#endif
/* end add */

#ifdef CONFIG_TOUCHSCREEN_GT9XX
        {
                I2C_BOARD_INFO("Goodix-TS", 0x5d),
        },
#endif
};

/* add by cym 20161012 */
static int __init set_touch_screen(char *options)
{
        if(strstr(options, "LDB-XGA"))  //9.7
        {
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
                ft5x0x_pdata1.screen_max_x   = 768;
                ft5x0x_pdata1.screen_max_y   = 1024;

                ft5x0x_pdata2.screen_max_x   = 768;
                ft5x0x_pdata2.screen_max_y   = 1024;
#endif
        }
        else if(strstr(options, "LDB-7inch")) //7.0
        {
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
                ft5x0x_pdata1.screen_max_x   = 800;
                ft5x0x_pdata1.screen_max_y   = 1280;

                ft5x0x_pdata2.screen_max_x   = 800;
                ft5x0x_pdata2.screen_max_y   = 1280;
#endif
        }
        else if(strstr(options, "VGA_1024600")) //1024x600
        {
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
                ft5x0x_pdata2.screen_max_x   = 1024;
                ft5x0x_pdata2.screen_max_y   = 600;
		ft5x0x_pdata2.gpio_irq = IMX_GPIO_NR(3, 9);

                mxc_i2c2_board_info[2].irq = gpio_to_irq(IMX_GPIO_NR(3, 9));

#ifdef CONFIG_TOUCHSCREEN_TSC2007
                mxc_i2c2_board_info[3].irq = NULL;
#endif
#endif
        }
	else if(strstr(options, "VGA_800480")) //1024x600
        {
#ifdef CONFIG_TOUCHSCREEN_FT5X0X
                ft5x0x_pdata2.screen_max_x   = 800;
                ft5x0x_pdata2.screen_max_y   = 480;
                ft5x0x_pdata2.gpio_irq = IMX_GPIO_NR(3, 9);

                mxc_i2c2_board_info[2].irq = gpio_to_irq(IMX_GPIO_NR(3, 9));

#ifdef CONFIG_TOUCHSCREEN_TSC2007
                mxc_i2c2_board_info[3].irq = NULL;
#endif
#endif
        }
}

early_param("video", set_touch_screen);
/* end add */

static int epdc_get_pins(void)
{
	int ret = 0;

	/* Claim GPIOs for EPDC pins - used during power up/down */
        ret |= gpio_request(TOPEET_EPDC_SDDO_0, "epdc_d0");
        ret |= gpio_request(TOPEET_EPDC_SDDO_1, "epdc_d1");
        ret |= gpio_request(TOPEET_EPDC_SDDO_2, "epdc_d2");
        ret |= gpio_request(TOPEET_EPDC_SDDO_3, "epdc_d3");
        ret |= gpio_request(TOPEET_EPDC_SDDO_4, "epdc_d4");
        ret |= gpio_request(TOPEET_EPDC_SDDO_5, "epdc_d5");
        ret |= gpio_request(TOPEET_EPDC_SDDO_6, "epdc_d6");
        ret |= gpio_request(TOPEET_EPDC_SDDO_7, "epdc_d7");
        ret |= gpio_request(TOPEET_EPDC_GDCLK, "epdc_gdclk");
        ret |= gpio_request(TOPEET_EPDC_GDSP, "epdc_gdsp");
        ret |= gpio_request(TOPEET_EPDC_GDOE, "epdc_gdoe");
        ret |= gpio_request(TOPEET_EPDC_GDRL, "epdc_gdrl");
        ret |= gpio_request(TOPEET_EPDC_SDCLK, "epdc_sdclk");
        ret |= gpio_request(TOPEET_EPDC_SDOE, "epdc_sdoe");
        //ret |= gpio_request(TOPEET_EPDC_SDLE, "epdc_sdle");
        ret |= gpio_request(TOPEET_EPDC_SDSHR, "epdc_sdshr");
        ret |= gpio_request(TOPEET_EPDC_BDR0, "epdc_bdr0");
        ret |= gpio_request(TOPEET_EPDC_SDCE0, "epdc_sdce0");
        ret |= gpio_request(TOPEET_EPDC_SDCE1, "epdc_sdce1");
        ret |= gpio_request(TOPEET_EPDC_SDCE2, "epdc_sdce2");

	return ret;
}

static void epdc_put_pins(void)
{
        gpio_free(TOPEET_EPDC_SDDO_0);
        gpio_free(TOPEET_EPDC_SDDO_1);
        gpio_free(TOPEET_EPDC_SDDO_2);
        gpio_free(TOPEET_EPDC_SDDO_3);
        gpio_free(TOPEET_EPDC_SDDO_4);
        gpio_free(TOPEET_EPDC_SDDO_5);
        gpio_free(TOPEET_EPDC_SDDO_6);
        gpio_free(TOPEET_EPDC_SDDO_7);
        gpio_free(TOPEET_EPDC_GDCLK);
        gpio_free(TOPEET_EPDC_GDSP);
        gpio_free(TOPEET_EPDC_GDOE);
        gpio_free(TOPEET_EPDC_GDRL);
        gpio_free(TOPEET_EPDC_SDCLK);
        gpio_free(TOPEET_EPDC_SDOE);
        //gpio_free(TOPEET_EPDC_SDLE);
        gpio_free(TOPEET_EPDC_SDSHR);
        gpio_free(TOPEET_EPDC_BDR0);
        gpio_free(TOPEET_EPDC_SDCE0);
        gpio_free(TOPEET_EPDC_SDCE1);
        gpio_free(TOPEET_EPDC_SDCE2);
}

static void epdc_enable_pins(void)
{
	/* Configure MUX settings to enable EPDC use */
        mxc_iomux_v3_setup_multiple_pads(mx6dl_topeet_epdc_enable_pads, \
                                ARRAY_SIZE(mx6dl_topeet_epdc_enable_pads));

        gpio_direction_input(TOPEET_EPDC_SDDO_0);
        gpio_direction_input(TOPEET_EPDC_SDDO_1);
        gpio_direction_input(TOPEET_EPDC_SDDO_2);
        gpio_direction_input(TOPEET_EPDC_SDDO_3);
        gpio_direction_input(TOPEET_EPDC_SDDO_4);
        gpio_direction_input(TOPEET_EPDC_SDDO_5);
        gpio_direction_input(TOPEET_EPDC_SDDO_6);
        gpio_direction_input(TOPEET_EPDC_SDDO_7);
        gpio_direction_input(TOPEET_EPDC_GDCLK);
        gpio_direction_input(TOPEET_EPDC_GDSP);
        gpio_direction_input(TOPEET_EPDC_GDOE);
        gpio_direction_input(TOPEET_EPDC_GDRL);
        gpio_direction_input(TOPEET_EPDC_SDCLK);
        gpio_direction_input(TOPEET_EPDC_SDOE);
        //gpio_direction_input(TOPEET_EPDC_SDLE);
        gpio_direction_input(TOPEET_EPDC_SDSHR);
        gpio_direction_input(TOPEET_EPDC_BDR0);
        gpio_direction_input(TOPEET_EPDC_SDCE0);
        gpio_direction_input(TOPEET_EPDC_SDCE1);
        gpio_direction_input(TOPEET_EPDC_SDCE2);
}

static void epdc_disable_pins(void)
{
	/* Configure MUX settings for EPDC pins to
	 * GPIO and drive to 0. */
        mxc_iomux_v3_setup_multiple_pads(mx6dl_topeet_epdc_disable_pads, \
                                ARRAY_SIZE(mx6dl_topeet_epdc_disable_pads));

        gpio_direction_output(TOPEET_EPDC_SDDO_0, 0);
        gpio_direction_output(TOPEET_EPDC_SDDO_1, 0);
        gpio_direction_output(TOPEET_EPDC_SDDO_2, 0);
        gpio_direction_output(TOPEET_EPDC_SDDO_3, 0);
        gpio_direction_output(TOPEET_EPDC_SDDO_4, 0);
        gpio_direction_output(TOPEET_EPDC_SDDO_5, 0);
        gpio_direction_output(TOPEET_EPDC_SDDO_6, 0);
        gpio_direction_output(TOPEET_EPDC_SDDO_7, 0);
        gpio_direction_output(TOPEET_EPDC_GDCLK, 0);
        gpio_direction_output(TOPEET_EPDC_GDSP, 0);
        gpio_direction_output(TOPEET_EPDC_GDOE, 0);
        gpio_direction_output(TOPEET_EPDC_GDRL, 0);
        gpio_direction_output(TOPEET_EPDC_SDCLK, 0);
        gpio_direction_output(TOPEET_EPDC_SDOE, 0);
        //gpio_direction_output(TOPEET_EPDC_SDLE, 0);
        gpio_direction_output(TOPEET_EPDC_SDSHR, 0);
        gpio_direction_output(TOPEET_EPDC_BDR0, 0);
        gpio_direction_output(TOPEET_EPDC_SDCE0, 0);
        gpio_direction_output(TOPEET_EPDC_SDCE1, 0);
        gpio_direction_output(TOPEET_EPDC_SDCE2, 0);
}

static struct fb_videomode e60_v110_mode = {
	.name = "E60_V110",
	.refresh = 50,
	.xres = 800,
	.yres = 600,
	.pixclock = 18604700,
	.left_margin = 8,
	.right_margin = 178,
	.upper_margin = 4,
	.lower_margin = 10,
	.hsync_len = 20,
	.vsync_len = 4,
	.sync = 0,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};
static struct fb_videomode e60_v220_mode = {
	.name = "E60_V220",
	.refresh = 85,
	.xres = 800,
	.yres = 600,
	.pixclock = 30000000,
	.left_margin = 8,
	.right_margin = 164,
	.upper_margin = 4,
	.lower_margin = 8,
	.hsync_len = 4,
	.vsync_len = 1,
	.sync = 0,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
	.refresh = 85,
	.xres = 800,
	.yres = 600,
};
static struct fb_videomode e060scm_mode = {
	.name = "E060SCM",
	.refresh = 85,
	.xres = 800,
	.yres = 600,
	.pixclock = 26666667,
	.left_margin = 8,
	.right_margin = 100,
	.upper_margin = 4,
	.lower_margin = 8,
	.hsync_len = 4,
	.vsync_len = 1,
	.sync = 0,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};
static struct fb_videomode e97_v110_mode = {
	.name = "E97_V110",
	.refresh = 50,
	.xres = 1200,
	.yres = 825,
	.pixclock = 32000000,
	.left_margin = 12,
	.right_margin = 128,
	.upper_margin = 4,
	.lower_margin = 10,
	.hsync_len = 20,
	.vsync_len = 4,
	.sync = 0,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

static struct imx_epdc_fb_mode panel_modes[] = {
	{
		&e60_v110_mode,
		4,      /* vscan_holdoff */
		10,     /* sdoed_width */
		20,     /* sdoed_delay */
		10,     /* sdoez_width */
		20,     /* sdoez_delay */
		428,    /* gdclk_hp_offs */
		20,     /* gdsp_offs */
		0,      /* gdoe_offs */
		1,      /* gdclk_offs */
		1,      /* num_ce */
	},
	{
		&e60_v220_mode,
		4,      /* vscan_holdoff */
		10,     /* sdoed_width */
		20,     /* sdoed_delay */
		10,     /* sdoez_width */
		20,     /* sdoez_delay */
		465,    /* gdclk_hp_offs */
		20,     /* gdsp_offs */
		0,      /* gdoe_offs */
		9,      /* gdclk_offs */
		1,      /* num_ce */
	},
	{
		&e060scm_mode,
		4,      /* vscan_holdoff */
		10,     /* sdoed_width */
		20,     /* sdoed_delay */
		10,     /* sdoez_width */
		20,     /* sdoez_delay */
		419,    /* gdclk_hp_offs */
		20,     /* gdsp_offs */
		0,      /* gdoe_offs */
		5,      /* gdclk_offs */
		1,      /* num_ce */
	},
	{
		&e97_v110_mode,
		8,      /* vscan_holdoff */
		10,     /* sdoed_width */
		20,     /* sdoed_delay */
		10,     /* sdoez_width */
		20,     /* sdoez_delay */
		632,    /* gdclk_hp_offs */
		20,     /* gdsp_offs */
		0,      /* gdoe_offs */
		1,      /* gdclk_offs */
		3,      /* num_ce */
	}
};

static struct imx_epdc_fb_platform_data epdc_data = {
	.epdc_mode = panel_modes,
	.num_modes = ARRAY_SIZE(panel_modes),
	.get_pins = epdc_get_pins,
	.put_pins = epdc_put_pins,
	.enable_pins = epdc_enable_pins,
	.disable_pins = epdc_disable_pins,
};

static void imx6q_topeet_usbotg_vbus(bool on)
{
	if (on)
                gpio_set_value(TOPEET_USB_OTG_PWR, 1);
	else
                gpio_set_value(TOPEET_USB_OTG_PWR, 0);
}

static void imx6q_topeet_host1_vbus(bool on)
{
	if (on)
                gpio_set_value(TOPEET_USB_H1_PWR, 1);
	else
                gpio_set_value(TOPEET_USB_H1_PWR, 0);
}

static void __init imx6q_topeet_init_usb(void)
{
	int ret = 0;

	imx_otg_base = MX6_IO_ADDRESS(MX6Q_USB_OTG_BASE_ADDR);
	/* disable external charger detect,
	 * or it will affect signal quality at dp .
	 */
        ret = gpio_request(TOPEET_USB_OTG_PWR, "usb-pwr");
	if (ret) {
                pr_err("failed to get GPIO TOPEET_USB_OTG_PWR: %d\n",
			ret);
		return;
	}
        gpio_direction_output(TOPEET_USB_OTG_PWR, 0);
	/* keep USB host1 VBUS always on */
        ret = gpio_request(TOPEET_USB_H1_PWR, "usb-h1-pwr");
	if (ret) {
                pr_err("failed to get GPIO TOPEET_USB_H1_PWR: %d\n",
			ret);
		return;
	}
        gpio_direction_output(TOPEET_USB_H1_PWR, 0);
	if (board_is_mx6_reva())
		mxc_iomux_set_gpr_register(1, 13, 1, 1);
	else
		mxc_iomux_set_gpr_register(1, 13, 1, 0);

        mx6_set_otghost_vbus_func(imx6q_topeet_usbotg_vbus);
        mx6_set_host1_vbus_func(imx6q_topeet_host1_vbus);

}

/* HW Initialization, if return 0, initialization is successful. */
static int mx6q_topeet_sata_init(struct device *dev, void __iomem *addr)
{
	u32 tmpdata;
	int ret = 0;
	struct clk *clk;

	sata_clk = clk_get(dev, "imx_sata_clk");
	if (IS_ERR(sata_clk)) {
		dev_err(dev, "no sata clock.\n");
		return PTR_ERR(sata_clk);
	}
	ret = clk_enable(sata_clk);
	if (ret) {
		dev_err(dev, "can't enable sata clock.\n");
		goto put_sata_clk;
	}

	/* Set PHY Paremeters, two steps to configure the GPR13,
	 * one write for rest of parameters, mask of first write is 0x07FFFFFD,
	 * and the other one write for setting the mpll_clk_off_b
	 *.rx_eq_val_0(iomuxc_gpr13[26:24]),
	 *.los_lvl(iomuxc_gpr13[23:19]),
	 *.rx_dpll_mode_0(iomuxc_gpr13[18:16]),
	 *.sata_speed(iomuxc_gpr13[15]),
	 *.mpll_ss_en(iomuxc_gpr13[14]),
	 *.tx_atten_0(iomuxc_gpr13[13:11]),
	 *.tx_boost_0(iomuxc_gpr13[10:7]),
	 *.tx_lvl(iomuxc_gpr13[6:2]),
	 *.mpll_ck_off(iomuxc_gpr13[1]),
	 *.tx_edgerate_0(iomuxc_gpr13[0]),
	 */
	tmpdata = readl(IOMUXC_GPR13);
	writel(((tmpdata & ~0x07FFFFFD) | 0x0593A044), IOMUXC_GPR13);

	/* enable SATA_PHY PLL */
	tmpdata = readl(IOMUXC_GPR13);
	writel(((tmpdata & ~0x2) | 0x2), IOMUXC_GPR13);

	/* Get the AHB clock rate, and configure the TIMER1MS reg later */
	clk = clk_get(NULL, "ahb");
	if (IS_ERR(clk)) {
		dev_err(dev, "no ahb clock.\n");
		ret = PTR_ERR(clk);
		goto release_sata_clk;
	}
	tmpdata = clk_get_rate(clk) / 1000;
	clk_put(clk);

#ifdef CONFIG_SATA_AHCI_PLATFORM
	ret = sata_init(addr, tmpdata);
	if (ret == 0)
		return ret;
#else
	usleep_range(1000, 2000);
	/* AHCI PHY enter into PDDQ mode if the AHCI module is not enabled */
	tmpdata = readl(addr + PORT_PHY_CTL);
	writel(tmpdata | PORT_PHY_CTL_PDDQ_LOC, addr + PORT_PHY_CTL);
	pr_info("No AHCI save PWR: PDDQ %s\n", ((readl(addr + PORT_PHY_CTL)
					>> 20) & 1) ? "enabled" : "disabled");
#endif

release_sata_clk:
	/* disable SATA_PHY PLL */
	writel((readl(IOMUXC_GPR13) & ~0x2), IOMUXC_GPR13);
	clk_disable(sata_clk);
put_sata_clk:
	clk_put(sata_clk);

	return ret;
}

#ifdef CONFIG_SATA_AHCI_PLATFORM
static void mx6q_topeet_sata_exit(struct device *dev)
{
	clk_disable(sata_clk);
	clk_put(sata_clk);
}

static struct ahci_platform_data mx6q_topeet_sata_data = {
        .init = mx6q_topeet_sata_init,
        .exit = mx6q_topeet_sata_exit,
};
#endif

static struct gpio mx6_flexcan_gpios[] = {
        //{ MX6_ARM2_CAN1_EN, GPIOF_OUT_INIT_LOW, "flexcan1-en" },
        { TOPEET_CAN1_STBY, GPIOF_OUT_INIT_HIGH, "flexcan1-stby" },
        { TOPEET_CAN2_STBY, GPIOF_OUT_INIT_HIGH, "flexcan2-stby" },
        //{ MX6_ARM2_CAN2_EN, GPIOF_OUT_INIT_LOW, "flexcan2-en" },
};

static void mx6q_topeet_flexcan0_switch(int enable)
{
	if (enable) {
		printk("fun:%s, line = %d(en = %d)\n", __FUNCTION__, __LINE__, enable);
                gpio_set_value(TOPEET_CAN1_STBY, 0);
	} else {
		printk("fun:%s, line = %d(en = %d)\n", __FUNCTION__, __LINE__, enable);
                gpio_set_value(TOPEET_CAN1_STBY, 1);
	}
}

static void mx6q_topeet_flexcan1_switch(int enable)
{
	if (enable) {
		printk("fun:%s, line = %d(en = %d)\n", __FUNCTION__, __LINE__, enable);
                gpio_set_value(TOPEET_CAN2_STBY, 0);
	} else {
		printk("fun:%s, line = %d(en = %d)\n", __FUNCTION__, __LINE__, enable);
                gpio_set_value(TOPEET_CAN2_STBY, 1);
	}
}

static const struct flexcan_platform_data
        mx6q_topeet_flexcan_pdata[] __initconst = {
	{
        	.transceiver_switch = mx6q_topeet_flexcan0_switch,
	},
	{
        	.transceiver_switch = mx6q_topeet_flexcan1_switch,
	}
	
};


static struct viv_gpu_platform_data imx6q_gpu_pdata __initdata = {
	.reserved_mem_size = SZ_128M + SZ_64M - SZ_16M,
};

static struct imx_asrc_platform_data imx_asrc_data = {
	.channel_bits = 4,
	.clk_map_ver = 2,
};

static void mx6_reset_mipi_dsi(void)
{
        gpio_set_value(TOPEET_DISP_PWR_EN, 1);
        gpio_set_value(TOPEET_DISP_RST_B, 1);
	udelay(10);
        gpio_set_value(TOPEET_DISP_RST_B, 0);
	udelay(50);
        gpio_set_value(TOPEET_DISP_RST_B, 1);

	/*
	 * it needs to delay 120ms minimum for reset complete
	 */
	msleep(120);
}

static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.ipu_id		= 0,
	.disp_id	= 1,
	.lcd_panel	= "TRULY-WVGA",
	.reset		= mx6_reset_mipi_dsi,
};

static struct ipuv3_fb_platform_data topeet_fb_data[] = {
	{ /*fb0*/
	.disp_dev = "ldb",
	.interface_pix_fmt = IPU_PIX_FMT_RGB666,
	.mode_str = "LDB-XGA",
	.default_bpp = 16,
	.int_clk = false,
	.late_init = false,
	}, {
	.disp_dev = "hdmi",
	.interface_pix_fmt = IPU_PIX_FMT_RGB24,
	.mode_str = "1920x1080M@60",
	.default_bpp = 32,
	.int_clk = false,
	.late_init = false,
	}, {
	.disp_dev = "ldb",
	.interface_pix_fmt = IPU_PIX_FMT_RGB666,
	.mode_str = "LDB-XGA",
	.default_bpp = 16,
	.int_clk = false,
	.late_init = false,
	},
};

static void hdmi_init(int ipu_id, int disp_id)
{
	int hdmi_mux_setting;

	if ((ipu_id > 1) || (ipu_id < 0)) {
		pr_err("Invalid IPU select for HDMI: %d. Set to 0\n", ipu_id);
		ipu_id = 0;
	}

	if ((disp_id > 1) || (disp_id < 0)) {
		pr_err("Invalid DI select for HDMI: %d. Set to 0\n", disp_id);
		disp_id = 0;
	}

	/* Configure the connection between IPU1/2 and HDMI */
	hdmi_mux_setting = 2*ipu_id + disp_id;

	/* GPR3, bits 2-3 = HDMI_MUX_CTL */
	mxc_iomux_set_gpr_register(3, 2, 2, hdmi_mux_setting);

	/* Set HDMI event as SDMA event2 while Chip version later than TO1.2 */
	if (hdmi_SDMA_check())
		mxc_iomux_set_gpr_register(0, 0, 1, 1);
}

/* On mx6x topeet board i2c2 iomux with hdmi ddc,
 * the pins default work at i2c2 function,
 when hdcp enable, the pins should work at ddc function */

static void hdmi_enable_ddc_pin(void)
{
	if (cpu_is_mx6dl())
                mxc_iomux_v3_setup_multiple_pads(mx6dl_topeet_hdmi_ddc_pads,
                        ARRAY_SIZE(mx6dl_topeet_hdmi_ddc_pads));
	else
                mxc_iomux_v3_setup_multiple_pads(mx6q_topeet_hdmi_ddc_pads,
                        ARRAY_SIZE(mx6q_topeet_hdmi_ddc_pads));
}

static void hdmi_disable_ddc_pin(void)
{
	if (cpu_is_mx6dl())
                mxc_iomux_v3_setup_multiple_pads(mx6dl_topeet_i2c2_pads,
                        ARRAY_SIZE(mx6dl_topeet_i2c2_pads));
	else
                mxc_iomux_v3_setup_multiple_pads(mx6q_topeet_i2c2_pads,
                        ARRAY_SIZE(mx6q_topeet_i2c2_pads));
}

static struct fsl_mxc_hdmi_platform_data hdmi_data = {
	.init = hdmi_init,
	.enable_pins = hdmi_enable_ddc_pin,
	.disable_pins = hdmi_disable_ddc_pin,
	.phy_reg_vlev = 0x0294,
	.phy_reg_cksymtx = 0x800d,
};

static struct fsl_mxc_hdmi_core_platform_data hdmi_core_data = {
	.ipu_id = 1,
	.disp_id = 0,
};

static struct fsl_mxc_lcd_platform_data lcdif_data = {
	.ipu_id = 0,
	.disp_id = 0,
	.default_ifmt = IPU_PIX_FMT_RGB565,
};

static struct fsl_mxc_ldb_platform_data ldb_data = {
	.ipu_id = 0,
	.disp_id = 1,
	.ext_ref = 1,
	.mode = LDB_SEP1,
	.sec_ipu_id = 0,
	.sec_disp_id = 0,
};

static struct max8903_pdata charger1_data = {
        .dok = TOPEET_CHARGE_DOK_B,
        .uok = TOPEET_CHARGE_UOK_B,
        .chg = TOPEET_CHARGE_CHG_1_B,
        .flt = TOPEET_CHARGE_FLT_1_B,
	.dcm_always_high = true,
	.dc_valid = true,
	.usb_valid = true,
};

static struct platform_device topeet_max8903_charger_1 = {
	.name	= "max8903-charger",
	.id	= 1,
	.dev	= {
		.platform_data = &charger1_data,
	},
};

static struct imx_ipuv3_platform_data ipu_data[] = {
	{
	.rev = 4,
	.csi_clk[0] = "clko_clk",
	.bypass_reset = false,
	}, {
	.rev = 4,
	.csi_clk[0] = "clko_clk",
	.bypass_reset = false,
	},
};

static struct ion_platform_data imx_ion_data = {
	.nr = 1,
	.heaps = {
		{
		.id = 0,
		.type = ION_HEAP_TYPE_CARVEOUT,
		.name = "vpu_ion",
		.size = SZ_16M,
		.cacheable = 0,
		},
	},
};

static struct fsl_mxc_capture_platform_data capture_data[] = {
	{
		.csi = 0,
		.ipu = 0,
		.mclk_source = 0,
		.is_mipi = 0,
	}, {
		.csi = 1,
		.ipu = 0,
		.mclk_source = 0,
		.is_mipi = 1,
	},
};

static void mx6q_sd_bt_reset(void)
{
	printk(KERN_INFO "mx6q_sd_bt_reset");
        gpio_request(TOPEET_BT_RESET, "bt-reset");
        gpio_direction_output(TOPEET_BT_RESET, 0);
	/* pull down reset pin at least >5ms */
	mdelay(6);
	/* pull up after power supply BT */
        gpio_direction_output(TOPEET_BT_RESET, 1);
        gpio_free(TOPEET_BT_RESET);
	msleep(100);
}

static int mx6q_sd_bt_power_change(int status)
{
	if (status)
		mx6q_sd_bt_reset();
	return 0;
}

static struct platform_device mxc_bt_rfkill = {
	.name = "mxc_bt_rfkill",
};

static struct imx_bt_rfkill_platform_data mxc_bt_rfkill_data = {
	.power_change = mx6q_sd_bt_power_change,
};

struct imx_vout_mem {
	resource_size_t res_mbase;
	resource_size_t res_msize;
};

static struct imx_vout_mem vout_mem __initdata = {
	.res_msize = 0,
};

static void topeet_suspend_enter(void)
{
	/* suspend preparation */
	/* Disable AUX 5V */
        gpio_set_value(TOPEET_AUX_5V_EN, 0);
}

static void topeet_suspend_exit(void)
{
	/* resume restore */
	/* Enable AUX 5V */
        gpio_set_value(TOPEET_AUX_5V_EN, 1);
}
static const struct pm_platform_data mx6q_topeet_pm_data __initconst = {
	.name = "imx_pm",
        .suspend_enter = topeet_suspend_enter,
        .suspend_exit = topeet_suspend_exit,
};

static struct regulator_consumer_supply topeet_vmmc_consumers[] = {
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.1"),
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.2"),
	REGULATOR_SUPPLY("vmmc", "sdhci-esdhc-imx.3"),
};

static struct regulator_init_data topeet_vmmc_init = {
        .num_consumer_supplies = ARRAY_SIZE(topeet_vmmc_consumers),
        .consumer_supplies = topeet_vmmc_consumers,
};

static struct fixed_voltage_config topeet_vmmc_reg_config = {
	.supply_name		= "vmmc",
	.microvolts		= 3300000,
	.gpio			= -1,
        .init_data		= &topeet_vmmc_init,
};

static struct platform_device topeet_vmmc_reg_devices = {
	.name	= "reg-fixed-voltage",
	.id	= 3,
	.dev	= {
                .platform_data = &topeet_vmmc_reg_config,
	},
};

static int __init imx6q_init_audio(void)
{
	if (board_is_mx6_reva()) {
                mxc_register_device(&mx6_topeet_audio_wm8958_device,
				    &wm8958_data);
                imx6q_add_imx_ssi(1, &mx6_topeet_ssi_pdata);

		mxc_wm8958_init();
	} else {
#ifdef CONFIG_SND_SOC_WM8962
                platform_device_register(&topeet_vwm8962_reg_devices);
                mxc_register_device(&mx6_topeet_audio_wm8962_device,
				    &wm8962_data);
                imx6q_add_imx_ssi(1, &mx6_topeet_ssi_pdata);

		mxc_wm8962_init();
#endif

#ifdef CONFIG_SND_SOC_WM8960
		platform_device_register(&topeet_vwm8960_reg_devices);
        	mxc_register_device(&mx6_topeet_audio_wm8960_device,
                	            &wm8960_data);
        	imx6q_add_imx_ssi(1, &mx6_topeet_ssi_pdata);

        	mxc_wm8960_init();
#endif
	}

	return 0;
}

static void gps_power_on(bool on)
{
	/* Enable/disable aux_3v15 */
        gpio_request(TOPEET_AUX_3V15_EN, "aux_3v15_en");
        gpio_direction_output(TOPEET_AUX_3V15_EN, 1);
        gpio_set_value(TOPEET_AUX_3V15_EN, on);
        gpio_free(TOPEET_AUX_3V15_EN);
	/*Enable/disable gps_en*/
        gpio_request(TOPEET_GPS_EN, "gps_en");
        gpio_direction_output(TOPEET_GPS_EN, 1);
        gpio_set_value(TOPEET_GPS_EN, on);
        gpio_free(TOPEET_GPS_EN);

}

#if defined(CONFIG_LEDS_TRIGGER) || defined(CONFIG_LEDS_GPIO)

#define GPIO_LED(gpio_led, name_led, act_low, state_suspend, trigger)	\
{									\
	.gpio			= gpio_led,				\
	.name			= name_led,				\
	.active_low		= act_low,				\
	.retain_state_suspended = state_suspend,			\
	.default_state		= 0,					\
	.default_trigger	= "max8903-"trigger,		\
}

/* use to show a external power source is connected
 * GPIO_LED(topeet_CHARGE_DONE, "chg_detect", 0, 1, "ac-online"),
 */
static struct gpio_led imx6q_gpio_leds[] = {
        GPIO_LED(TOPEET_CHARGE_NOW, "chg_now_led", 0, 1,
		"charger-charging"),
/* For the latest B4 board, this GPIO_1 is connected to POR_B,
which will reset the whole board if this pin's level is changed,
so, for the latest board, we have to avoid using this pin as
GPIO.
        GPIO_LED(TOPEET_CHARGE_DONE, "chg_done_led", 0, 1,
			"charger-full"),
*/
};

static struct gpio_led_platform_data imx6q_gpio_leds_data = {
	.leds		= imx6q_gpio_leds,
	.num_leds	= ARRAY_SIZE(imx6q_gpio_leds),
};

static struct platform_device imx6q_gpio_led_device = {
	.name		= "leds-gpio",
	.id		= -1,
	.num_resources  = 0,
	.dev		= {
		.platform_data = &imx6q_gpio_leds_data,
	}
};

/* For BT_PWD_L is conflict with charger's LED trigger gpio on topeet_revC.
 * add mutual exclusion here to be decided which one to be used by board config
 */
static void __init imx6q_add_device_gpio_leds(void)
{
	if (!uart5_enabled)
		platform_device_register(&imx6q_gpio_led_device);
}
#else
static void __init imx6q_add_device_gpio_leds(void) {}
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#define GPIO_BUTTON(gpio_num, ev_code, act_low, descr, wake, debounce)	\
{								\
	.gpio		= gpio_num,				\
	.type		= EV_KEY,				\
	.code		= ev_code,				\
	.active_low	= act_low,				\
	.desc		= "btn " descr,				\
	.wakeup		= wake,					\
	.debounce_interval = debounce,				\
}

static struct gpio_keys_button topeet_buttons[] = {
        GPIO_BUTTON(TOPEET_VOLUME_UP, KEY_VOLUMEUP, 1, "volume-up", 0, 1),
        GPIO_BUTTON(TOPEET_VOLUME_DN, KEY_POWER, 1, "volume-down", 1, 1),
};

static struct gpio_keys_platform_data topeet_button_data = {
        .buttons	= topeet_buttons,
        .nbuttons	= ARRAY_SIZE(topeet_buttons),
};

static struct gpio_keys_button new_topeet_buttons[] = {
        GPIO_BUTTON(TOPEET_VOLUME_UP, KEY_VOLUMEUP, 1, "volume-up", 0, 1),
        GPIO_BUTTON(TOPEET_VOLUME_DN, KEY_VOLUMEDOWN, 1, "volume-down", 0, 1),
	GPIO_BUTTON(TOPEET_BACK, KEY_BACK, 1, "key-back", 0, 1),
        GPIO_BUTTON(TOPEET_POWER_OFF, KEY_POWER, 1, "power-key", 1, 1),
};

static struct gpio_keys_platform_data new_topeet_button_data = {
        .buttons	= new_topeet_buttons,
        .nbuttons	= ARRAY_SIZE(new_topeet_buttons),
};

static struct platform_device topeet_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources  = 0,
};

static void __init imx6q_add_device_buttons(void)
{
	/* fix me */
        /* For new topeet(RevB4 ane above) change the
	 * ONOFF key(SW1) design, the SW1 now connect
	 * to GPIO_3_29, it can be use as a general power
         * key that Android reuired. But those old topeet
	 * such as RevB or older could not support this
	 * change, so it needs a way to distinguish different
	 * boards. Before board id/rev are defined cleary,
	 * there is a simple way to achive this, that is using
	 * SOC revison to identify differnt board revison.
	 *
         * With the new topeet change and SW mapping the
	 * SW1 as power key, below function related to power
         * key are OK on new topeet board(B4 or above).
	 * 	1 Act as power button to power on the device when device is power off
	 * 	2 Act as power button to power on the device(need keep press SW1 >5s)
	 *	3 Act as power key to let device suspend/resume
	 *	4 Act screenshort(hold power key and volume down key for 2s)
	 */
	if (mx6q_revision() >= IMX_CHIP_REVISION_1_2 ||
			mx6dl_revision() >= IMX_CHIP_REVISION_1_1)
                platform_device_add_data(&topeet_button_device,
                                &new_topeet_button_data,
                                sizeof(new_topeet_button_data));
	else
                platform_device_add_data(&topeet_button_device,
                                &topeet_button_data,
                                sizeof(topeet_button_data));

        platform_device_register(&topeet_button_device);
}
#else
static void __init imx6q_add_device_buttons(void) {}
#endif

static struct platform_pwm_backlight_data mx6_topeet_pwm_backlight_data = {
	.pwm_id = 0,
	.max_brightness = 248,
	.dft_brightness = 128,
	.pwm_period_ns = 50000,
};

#ifdef CONFIG_HAVE_EPIT
static struct platform_ir_data mx6_topeet_ir_data = {
    .pwm_id = 1,
    .epit_id = 0,
    .gpio_id = 0,
};
#endif

static struct mxc_dvfs_platform_data topeet_dvfscore_data = {
	.reg_id = "VDDCORE",
	.soc_id	= "VDDSOC",
	.clk1_id = "cpu_clk",
	.clk2_id = "gpc_dvfs_clk",
	.gpc_cntr_offset = MXC_GPC_CNTR_OFFSET,
	.ccm_cdcr_offset = MXC_CCM_CDCR_OFFSET,
	.ccm_cacrr_offset = MXC_CCM_CACRR_OFFSET,
	.ccm_cdhipr_offset = MXC_CCM_CDHIPR_OFFSET,
	.prediv_mask = 0x1F800,
	.prediv_offset = 11,
	.prediv_val = 3,
	.div3ck_mask = 0xE0000000,
	.div3ck_offset = 29,
	.div3ck_val = 2,
	.emac_val = 0x08,
	.upthr_val = 25,
	.dnthr_val = 9,
	.pncthr_val = 33,
	.upcnt_val = 10,
	.dncnt_val = 10,
	.delay_time = 80,
};

static void __init fixup_mxc_board(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
	char *str;
	struct tag *t;
	int i = 0;
        struct ipuv3_fb_platform_data *pdata_fb = topeet_fb_data;

	for_each_tag(t, tags) {
		if (t->hdr.tag == ATAG_CMDLINE) {
			str = t->u.cmdline.cmdline;
			str = strstr(str, "fbmem=");
			if (str != NULL) {
				str += 6;
				pdata_fb[i++].res_size[0] = memparse(str, &str);
				while (*str == ',' &&
                                        i < ARRAY_SIZE(topeet_fb_data)) {
					str++;
					pdata_fb[i++].res_size[0] = memparse(str, &str);
				}
			}
			/* ION reserved memory */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "ionmem=");
			if (str != NULL) {
				str += 7;
				imx_ion_data.heaps[0].size = memparse(str, &str);
			}
			/* Primary framebuffer base address */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "fb0base=");
			if (str != NULL) {
				str += 8;
				pdata_fb[0].res_base[0] =
						simple_strtol(str, &str, 16);
			}
			/* GPU reserved memory */
			str = t->u.cmdline.cmdline;
			str = strstr(str, "gpumem=");
			if (str != NULL) {
				str += 7;
				imx6q_gpu_pdata.reserved_mem_size = memparse(str, &str);
			}
			break;
		}
	}
}

static struct mipi_csi2_platform_data mipi_csi2_pdata = {
	.ipu_id	 = 0,
	.csi_id = 1,
	.v_channel = 0,
	.lanes = 2,
	.dphy_clk = "mipi_pllref_clk",
	.pixel_clk = "emi_clk",
};

static int __init caam_setup(char *__unused)
{
	caam_enabled = 1;
	return 1;
}
early_param("caam", caam_setup);

#define SNVS_LPCR 0x38
static void mx6_snvs_poweroff(void)
{

	void __iomem *mx6_snvs_base =  MX6_IO_ADDRESS(MX6Q_SNVS_BASE_ADDR);
	u32 value;
	value = readl(mx6_snvs_base + SNVS_LPCR);
	/*set TOP and DP_EN bit*/
	writel(value | 0x60, mx6_snvs_base + SNVS_LPCR);
}

static const struct imx_pcie_platform_data mx6_topeet_pcie_data __initconst = {
        .pcie_pwr_en	= TOPEET_PCIE_PWR_EN,
        .pcie_rst	= TOPEET_PCIE_RST_B_REVB,
        .pcie_wake_up	= TOPEET_PCIE_WAKE_B,
        .pcie_dis	= TOPEET_PCIE_DIS_B,
#ifdef CONFIG_IMX_PCIE_EP_MODE_IN_EP_RC_SYS
	.type_ep	= 1,
#else
	.type_ep	= 0,
#endif
	.pcie_power_always_on = 1,
};

#ifdef CONFIG_ANDROID_RAM_CONSOLE
static struct resource ram_console_resource = {
	.name = "android ram console",
	.flags = IORESOURCE_MEM,
};

static struct platform_device android_ram_console = {
	.name = "ram_console",
	.num_resources = 1,
	.resource = &ram_console_resource,
};

static int __init imx6x_add_ram_console(void)
{
	return platform_device_register(&android_ram_console);
}
#else
#define imx6x_add_ram_console() do {} while (0)
#endif

static iomux_v3_cfg_t mx6q_uart5_pads[] = {
	MX6Q_PAD_KEY_ROW1__UART5_RXD,
	MX6Q_PAD_KEY_COL1__UART5_TXD,
	MX6Q_PAD_KEY_COL4__UART5_RTS,
	MX6Q_PAD_KEY_ROW4__UART5_CTS,
	/* gpio for reset */
	MX6Q_PAD_GPIO_2__GPIO_1_2,
};

static iomux_v3_cfg_t mx6dl_uart5_pads[] = {
	MX6DL_PAD_KEY_ROW1__UART5_RXD,
	MX6DL_PAD_KEY_COL1__UART5_TXD,
	MX6DL_PAD_KEY_COL4__UART5_RTS,
	MX6DL_PAD_KEY_ROW4__UART5_CTS,
	/* gpio for reset */
	MX6DL_PAD_GPIO_2__GPIO_1_2,
};
static int __init uart5_setup(char * __unused)
{
	uart5_enabled = 1;
	return 1;
}
__setup("bluetooth", uart5_setup);

static void __init uart5_init(void)
{
	printk(KERN_INFO "uart5 is added\n");
	if (cpu_is_mx6q())
		mxc_iomux_v3_setup_multiple_pads(mx6q_uart5_pads,
				ARRAY_SIZE(mx6q_uart5_pads));
	else if (cpu_is_mx6dl())
		mxc_iomux_v3_setup_multiple_pads(mx6dl_uart5_pads,
				ARRAY_SIZE(mx6dl_uart5_pads));
	imx6q_add_imx_uart(4, &mx6q_sd_uart5_data);
}

/* add by cym 20160708 */
#if defined(CONFIG_MTK_COMBO_MT66XX)
#include <linux/combo_mt66xx.h>

void setup_mt6620_wlan_power_for_onoff(int on)
{

    int chip_pwd_low_val;
    int outValue;

    printk("[mt6620] +++ %s : wlan power %s\n",__func__, on?"on":"off");
#if 1
	gpio_request(CFG_IO_MT6620_CD_PIN, "CFG_IO_MT6620_CD_PIN");
	gpio_direction_input(CFG_IO_MT6620_CD_PIN);
    int value_before = gpio_get_value(CFG_IO_MT6620_CD_PIN);
    printk("[mt6620] --- %s---CFG_IO_MT6620_CD_PIN  first is %d\n",__func__,value_before);
    msleep(100);

    if (on) {
        outValue = 0;
    } else {
        outValue = 1;
    }
	gpio_request(CFG_IO_MT6620_TRIGGER_PIN, "CFG_IO_MT6620_TRIGGER_PIN");
	gpio_direction_output(CFG_IO_MT6620_TRIGGER_PIN, outValue);

    msleep(100);



   int value = gpio_get_value(CFG_IO_MT6620_CD_PIN);
   // int value = nxp_soc_gpio_get_in_value(CFG_SDMMC0_DETECT_IO);

	gpio_free(CFG_IO_MT6620_CD_PIN);
	gpio_free(CFG_IO_MT6620_TRIGGER_PIN);

    printk("[mt6620] --- %s---CFG_IO_MT6620_CD_PIN  second is %d\n",__func__,value);
#endif
    printk("[mt6620] --- %s\n",__func__);

}
EXPORT_SYMBOL(setup_mt6620_wlan_power_for_onoff);

static struct mtk_wmt_platform_data mtk_wmt_pdata = {
#if 1
    .pmu = CFG_IO_MT6620_POWER_PIN,  //EXYNOS4_GPC1(0), //RK30SDK_WIFI_GPIO_POWER_N,//RK30_PIN0_PB5, //MUST set to pin num in target system
    .rst =  CFG_IO_MT6620_SYSRST_PIN, //EXYNOS4_GPC1(1),//RK30SDK_WIFI_GPIO_RESET_N,//RK30_PIN3_PD0, //MUST set to pin num in target system
    .bgf_int = CFG_IO_MT6620_BGF_INT_PIN, // EXYNOS4_GPX2(4), //IRQ_EINT(20),//RK30SDK_WIFI_GPIO_BGF_INT_B,//RK30_PIN0_PA5,//MUST set to pin num in target system if use UART interface.
#endif
    .urt_cts = -EINVAL, // set it to the correct GPIO num if use common SDIO, otherwise set it to -EINVAL.
    .rtc = -EINVAL, //Optipnal. refer to HW design.
    .gps_sync = -EINVAL, //Optional. refer to HW design.
    .gps_lna = -EINVAL, //Optional. refer to HW design.
};
static struct mtk_sdio_eint_platform_data mtk_sdio_eint_pdata = {
   // .sdio_eint = EXYNOS4_GPX2(5),//IRQ_EINT(21) ,//RK30SDK_WIFI_GPIO_WIFI_INT_B,//53, //MUST set pin num in target system.
     .sdio_eint = CFG_IO_MT6620_WIFI_INT_PIN,
};
static struct platform_device mtk_wmt_dev = {
    .name = "mtk_wmt",
    .id = 1,
    .dev = {


        .platform_data = &mtk_wmt_pdata,
    },
};
static struct platform_device mtk_sdio_eint_dev = {
    .name = "mtk_sdio_eint",
    .id = 1,
    .dev = {
        .platform_data = &mtk_sdio_eint_pdata,
    },
};
static void __init mtk_combo_init(void)
{

    unsigned int power_io = CFG_IO_MT6620_POWER_PIN;//CFG_IO_MT6620_POWER_ENABLE;
    unsigned int reset_io = CFG_IO_MT6620_SYSRST_PIN;//CFG_IO_MT6620_SYSRST;
    unsigned int wifi_interrupt_io = CFG_IO_MT6620_WIFI_INT_PIN;//CFG_IO_MT6620_WIFI_INT;
    unsigned int bga_interrupt_io  =  CFG_IO_MT6620_BGF_INT_PIN;//CFG_IO_MT6620_BGF_INT;
    unsigned int carddetect_io = CFG_IO_MT6620_CD_PIN;//CFG_IO_MT6620_CD;
    unsigned int trigger_io = CFG_IO_MT6620_TRIGGER_PIN;//CFG_IO_MT6620_TRIGGER;


    /* Power Enable  signal init*/
	gpio_request(power_io, "power_io");
	gpio_direction_output(power_io, 0);
	gpio_free(power_io);
    //gpio_set_value(power_io, 0);
    //nxp_soc_gpio_set_io_dir(power_io, 1);
    //nxp_soc_gpio_set_io_func(power_io, nxp_soc_gpio_get_altnum(power_io));



    /* SYSRST  signal init*/
	gpio_request(reset_io, "reset_io");
        gpio_direction_output(reset_io, 0);
        gpio_free(reset_io);
    //gpio_set_value(reset_io, 0);
    //nxp_soc_gpio_set_io_dir(reset_io, 1);
    //nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(reset_io));


    mdelay(5);

#if 1
    //need config eint models for Wifi & BGA Interrupt
	gpio_request(wifi_interrupt_io, "wifi_interrupt_io");
	gpio_direction_input(wifi_interrupt_io);
	gpio_free(wifi_interrupt_io);
    //nxp_soc_gpio_set_io_dir(wifi_interrupt_io, 0);
    //nxp_soc_gpio_set_io_func(wifi_interrupt_io, nxp_soc_gpio_get_altnum(wifi_interrupt_io));


	gpio_request(bga_interrupt_io, "bga_interrupt_io");
	gpio_direction_input(bga_interrupt_io);
	gpio_free(bga_interrupt_io);
    //nxp_soc_gpio_set_io_dir(bga_interrupt_io, 0);
    //nxp_soc_gpio_set_io_func(bga_interrupt_io, nxp_soc_gpio_get_altnum(bga_interrupt_io));
#endif

    //init trigger pin and cd detect pin
	gpio_request(trigger_io, "trigger_io");
	gpio_direction_output(trigger_io, 1);
	gpio_free(trigger_io);
    //nxp_soc_gpio_set_out_value(trigger_io, 1);
    //nxp_soc_gpio_set_io_dir(trigger_io, 1);
    //nxp_soc_gpio_set_io_func(trigger_io, nxp_soc_gpio_get_altnum(trigger_io));

	gpio_request(carddetect_io, "carddetect_io");
	gpio_direction_input(carddetect_io);
	gpio_free(carddetect_io);
    //nxp_soc_gpio_set_io_dir(carddetect_io, 0);
    //nxp_soc_gpio_set_io_func(carddetect_io, nxp_soc_gpio_get_altnum(carddetect_io));

    return;
}

static int  itop6x_wifi_combo_module_gpio_init(void)
{

    mtk_combo_init();
    platform_device_register(&mtk_wmt_dev);
    platform_device_register(&mtk_sdio_eint_dev);
}

#endif
/* end add */

/* add by cym 20161214 */
#if defined(CONFIG_LEDS_CTL)
struct platform_device leds_plat_device = {
        .name   = "leds_ctl",
        .id             = -1,
};

static int itop6x_leds_init(void)
{
	printk("plat: add device leds\n");
    	platform_device_register(&leds_plat_device);
}
#endif

#if defined(CONFIG_BUZZER_CTL)
struct platform_device buzzer_plat_device = {
        .name   = "buzzer_ctl",
        .id             = -1,
};

static int itop6x_buzzer_init(void)
{
        printk("plat: add device buzzer\n");
        platform_device_register(&buzzer_plat_device);
}
#endif

#if defined(CONFIG_MAX485_CTL)
struct platform_device max485_plat_device = {
        .name   = "max485_ctl",
        .id             = -1,
};

static int itop6x_max485_ctl_init(void)
{
        printk("plat: add device max485_ctl\n");
        platform_device_register(&max485_plat_device);
}
#endif

#ifdef CONFIG_RELAY_CTL
struct platform_device relay_plat_device = {
        .name   = "relay_ctl",
        .id             = -1,
};

static int itop6x_relay_init(void)
{
        printk("plat: add device relay\n");
        platform_device_register(&relay_plat_device);
}
#endif
/* end add */

/*!
 * Board specific initialization.
 */
static void __init mx6_topeet_board_init(void)
{
	int i;
	int ret;
	struct clk *clko, *clko2;
	struct clk *new_parent;
	int rate;
	struct platform_device *voutdev;

	if (cpu_is_mx6q()) {
                mxc_iomux_v3_setup_multiple_pads(mx6q_topeet_pads,
                        ARRAY_SIZE(mx6q_topeet_pads));
		if (enet_to_gpio_6) {
			iomux_v3_cfg_t enet_gpio_pad =
				MX6Q_PAD_GPIO_6__ENET_IRQ_TO_GPIO_6;
			mxc_iomux_v3_setup_pad(enet_gpio_pad);
		} else {
			iomux_v3_cfg_t i2c3_pad =
				MX6Q_PAD_GPIO_6__I2C3_SDA;
			mxc_iomux_v3_setup_pad(i2c3_pad);
		}
	} else if (cpu_is_mx6dl()) {
                mxc_iomux_v3_setup_multiple_pads(mx6dl_topeet_pads,
                        ARRAY_SIZE(mx6dl_topeet_pads));

		if (enet_to_gpio_6) {
			iomux_v3_cfg_t enet_gpio_pad =
				MX6DL_PAD_GPIO_6__ENET_IRQ_TO_GPIO_6;
			mxc_iomux_v3_setup_pad(enet_gpio_pad);
		} else {
			iomux_v3_cfg_t i2c3_pad =
				MX6DL_PAD_GPIO_6__I2C3_SDA;
			mxc_iomux_v3_setup_pad(i2c3_pad);
		}
	}


#ifdef CONFIG_FEC_1588
	/* Set GPIO_16 input for IEEE-1588 ts_clk and RMII reference clock
	 * For MX6 GPR1 bit21 meaning:
	 * Bit21:       0 - GPIO_16 pad output
	 *              1 - GPIO_16 pad input
	 */
	 mxc_iomux_set_gpr_register(1, 21, 1, 1);
#endif

        gp_reg_id = topeet_dvfscore_data.reg_id;
        soc_reg_id = topeet_dvfscore_data.soc_id;
        mx6q_topeet_init_uart();
	imx6x_add_ram_console();

	/*add bt support*/
	if (uart5_enabled) {
		uart5_init();
		mxc_register_device(&mxc_bt_rfkill, &mxc_bt_rfkill_data);
	}
	/*
	 * MX6DL/Solo only supports single IPU
	 * The following codes are used to change ipu id
	 * and display id information for MX6DL/Solo. Then
	 * register 1 IPU device and up to 2 displays for
	 * MX6DL/Solo
	 */
	if (cpu_is_mx6dl()) {
		ldb_data.ipu_id = 0;
		ldb_data.disp_id = 1;
		hdmi_core_data.ipu_id = 0;
		hdmi_core_data.disp_id = 0;
		mipi_dsi_pdata.ipu_id = 0;
		mipi_dsi_pdata.disp_id = 1;
		ldb_data.sec_ipu_id = 0;
	}
	imx6q_add_mxc_hdmi_core(&hdmi_core_data);

	imx6q_add_ipuv3(0, &ipu_data[0]);
	if (cpu_is_mx6q()) {
		imx6q_add_ipuv3(1, &ipu_data[1]);
                for (i = 0; i < 4 && i < ARRAY_SIZE(topeet_fb_data); i++)
                        imx6q_add_ipuv3fb(i, &topeet_fb_data[i]);
	} else
                for (i = 0; i < 2 && i < ARRAY_SIZE(topeet_fb_data); i++)
                        imx6q_add_ipuv3fb(i, &topeet_fb_data[i]);

	imx6q_add_vdoa();
	imx6q_add_mipi_dsi(&mipi_dsi_pdata);
	imx6q_add_lcdif(&lcdif_data);
	imx6q_add_ldb(&ldb_data);
	voutdev = imx6q_add_v4l2_output(0);
	if (vout_mem.res_msize && voutdev) {
		dma_declare_coherent_memory(&voutdev->dev,
					    vout_mem.res_mbase,
					    vout_mem.res_mbase,
					    vout_mem.res_msize,
					    (DMA_MEMORY_MAP |
					     DMA_MEMORY_EXCLUSIVE));
	}

	imx6q_add_v4l2_capture(0, &capture_data[0]);
	imx6q_add_v4l2_capture(1, &capture_data[1]);
	imx6q_add_mipi_csi2(&mipi_csi2_pdata);
	imx6q_add_imx_snvs_rtc();

	if (1 == caam_enabled)
		imx6q_add_imx_caam();

	if (board_is_mx6_reva()) {
		strcpy(mxc_i2c0_board_info[0].type, "wm8958");
		mxc_i2c0_board_info[0].platform_data = &wm8958_config_data;
	} else {
#ifdef CONFIG_SND_SOC_WM8962
		strcpy(mxc_i2c0_board_info[0].type, "wm8962");
		mxc_i2c0_board_info[0].platform_data = &wm8962_config_data;
#endif
	}

/* remove by cym 20170927 */
#if 0
	imx6q_add_device_gpio_leds();
#endif
/* end remove */

        imx6q_add_imx_i2c(0, &mx6q_topeet_i2c_data);
        imx6q_add_imx_i2c(1, &mx6q_topeet_i2c_data);
        imx6q_add_imx_i2c(2, &mx6q_topeet_i2c_data);
	if (cpu_is_mx6dl())
                imx6q_add_imx_i2c(3, &mx6q_topeet_i2c_data);
	i2c_register_board_info(0, mxc_i2c0_board_info,
			ARRAY_SIZE(mxc_i2c0_board_info));
	i2c_register_board_info(1, mxc_i2c1_board_info,
			ARRAY_SIZE(mxc_i2c1_board_info));
	i2c_register_board_info(2, mxc_i2c2_board_info,
			ARRAY_SIZE(mxc_i2c2_board_info));
#if 0	//remove by cym 20160722
        ret = gpio_request(TOPEET_PFUZE_INT, "pFUZE-int");
	if (ret) {
		printk(KERN_ERR"request pFUZE-int error!!\n");
		return;
	} else {
                gpio_direction_input(TOPEET_PFUZE_INT);
                mx6q_sabresd_init_pfuze100(TOPEET_PFUZE_INT);
	}
#endif

/* add by cym 20161214 */
#if 1
#define TOPEET_EC20_RESET IMX_GPIO_NR(6, 11)

        gpio_set_value(TOPEET_EC20_RESET, 1);
        mdelay(20);
        gpio_set_value(TOPEET_EC20_RESET, 0);
        mdelay(30);
        gpio_set_value(TOPEET_EC20_RESET, 1);
#endif
/* end add */

	/* SPI */
        imx6q_add_ecspi(0, &mx6q_topeet_spi_data);
	/* add by cym 20170809 */
	imx6q_add_ecspi(1, &mx6q_topeet_spi2_data);
	/* end add */
	spi_device_init();

	imx6q_add_mxc_hdmi(&hdmi_data);

        imx6q_add_anatop_thermal_imx(1, &mx6q_topeet_anatop_thermal_data);

	if (enet_to_gpio_6)
		/* Make sure the IOMUX_OBSRV_MUX1 is set to ENET_IRQ. */
		mxc_iomux_set_specialbits_register(
			IOMUX_OBSRV_MUX1_OFFSET,
			OBSRV_MUX1_ENET_IRQ,
			OBSRV_MUX1_MASK);
	else
		fec_data.gpio_irq = -1;
	imx6_init_fec(fec_data);

        imx6q_add_pm_imx(0, &mx6q_topeet_pm_data);

	/* Move sd4 to first because sd4 connect to emmc.
	   Mfgtools want emmc is mmcblk0 and other sd card is mmcblk1.
	*/
        imx6q_add_sdhci_usdhc_imx(3, &mx6q_topeet_sd4_data);
        imx6q_add_sdhci_usdhc_imx(2, &mx6q_topeet_sd3_data);
        imx6q_add_sdhci_usdhc_imx(1, &mx6q_topeet_sd2_data);
	imx_add_viv_gpu(&imx6_gpu_data, &imx6q_gpu_pdata);
        imx6q_topeet_init_usb();
	/* SATA is not supported by MX6DL/Solo */
	if (cpu_is_mx6q()) {
#ifdef CONFIG_SATA_AHCI_PLATFORM
                imx6q_add_ahci(0, &mx6q_topeet_sata_data);
#else
                mx6q_topeet_sata_init(NULL,
			(void __iomem *)ioremap(MX6Q_SATA_BASE_ADDR, SZ_4K));
#endif
	}
	imx6q_add_vpu();
	imx6q_init_audio();
        platform_device_register(&topeet_vmmc_reg_devices);
	imx_asrc_data.asrc_core_clk = clk_get(NULL, "asrc_clk");
	imx_asrc_data.asrc_audio_clk = clk_get(NULL, "asrc_serial_clk");
	imx6q_add_asrc(&imx_asrc_data);

	/*
	 * Disable HannStar touch panel CABC function,
	 * this function turns the panel's backlight automatically
	 * according to the content shown on the panel which
	 * may cause annoying unstable backlight issue.
	 */
        gpio_request(TOPEET_CABC_EN0, "cabc-en0");
        gpio_direction_output(TOPEET_CABC_EN0, 0);
        gpio_request(TOPEET_CABC_EN1, "cabc-en1");
        gpio_direction_output(TOPEET_CABC_EN1, 0);
	gpio_free(TOPEET_CABC_EN0);
	gpio_free(TOPEET_CABC_EN1);

#ifdef CONFIG_HAVE_EPIT
	imx6q_add_mxc_epit(0);
	imx6q_add_mxc_epit(1);
#endif

	imx6q_add_mxc_pwm(0);
	imx6q_add_mxc_pwm(1);
	imx6q_add_mxc_pwm(2);
	imx6q_add_mxc_pwm(3);
        imx6q_add_mxc_pwm_backlight(0, &mx6_topeet_pwm_backlight_data);

#ifdef CONFIG_MX6_IR
	/* add MXC IR device */
        imx6q_add_mxc_ir(0, &mx6_topeet_ir_data);
#endif

	imx6q_add_otp();
	imx6q_add_viim();
	imx6q_add_imx2_wdt(0, NULL);
	imx6q_add_dma();

        imx6q_add_dvfs_core(&topeet_dvfscore_data);

	if (imx_ion_data.heaps[0].size)
		imx6q_add_ion(0, &imx_ion_data,
			sizeof(imx_ion_data) + sizeof(struct ion_platform_heap));

	imx6q_add_device_buttons();

	/* enable sensor 3v3 and 1v8 */
        gpio_request(TOPEET_SENSOR_EN, "sensor-en");
        gpio_direction_output(TOPEET_SENSOR_EN, 1);

	/* enable accel intr */
        gpio_request(TOPEET_ACCL_INT, "accel-int");
        gpio_direction_input(TOPEET_ACCL_INT);

	/* enable ecompass intr */
    //<<
    //    gpio_request(TOPEET_eCOMPASS_INT, "ecompass-int");
    //    gpio_direction_input(TOPEET_eCOMPASS_INT);
    //>> //Closed by Caesar.

	/* enable light sensor intr */
        gpio_request(TOPEET_ALS_INT, "als-int");
        gpio_direction_input(TOPEET_ALS_INT);

	imx6q_add_hdmi_soc();
	imx6q_add_hdmi_soc_dai();

	if (cpu_is_mx6dl()) {
		imx6dl_add_imx_pxp();
		imx6dl_add_imx_pxp_client();
		if (epdc_enabled) {
			mxc_register_device(&max17135_sensor_device, NULL);
			imx6dl_add_imx_epdc(&epdc_data);
		}
	}
#if 1
        ret = gpio_request_array(mx6_flexcan_gpios,
                        ARRAY_SIZE(mx6_flexcan_gpios));
	if (ret)
		pr_err("failed to request flexcan1-gpios: %d\n", ret);
	else{
                imx6q_add_flexcan0(&mx6q_topeet_flexcan_pdata[0]);
                imx6q_add_flexcan1(&mx6q_topeet_flexcan_pdata[1]);
	}
#endif

	clko2 = clk_get(NULL, "clko2_clk");
	if (IS_ERR(clko2))
		pr_err("can't get CLKO2 clock.\n");

	new_parent = clk_get(NULL, "osc_clk");
	if (!IS_ERR(new_parent)) {
		clk_set_parent(clko2, new_parent);
		clk_put(new_parent);
	}
	rate = clk_round_rate(clko2, 24000000);
	clk_set_rate(clko2, rate);
	clk_enable(clko2);

	/* Camera and audio use osc clock */
	clko = clk_get(NULL, "clko_clk");
	if (!IS_ERR(clko))
		clk_set_parent(clko, clko2);

	/* Enable Aux_5V */
        gpio_request(TOPEET_AUX_5V_EN, "aux_5v_en");
        gpio_direction_output(TOPEET_AUX_5V_EN, 1);
        gpio_set_value(TOPEET_AUX_5V_EN, 1);

	gps_power_on(true);
	/* Register charger chips */
        platform_device_register(&topeet_max8903_charger_1);
	pm_power_off = mx6_snvs_poweroff;
	imx6q_add_busfreq();

	/* Add PCIe RC interface support
	 * uart5 has pin mux with pcie. or you will use uart5 or use pcie
	 */
	if (!uart5_enabled)
                imx6q_add_pcie(&mx6_topeet_pcie_data);
	if (cpu_is_mx6dl()) {
		mxc_iomux_v3_setup_multiple_pads(mx6dl_arm2_elan_pads,
						ARRAY_SIZE(mx6dl_arm2_elan_pads));

		/* ELAN Touchscreen */
                gpio_request(TOPEET_ELAN_INT, "elan-interrupt");
                gpio_direction_input(TOPEET_ELAN_INT);

                gpio_request(TOPEET_ELAN_CE, "elan-cs");
                gpio_direction_output(TOPEET_ELAN_CE, 1);
                gpio_direction_output(TOPEET_ELAN_CE, 0);

                gpio_request(TOPEET_ELAN_RST, "elan-rst");
                gpio_direction_output(TOPEET_ELAN_RST, 1);
                gpio_direction_output(TOPEET_ELAN_RST, 0);
		mdelay(1);
                gpio_direction_output(TOPEET_ELAN_RST, 1);
                gpio_direction_output(TOPEET_ELAN_CE, 1);
	}

	imx6_add_armpmu();
	imx6q_add_perfmon(0);
	imx6q_add_perfmon(1);
	imx6q_add_perfmon(2);

/* add by cym 20160712 */
#if defined(CONFIG_MTK_COMBO_MT66XX)
        itop6x_wifi_combo_module_gpio_init();
#endif
/* end add */

/* add by cym 20161214 */
#if defined(CONFIG_LEDS_CTL)
	itop6x_leds_init();
#endif

#if defined(CONFIG_BUZZER_CTL)
	itop6x_buzzer_init();
#endif

#if defined(CONFIG_MAX485_CTL)
	itop6x_max485_ctl_init();
#endif

#ifdef CONFIG_RELAY_CTL
	itop6x_relay_init();
#endif
/* end add */

/* add by cym 20170228 */
#if defined(CONFIG_KEYBOARD_HS0038) || defined(CONFIG_KEYBOARD_HS0038_MODULE)
	platform_device_register(&hs0038_device);
#endif
/* end add */
}

extern void __iomem *twd_base;
static void __init mx6_topeet_timer_init(void)
{
	struct clk *uart_clk;
#ifdef CONFIG_LOCAL_TIMERS
	twd_base = ioremap(LOCAL_TWD_ADDR, SZ_256);
	BUG_ON(!twd_base);
#endif
	mx6_clocks_init(32768, 24000000, 0, 0);

	uart_clk = clk_get_sys("imx-uart.0", NULL);
	early_console_setup(UART1_BASE_ADDR, uart_clk);
}

static struct sys_timer mx6_topeet_timer = {
        .init   = mx6_topeet_timer_init,
};

static void __init mx6q_topeet_reserve(void)
{
	phys_addr_t phys;
	int i, fb0_reserved = 0, fb_array_size;

	/*
	 * Reserve primary framebuffer memory if its base address
	 * is set by kernel command line.
	 */
        fb_array_size = ARRAY_SIZE(topeet_fb_data);
        if (fb_array_size > 0 && topeet_fb_data[0].res_base[0] &&
            topeet_fb_data[0].res_size[0]) {
                if (topeet_fb_data[0].res_base[0] > SZ_2G)
			printk(KERN_INFO"UI Performance downgrade with FB phys address %x!\n",
                            topeet_fb_data[0].res_base[0]);
                memblock_reserve(topeet_fb_data[0].res_base[0],
                                 topeet_fb_data[0].res_size[0]);
                memblock_remove(topeet_fb_data[0].res_base[0],
                                topeet_fb_data[0].res_size[0]);
                topeet_fb_data[0].late_init = true;
		ipu_data[ldb_data.ipu_id].bypass_reset = true;
		fb0_reserved = 1;
	}
	for (i = fb0_reserved; i < fb_array_size; i++)
                if (topeet_fb_data[i].res_size[0]) {
			/* Reserve for other background buffer. */
                        phys = memblock_alloc_base(topeet_fb_data[i].res_size[0],
						SZ_4K, SZ_2G);
                        memblock_remove(phys, topeet_fb_data[i].res_size[0]);
                        topeet_fb_data[i].res_base[0] = phys;
		}

#ifdef CONFIG_ANDROID_RAM_CONSOLE
	phys = memblock_alloc_base(SZ_1M, SZ_4K, SZ_1G);
	memblock_remove(phys, SZ_1M);
	memblock_free(phys, SZ_1M);
	ram_console_resource.start = phys;
	ram_console_resource.end   = phys + SZ_1M - 1;
#endif

#if defined(CONFIG_MXC_GPU_VIV) || defined(CONFIG_MXC_GPU_VIV_MODULE)
	if (imx6q_gpu_pdata.reserved_mem_size) {
		phys = memblock_alloc_base(imx6q_gpu_pdata.reserved_mem_size,
					   SZ_4K, SZ_2G);
		memblock_remove(phys, imx6q_gpu_pdata.reserved_mem_size);
		imx6q_gpu_pdata.reserved_mem_base = phys;
	}
#endif

#if defined(CONFIG_ION)
	if (imx_ion_data.heaps[0].size) {
		phys = memblock_alloc(imx_ion_data.heaps[0].size, SZ_4K);
		memblock_remove(phys, imx_ion_data.heaps[0].size);
		imx_ion_data.heaps[0].base = phys;
	}
#endif

	if (vout_mem.res_msize) {
		phys = memblock_alloc_base(vout_mem.res_msize,
					   SZ_4K, SZ_1G);
		memblock_remove(phys, vout_mem.res_msize);
		vout_mem.res_mbase = phys;
	}
}

/*
 * initialize __mach_desc_MX6Q_topeet data structure.
 */
MACHINE_START(MX6Q_TOPEET, "Freescale i.MX 6Quad  TOPEET Board")
	/* Maintainer: Freescale Semiconductor, Inc. */
	.boot_params = MX6_PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mx6_map_io,
	.init_irq = mx6_init_irq,
        .init_machine = mx6_topeet_board_init,
        .timer = &mx6_topeet_timer,
        .reserve = mx6q_topeet_reserve,
MACHINE_END
