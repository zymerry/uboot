/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the SAMSUNG SMDK2410 board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T			1	/* This is an ARM920T Core	*/
#define	CONFIG_S3C2410			1	/* in a SAMSUNG S3C2410 SoC     */
#define CONFIG_SMDK2410		1	/* on a SAMSUNG SMDK2410 Board  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ		12000000/* the TX2440 has 12MHz input clock */
//#define CONFIG_SYS_CLK_FREQ		16934400 /* the SMDK2410 has 12MHz input clock */

#define USE_920T_MMU			1
#define CONFIG_USE_MMU


#define CONFIG_64MB_Nand		0		//添加了对64MB Nand Flash支持
#define CONFIG_128MB_Nand		0		//添加了对128MB Nand Flash支持，当为1时就是128MB的Nand Flash

/* enable 128MB SDRAM */
#define CONFIG_128MB_SDRAM		0

/* enable SDRAM 133MHz */
#define CONFIG_133MHZ_SDRAM		0

#define CONFIG_JFFS2_CMDLINE		1
#define CONFIG_JFFS2_NAND		1

#define MTDIDS_DEFAULT			"nand0=nandflash0"

#define MTDPARTS_DEFAULT		"mtdparts=nandflash0:1m@0(bios)," \
                           		 "128k(params)," \
                           		 "4m(kernel)," \
                           		 "-(root)"

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE		128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x20000300
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE + 4)
#define CONFIG_DM9000_USE_16BIT

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SMDK2410 */

/************************************************************
 * RTC
 ************************************************************/
#define	CONFIG_RTC_S3C24X0		1


/* enable TX2440 Logo	*/
#define CONFIG_TX2440_LOGO		1

/* set LCD params */
#ifdef CONFIG_TX2440_LOGO

#define TFT800600				1
#define TFT800480				2
#define VGA					3
#define TFT480272				4
#define TFT240320				5
#define W35					6

//#define LCD_TFT					TFT480272
#define LCD_TFT					TFT800480

#if(LCD_TFT == W35)
#define	CONFIG_TX2440_LCD_VBPD		12
#define	CONFIG_TX2440_LCD_VFPD		4
#define	CONFIG_TX2440_LCD_VSPW		5
#define	CONFIG_TX2440_LCD_HBPD		21
#define	CONFIG_TX2440_LCD_HFPD		32
#define	CONFIG_TX2440_LCD_HSPW		44
#define	CONFIG_TX2440_LCD_CLKVAL		6

#elif(LCD_TFT == TFT480272)
#define	CONFIG_TX2440_LCD_VBPD		2
#define	CONFIG_TX2440_LCD_VFPD		4
#define	CONFIG_TX2440_LCD_VSPW		8
#define	CONFIG_TX2440_LCD_HBPD		10
#define	CONFIG_TX2440_LCD_HFPD		19
#define	CONFIG_TX2440_LCD_HSPW		30
#define	CONFIG_TX2440_LCD_CLKVAL		4

#elif(LCD_TFT == TFT800480)
#define	CONFIG_TX2440_LCD_VBPD		5
#define	CONFIG_TX2440_LCD_VFPD		6
#define	CONFIG_TX2440_LCD_VSPW		1
#define	CONFIG_TX2440_LCD_HBPD		28
#define	CONFIG_TX2440_LCD_HFPD		14
#define	CONFIG_TX2440_LCD_HSPW		50
#define	CONFIG_TX2440_LCD_CLKVAL	2

#endif

#endif

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

/* for tag(s) to transfer message to kernel, www.txmcu.com */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_CMDLINE_TAG		1

/***********************************************************
 * Command definition
 ***********************************************************/
#define CONFIG_COMMANDS \
			(CONFIG_CMD_DFL	 | \
			CFG_CMD_CACHE	 | \
			CFG_CMD_PING     | \
			CFG_CMD_JFFS2    | \
			CFG_CMD_NAND	 | \
			/*CFG_CMD_EEPROM |*/ \
			/*CFG_CMD_I2C	 |*/ \
			CFG_CMD_REGINFO | \
		    CFG_CMD_ELF		| \
			CFG_CMD_DATE	| \
			CFG_CMD_USB	 	| \
			/*CFG_CMD_MMC	    |*/ \
			CFG_CMD_FAT)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY		0

#if CONFIG_128MB_SDRAM
#define CONFIG_BOOTARGS		"noinitrd root=/dev/mtdblock2 init=/linuxrc console=ttySAC0 mem=128M"
#else
#define CONFIG_BOOTARGS		"noinitrd root=/dev/mtdblock2 init=/linuxrc console=ttySAC0"
#endif
#define CONFIG_ETHADDR			1a:2b:3c:4d:5e:6f
#define CONFIG_NETMASK			255.255.255.0
#define CONFIG_IPADDR			125.223.119.10
#define CONFIG_SERVERIP			125.223.119.42
/*#define CONFIG_BOOTFILE		"elinos-lart" */
#define CONFIG_BOOTCOMMAND		"nboot 0x32000000 kernel; bootm 0x32000000"

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE		115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX		1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT			"TX-2440> "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE			256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE			(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS			16		/* max number of command args	*/
#define	CFG_BARGSIZE			CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define	CFG_MEMTEST_START		0x30000000	/* memtest works on	*/
#ifdef	CONFIG_128MB_SDRAM
#define	CFG_MEMTEST_END		0x37F00000	/* 127 MB in DRAM	*/
#else
#define	CFG_MEMTEST_END		0x33F00000	/* 63 MB in DRAM	*/
#endif

#undef  CFG_CLKS_IN_HZ				/* everything, incl board info, in Hz */

#ifdef CONFIG_128MB_SDRAM
#define CFG_LOAD_ADDR			0x37000000	/* default load address	*/
#else
#define	CFG_LOAD_ADDR		0x33000000	/* default load address	*/
#endif

/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
#define	CFG_HZ				1562500

/* valid baudrates */
#define CFG_BAUDRATE_TABLE		{ 9600, 19200, 38400, 57600, 115200 }

/* ------------- USB Host (Mass Storage) --------------- */
#if (CONFIG_COMMANDS & CFG_CMD_USB)
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
#define LITTLEENDIAN
//#define CONFIG_SUPPORT_VFAT /* already defined in fat.h */
#endif

/* ------------- USB Slave(Device) --------------- */
#define CONFIG_USB_DEVICE		1

#if (CONFIG_USB_DEVICE || (CONFIG_COMMANDS & CFG_CMD_USB))
#define CONFIG_USE_IRQ			1
#endif

/* ------------- SD/MMC Card -------------*/
#if (CONFIG_COMMANDS & CFG_CMD_MMC)
#define CONFIG_GENERIC_MMC	1
#define CONFIG_MMC_S3C 		1 	/* Enabling the MMC driver */
//#define CONFIG_MMC_TRACE		1	/* define this to printf debug info */
#endif

#if (CONFIG_COMMANDS & CFG_CMD_USB) || \
	(CONFIG_COMMANDS & CFG_CMD_MMC)
#if (CONFIG_COMMANDS & CFG_CMD_FAT)
#define CONFIG_DOS_PARTITION	/* add for support FAT fs, for SD/MMC or USB Host */
#endif
#endif
/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ		(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ		(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1			0x30000000 /* SDRAM Bank #1 */
#if CONFIG_128MB_SDRAM
#define PHYS_SDRAM_1_SIZE		0x08000000 /* 128 MB */
#else
#define PHYS_SDRAM_1_SIZE		0x04000000 /* 64 MB */
#endif

#define PHYS_FLASH_1			0x00000000 /* Flash Bank #1 */

#define CFG_FLASH_BASE			PHYS_FLASH_1

#if CONFIG_64MB_Nand
#define CONFIG_SURPORT_WINCE    1			//因为现在还不支持2K页面的wince
#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#if 0
#define CONFIG_AMD_LV400		1	/* uncomment this if you have a LV400 flash */
#endif
#define CONFIG_AMD_LV800		1	/* uncomment this if you have a LV800 flash */

#define CFG_MAX_FLASH_BANKS		1	/* max number of memory banks */
#ifdef CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE			0x00200000 /* 1MB */
#define CFG_MAX_FLASH_SECT		(19)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR			(CFG_FLASH_BASE + 0x1F0000) /* addr of environment */
#endif
#ifdef CONFIG_AMD_LV400
#define PHYS_FLASH_SIZE			0x00080000 /* 512KB */
#define CFG_MAX_FLASH_SECT		(11)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR			(CFG_FLASH_BASE + 0x070000) /* addr of environment */
#endif

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT		(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT		(5*CFG_HZ) /* Timeout for Flash Write */

//#define	CFG_ENV_IS_IN_FLASH		1
#define CFG_ENV_IS_IN_NAND		1
#define CFG_ENV_OFFSET			0x100000
#define CFG_ENV_SIZE64			0xc000	/* Total Size of Environment Sector */
#define CFG_ENV_SIZE			0x20000	/* Total Size of Environment Sector */

/*-----------------------------------------------------------------------
 * NAND flash settings
 */
#define CFG_NAND_BASE			0
#define CFG_MAX_NAND_DEVICE		1
#define NAND_MAX_CHIPS			1

#endif	/* __CONFIG_H */
