
#include <common.h>
#include <s3c2410.h>
#include <s3c24x0.h>

#ifdef CONFIG_TX2440_LOGO

#include "tx2440_lcd_config.h"
#include "tx2440_lcd.h"
#include "tx2440_lcd_usbmode.h"

#define U32 unsigned int

#define M5D(n)			( ( n ) & 0x1fffff )				// To get lower 21bits
extern char bLARGEBLOCK;			//           
extern char b128MB;				//           
extern int NF_ReadID(void);			//           
extern int nand_read_ll(unsigned char*, unsigned long, int);
extern int nand_read_ll_lp(unsigned char*, unsigned long, int);

#define SCR_XSIZE_TFT	 	(LCD_XSIZE_TFT)
#define SCR_YSIZE_TFT	 	(LCD_YSIZE_TFT)

#define HOZVAL_TFT		( LCD_XSIZE_TFT - 1 )
#define LINEVAL_TFT		( LCD_YSIZE_TFT - 1 )

volatile unsigned short tx2440_LCD_BUFFER[SCR_YSIZE_TFT][SCR_XSIZE_TFT] ;//= { 0, } ;

volatile char vbpd = 1, vfpd = 1, vspw = 1, hbpd = 1, hfpd = 1, hspw = 1, clkval_tft = 1 ;


/**************************************************************
TFT LCD全屏填充特定颜色单元或清屏
**************************************************************/
void tx2440_lcd_ClearScr( U32 c)
{
	unsigned int x,y ;

	for( y = 0 ; y < LCD_YSIZE_TFT ; y++ )
	{
		for( x = 0 ; x < (SCR_XSIZE_TFT) ; x++ )
		{
			tx2440_LCD_BUFFER[y][x] = c;
		}
	}
}

/**************************************************************
TFT LCD指定大小的矩形填充特定颜色单元或清屏
**************************************************************/
void tx2440_lcd_ClearScr_Rectangle( int x0,int y0,int width,int height, U32 c)
{
	unsigned int x,y ;
		
	for( y = y0 ; y < (y0 + height) ; y++ )
	{
		for( x = x0 ; x < (width + x0) ; x++ )
		{
			tx2440_LCD_BUFFER[y][x] = c;
		}
	}
}

/**************************************************************
在LCD屏幕上指定坐标点画一个指定大小的图片
**************************************************************/
void tx2440_paint_Bmp(int x0,int y0,int width,int height, unsigned char bmp[])
{
	int x, y ;
	U32 c ;
	int p = 0 ;

	for( y = y0 ; y < (height + y0) ; y++ )
	{
		for( x = x0 ; x < (width + x0) ; x++ )
		{
			c = bmp[p+1] | (bmp[p] << 8) ;

			if ( ( x <= (SCR_XSIZE_TFT) ) && ( y <= LCD_YSIZE_TFT ) )
				tx2440_LCD_BUFFER[y][x] = c;
			p = p + 2 ;
		}
	}

}

/**************************************************************
在LCD上指定位置画点
**************************************************************/
void putPixel(U32 x, U32 y, U32 c)
{
	if( (x < LCD_XSIZE_TFT) && (y < LCD_YSIZE_TFT))
		tx2440_LCD_BUFFER[y][x] = c;
}

/*******************************************************************
LCD滚动条的显示(在Nand Flash读取时调用)
current当前字节数，total总共的字节数，PBcolor滚动条的颜色
*******************************************************************/
void drawProcessBar(U32 total, U32 current )
{
	U32 const bar_height = 8;
	U32 bar_base = LCD_YSIZE_TFT - bar_height;
	int i = (int) LCD_XSIZE_TFT / 8;
	U32 currWidth = 0;
	U32 j;

	int pbcolor ;
	if(total != -1)
	{
		j = 0;
		int bar_width = (int) LCD_XSIZE_TFT * ((current * 1.0) / total);
		if (bar_width <= i)
			pbcolor = 0x7FF;					//sky blue
		else if((bar_width > i) && (bar_width <= i * 2))
			pbcolor = 0x1F;						//blue
		else if((bar_width > i * 2) && (bar_width <= i * 3))
			pbcolor = 0x0;						//black
		else if((bar_width > i * 3) && (bar_width <= i * 4))
			pbcolor = 0xF81F;					//purple
		else if((bar_width > i * 4) && (bar_width <= i * 5))
			pbcolor = 0xFFFF;					//white
		else if((bar_width > i * 5) && (bar_width <= i * 6))
			pbcolor = 0xF800;					//red
		else if((bar_width > i * 6) && (bar_width <= i * 7))
			pbcolor = 0xFFE0;					//yellow
		else if((bar_width > i * 7) && (bar_width <= i * 8))
			pbcolor = 0x7E0;					//green

		if(bar_width > currWidth)
		{
			for ( ; j < bar_height; j++)
			{
				putPixel(bar_width, j + bar_base, pbcolor);
			}
			currWidth = bar_width;
		}
	}
}

void tx2440_drawProcessBar(U32 total, U32 current, U32 bar_height, U32 bar_base )
{
	int i = (int) LCD_XSIZE_TFT / 8;
	U32 j;
	U32 currWidth = 0;

	int pbcolor; //=0x1F;

	if(total != -1)
	{
		j = 0;
		int bar_width = (int) LCD_XSIZE_TFT * ((current * 1.0) / total);
#if 0 //color
		if (bar_width <= i)
			pbcolor = 0x7FF;					// blue
		else if((bar_width > i) && (bar_width <= i * 2))
			pbcolor = 0x1F;						//blue
		else if((bar_width > i * 2) && (bar_width <= i * 3))
			pbcolor = 0x0;						//black
		else if((bar_width > i * 3) && (bar_width <= i * 4))
			pbcolor = 0xF81F;					//purple
		else if((bar_width > i * 4) && (bar_width <= i * 5))
			pbcolor = 0xFFFF;					//white
		else if((bar_width > i * 5) && (bar_width <= i * 6))
			pbcolor = 0xF800;					//red
		else if((bar_width > i * 6) && (bar_width <= i * 7))
			pbcolor = 0xFFE0;					//yellow
		else if((bar_width > i * 7) && (bar_width <= i * 8))
			pbcolor = 0x7E0;					//green
#else
		pbcolor = 0xF800;
#endif

		if(bar_width > currWidth)
		{
			for ( ; j < bar_height; j++)
			{
				putPixel(bar_width, j + bar_base, pbcolor);
				tx2440_lcd_ClearScr_Rectangle(0,bar_base,bar_width,bar_height, pbcolor);
			}
			currWidth = bar_width;
		}
	}
}

/**************************************************************
TFT LCD 电源控制引脚使能
**************************************************************/
void tx2440_lcd_PowerEnable(int invpwren , int pwren)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO() ;
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD() ;
										//GPG4 is setted as LCD_PWREN
	gpio -> GPGUP = gpio -> GPGUP & (( ~( 1 << 4) ) | ( 1 << 4) );		// Pull-up disable
	gpio -> GPGCON = gpio -> GPGCON & (( ~( 3 << 8) ) | ( 3 << 8) );		//GPG4=LCD_PWREN
	gpio -> GPGDAT = gpio -> GPGDAT | (1 << 4 ) ;
										//invpwren=pwren;
										//Enable LCD POWER ENABLE Function
	lcd -> LCDCON5 = lcd -> LCDCON5 & (( ~( 1 << 3 ) ) | ( pwren << 3 ) );	// PWREN
	lcd -> LCDCON5 = lcd -> LCDCON5 & (( ~( 1 << 5 ) ) | ( invpwren << 5 ) );	// INVPWREN
}

/**************************************************************
LCD视频和控制信号输出或者停止，1开启视频输出
**************************************************************/
void tx2440_lcd_EnvidOnOff(int onoff)
{
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD() ;

	if( onoff == 1 )
		lcd -> LCDCON1 |= 1 ;						// ENVID=ON
	else
		lcd -> LCDCON1 = lcd -> LCDCON1 & 0x3fffe ;			// ENVID Off
}

/**************************************************************
LCD初始化程序(在board/TX2440/目录下的boot_init.c文件的60行左右,调用的)
**************************************************************/
void tx2440_lcd_Init(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO() ;
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD() ;

	char *s_lcd;

	lcd -> LCDSADDR1 = ( ( ( U32 ) tx2440_LCD_BUFFER >> 22 ) << 21 ) | M5D ( ( U32 ) tx2440_LCD_BUFFER >> 1 ) ;
	lcd -> LCDSADDR2 = M5D( ( ( U32) tx2440_LCD_BUFFER + ( SCR_XSIZE_TFT * LCD_YSIZE_TFT * 2 ) ) >> 1 ) ;
	lcd -> LCDSADDR3 = ( ( ( SCR_XSIZE_TFT - LCD_XSIZE_TFT ) / 1 ) << 11 ) | ( LCD_XSIZE_TFT /1 ) ;

	s_lcd = getenv ("dwVBPD");
	vbpd = s_lcd ? (int)simple_strtol(s_lcd, NULL, 10) : CONFIG_TX2440_LCD_VBPD;

	s_lcd = getenv ("dwVFPD");
	vfpd = s_lcd ? (int)simple_strtol(s_lcd, NULL, 10) : CONFIG_TX2440_LCD_VFPD;

	s_lcd = getenv ("dwVSPW");
	vspw = s_lcd ? (int)simple_strtol(s_lcd, NULL, 10) : CONFIG_TX2440_LCD_VSPW;

	s_lcd = getenv ("dwHBPD");
	hbpd = s_lcd ? (int)simple_strtol(s_lcd, NULL, 10) : CONFIG_TX2440_LCD_HBPD;

	s_lcd = getenv ("dwHFPD");
	hfpd = s_lcd ? (int)simple_strtol(s_lcd, NULL, 10) : CONFIG_TX2440_LCD_HFPD;

	s_lcd = getenv ("dwHSPW");
	hspw = s_lcd ? (int)simple_strtol(s_lcd, NULL, 10) : CONFIG_TX2440_LCD_HSPW;

	s_lcd = getenv ("dwCLKVAL");
	clkval_tft = s_lcd ? (int)simple_strtol(s_lcd, NULL, 10) : CONFIG_TX2440_LCD_CLKVAL;

	tx2440_lcd_ClearScr( 0x0 ) ;

	gpio -> GPCUP  = 0xffffffff ;
	gpio -> GPCCON = 0xaaaaaaaa ;						//Initialize VD[0:7]	 
	 
	gpio -> GPDUP  = 0xffffffff ;
	gpio -> GPDCON = 0xaaaaaaaa ;						//Initialize VD[15:8]

	lcd -> LCDCON1 = ( clkval_tft << 8 ) | ( MVAL_USED << 7 ) | (3 << 5 ) | ( 12 << 1 ) | 0 ;
										// TFT LCD panel,16bpp TFT,ENVID=off
	lcd -> LCDCON2 = ( vbpd << 24 ) | ( LINEVAL_TFT << 14 ) | ( vfpd << 6 ) | ( vspw ) ;
	lcd -> LCDCON3 = ( hbpd << 19 ) | ( HOZVAL_TFT << 8 ) | ( hfpd ) ;
	lcd -> LCDCON4 = ( MVAL << 8 ) | ( hspw ) ;
#if( LCD_TFT == VGA1024768 )
	lcd -> LCDCON5 = ( 1 << 11) | ( HWSWP ) ;
#else
	lcd -> LCDCON5 = ( 1 << 11) | ( 0 << 10 ) | ( 1 << 9 ) | ( 1 << 8 ) | ( 0 << 7 ) | ( 0 << 6 ) | ( 1 << 3 ) | ( BSWP << 1 ) | ( HWSWP ) ;
#endif

	lcd -> LCDINTMSK |= (3) ;						// MASK LCD Sub Interrupt
#if( LCD_TFT == VGA1024768 )
	lcd -> LPCSEL &= 0xf82;
#else
	lcd -> LPCSEL &= ( ~7 ) ;						// Disable LPC3480
#endif
	lcd -> TPAL = 0x0 ;							// Disable Temp Palette

	tx2440_lcd_PowerEnable( 0, 1 ) ;
}

/****************************************************************
LCD显示天祥电子下载logo程序(在common/目录下的main.c文件的277行左右调用的)
****************************************************************/
void tx2440_logo(void)
{
	if(1)
	{
		tx2440_paint_Bmp(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, down_mode);
	}
	
	else
		tx2440_paint_Bmp((LCD_XSIZE_TFT -180) / 2, (LCD_YSIZE_TFT -20) / 2, 180, 20, down_mode);		//显示进入下载模式，中间	

	tx2440_lcd_EnvidOnOff ( 1 ) ;
}

/****************************************************************
LCD显示用户开机logo程序(在common/目录下的main.c文件的277行左右调用的)
****************************************************************/
void tx2440_user_logo(void)
{
	size_t retlen = SCR_YSIZE_TFT * SCR_XSIZE_TFT * (16 >> 3);

	if (NF_ReadID() == 0x76)
	{
		if(( retlen % 0x200) != 0)				//           
			retlen = (retlen / 0x200) * 0x200 + 0x200;	//           
		#if(CONFIG_64MB_Nand == 1)
		nand_read_ll(tx2440_LCD_BUFFER, 0x70000, retlen);
		#else
			#if( LCD_TFT == VGA1024768 )
			nand_read_ll(tx2440_LCD_BUFFER, 0x0C0000, retlen);
			#else
			nand_read_ll(tx2440_LCD_BUFFER, 0x100000, retlen);
			#endif
		#endif
	}
	else
	{
		if(( retlen % 0x800) != 0)				//           
			retlen = (retlen / 0x800) * 0x800 + 0x800;	
		#if( LCD_TFT == VGA1024768 )
			nand_read_ll_lp(tx2440_LCD_BUFFER, 0xC0000, retlen);
		#else
			nand_read_ll_lp(tx2440_LCD_BUFFER, 0x100000, retlen);
		#endif
	}
	tx2440_lcd_ClearScr_Rectangle(0, LCD_YSIZE_TFT-8, LCD_XSIZE_TFT, 8, 0xFFFF);

	tx2440_lcd_EnvidOnOff(1);
}

//----------------------------------- Nand Flash Read -------------------------------------------

#define NAND_SECTOR_SIZE		512
#define NAND_BLOCK_MASK			(NAND_SECTOR_SIZE - 1)

#define NAND_SECTOR_SIZE_LP		2048
#define NAND_BLOCK_MASK_LP		(NAND_SECTOR_SIZE_LP - 1)

/* 等待NAND Flash就绪 */
static void TX2440_wait_idle(void)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFSTAT;

	while(!(*p & 1))
	for(i=0; i<10; i++);
}

/* 发出片选信号 */
static void TX2440_nand_select_chip(void)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

	s3c2440nand->NFCONT &= ~(1<<1);
	for(i=0; i<10; i++);
}

/* 取消片选信号 */
static void TX2440_nand_deselect_chip(void)
{
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

	s3c2440nand->NFCONT |= (1<<1);
}

/* 发出命令 */
static void TX2440_write_cmd(int cmd)
{
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFCMD;
	*p = cmd;
}

/* 发出地址 */
static void TX2440_write_addr(unsigned int addr)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;

	*p = addr & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 9) & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 17) & 0xff;
	for(i=0; i<10; i++);
	*p = (addr >> 25) & 0xff;
	for(i=0; i<10; i++);
}

/* 发出地址 */
static void TX2440_write_addr_lp(unsigned int addr)
{
	int i;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;
	int col, page;

	col = addr & NAND_BLOCK_MASK_LP;
	page = addr / NAND_SECTOR_SIZE_LP;
	
	*p = col & 0xff;			/* Column Address A0~A7 */
	for(i=0; i<10; i++);
	*p = (col >> 8) & 0x0f;		/* Column Address A8~A11 */
	for(i=0; i<10; i++);
	*p = page & 0xff;			/* Row Address A12~A19 */
	for(i=0; i<10; i++);
	*p = (page >> 8) & 0xff;	/* Row Address A20~A27 */
	for(i=0; i<10; i++);
#if (b128MB == 0)
	*p = (page >> 16) & 0x03;	/* Row Address A28~A29 */
	for(i=0; i<10; i++);
#endif
}

/* 读取数据 */
static unsigned char TX2440_read_data(void)
{
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFDATA;
	return *p;
}


/* 读函数 添加了LCD的滚动条 */
int nand_read_ll_lcd(unsigned char *buf, unsigned long start_addr, int size)
{
	int i, j;
	char dat;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;


	int current = 0;				// 滚动条，当前的字节数
	int total = 0;				// 滚动条

	if ((start_addr & 511) || (size & 511))
	{
		return -1;		/* 地址或长度不对齐 */
	}

	/* 选中芯片 */
	TX2440_nand_select_chip();

	total = size;			// 滚动条

	for(i=start_addr; i < (start_addr + size);)
	{
if(1){
/* Check Bad Block */
		/* 发出READ0命令 */
		TX2440_write_cmd(0x50);

		*p = 5;
		for(j=0; j<10; j++);
		*p = (i >> 9) & 0xff;
		for(j=0; j<10; j++);
		*p = (i >> 17) & 0xff;
		for(j=0; j<10; j++);
		*p = (i >> 25) & 0xff;
		for(j=0; j<10; j++);
		TX2440_wait_idle();

		dat = TX2440_read_data();
		TX2440_write_cmd(0);
		
		/* 取消片选信号 */
		TX2440_nand_deselect_chip();
		if(dat != 0xff)
		{
			i += 16384;		// 1 Block = 512*32= 16384
			printf("\nBad block at 0x%lx ,will be skipped\n",i);
		}
/* Read Page */
		/* 选中芯片 */
		TX2440_nand_select_chip();
}
		/* 发出READ0命令 */
		TX2440_write_cmd(0);

		/* Write Address */
		TX2440_write_addr(i);
		TX2440_wait_idle();

		for(j=0; j < 512; j++, i++)
		{
			*buf = TX2440_read_data();
			buf++;
		}

		current += 512 ;					// 滚动条
		drawProcessBar(total, current );			// 滚动条
	}

	/* 取消片选信号 */
	TX2440_nand_deselect_chip();

	return 0;
}

/* 读函数 添加了LCD的滚动条 */
int nand_read_ll_lp_lcd(unsigned char *buf, unsigned long start_addr, int size)
{
	int i, j;
	int current = 0;				// 滚动条，当前的字节数
	int total = 0;				// 滚动条

	char dat;
	S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;

	if ((start_addr & NAND_BLOCK_MASK_LP) || (size & NAND_BLOCK_MASK_LP))
	{
		return -1;    /* 地址或长度不对齐 */
	}

	/* 选中芯片 */
	TX2440_nand_select_chip();

	total = size;			// 滚动条

	for(i=start_addr; i < (start_addr + size);)
	{
/* Check Bad Block */
if(1){
		int col, page;

		col = i & NAND_BLOCK_MASK_LP;
		page = i / NAND_SECTOR_SIZE_LP;
		/* 发出READ0命令 */
		TX2440_write_cmd(0x00);

		*p = 5;
		for(j=0; j<10; j++);
		*p = 8;
		for(j=0; j<10; j++);
		*p = page & 0xff;		/* Row Address A12~A19 */
		for(j=0; j<10; j++);
		*p = (page >> 8) & 0xff;		/* Row Address A20~A27 */
		for(j=0; j<10; j++);
#if (b128MB== 0)
		*p = (page >> 16) & 0x03;		/* Row Address A28~A29 */
		for(j=0; j<10; j++);
#endif
		TX2440_write_cmd(0x30);
		TX2440_wait_idle();

		dat = TX2440_read_data();
		
		/* 取消片选信号 */
		TX2440_nand_deselect_chip();
		if(dat != 0xff)
		{
			i += 131072;		// 1 Block = 2048*64= 131072
			printf("\nBad block at 0x%lx ,will be skipped\n",i);
		}
/* Read Page */
		/* 选中芯片 */
		TX2440_nand_select_chip();
}
		/* 发出READ0命令 */
		TX2440_write_cmd(0);

		/* Write Address */
		TX2440_write_addr_lp(i);
		TX2440_write_cmd(0x30);
		TX2440_wait_idle();

		for(j=0; j < NAND_SECTOR_SIZE_LP; j++, i++)
		{
			*buf = TX2440_read_data();
			buf++;
		}

		current += 2048 ;				// 滚动条
		//drawProcessBar(total, current );			// 滚动条

	}

	/* 取消片选信号 */
	TX2440_nand_deselect_chip();

	return 0;
}

/***********************************************************************/
#define BACKUP_COLOR 0x0
#define PERCENT_COLOR 0xCE35

//绘制大小为16×16的中文字符
void tx2440_Draw_Text16(U32 x,U32 y,U32 color,const unsigned char ch[])
{
       unsigned short int i,j;
       unsigned char mask,buffer;
      
       for(i=0;i<16;i++)
       {
              mask=0x80;                  //掩码
              buffer=ch[i*2];             //提取一行的第一个字节
              for(j=0;j<8;j++)
              {                  
                     if(buffer&mask)
                     {
                            putPixel(x+j,y+i,color);        //为笔画上色
                     }
                     mask=mask>>1;                  
              }
              mask=0x80;                  //掩码
              buffer=ch[i*2+1];         //提取一行的第二个字节
              for(j=0;j<8;j++)
              {                  
                     if(buffer&mask)
                     {
                            putPixel(x+j+8,y+i,color);           //为笔画上色
                     }
                     mask=mask>>1;                  
              }
       }
}

//绘制大小为8×16的ASCII码
void tx2440_Draw_ASCII(U32 x,U32 y,U32 color,const unsigned char ch[])
{
       unsigned short int i,j;
       unsigned char mask,buffer;
      
       for(i=0;i<16;i++)
       {
              mask=0x80;
              buffer=ch[i];
              for(j=0;j<8;j++)
              {                  
                     if(buffer&mask)
                     {
                            putPixel(x+j,y+i,color);
                     }
                     mask=mask>>1;                  
              }
       }
}

void tx2440_percent(int num, int xx, int yy)
{
	int i,j;

	tx2440_lcd_ClearScr_Rectangle(xx,yy,50,20,BACKUP_COLOR );
	tx2440_Draw_ASCII(xx+30,yy,PERCENT_COLOR,arr_percent);

	if (100 == num) {
		tx2440_Draw_ASCII(xx,yy,PERCENT_COLOR,arr_digital[1]);
		tx2440_Draw_ASCII(xx+10,yy,PERCENT_COLOR,arr_digital[0]);
		tx2440_Draw_ASCII(xx+20,yy,PERCENT_COLOR,arr_digital[0]);
	}
	else {
		i = num/10;
		j = num%10;
	
		if (i != 0)
			tx2440_Draw_ASCII(xx+10,yy,PERCENT_COLOR,arr_digital[i]);
		tx2440_Draw_ASCII(xx+20,yy,PERCENT_COLOR,arr_digital[j]);
	}
}


void tx2440_usbmode_lcd(void)
{
	int i=0,j=80;

	tx2440_lcd_ClearScr(BACKUP_COLOR);//绘制背景

	for (i =0;i<30;i++){ 
		#if ( LCD_TFT == TFT480272 )
		tx2440_Draw_ASCII(j,50,0x434658,arr_title[i]);
		#elif ( LCD_TFT == TFT800480 )
		tx2440_Draw_ASCII(j+150,100,0x434658,arr_title[i]);
		#endif
		j += 10;
	}

	j =80;
	for (i =0;i<4;i++){
		#if ( LCD_TFT == TFT480272 )
		tx2440_Draw_ASCII(j,125,0xe010,arr_burn[i]);
		#elif ( LCD_TFT == TFT800480 )
		tx2440_Draw_ASCII(j+100,200,0xe010,arr_burn[i]);
		#endif
		j += 10;
	}

	j =80;
	for (i =0;i<5;i++){
		#if ( LCD_TFT == TFT480272 )
		tx2440_Draw_ASCII(j,160,0xc1F0,arr_Read[i]);
		#elif ( LCD_TFT == TFT800480 )
		tx2440_Draw_ASCII(j+100,230,0xc1F0,arr_Read[i]);
		#endif
		j += 10;
	}

	j =80;
	for (i =0;i<6;i++){
		#if ( LCD_TFT == TFT480272 )
		tx2440_Draw_ASCII(j,220,0xc10F,arr_Write[i]);
		#elif ( LCD_TFT == TFT800480 )
		tx2440_Draw_ASCII(j+100,300,0xc10F,arr_Write[i]);
		#endif
		j += 10;
	}

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


	tx2440_lcd_EnvidOnOff(1) ; //开启LCD显示

}

#endif
