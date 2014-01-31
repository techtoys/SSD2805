/*********************************************************************
 *
 *	Hardware specific definitions
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
 * Processor:       PIC32MX250F128D
 * Compiler:        Microchip XC32 v1.20 or higher

 * Company:         TechToys Company
 *
 * Software License Agreement
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS?WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Date		    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 02/24/11	    For Graphics Library Version 3.00
 * 01/20/12	    Modified for Graphics Library Version 3.04
 ********************************************************************/

/*
*********************************************************************
* Hardware profile for
* PIC32MX575F512L as the host in MCU mode, 8-bit 8080 (PCB Version 09102013)
* SSD2805 MIPI Bridge (PCB revision 27092013)
* LG LH154Q01 1.54" 1-lane MIPI display application board (PCB Rev10102013)
*
* Programmer: John Leung @ TechToys
* Web: www.TechToys.com.hk
* 18-10-2013
*********************************************************************
*/

#ifndef __HARDWARE_PROFILE_H
    #define __HARDWARE_PROFILE_H

/*********************************************************************
* PIC Device Specific includes
*********************************************************************/
    #include "Compiler.h"

/*********************************************************************
* GetSystemClock() returns system clock frequency.
*
* GetPeripheralClock() returns peripheral clock frequency.
*
* GetInstructionClock() returns instruction clock frequency.
*
********************************************************************/

/*********************************************************************
* Macro: #define	GetSystemClock()
*
* Overview: This macro returns the system clock frequency in Hertz.
*			* value is 8 MHz x 4 PLL for PIC24
*			* value is 8 MHz/2 x 18 PLL for PIC32
*
********************************************************************/
    #if defined(__PIC24F__)
        #define GetSystemClock()    (32000000ul)
    #elif defined(__PIC32MX__)
        #if defined(__32MX250F128D__)
        #define GetSystemClock()    (40000000ul)
        #else
        #define GetSystemClock()    (80000000ul)
        #endif
    #elif defined(__dsPIC33F__) || defined(__PIC24H__)
        #define GetSystemClock()    (80000000ul)
    #elif defined(__dsPIC33E__) || defined(__PIC24E__)
    	#define GetSystemClock()    (120000000ul)
    #endif

/*********************************************************************
* Macro: #define	GetPeripheralClock()
*
* Overview: This macro returns the peripheral clock frequency
*			used in Hertz.
*			* value for PIC24 is <PRE>(GetSystemClock()/2) </PRE>
*			* value for PIC32 is <PRE>(GetSystemClock()/(1<<OSCCONbits.PBDIV)) </PRE>
*
********************************************************************/
    #if defined(__PIC24F__) || defined(__PIC24H__) || defined(__dsPIC33F__) || defined(__dsPIC33E__) || defined(__PIC24E__)
        #define GetPeripheralClock()    (GetSystemClock() / 2)
    #elif defined(__PIC32MX__)
        #define GetPeripheralClock()    (GetSystemClock() / (1 << OSCCONbits.PBDIV))
    #endif

/*********************************************************************
* Macro: #define	GetInstructionClock()
*
* Overview: This macro returns instruction clock frequency
*			used in Hertz.
*			* value for PIC24 is <PRE>(GetSystemClock()/2) </PRE>
*			* value for PIC32 is <PRE>(GetSystemClock()) </PRE>
*
********************************************************************/
    #if defined(__PIC24F__) || defined(__PIC24H__) || defined(__dsPIC33F__) || defined(__dsPIC33E__) || defined(__PIC24E__)
        #define GetInstructionClock()   (GetSystemClock() / 2)
    #elif defined(__PIC32MX__)
        #define GetInstructionClock()   (GetSystemClock())
    #endif


/* ################################################################## */
/*********************************************************************
* START OF GRAPHICS RELATED MACROS
********************************************************************/
/* ################################################################## */

#define GFX_USE_SSD2805_MIPI_BRIDGE
#define USE_GFX_PMP		//use 8080-addressing, it is PMP for Microchip

//Two possible MCU interfaces, 8-bit 8080 or 16-bit 8080
//It is important to change dip switch for PS[3:2]
#if defined (USE_GFX_PMP)
#define USE_SSD2805_CMD_MODE
#endif

#define GFX_USE_DISPLAY_PANEL_TFT_LH154Q01	//LG 1.54" TFT in 1-lane Mipi interface 240x240

// -----------------------------------
// For LG display 1.54" 240x240
// -----------------------------------
#if defined (GFX_USE_DISPLAY_PANEL_TFT_LH154Q01)
	#define DISP_ORIENTATION    0
    #define DISP_HOR_RESOLUTION 240
    #define DISP_VER_RESOLUTION 240
    #define GFX_LCD_TYPE            GFX_LCD_TFT
#endif



/*********************************************************************
* HARDWARE PROFILE FOR DISPLAY CONTROLLER INTERFACE
*********************************************************************/
		// -----------------------------------
		// When using AIC1896PG as the backlight driver
		// -----------------------------------
	    // Depending on the backlight driver (IC) used, the backlight control may be
	    // different. This portion sets up the correct backlight level to
	    // turn on or off the backlight.
	    #define BACKLIGHT_ENABLE_LEVEL      1
	    #define BACKLIGHT_DISABLE_LEVEL     0

		//VDDIO configuration. VDDIO is mosfet controlled for ON/OFF for SSD2805 VDDIO
                //There is no pin assigned for SSD power control on PIC32MX250F128D MIPI Driver board
                #define SSD_PowerConfig()      		//TRISDbits.TRISD9  = 0
                #define SSD_PowerOn()            	//LATDbits.LATD9    = 0
                #define SSD_PowerOff()    		//LATDbits.LATD9    = 1

                //ANSELx has default value of 0xffff for analog input. It is required to set it digital
		//for individual pin if digital operation is required.
		#define LH_PowerConfig()    		{TRISCbits.TRISC0 = 0; ANSELCbits.ANSC0 = 0;}
		#define LH_PowerOn()			LATCbits.LATC0	 = 0
		#define LH_PowerOff()			LATCbits.LATC0   = 1

		//For some unknown reason, it is not possible to set RC2 as GPIO when PMP module is enabled
		//To work around RC2 is set as PMA2 address line.
                #define SSD_ResetConfig()        	{TRISCbits.TRISC4  = 0; PMAENbits.PTEN4 = 1;}
		#define SSD_ResetEnable()		PMADDR &= 0xffef	//PMA4 set 0
		#define SSD_ResetDisable() 		PMADDR |= 0x0010	//PMA4 set 1

                //definitions for DisplayResetXXXX() to maintain consistency in gfxpmp.h only
                #define DisplayResetConfig()    SSD_ResetConfig()
                #define DisplayResetEnable()    SSD_ResetEnable()
                #define DisplayResetDisable()   SSD_ResetDisable()

        // Definitions for backlight control pin
        #define DisplayBacklightConfig()    {TRISCbits.TRISC3 = 0; ANSELCbits.ANSC3 = 0;}
        #define DisplayBacklightOn()        LATCbits.LATC3 = BACKLIGHT_ENABLE_LEVEL
        #define DisplayBacklightOff()       LATCbits.LATC3 = BACKLIGHT_DISABLE_LEVEL

		/*
		*****************************************************************
		* Note: There are two operating clock speed before and after PLL
		*		locked in SSD2805. External oscillator is 20MHz. After
		*		PLL, the operating frequency can be as high as 500MHz.
		*		PMP_DATA_SETUP_TIME, _WAIT_TIME, & _HOLD_TIME set here
		*		fit the timing requirement before PLL is locked, therefore
		*		they are timing for operating freq. @ 20MHz.
		*		New PMP timing values are set in ResetDevice() of SSD2805.c
		*		for faster data I/O after PLL is locked.
		*****************************************************************
		*/
      	#if defined (USE_GFX_PMP)
            #define PMP_DATA_SETUP_TIME	(18)
            #define PMP_DATA_WAIT_TIME	(82)
            #define PMP_DATA_HOLD_TIME	(0)

            #define DisplayCmdDataConfig()      {TRISCbits.TRISC7 = 0; PMAENbits.PTEN0 =1;}	//assumed IF_MUX=0, 8080 interface connection checked OK
            #define DisplaySetCommand()         PMADDR &= 0xfffe	//PMA0 set 0
            #define DisplaySetData()            PMADDR |= 0x0001	//PMA0 set 1

            #define DisplayConfig()             {TRISBbits.TRISB15 = 0; ANSELBbits.ANSB15 = 0;} //CSX, chip select for MCU interface
            #define DisplayEnable()             LATBbits.LATB15   = 0				//connection checked OK
            #define DisplayDisable()            LATBbits.LATB15   = 1

	//Remarks: WR# & RD# wired to RB3/PMWR & RB13/PMRD lines respectively
	//WR# and RD# are strobed automatically by PMP module. No definition here is required.

		#else
			#error "USE_GFX_PMP is defined but no timing values are defined in hardware profile."
		#endif	//#ifdef (USE_GFX_PMP)


/*********************************************************************
* IOS FOR THE SWITCHES (SIDE BUTTONS)
*********************************************************************/
typedef enum
{
    HW_BUTTON_PRESS = 0,
    HW_BUTTON_RELEASE = 1
}HW_BUTTON_STATE;

#ifdef GFX_USE_DISPLAY_PANEL_TFT_LH154Q01
    #define HardwareButtonInit()		{TRISAbits.TRISA7 = 1;TRISAbits.TRISA9 = 1; TRISAbits.TRISA10 = 1;}
	#define GetHWButtonProgram()		PORTAbits.RA9
	#define GetHWButtonUp()			PORTAbits.RA10
	#define GetHWButtonDown()		PORTAbits.RA7
#endif

#endif // __HARDWARE_PROFILE_H




