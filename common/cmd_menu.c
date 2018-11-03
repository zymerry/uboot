/*
 * 相广超制作
 *
 */

#include <common.h>
#include <command.h>
#include <def.h>
#include <nand.h>

#ifdef CONFIG_SURPORT_WINCE
#include "../wince/loader.h"
#endif

#include <images_arr_dnw.h>

extern char console_buffer[];
extern int readline (const char *const prompt);
//extern char awaitkey(unsigned long delay, int* error_p);
extern void download_nkbin_to_flash(void);
extern int boot_zImage(ulong from, size_t size);
extern char bLARGEBLOCK;

static char awaitkey(unsigned long delay, int* error_p)
{
 int i;
// char c;
 if (delay == -1)
 {
  while (1)
  {
   if (tstc()) /* we got a key press */
   return getc();
  }
 }
 else
 {
  for (i = 0; i < delay; i++)
  {
   if (tstc()) /* we got a key press */
   return getc();
   udelay (10*1000);
  }
 }
 if (error_p)
 *error_p = -1;

 return 0;
}


/**
 * Parses a string into a number.  The number stored at ptr is
 * potentially suffixed with K (for kilobytes, or 1024 bytes),
 * M (for megabytes, or 1048576 bytes), or G (for gigabytes, or
 * 1073741824).  If the number is suffixed with K, M, or G, then
 * the return value is the number multiplied by one kilobyte, one
 * megabyte, or one gigabyte, respectively.
 *
 * @param ptr where parse begins
 * @param retptr output pointer to next char after parse completes (output)
 * @return resulting unsigned int
 */
static unsigned long memsize_parse2 (const char *const ptr, const char **retptr)
{
	unsigned long ret = simple_strtoul(ptr, (char **)retptr, 0);
	int sixteen = 1;

	switch (**retptr) {
		case 'G':
		case 'g':
			ret <<= 10;
		case 'M':
		case 'm':
			ret <<= 10;
		case 'K':
		case 'k':
			ret <<= 10;
			(*retptr)++;
			sixteen = 0;
		default:
			break;
	}

	if (sixteen)
		return simple_strtoul(ptr, NULL, 16);
	
	return ret;
}


void param_menu_usage()
{
    printf("\r\n##### Parameter Menu #####\r\n");
    printf("[1] 查看参数\r\n");
    printf("[2] 设置参数 \r\n");
    printf("[3] 删除参数 \r\n");
    printf("[4] 写入参数到flash \r\n");
    printf("[5] 返回 \r\n");
	printf("Enter your selection: ");
}


void param_menu_shell(void)
{
	char c;
	char cmd_buf[256];
	char name_buf[20];
	char val_buf[256];
	char param_buf1[25];
	char param_buf2[25];
	char param_buf3[25];
	char param_buf4[64];

	while (1)
	{
		param_menu_usage();
		c = awaitkey(-1, NULL);
		printf("%c\n", c);
		switch (c)
		{
            case '1':
            {
                strcpy(cmd_buf, "printenv ");
                printf("Name(enter to view all paramters): ");
                readline(NULL);
                strcat(cmd_buf, console_buffer);
                run_command(cmd_buf, 0);
                break;
            }
            
            case '2':
            {
                sprintf(cmd_buf, "setenv ");

                printf("Name: ");
                readline(NULL);
                strcat(cmd_buf, console_buffer);

                printf("Value: ");
                readline(NULL);
                strcat(cmd_buf, " ");
                strcat(cmd_buf, console_buffer);

                run_command(cmd_buf, 0);
                break;
            }
            
            case '3':
            {
                sprintf(cmd_buf, "setenv ");

                printf("Name: ");
                readline(NULL);
                strcat(cmd_buf, console_buffer);

                run_command(cmd_buf, 0);
                break;
            }
            
            case '4':
            {
                sprintf(cmd_buf, "saveenv");
                run_command(cmd_buf, 0);
                break;
            }
            
            case '5':
            {
                return;
                break;
            }
		}
	}
}


void erase_menu_usage()
{
	printf("\r\n##### Erase Nand Menu #####\r\n");
	printf("[1] Nand scrub - really clean NAND erasing bad blocks (UNSAFE) \r\n");
	printf("[2] Nand earse - clean NAND eraseing \r\n");
	printf("[q] Return main Menu \r\n");
	printf("Enter your selection: ");
}


void erase_menu_shell(void)
{
	char c;
	char cmd_buf[256];
	char *p = NULL;
	unsigned long size;
	unsigned long offset;
	struct mtd_info *mtd = &nand_info[nand_curr_device];

	while (1)
	{
		erase_menu_usage();
		c = awaitkey(-1, NULL);
		printf("%c\n", c);
		switch (c)
		{
			case '1':
			{
				strcpy(cmd_buf, "nand scrub ");
				run_command(cmd_buf, 0);
				break;
			}

			case '2':
			{
				strcpy(cmd_buf, "nand erase ");

				printf("Start address: ");
				readline(NULL);
				strcat(cmd_buf, console_buffer);

				printf("Size(eg. 4000000, 0x4000000, 64m and so on): ");
				readline(NULL);
				p = console_buffer;
				size = memsize_parse2(p, &p);
				sprintf(console_buffer, " %x", size);
				strcat(cmd_buf, console_buffer);

				run_command(cmd_buf, 0);
				break;
			}

			case 'q':
			{
				return;
				break;
			}
		}
	}
}

// usb 
void usb_menu_usage()
{
	printf("\r\n##### U盘 烧写菜单 #####\r\n");
	printf("[1] 烧写uboot镜像 u-boot.bin\r\n");
    printf("[2] 烧写Linux内核镜像 zImage\r\n");
    printf("[3] 烧写yaffs2文件系统镜像 system.img\r\n");
	printf("[4] 返回到主菜单\r\n");
	printf("Enter your selection: ");
}

void usb_menu_shell(void)
{
	char c;
	char cmd_buf[256];
	char *p = NULL;
	unsigned long size;
	unsigned long offset;
	struct mtd_info *mtd = &nand_info[nand_curr_device];

	strcpy(cmd_buf, "usb start");
	run_command(cmd_buf, 0);

	while (1)
	{
		usb_menu_usage();
		c = awaitkey(-1, NULL);
		printf("%c\n", c);
		switch (c)
		{
			case '1': //u-boot.bin
			{
				strcpy(cmd_buf, "fatload usb 0 0x30000000 u-boot.bin; nand erase bios; nand write.jffs2 0x30000000 bios $(filesize)");
				run_command(cmd_buf, 0);
				break;
			}
			case '2': //zImage
			{
				strcpy(cmd_buf, "fatload usb 0 0x30000000 zImage; nand erase kernel; nand write.jffs2 0x30000000 kernel $(filesize)");
				run_command(cmd_buf, 0);				
				break;
			}
			case '3': //yaffs
			{
				strcpy(cmd_buf, "fatload usb 0 0x30000000 system.img; nand erase root; nand write.yaffs 0x30000000 root $(filesize)");
				run_command(cmd_buf, 0);
				break;
			}
			case '4':
			{
				tx2440_logo();
				usb_stop();
				return;
				break;
			}
		}
	}
}
// usb end
// dnw 
void dnw_menu_usage()
{
	printf("\r\n##### DNW 下载菜单 #####\r\n");
	printf("[1] 下载bootloader或WINCE的STEPLDR.nb1\r\n");
    printf("[2] 下载Linux内核镜像\r\n");
    printf("[3] 下载yaffs2文件系统\r\n");
    printf("[4] 下载cramfs文件系统\r\n");
    printf("[5] 下载到SDRAM中运行\r\n");
 	printf("[6] 下载WINCE的EBOOT(eboot.nb0)\r\n");
	printf("[7] 下载WINCE的开机LOGO(.bin文件)\r\n");	
	if (bBootFrmNORFlash())
		printf("[o] 下载bootloader到NorFlash\r\n");
	printf("[8] 返回到主菜单\r\n");
	printf("Enter your selection: ");
}

#define BACK_COLOR 0x0
#if ( LCD_TFT == TFT480272 )
#define LCD_XSIZE_TFT       (480)
#define LCD_YSIZE_TFT       (272)
#elif ( LCD_TFT == TFT800480 )
#define LCD_XSIZE_TFT       (800)
#define LCD_YSIZE_TFT       (480)
#endif


void dnw_menu_shell(void)
{
	char c;
	char cmd_buf[256];
	char *p = NULL;
	unsigned long size;
	unsigned long offset;
	struct mtd_info *mtd = &nand_info[nand_curr_device];

	int myi =0,myj =80;
	tx2440_lcd_ClearScr_Rectangle( 70, 125, 370, 20, BACK_COLOR);
	for(myi=0;myi<4;myi++){
		#if ( LCD_TFT == TFT480272 )
	    tx2440_Draw_ASCII(myj,125,0xe010,arr_burn_dnw[myi]);
		#elif ( LCD_TFT == TFT800480 )
	    tx2440_Draw_ASCII(myj+100,200,0xe010,arr_burn_dnw[myi]);
		#endif
	    myj += 10;
	}

	#if ( LCD_TFT == TFT480272 )
	tx2440_lcd_ClearScr_Rectangle( 0,50,480,20, BACK_COLOR);
	#elif ( LCD_TFT == TFT800480 )
	tx2440_lcd_ClearScr_Rectangle( 0,100,800,20, BACK_COLOR);
	#endif

	myj = 100;
	for (myi =0; myi<27; myi++){
		#if ( LCD_TFT == TFT480272 )
	    tx2440_Draw_ASCII(myj,50,0x434658,arr_title_dnw[myi]);
		#elif ( LCD_TFT == TFT800480 )
	    tx2440_Draw_ASCII(myj+150,100,0x434658,arr_title_dnw[myi]);
		#endif
	    myj += 10;
	}

	while (1)
	{
		dnw_menu_usage();
		c = awaitkey(-1, NULL);
		printf("%c\n", c);
		switch (c)
		{
			case '1': //download bootloader or STEPLDR.nb1
			{
				#if ( LCD_TFT == TFT480272 )
				tx2440_percent(0, 200, 160);
				tx2440_lcd_ClearScr_Rectangle(0,LCD_YSIZE_TFT - 90,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				tx2440_percent(0, 200, 220);
				tx2440_lcd_ClearScr_Rectangle(0,LCD_YSIZE_TFT - 20,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				#elif ( LCD_TFT == TFT800480 )
				tx2440_percent(0, 350, 230);
				tx2440_lcd_ClearScr_Rectangle(0,260,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				tx2440_percent(0, 350, 300);
				tx2440_lcd_ClearScr_Rectangle(0,330,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				#endif

				#if ( LCD_TFT == TFT480272 )
				tx2440_lcd_ClearScr_Rectangle( 190,125,200, 20, BACK_COLOR);
				#elif ( LCD_TFT == TFT800480 )
				tx2440_lcd_ClearScr_Rectangle( 350,200,200, 20, BACK_COLOR);
				#endif
				myj = 190;
				for(myi=0;myi<10;myi++){
					#if ( LCD_TFT == TFT480272 )
				    tx2440_Draw_ASCII(myj,125,0xe010,arr_bootloader[myi]);
					#elif ( LCD_TFT == TFT800480 )
				    tx2440_Draw_ASCII(myj+160,200,0xe010,arr_bootloader[myi]);
					#endif
				    myj += 10;
				}

				strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase bios; nand write.jffs2 0x30000000 bios $(filesize)");
				run_command(cmd_buf, 0);
				break;
			}

			case '2': //kernel
			{
				#if ( LCD_TFT == TFT480272 )
				tx2440_percent(0, 200, 160);
				tx2440_lcd_ClearScr_Rectangle(0,LCD_YSIZE_TFT - 90,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				tx2440_percent(0, 200, 220);
				tx2440_lcd_ClearScr_Rectangle(0,LCD_YSIZE_TFT - 20,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				#elif ( LCD_TFT == TFT800480 )
				tx2440_percent(0, 350, 230);
				tx2440_lcd_ClearScr_Rectangle(0,260,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				tx2440_percent(0, 350, 300);
				tx2440_lcd_ClearScr_Rectangle(0,330,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				#endif

				myj = 190;
				#if ( LCD_TFT == TFT480272 )
				tx2440_lcd_ClearScr_Rectangle( 190,125,200, 20, BACK_COLOR);
				#elif ( LCD_TFT == TFT800480 )
				tx2440_lcd_ClearScr_Rectangle( 350,200,200, 20, BACK_COLOR);
				#endif
				for(myi=0;myi<6;myi++){
					#if ( LCD_TFT == TFT480272 )
				    tx2440_Draw_ASCII(myj,125,0xe010,arr_kernel[myi]);
					#elif ( LCD_TFT == TFT800480 )
				    tx2440_Draw_ASCII(myj+160,200,0xe010,arr_kernel[myi]);
					#endif
				    myj += 10;
				}

				strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase kernel; nand write.jffs2 0x30000000 kernel $(filesize)");
				run_command(cmd_buf, 0);				
				break;
			}

			case '3': //yaffs
			{
				#if ( LCD_TFT == TFT480272 )
				tx2440_percent(0, 200, 160);
				tx2440_lcd_ClearScr_Rectangle(0,LCD_YSIZE_TFT - 90,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				tx2440_percent(0, 200, 220);
				tx2440_lcd_ClearScr_Rectangle(0,LCD_YSIZE_TFT - 20,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				#elif ( LCD_TFT == TFT800480 )
				tx2440_percent(0, 350, 230);
				tx2440_lcd_ClearScr_Rectangle(0,260,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				tx2440_percent(0, 350, 300);
				tx2440_lcd_ClearScr_Rectangle(0,330,LCD_XSIZE_TFT,10, 0x1F/*blue*/);
				#endif

				myj = 190;
				#if ( LCD_TFT == TFT480272 )
				tx2440_lcd_ClearScr_Rectangle( 190,125,200, 20, BACK_COLOR);
				#elif ( LCD_TFT == TFT800480 )
				tx2440_lcd_ClearScr_Rectangle( 350,200,200, 20, BACK_COLOR);
				#endif
				for(myi=0;myi<6;myi++){
					#if ( LCD_TFT == TFT480272 )
				    tx2440_Draw_ASCII(myj,125,0xe010,arr_system_dnw[myi]);
					#elif ( LCD_TFT == TFT800480 )
				    tx2440_Draw_ASCII(myj+160,200,0xe010,arr_system_dnw[myi]);
					#endif
				    myj += 10;
				}

				strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase root; nand write.yaffs 0x30000000 root $(filesize)");
				run_command(cmd_buf, 0);
				break;
			}

			case '4': //cramfs
			{
				strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase root; nand write.jffs2 0x30000000 root $(filesize)");
				run_command(cmd_buf, 0);
				break;
			}

			case '5':  //sdram
			{
				/*extern*/ volatile U32 downloadAddress;
				/*extern*/ int download_run;
				
				download_run = 1;
				strcpy(cmd_buf, "usbslave 1 0x30000000");
				run_command(cmd_buf, 0);
				download_run = 0;
				sprintf(cmd_buf, "go %x", downloadAddress);
				run_command(cmd_buf, 0);
				break;
			}			

			case '6': //EBOOT
			{
				strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase 0x80000 0x80000; nand write.jffs2 0x30000000 0x80000 $(filesize)");
				run_command(cmd_buf, 0);				
				break;
			}

			case '7': //WINCE LOGO
			{
				strcpy(cmd_buf, "usbslave 1 0x30000000; nand erase 0x100000 0x100000; nand write.jffs2 0x30000000 0x100000 $(filesize)");
				run_command(cmd_buf, 0);				
				break;
			}
			
			case 'O':
			case 'o':  //download uboot to NOR
			{
				if (bBootFrmNORFlash())
				{
					strcpy(cmd_buf, "usbslave 1 0x30000000; protect off all; erase 0 +$(filesize); cp.b 0x30000000 0 $(filesize)");
					run_command(cmd_buf, 0);
				}
				break;
			}
			
			case '8':
			{
				tx2440_logo();
				return;
				break;
			}
		}
	}
}
// dnw end



void lcd_menu_usage()
{
	printf("\r\n##### LCD Parameters Menu #####\r\n");
#if USER_SET
	printf("[1] LCD TYPE   - Set LCD TYPE \r\n");
	printf("[2] VBPD       - Set VBPD \r\n");
	printf("[3] VFPD       - Set VFPD \r\n");
	printf("[4] VSPW       - Set VSPW \r\n");
	printf("[5] HBPD       - Set HBPD \r\n");
	printf("[6] HFPD       - Set HFPD \r\n");
	printf("[7] HSPW       - Set HSPW \r\n");
	printf("[8] CLKVAL     - Set CLKVAL \r\n");
#else
	printf("[1] VBPD       - Set VBPD \r\n");
	printf("[2] VFPD       - Set VFPD \r\n");
	printf("[3] VSPW       - Set VSPW \r\n");
	printf("[4] HBPD       - Set HBPD \r\n");
	printf("[5] HFPD       - Set HFPD \r\n");
	printf("[6] HSPW       - Set HSPW \r\n");
	printf("[7] CLKVAL     - Set CLKVAL \r\n");
#endif
	printf("[s] Save the parameters to Nand Flash \r\n");
	printf("[q] Return main Menu \r\n");
	printf("Enter your selection: ");
}


void lcd_menu_shell(void)
{
	char c;
	char cmd_buf[256];
	char *p = NULL;
	unsigned long size;
	unsigned long offset;
	struct mtd_info *mtd = &nand_info[nand_curr_device];

	while (1)
	{
		lcd_menu_usage();
		c = awaitkey(-1, NULL);
		printf("%c\n", c);
		switch (c)
		{
#if USER_SET
			case '1':
			{
				sprintf(cmd_buf, "setenv dwLCD_TYPE");

				printf("1: 800*600\t\t    2: 800*480\n3: 640*480\t\t    4: 480*272\n5: 240*320\t\t    6: 320*240\n"); 
				printf("Please enter LCD TYPE Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '2':
			{
				sprintf(cmd_buf, "setenv dwVBPD");

				printf("Please enter VBPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '3':
			{
				sprintf(cmd_buf, "setenv dwVFPD");

				printf("Please enter VFPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '4':
			{
				sprintf(cmd_buf, "setenv dwVSPW");

				printf("Please enter VSPW Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '5':
			{
				sprintf(cmd_buf, "setenv dwHBPD");

				printf("Please enter HBPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '6':
			{
				sprintf(cmd_buf, "setenv dwHFPD");

				printf("Please enter HFPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '7':
			{
				sprintf(cmd_buf, "setenv dwHSPW");

				printf("Please enter HSPW Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '8':
			{
				sprintf(cmd_buf, "setenv dwCLKVAL");

				printf("Please enter CLKVAL Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

#else
			case '1':
			{
				sprintf(cmd_buf, "setenv dwVBPD");

				printf("Please enter VBPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '2':
			{
				sprintf(cmd_buf, "setenv dwVFPD");

				printf("Please enter VFPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '3':
			{
				sprintf(cmd_buf, "setenv dwVSPW");

				printf("Please enter VSPW Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '4':
			{
				sprintf(cmd_buf, "setenv dwHBPD");

				printf("Please enter HBPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '5':
			{
				sprintf(cmd_buf, "setenv dwHFPD");

				printf("Please enter HFPD Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '6':
			{
				sprintf(cmd_buf, "setenv dwHSPW");

				printf("Please enter HSPW Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

			case '7':
			{
				sprintf(cmd_buf, "setenv dwCLKVAL");

				printf("Please enter CLKVAL Value: ");
				readline(NULL);
				strcat(cmd_buf, " ");
				strcat(cmd_buf, console_buffer);
				printf("%s\n",cmd_buf);

				run_command(cmd_buf, 0);
				break;
			}

#endif
			case 's':
			{
				sprintf(cmd_buf, "saveenv");
				run_command(cmd_buf, 0);
				break;
			}

			case 'q':
			{
				//sprintf(cmd_buf, "saveenv");
				//run_command(cmd_buf, 0);
				return;
				break;
			}
		}
	}
}


void main_menu_usage(void)
{

	if (bBootFrmNORFlash())
	{	
		printf("****************************************\r\n");
		printf("****  U-BOOT NOR FLASH 启动模式  ****\r\n");
		printf("****************************************\r\n");
	}
	else
	{
		printf("*************************************\r\n");
		printf("****  U-BOOT NAND FLASH 启动模式 ****\r\n");
		printf("*************************************\r\n");
	}

	printf("[1] 使用U盘烧写镜像\r\n");
	printf("[2] 使用DNW下载镜像\r\n");
    printf("[3] 格式化Nand Flash\r\n");
    printf("[4] 启动Linux系统\r\n");
    printf("[5] 设置启动参数\r\n");
    printf("[6] 重启u-boot\r\n");
    printf("[7] 退出目录\r\n");
   	printf("Enter your selection: ");
}


void menu_shell(void)
{
	char c;
	char cmd_buf[200];
	char *p = NULL;
	unsigned long size;
	unsigned long offset;
	struct mtd_info *mtd = &nand_info[nand_curr_device];

	while (1)
	{
		main_menu_usage();
		c = awaitkey(-1, NULL);
		printf("%c\n", c);
		switch (c)
		{
			case '1':  // USB
			{
				tx2440_usbmode_lcd();
				usb_menu_shell();
				break;
			}
			
			case '2':  // DNW
			{
				tx2440_usbmode_lcd();
				dnw_menu_shell();
				break;
			}
			
			case '3':  //format
			{
				erase_menu_shell();
				break;
			}

			case '4':  //boot
			{
				printf("Start Linux ...\n");
				strcpy(cmd_buf, "boot_zImage");
				run_command(cmd_buf, 0);
				break;
			}

			case '5':  //set param
			{
				param_menu_shell();
				break;
			}

			case '6':  //reset
			{
				strcpy(cmd_buf, "reset");
				run_command(cmd_buf, 0);
				break;
			}
			
			case '7':  //quit
			{
				return;	
				break;
			}
		}		
	}
}

int do_menu (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	menu_shell();
	return 0;
}

U_BOOT_CMD(
	menu,	3,	0,	do_menu,
	"menu - display a menu, to select the items to do something\n",
	" - display a menu, to select the items to do something"
);

