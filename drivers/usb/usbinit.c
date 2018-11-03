#include <common.h>
#if defined(CONFIG_S3C2400)
#include <s3c2400.h>
#elif defined(CONFIG_S3C2410)
#include <s3c2410.h>
#endif

#include "usbmain.h"
#include "usbout.h"
#include "usblib.h"
#include "2440usb.h"


extern S3C24X0_INTERRUPT * intregs;
S3C24X0_USB_DEVICE * usbdevregs;
S3C24X0_DMAS * dmaregs;
S3C24X0_CLOCK_POWER * clk_powerregs;
S3C24X0_GPIO * gpioregs;

void Isr_Init(void);
void HaltUndef(void);
void HaltSwi(void);
void HaltPabort(void);
void HaltDabort(void);
void Lcd_Off(void);

__u32 usb_receive(char *buf, size_t len, U32 wait);
void Menu(void);

extern void Timer_InitEx(void);
extern void Timer_StartEx(void);
extern unsigned int Timer_StopEx(void);

extern void (*isr_handle_array[])(void);

/*
 * Reads and returns a character from the serial port
 *   - Times out after delay iterations checking for presence of character
 *   - Sets *error_p to UART error bits or - on timeout
 *   - On timeout, sets *error_p to -1 and returns 0
 */
char awaitkey(unsigned long delay, int* error_p)
{
    int i;
    char c;

    if (delay == -1) {
        while (1) {
            if (tstc()) /* we got a key press	*/
                return getc();
        }
    }
    else {        
        for (i = 0; i < delay; i++) {
    		if (tstc()) /* we got a key press	*/
    			return getc();
            udelay (10*1000);
        }
    }
    
    if (error_p)
        *error_p = -1;
    return 0;
}

#define CTRL(x)   (x & 0x1f)
#define INTR      CTRL('C')

void Clk0_Enable(int clock_sel);	
void Clk1_Enable(int clock_sel);
void Clk0_Disable(void);
void Clk1_Disable(void);

volatile U32 downloadAddress;

void (*restart)(void)=(void (*)(void))0x0;
void (*run)(void);

volatile unsigned char *downPt;
volatile U32 downloadFileSize;
volatile U16 checkSum;
volatile unsigned int err=0;
volatile U32 totalDmaCount;

volatile int isUsbdSetConfiguration;

int download_run=0;
volatile U32 tempDownloadAddress;
int menuUsed=0;

volatile U32 dwUSBBufReadPtr;
volatile U32 dwUSBBufWritePtr;
volatile U32 dwWillDMACnt;
volatile U32 bDMAPending;
volatile U32 dwUSBBufBase;
volatile U32 dwUSBBufSize;

int consoleNum;


//**********[ PORTS ]********
void Port_Init(void)
{    
    gpioregs = S3C24X0_GetBase_GPIO();
    //CAUTION:Follow the configuration order for setting the ports. 
    // 1) setting value(GPnDAT) 
    // 2) setting control register  (GPnCON)
    // 3) configure pull-up resistor(GPnUP)  

    //32bit data bus configuration  
    //*** PORT A GROUP
    //Ports  : GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12  
    //Signal : nFCE nRSTOUT nFRE   nFWE  ALE   CLE  nGCS5 nGCS4 nGCS3 nGCS2 nGCS1 
    //Binary :  1     1      1  , 1   1   1    1   ,  1     1     1     1
    //Ports  : GPA11   GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1  GPA0
    //Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0 
    //Binary :  1       1      1      1   , 1       1      1      1   ,  1       1     1      1         
    gpioregs->GPACON = 0x7fffff; 

    //**** PORT B GROUP
    //Ports  : GPB10    GPB9    GPB8    GPB7    GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
    //Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
    //Setting: INPUT  OUTPUT   INPUT  OUTPUT   INPUT   OUTPUT   OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT 
    //Binary :   00  ,  01       00  ,   01      00   ,  01       01  ,   01     01   ,  01        01  
    gpioregs->GPBCON = 0x044555;
    gpioregs->GPBUP  = 0x7ff;     // The pull up function is disabled GPB[10:0]
	gpioregs->GPBDAT &= ~(1<<0);	 
	gpioregs->GPBDAT &= ~(1<<9);	/* disable USB Device, enable later, */

    //*** PORT C GROUP
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7    GPC6     GPC5     GPC4 GPC3     GPC2  GPC1 GPC0
    //Signal : VD7    VD6    VD5     VD4     VD3     VD2     VD1  VD0    LCDVF2 LCDVF1 OUTPUT VM    VFRAME VLINE VCLK LEND  
    //Binary :  10     10  ,    10       10  ,     10       10  ,    10     10  ,   10         10  ,      01          10 ,   10         10 ,     10     10
    gpioregs->GPCCON = 0xaaaaa6aa;       
    gpioregs->GPCUP  = 0xffff;     // The pull up function is disabled GPC[15:0] 
	gpioregs->GPCDAT &= ~(1<<5);	/* disable USB Device, enable later */

    //*** PORT D GROUP
    //Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
    //Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
    //Binary : 10    10  , 10    10  , 10    10  , 10   10 , 10   10 , 10   10 , 10   10 ,10   10
    gpioregs->GPDCON = 0xaaaaaaaa;       
    gpioregs->GPDUP  = 0xffff;     // The pull up function is disabled GPD[15:0]

    //*** PORT E GROUP
    //Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7  GPE6  GPE5   GPE4  
    //Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK IN 
    //Binary :  10     10  ,  10      10  ,  10      10   ,  10      10   ,   10    10  , 10     00  ,     
    //-------------------------------------------------------------------------------------------------------
    //Ports  :  GPE3   GPE2  GPE1    GPE0    
    //Signal :  IN     IN    IN      IN  
    //Binary :  00     00  ,  00      00 
    //rGPECON = 0xaaaaaaaa;       
    //rGPEUP  = 0xffff;     // The pull up function is disabled GPE[15:0]
	gpioregs->GPECON = 0xaaaaa800; // For added AC97 setting      
    gpioregs->GPEUP  = 0xffff;     

     //*** PORT F GROUP
    //Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
    //Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
    //Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
    //Binary :  01      01 ,  01     01  ,     10       10  , 10     10
    gpioregs->GPFCON = 0x55aa;
    gpioregs->GPFUP  = 0xff;     // The pull up function is disabled GPF[7:0]

    //*** PORT G GROUP
    //Ports  : GPG15 GPG14 GPG13 GPG12 GPG11    GPG10    GPG9     GPG8     GPG7      GPG6    
    //Signal : nYPON  YMON nXPON XMON  EINT19 DMAMODE1 DMAMODE0 DMASTART KBDSPICLK KBDSPIMOSI
    //Setting: nYPON  YMON nXPON Output EINT19  Output   Output   Output   SPICLK1    SPIMOSI1
    //Binary :   11    11 , 11    01  , 10      01    ,   01       01   ,    11         11
    //-----------------------------------------------------------------------------------------
    //Ports  :    GPG5       GPG4    GPG3    GPG2    GPG1    GPG0    
    //Signal : KBDSPIMISO LCD_PWREN EINT11 nSS_SPI IRQ_LAN IRQ_PCMCIA
    //Setting:  SPIMISO1  LCD_PWRDN EINT11   nSS0   EINT9    EINT8
    //Binary :     11         11   ,  10      11  ,  10        10
    //rGPGCON = 0xff95ffba;
    gpioregs->GPGCON = 0xfd95ffba;	/*  GPG12-->OUTPUT, GPG9-->OUTPUT for FS2410, as USB Device enable */
    gpioregs->GPGUP  = 0xffff;    // The pull up function is disabled GPG[15:0]
	gpioregs->GPGDAT &= ~((1<<9) | (1<<12));	/* disable USB Device, enable later,  */

    //*** PORT H GROUP
    //Ports  :  GPH10    GPH9  GPH8 GPH7  GPH6  GPH5 GPH4 GPH3 GPH2 GPH1  GPH0 
    //Signal : CLKOUT1 CLKOUT0 UCLK nCTS1 nRTS1 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
    //Binary :   10   ,  10     10 , 11    11  , 10   10 , 10   10 , 10    10
    gpioregs->GPHCON = 0x2afaaa;
    gpioregs->GPHUP  = 0x7ff;    // The pull up function is disabled GPH[10:0]

	// Added for S3C2440X, DonGo
	//*** PORT J GROUP
    //Ports  : GPJ12   GPJ11       GPJ10    GPJ9    GPJ8      GPJ7      GPJ6      GPJ5      GPJ4      GPJ3      GPJ2      GPJ1     GPJ0 
    //Signal : CAMRESET CAMPCLKOUT CAMHREF CAMVSYNC CAMPCLKIN CAMDAT[7] CAMDAT[6] CAMDAT[5] CAMDAT[4] CAMDAT[3] CAMDAT[2] CAMDAT[1] CAMDAT[0] 
    //Binary :   10      10       10        10       10        10        10        10       10         10        10        10      10
    gpioregs->GPJCON = 0x02aaaaaa;
    gpioregs->GPJUP  = 0x1fff;    // The pull up function is disabled GPH[10:0]
    
    //External interrupt will be falling edge triggered. 
    gpioregs->EXTINT0 = 0x22222222;    // EINT[7:0]
    gpioregs->EXTINT1 = 0x22222222;    // EINT[15:8]
    gpioregs->EXTINT2 = 0x22222222;    // EINT[23:16]
}


void usb_init(void)
{
	char *mode;
	int i;
	U8 key;
	U32 mpll_val, upll_val, divn_upll=0;
    extern int bBootFrmNORFlash(void);

    clk_powerregs = S3C24X0_GetBase_CLOCK_POWER();

    usbdevregs = S3C24X0_GetBase_USB_DEVICE();
    dmaregs = S3C24X0_GetBase_DMAS();

    udelay(100000);
#if 0	
	// USB device detection control
	rGPGCON &= ~(3<<24);
	rGPGCON |=  (1<<24); // output
	rGPGUP  |=  (1<<12); // pullup disable
	rGPGDAT |=  (1<<12); // output	
#endif

	//ChangeUPllValue(60,4,2);		// 48MHz
	//for(i=0; i<7; i++);
	//ChangeClockDivider(13,12);
	//ChangeMPllValue(97,1,2);		//296Mhz

	Isr_Init();


	gpioregs->MISCCR=gpioregs->MISCCR&~(1<<3); // USBD is selected instead of USBH1 
	gpioregs->MISCCR=gpioregs->MISCCR&~(1<<13); // USB port 1 is enabled.

//
//  USBD should be initialized first of all.
//
	isUsbdSetConfiguration=0;

#if 0
	UsbdMain(); 
	MMU_Init(); //MMU should be reconfigured or turned off for the debugger, 
	//After downloading, MMU should be turned off for the MMU based program,such as WinCE.	
#else
//    MMU_EnableICache();  
	UsbdMain(); 
    udelay(100000);
    gpioregs->GPGDAT |= ((1<<9) | (1<<12));  /* enable USB Device,  */
    gpioregs->GPBDAT |= (1<<9);   /* enable USB Device,  */
    gpioregs->GPCDAT |= (1<<5);   /* enable USB Device,  */
#endif

#if USBDMA
	mode="DMA";
#else
	mode="Int";
#endif

	// CLKOUT0/1 select.
	//printf("CLKOUT0:MPLL in, CLKOUT1:RTC clock.\n");
	//Clk0_Enable(0);	// 0:MPLLin, 1:UPLL, 2:FCLK, 3:HCLK, 4:PCLK, 5:DCLK0
	//Clk1_Enable(2);	// 0:MPLLout, 1:UPLL, 2:RTC, 3:HCLK, 4:PCLK, 5:DCLK1	
//	Clk0_Disable();
//	Clk1_Disable();
	
	mpll_val = clk_powerregs->MPLLCON;
	upll_val = clk_powerregs->UPLLCON; 


    if (1) //(!bBootFrmNORFlash())
    {
    	printf("\n\n");
    	printf("+----------------------------------------------+\n");
    	printf("|  TXMCU ELC COM WWW.TXMCU.COM U-BOOT-1.1.6V2  |\n");
    	printf("|        TX2440 ARM9 DEVELOPMENT BOARD         |\n");
    	printf("+----------------------------------------------+\n");
    	printf("\n");
    }
	download_run=0; //The default menu is the Download & Run mode.

}

__u32 usb_receive(char *buf, size_t len, U32 wait)
{
    int first=1;
    U8 tempMem[16];
    U32 j;
    unsigned int dwRecvTimeSec = 0;
	char c;

    dwUSBBufReadPtr = dwUSBBufBase; // USB_BUF_BASE
    dwUSBBufWritePtr = dwUSBBufBase; // USB_BUF_BASE
    bDMAPending = 0;

    tempDownloadAddress = dwUSBBufBase; // USB_BUF_BASE; 

    downloadAddress=(U32)tempMem; //_RAM_STARTADDRESS; 
    downPt=(unsigned char *)downloadAddress;
	//This address is used for receiving first 8 byte.
    downloadFileSize=0;
    

    /*******************************/
    /*    File download    */
    /*******************************/
    if(isUsbdSetConfiguration==0)
    {
	    printf("USB host is not connected yet.\n");
    }

    while(downloadFileSize==0) /* wait until send a file */
    {
        if(first==1 && isUsbdSetConfiguration!=0)
        {
            printf("USB host is connected. Waiting a download.\n");
            first=0;
        }
		c = awaitkey(1, 0);
		if ((c & 0x7f) == INTR)
		{
			printf("Cancelled by user\n");
			return 0;
		}
    }

    if (downloadFileSize - 10 > len)
    {
        printf("Length of file is too big : %d > %d\n", downloadFileSize - 10, len);
        return 0;
    }
    
    Timer_InitEx();
    Timer_StartEx();
        
#if USBDMA    

    intregs->INTMSK&=~(BIT_DMA2);  

    ClearEp3OutPktReady(); 
    	// indicate the first packit is processed.
    	// has been delayed for DMA2 cofiguration.

    if(downloadFileSize>EP3_PKT_SIZE)
    {
        if(downloadFileSize - EP3_PKT_SIZE<=(0x80000))
        {
            /* set the source and length */
            dwUSBBufWritePtr = downloadAddress + EP3_PKT_SIZE-8;
            dwWillDMACnt = downloadFileSize - EP3_PKT_SIZE;
	    }
      	else
      	{
            dwUSBBufWritePtr = downloadAddress + EP3_PKT_SIZE - 8;
            dwWillDMACnt = 0x80000 + 8 - EP3_PKT_SIZE;
    	}
     	totalDmaCount = 0;
  	    ConfigEp3DmaMode(dwUSBBufWritePtr, dwWillDMACnt);
    }
    else
    {
        dwUSBBufWritePtr = downloadAddress + downloadFileSize - 8;
	    totalDmaCount = downloadFileSize;
    }
#endif

    printf("\nNow, Downloading [ADDRESS:%xh,TOTAL:%d]\n",
    		downloadAddress,downloadFileSize);


#if ( LCD_TFT == TFT480272 )
#define LCD_XSIZE_TFT       (480)
#define LCD_YSIZE_TFT       (272)
#elif ( LCD_TFT == TFT800480 )
#define LCD_XSIZE_TFT       (800)
#define LCD_YSIZE_TFT       (480)
#endif

    if (wait)
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
		
        printf("RECEIVED FILE SIZE:%8d",0);

        j = totalDmaCount + 0x10000;
        while (totalDmaCount != downloadFileSize)
		{
			if (totalDmaCount > j)
		    {	
				//当j 增大到一定程度时， j*100 会超出 unsigned int 的范围，导致j*100/downloadFileSize 的值为零!
				//tx2440_percent((j*100/downloadFileSize),200,160); 

				int percent = (int) 100 * ((j * 1.0) / downloadFileSize);
				#if ( LCD_TFT == TFT480272 )
				tx2440_percent(percent,200,160);
				tx2440_drawProcessBar(downloadFileSize, j, 10, LCD_YSIZE_TFT - 90);
				#elif ( LCD_TFT == TFT800480 )
				tx2440_percent(percent,350,230);
				tx2440_drawProcessBar(downloadFileSize, j, 10, 260);
				#endif
							
			    printf("\b\b\b\b\b\b\b\b%8d", j);
		        j = totalDmaCount + 0x10000;
		    }
		}

		#if ( LCD_TFT == TFT480272 )
		tx2440_percent(100,200,160);
		tx2440_drawProcessBar(1, 1, 10, LCD_YSIZE_TFT - 90);
		#elif ( LCD_TFT == TFT800480 )
		tx2440_percent(100,350,230);
		tx2440_drawProcessBar(1, 1, 10, 260);
		#endif

	    printf("\b\b\b\b\b\b\b\b%8d ", totalDmaCount);
        dwRecvTimeSec = Timer_StopEx();
        if (dwRecvTimeSec == 0)
        {
            dwRecvTimeSec = 1;
        }
        printf("(%dKB/S, %dS)\n", (downloadFileSize/dwRecvTimeSec/1024), dwRecvTimeSec);
    }


    return downloadFileSize - 10;

}


void HaltUndef(void)
{
	printf("Undefined instruction exception!!!\n");
	while(1);
}

void HaltSwi(void)
{
	printf("SWI exception!!!\n");
	while(1);
}

void HaltPabort(void)
{
	printf("Pabort exception!!!\n");
	while(1);
}

void HaltDabort(void)
{
	printf("Dabort exception!!!\n");
	while(1);
}

void Clk0_Enable(int clock_sel)	
{	// 0:MPLLin, 1:UPLL, 2:FCLK, 3:HCLK, 4:PCLK, 5:DCLK0
	gpioregs->MISCCR = gpioregs->MISCCR&~(7<<4) | (clock_sel<<4);
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<18) | (2<<18);
}
void Clk1_Enable(int clock_sel)
{	// 0:MPLLout, 1:UPLL, 2:RTC, 3:HCLK, 4:PCLK, 5:DCLK1	
	gpioregs->MISCCR = gpioregs->MISCCR&~(7<<8) | (clock_sel<<8);
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<20) | (2<<20);
}
void Clk0_Disable(void)
{
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<18);	// GPH9 Input
}
void Clk1_Disable(void)
{
	gpioregs->GPHCON = gpioregs->GPHCON&~(3<<20);	// GPH10 Input
}

