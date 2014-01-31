/*****************************************************************************
 *  Module for Microchip Graphics Library
 *  Solomon Systech. SSD2805 MCU interface driver
 *****************************************************************************
 * FileName:        SSD2805.h
 * Processor:       PIC32
 * Compiler:        MPLAB C32/XC32
 * Company:         TechToys Company
 *
 * Software License Agreement
 *
 * You should refer to the license agreement accompanying this Software
 * for additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY
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
*
* Regarding timing:
*			Minimum WR# low time (PWcsl)is 3T ns with T=PLL output clock.
*			Before PLL is locked, SSD2805 runs at the speed
*			of external oscillator (20MHz) in our hardware. 
*			Therefore PWcsl > 3T=3*1/20=150ns.
*			Similarly, PWcsh > 3T=150ns.
*			Therefore a WR# strobe low->high is 300ns before PPL is locked.
*			After PLL is locked at 500MHz (say), the min. write speed
*			can be calculated as 2*3*1/500 = 12ns!
*			Source of timing comes from SSD2805 Advance Information Rev1.1 
*****************************************************************************
*/
#ifndef _SSD2805_H
#define _SSD2805_H

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"

#if defined (USE_GFX_PMP)
    #include "Graphics/gfxpmp.h"
#elif defined (USE_GFX_EPMP)
    #include "Graphics/gfxepmp.h"	//for certain PIC24 device only, not relevant here
#endif 

#if defined (USE_SSD2805_VIDEO_MODE)
	#include "drv_spi.h"
#endif

BYTE SSD2805ReadDCS(BYTE reg);
/************************************************************************
* Function: void SSD2805WriteReg(BYTE reg, WORD data)                                           
*                                                                       
* Overview: this function writes a 16-bit word to the register specified
*                                                                       
* Input: data to be written and register address 'reg'
*                                                                       
* Output: none                                                         
*                                                                       
************************************************************************/
void SSD2805WriteReg(BYTE reg, WORD data);

/************************************************************************
* Function: WORD SSD2805ReadWord(BYTE reg)             
*                                                                       
* Overview: this function reads a 16-bit word from the register specified         
*                                                                       
* Input: register address                                                    
*                                                                       
* Output: data read
*                                                                       
************************************************************************/
WORD SSD2805ReadReg(BYTE reg);

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
void ClearDevice(void);

#endif
