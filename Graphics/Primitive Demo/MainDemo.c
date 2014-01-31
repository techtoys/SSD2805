/*****************************************************************************
 * SSD2805 Demo Application
 * This program shows how to drive SSD2805 MIPI bridge with a PIC32 MCU.
 *****************************************************************************
 * FileName:        MainDemo.c
 * Processor:       PIC32MX250F128D
 * Compiler:        MPLAB XC32 v1.20
 * Company:         TechToys Company
 *
 * Date         Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 24-01-2014	Programmer : John Leung @ TechToys
 *****************************************************************************/

/*
 *****************************************************************************
 * Project modified to display single pixel on 1.54" LH154Q01 MIPI display
 * Hardware:
 * (Host)
 * PIC32MX250 MIPI Driver Board configured in 8-bit 8080 interface mode
 * PCB revision: 23122013
 * (SSD2805 MIPI Bridge)
 * PCB revision 27092013
 * (Display is 1.54" LH154Q01 MIPI Display)
 * PCB version 01112013
 *****************************************************************************
*/
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "Graphics/DisplayDriver.h"
#include "TimeDelay.h"

// Configuration bits
#if defined EXPLORER_16 || defined __32MX250F128D__
#pragma config FPLLIDIV = DIV_2
#else
#pragma config FPLLIDIV = DIV_3
#pragma config UPLLIDIV = DIV_3         // USB PLL Input Divider since it is a 12MHz crystal onboard
#endif

#ifdef __32MX250F128D__	//set to 40MHz SYSCLK, PBCLK
		//8MHz external crystal onboard
		#pragma config FPLLMUL = MUL_20, FPLLODIV = DIV_2
		#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_1, FSOSCEN = OFF
		#pragma config UPLLIDIV = DIV_2, FVBUSONIO = OFF, FUSBIDIO = OFF
		#pragma config FWDTEN = OFF, JTAGEN = OFF, DEBUG = ON    
		#pragma config IESO = OFF, IOL1WAY = ON, PMDL1WAY = ON, OSCIOFNC = OFF       
		#pragma config ICESEL = ICS_PGx4  
#else
        #pragma config FPLLMUL  = MUL_20        // PLL Multiplier
        #pragma config FPLLODIV = DIV_1         // PLL Output Divider
        #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
        #pragma config FWDTEN   = OFF           // Watchdog Timer
        #pragma config WDTPS    = PS1           // Watchdog Timer Postscale
        #pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
        #pragma config OSCIOFNC = OFF           // CLKO Enable
        #pragma config POSCMOD  = HS            // Primary Oscillator
        #pragma config IESO     = OFF           // Internal/External Switch-over
        #pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (KLO was off)
        #pragma config FNOSC    = PRIPLL        // Oscillator Selection
        #pragma config CP       = OFF           // Code Protect
        #pragma config BWP      = OFF           // Boot Flash Write Protect
        #pragma config PWP      = OFF           // Program Flash Write Protect
        #pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select
#endif
      
/////////////////////////////////////////////////////////////////////////////
//                                  MAIN
/////////////////////////////////////////////////////////////////////////////
int main(void)
{
    WORD x, y;

        //MCHP MCU specific
        SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES|SYS_CFG_PCACHE);
        INTEnableSystemMultiVectoredInt();
	mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks
        ResetDevice();
        while(1)
        {
            SetColor(BRIGHTBLUE);
            ClearDevice();
            DelayMs(1000);
            SetColor(BRIGHTYELLOW);
            for(x=0; x<240;x++)
            {
                for(y=0; y<240; y++) PutPixel(x,y);
            }
            DelayMs(1000);
        }
}