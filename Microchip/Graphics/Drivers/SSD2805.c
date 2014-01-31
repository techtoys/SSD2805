/*****************************************************************************
 *  Module for Microchip Graphics Library
 *  Solomon Systech. SSD2805 MCU interface driver
 *****************************************************************************
 * FileName:        SSD2805.c
 * Processor:       PIC32
 * Compiler:       	MPLAB C32/XC32
 * Company:         TechToys Company
 *
 * Software License Agreement
 *
 * You should refer to the license agreement accompanying this Software
 * for additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS?WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 * BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY
 * CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
 * OR OTHER SIMILAR COSTS.
 *
 * Date         Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 08/08/13
 * Programmer: John Leung @ TechToys Co. (www.TechToys.com.hk) 
 *****************************************************************************/

/*
*****************************************************************************
* Remarks: The MCU interface is used to program the local registers of 
* SSD2805, configure and send display data to the smart display panels in
* command mode. Smart panels are displays with controller integrated.
* AUO H154QN01 V2 is one of these with Samsung S6D04D2 as the controller.
* SSD2805 can drive multiple smart panels with data packets selected by
* different Virtual Channel (VC) fields. Local registers of SSD2805 ranges
* from 0xB0 to 0xD9. User can access these registers to configure and control
* SSD2805. The address range 0x00 to 0xAF and 0xDA to 0xFF is used for MIPI
* DCS command, that is a standardized command set specified by the MIPI
* Alliance. Details of the MIPI command set can be found in the document
* "MIPI Alliance Standard for Display Command Set Version 1.01.00-22 June 2006".
* As a result, the MCU is able to access both SSD2805 and the display
* panel with the same MCU interface. When the MCU write a command in the 
* range 0xB0 to 0xD9 it is designated for SSD2805 whereas outside this range
* in 0x00-0xAF and 0xDA-0xFF is designated for the MIPI panel with protocol
* conversion done automatically by SSD2805 between MCU<->MIPI serial link.
*****************************************************************************
*/

#include "HardwareProfile.h"

#if defined (GFX_USE_SSD2805_MIPI_BRIDGE)
#include "Compiler.h"
#include "TimeDelay.h"
#include "GenericTypeDefs.h"
#include "Graphics/DisplayDriver.h"
#include "Graphics/SSD2805.h"

// Clipping region control
SHORT       _clipRgn;

// Clipping region borders
SHORT       _clipLeft;
SHORT       _clipTop;
SHORT       _clipRight;
SHORT       _clipBottom;

// Color
GFX_COLOR   _color;
#ifdef USE_TRANSPARENT_COLOR
GFX_COLOR   _colorTransparent;
SHORT       _colorTransparentEnable;
#endif

//swap hi-byte to lo-byte and vice versa
#define Swap(x) (((WORD_VAL)x).v[0]<<8 | (((WORD_VAL)x).v[1]&0x00FF))


inline void SSD2805WriteCmd(BYTE cmd)
{
#ifdef USE_SSD2805_CMD_MODE
	DisplayEnable();
	DisplaySetCommand();
	DeviceWrite(cmd);	//pmp write in 8080 addressing
	DisplayDisable();
#elif defined USE_SSD2805_VIDEO_MODE
	//config via SPI mode
#endif
}

#ifdef USE_SSD2805_CMD_MODE
	#if defined (USE_16BIT_PMP)
	inline void SSD2805WriteData(WORD data)
	{
		DisplayEnable();
		DisplaySetData();
		DeviceWrite(data);
		DisplayDisable();
	}
	//#elif defined (USE_8BIT_PMP)
	#else
	inline void SSD2805WriteData(BYTE data)
	{
		DisplayEnable();
		DisplaySetData();
		DeviceWrite((BYTE)data);
		DisplayDisable();
	}
	#endif
#elif defined USE_SSD2805_VIDEO_MODE
	inline void SSD2805WriteData(BYTE data)
	{
		//spi functions
	}
#endif



/************************************************************************
* Function: void SSD2805WriteReg(BYTE reg, WORD data)                                           
*                                                                       
* Overview: this function writes a 16-bit word to the local register 
*			of SSD2805 specified by 'data'
*                                                                       
* Input: 'data' to be written and register address 'reg'
*                                                                       
* Output: none                                                         
*                                                                       
************************************************************************/
void SSD2805WriteReg(BYTE reg, WORD data)
{
	SSD2805WriteCmd(reg);
#if defined (USE_16BIT_PMP)
	SSD2805WriteData(data);
//#elif defined (USE_8BIT_PMP) || defined (USE_SSD2805_VIDEO_MODE)
#else
	SSD2805WriteData((BYTE)data);		//lower byte write first
	SSD2805WriteData((BYTE)(data>>8));
#endif
}

/************************************************************************
* Function: WORD SSD2805ReadReg(BYTE reg)             
*                                                                       
* Overview: this function reads a 16-bit word from the register specified         
*                                                                       
* Input: register address                                                    
*                                                                       
* Output: data read
*                                                                       
************************************************************************/
WORD SSD2805ReadReg(BYTE reg)
{
	WORD_VAL temp;

	DisplayEnable();
	DisplaySetCommand();
	DeviceWrite(reg);
//	SSD2805WriteCmd(0xFA);	//send read command, no need for MCU IF
	DisplaySetData();		//set data for read
#if defined (USE_16BIT_PMP)
	temp.Val = DeviceRead();
//#elif defined (USE_8BIT_PMP) || defined (USE_SSD2805_VIDEO_MODE)
#else
	temp.v[0] = DeviceRead();
	temp.v[1] = DeviceRead();
#endif
	DisplayDisable();

	return (temp.Val);
}
 

BYTE SSD2805ReadDCS(BYTE reg)
{
	WORD_VAL temp;
	WORD byteCount;
	BYTE value;

	temp.Val = SSD2805ReadReg(0xb7); //store present value of 0xB7
	SSD2805WriteReg(0xb7, temp.Val|0x00c0);


	SSD2805WriteReg(0xbc, 0x0000);			//define TDC size
	SSD2805WriteReg(0xbd, 0x0000);			//this is required after ClearDevice(), since 0xbd set to 0x400 in ClearDevice()
	SSD2805WriteCmd(reg);					//register to read from MIPI slave	
	
	//SSD2805WriteDCS(reg, 0, NULL);		//issue a DCS read command e.g. 0x0c to read pixel format

	while(!SSD2805ReadReg(0xc6)&0x01);

	byteCount = SSD2805ReadReg(0xc2);
	DisplayEnable();
	DisplaySetCommand();
	DeviceWrite(0xD7);
	DisplaySetData();		//set data for read
	while(byteCount--)
	{
		value = DeviceRead();
	}
	DisplayDisable();

	SSD2805WriteReg(0xb7, temp.Val);

	return (value);
}

/*********************************************************************
* Function:  SetAddress(WORD x, WORD y)
*
* Overview: Writes Column and Page Addresses as position (x,y)
*
* PreCondition: none
*
* Input: WORD x, WORD y
*
* Output: none
*
* Side Effects: none
* Remarks:  From end-of-column (EC) and end-of-page (EP) limits we know
*			it is possible to scroll either horizontally or vertically
*			but not both.
********************************************************************/

inline void SetAddress(WORD x, WORD y)
{
	SSD2805WriteReg(0xbc, 0x0004);	//Set packet size TDC[15:0]
	SSD2805WriteReg(0xbd, 0x0000);	//Set packet size TDC[31:16]

	#if defined (USE_16BIT_PMP)
                WORD x1 = GetMaxX(), y1=GetMaxY();

		DisplayEnable();
		DisplaySetCommand();
		DeviceWrite(0x2A);
		DisplaySetData();
		DeviceWrite(Swap(x));	//SC[15:0]
		DeviceWrite(Swap(x1));	//EC[15:0]	
		DisplaySetCommand();
		DeviceWrite(0x2B);
		DisplaySetData();
		DeviceWrite(Swap(y));	//SP[15:0]
		DeviceWrite(Swap(y1));	//EP[15:0]
		DisplayDisable();	

	//#elif defined (USE_8BIT_PMP)
	#else
		SSD2805WriteCmd(0x2A);	//DCS command to MIPI slave, Column address set
		SSD2805WriteData((BYTE)(x>>8));				//SC[15:8]
		SSD2805WriteData((BYTE)x);				//SC[7:0]
		SSD2805WriteData((BYTE)((GetMaxX())>>8));	//EC[15:8]
		SSD2805WriteData((BYTE)(GetMaxX()));		//EC[7:0]
		SSD2805WriteCmd(0x2B);	//DCS command to MIPI slave, Page address set
		SSD2805WriteData((BYTE)(y>>8));				//SP[15:8]
		SSD2805WriteData((BYTE)y);					//SP[7:0]
		SSD2805WriteData((BYTE)((GetMaxY())>>8));	//EP[15:8]
		SSD2805WriteData((BYTE)GetMaxY());			//EP[7:0]

	#endif
}


/*********************************************************************
* Function:  void ResetDevice()
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: resets SSD2805, initializes PMP/SPI interface
*
* Note: There are two operating clock speed before and after PLL
*		locked in SSD2805. External oscillator is 20MHz. After
*		PLL, the operating frequency can be as high as 500MHz.
*		
********************************************************************/
#ifdef USE_SSD2805_CMD_MODE
void ResetDevice(void)
{
    SSD_PowerOn();
    SSD_PowerConfig();			// VDDIO switch for SSD2805

    DisplayCmdDataConfig();     // enable RS line
    DisplayDisable();
    DisplayConfig();			// CSX# as chip select, driven high on start-up
 
    DisplayBacklightOn();       // initially set the backlight to on
    DisplayBacklightConfig();   // set the backlight control pin
   
    /////////////////////////////////////////////////////////////////////
    // Initialize PMP interface for 8080
    /////////////////////////////////////////////////////////////////////
	#if defined (USE_GFX_PMP)
	    // PMP setup
	    PMMODE = 0;
	    PMCON = 0;
	    PMMODEbits.MODE = 2;                //Intel 80 master interface
            PMMODEbits.WAITB = 0;		//Before PLL is locked, SSD2805 running @20MHz (dictated by external oscillator)
            PMMODEbits.WAITM = 15;		//A min. WR# cycle of 3T is required with T=(1/20)*1000ns, result: WR# > 150ns.
            PMMODEbits.WAITE = 0;		//Thus it is not advised to run too fast for WR# strobe before PLL is locked.
						//WAITB<1:0>, WAITM<3:0> & WAITE<1:0> are parameters for data setup time, WR#/RD# strobe 
						//time, and data hold time. 
						//Dump screen from a logic analyzer shows a WR# low pulse time measured at 
						//200ns at C32 full optimization for speed. It is well within the limit 150ns.
	    #if defined(USE_16BIT_PMP)
	    PMMODEbits.MODE16 = 1;              // 16 bit mode
	    #elif defined(USE_8BIT_PMP)
	    PMMODEbits.MODE16 = 0;              // 8 bit mode
	    #endif
  
    	PMCONbits.PTRDEN = 1;               // enable RD line
    	PMCONbits.PTWREN = 1;               // enable WR line			
    	PMCONbits.PMPEN = 1;                // enable PMP	
	#endif
		
	SSD_ResetDisable();		// Keep RESET pin high for SSD2805 & LH154Q01  >250ms (mainly for LH154Q01) 		
    SSD_ResetConfig();			// set RESET pin an output
	DelayMs(300);			// delay for 300ms
    SSD_ResetEnable();                  // Keep RESET pin low for SSD2805 & LH154Q01 >5ms
	DelayMs(10);
	SSD_ResetDisable();		// now release RESET from low and keep RESET high for normal operation
	DelayMs(1);
	LH_PowerOn();			// turn VDDIO of LH154Q01 on
	LH_PowerConfig();

	DelayMs(10);			// wait 10ms to start MIPI command

		//Step 1: Set PLL
		SSD2805WriteReg(0xba, 0x000f);	//PLL 	= clock*MUL/(PDIV*DIV) 
						//		= clock*(BAh[7:0]+1)/((BAh[15:12]+1)*(BAh[11:8]+1))
						//		= 20*(0x0f+1)/1*1 = 20*16 = 320MHz
						//Remark: 350MHz >= fvco >= 225MHz for SSD2805 since the max. speed per lane is 350Mbps
		SSD2805WriteReg(0xb9, 0x0001);	//enable PLL

		//	while((SSD2805ReadReg(0xc6)&0x0080)!=0x0080)	//infinite loop to wait for PLL to lock
		//		;

		DelayMs(2);		//simply wait for 2 ms for PLL lock, more stable as SSD2805ReadReg(arg) doesn't work at full compiler optimzation
		//Step 2: Now it is safe to set PMP at max. speed
		PMCONbits.PMPEN = 0;				
		PMMODEbits.WAITM = 0;				//after PLL locked, it is possible to set PMP at max. speed
		PMCONbits.PMPEN = 1;
		
		//Step 3: set clock control register for SYS_CLK & LP clock speed
		//SYS_CLK = TX_CLK/(BBh[7:6]+1), TX_CLK = external oscillator clock speed
		//In this case, SYS_CLK = 20MHz/(1+1)=10MHz. Measure SYS_CLK pin to verify it.
		//LP clock = PLL/(8*(BBh[5:0]+1)) = 320/(8*(4+1)) = 8MHz, conform to AUO panel's spec, default LP = 8Mbps
		//S6D04D2 is the controller of AUO 1.54" panel.
		SSD2805WriteReg(0xBB, 0x0044);
		SSD2805WriteReg(0xD6, 0x0100);	//output sys_clk for debug. Now check sys_clk pin for 10MHz signal

		//Step 4: Set MIPI packet format
		SSD2805WriteReg(0xB7, 0x0243);	//EOT packet enable, write operation, it is a DCS packet
										//HS clock is disabled, video mode disabled, in HS mode to send data

		//Step 5: set Virtual Channel (VC) to use
		SSD2805WriteReg(0xB8, 0x0000);


		//Step 6: Now write DCS command to AUO panel for system power-on upon reset
		SSD2805WriteReg(0xbc, 0x0000);			//define TDC size
		SSD2805WriteReg(0xbd, 0x0000);
		SSD2805WriteCmd(0x11);				//DCS sleep-out command

                DelayMs(100);					//wait for AUO/LG  panel after sleep out

		//Step 7: Now configuration parameters sent to AUO
		SSD2805WriteReg(0xbc, 0x0001);			//define TDC size to be 1
		SSD2805WriteReg(0xbd, 0x0000);
		SSD2805WriteCmd(0x3a);				//DCS command sent to register @ 0x3a, set 16-bit pixel format
		SSD2805WriteData(0x05);				//Set 0x3a to 0x05 (16-bit color)

		SetColor(0);					//debug here for different color on POR
		ClearDevice();

		SSD2805WriteReg(0xbc, 0x0000);			//define TDC size
		SSD2805WriteReg(0xbd, 0x0000);			//this is required after ClearDevice(), since 0xbd set to 0x400 in ClearDevice()
		SSD2805WriteCmd(0x29);				//display ON DCS command to AUO panel
}
#endif //USE_SSD2805_CMD_MODE

#ifdef USE_TRANSPARENT_COLOR
/*********************************************************************
* Function:  void TransparentColorEnable(GFX_COLOR color)
*
* Overview: Sets current transparent color.
*
* PreCondition: none
*
* Input: color - Color value chosen.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void TransparentColorEnable(GFX_COLOR color)
{
    _colorTransparent = color;    
    _colorTransparentEnable = TRANSPARENT_COLOR_ENABLE;

}
#endif


/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* Overview: Put pixel with the given x,y coordinate position.
*
* PreCondition: none
*
* Input: x - x position of the pixel.
*		 y - y position of the pixel.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
#ifdef USE_SSD2805_CMD_MODE
void PutPixel(SHORT x, SHORT y)
{
	SetAddress(x,y);

	SSD2805WriteReg(0xbc, 0x0002);	//write to registers of SSD2805 at 0xbc, 0xbd
	SSD2805WriteReg(0xbd, 0x0000);	//to set the size of block write, this time,
									//a single pixel of 2 bytes required for 16-bit color
	DisplayEnable();										
	DisplaySetCommand(); DeviceWrite(0x2c);		
								
	DisplaySetData();
		#if defined (USE_16BIT_PMP)
			DeviceWrite(_color);
		//#elif defined (USE_8BIT_PMP)
		#else	
			DeviceWrite(_color);
			DeviceWrite(_color>>8);
		#endif

	DisplayDisable();
}
#endif	//USE_SSD2805_CMD_MODE

/*********************************************************************
* Function: WORD GetPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: pixel position
*
* Output: pixel color
*
* Side Effects: none
*
* Overview: returns pixel at given position
*
* Note: Not working yet
*
********************************************************************/
#ifdef USE_SSD2805_CMD_MODE
GFX_COLOR GetPixel(SHORT x, SHORT y)
{
	GFX_COLOR color;

	SetAddress(x,y);

	color = SSD2805ReadDCS(0x2E);
	color = (SSD2805ReadDCS(0x2E)<<8)|color;
	return (color);
}
#endif

/*********************************************************************
* Function: IsDeviceBusy()
*
* Overview: Returns non-zero if LCD controller is busy 
*           (previous drawing operation is not completed).
*
* PreCondition: none
*
* Input: none
*
* Output: Busy status.
*
* Side Effects: none
*
********************************************************************/

WORD IsDeviceBusy(void)
{  
    return (0);
}

/*********************************************************************
* Function: void ClearDevice(void)
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: clears screen with current color 
*
* Note: none
*
********************************************************************/
void ClearDevice(void)
{
	DWORD_VAL byteCount;
	
	DWORD pixelCount, i;

	byteCount.Val = (DWORD)DISP_HOR_RESOLUTION*DISP_VER_RESOLUTION*(COLOR_DEPTH/8);

	SetAddress(0,0);
	SSD2805WriteReg(0xbc, byteCount.w[0]);	//write to registers of SSD2805 at 0xbc, 0xbd
	SSD2805WriteReg(0xbd, byteCount.w[1]);	//to set the size of block write
	SSD2805WriteReg(0xbe, 0x0400);		//set threshold to max size for block write

	DisplayEnable();										
	DisplaySetCommand(); 
	DeviceWrite(0x2c);			

	#if defined (USE_16BIT_PMP)
	pixelCount = byteCount.Val;
	//#elif defined (USE_8BIT_PMP)
	#else
	pixelCount = byteCount.Val>>1;
	#endif

	DisplaySetData();
	for(i=0; i<pixelCount; i++)
		{
		#if defined (USE_16BIT_PMP)
			DeviceWrite(_color);
		//#elif defined (USE_8BIT_PMP)
		#else
			DeviceWrite(_color);
			DeviceWrite(_color>>8);
		#endif
		}
	DisplayDisable();
}

#endif //#if defined (GFX_USE_SSD2805_MIPI_BRIDGE)
