//#############################################################################
//
// FILE:    hrpwm_ex2_prdupdown_sfo_v8.c
//
// TITLE:   HRPWM SFO V8 High-Resolution Period (Up-Down Count) example
//
//! \addtogroup bitfield_example_list
//! <h1>HRPWM Period Up-Down Count</h1>
//!
//! This example modifies the MEP control registers to show edge displacement
//! for high-resolution period with ePWM in Up-Down count mode
//! due to the HRPWM control extension of the respective ePWM module.
//!
//! This example calls the following TI's MEP Scale Factor Optimizer (SFO)
//! software library V8 functions:
//!
//! \b int \b SFO(); \n
//! updates MEP_ScaleFactor dynamically when HRPWM is in use
//! updates HRMSTEP register (exists only in EPwm1Regs register space)
//! with MEP_ScaleFactor value
//! - returns 2 if error: MEP_ScaleFactor is greater than maximum value of 255
//!   (Auto-conversion may not function properly under this condition)
//! - returns 1 when complete for the specified channel
//! - returns 0 if not complete for the specified channel
//!
//! This example is intended to explain the HRPWM capabilities. The code can be
//! optimized for code efficiency. Refer to TI's Digital power application
//! examples and TI Digital Power Supply software libraries for details.
//!
//! To run this example:
//! -# Run this example at maximum SYSCLKOUT
//! -# Activate Real time mode
//! -# Run the code
//!
//! \b External \b Connections \n
//!  - Monitor ePWM1 A/B pins on an oscilloscope.
//!
//! \b Watch \b Variables \n
//!  - UpdateFine - Set to 1 use HRPWM capabilities and observe in fine MEP
//!                 steps(default)
//!                 Set to 0 to disable HRPWM capabilities and observe in
//!                 coarse SYSCLKOUT cycle steps
//!
//
//#############################################################################
// $TI Release: F28004x Support Library v1.10.00.00 $
// $Release Date: Tue May 26 17:06:03 IST 2020 $
// $Copyright:
// Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//

#include "F28x_Project.h"
#include "SFO_V8.h"
#include "PWM_CONFIG.h"
#include "ADC_CONFIG.h"
#include "GPIO_CONFIG.h"
extern void InitCpuTimers(void);
extern void ConfigCpuTimer(struct CPUTIMER_VARS *, float, float);
extern void Init_ADC_converter(void);
interrupt void adc_isr(void);
interrupt void cpu_timer0_isr(void);
__interrupt void adcA1ISR(void);
//
// Defines
//
#define PWM_CH            3        // # of PWM channels - 1
#define STATUS_SUCCESS    1
#define STATUS_FAIL       0
#define RESULTS_BUFFER_SIZE     256
int32  adcAResults1=0;
uint16_t adcAResults[RESULTS_BUFFER_SIZE];   // Buffer for results
uint16_t adcAResults2;   // Buffer for results
uint16_t index;                              // Index into result buffer
volatile uint16_t bufferFull;                // Flag to indicate buffer is full


//
// Globals
//
uint16_t UpdateFine, PeriodFine, status;
int MEP_ScaleFactor; // Global variable used by the SFO library
                     // Result can be used for all HRPWM channels
                     // This variable is also copied to HRMSTEP
                     // register by SFO(0) function.

// Used by SFO library (ePWM[0] is a dummy value that isn't used)
volatile struct EPWM_REGS *ePWM[PWM_CH] = {&EPwm1Regs, &EPwm1Regs};
//volatile struct AdcRegs *Adc[PWM_CH] = {&Adc1Regs, &Adc1Regs};

//
// Function Prototypes
//
void initHRPWM1GPIO(void);
void configHRPWM(uint16_t period);
void error(void);

//
// Main
//
void main(void)
{
    uint16_t i;

    //
    // Initialize device clock and peripherals
    //
    InitSysCtrl();

    //
    // Initialize GPIO
    //
    InitGpio();


    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    InitPieVectTable();
    EALLOW;
    PieVectTable.ADCA1_INT = &adcA1ISR;     // Function for ADCA interrupt 1
    EDIS;
    //

    // Configure the ADC and power it up
     //
     initADC();

     //
     // Configure the ePWM
     //
     initEPWM();

     //
     // Setup the ADC for ePWM triggered conversions on channel 1
     //
     initADCSOC();


    initHRPWM1GPIO();

    IER |= M_INT1;  // Enable group 1 interrupts

    EINT;           // Enable Global interrupt INTM
    ERTM;           // Enable Global realtime interrupt DBGM


    //
    // Setup example variables
    //
    UpdateFine = 1;
    PeriodFine = 0;
    status = SFO_INCOMPLETE;

    for(index = 0; index < RESULTS_BUFFER_SIZE; index++)
    {
        adcAResults[index] = 0;
    }

    index = 0;
    bufferFull = 0;

    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;


    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;
    //
    // ePWM and HRPWM register initialization
    //
    for(i=1; i<PWM_CH; i++)
    {
        // Change clock divider to /1
        // (PWM clock needs to be > 60MHz)
        (*ePWM[i]).TBCTL.bit.HSPCLKDIV = 0;
    }
    configHRPWM(500);

    //
    // Calling SFO() updates the HRMSTEP register with calibrated MEP_ScaleFactor.
    // HRMSTEP must be populated with a scale factor value prior to enabling
    // high resolution period control.
    //
    while(status == SFO_INCOMPLETE)
    {

        status = SFO();
        if (status == SFO_ERROR)
        {
            error();    // SFO function returns 2 if an error occurs & # of MEP
        }               // steps/coarse step exceeds maximum of 255.

    }




    for(;;)
    {
       // adcAResults1 = AdcaResultRegs.ADCRESULT0;
        //
        // Sweep PeriodFine as a Q16 number from 0.2 - 0.999
        //
        for(PeriodFine = 0x3333; PeriodFine < 0xFFFF; PeriodFine++)
        {
            if(UpdateFine)
            {
                //

                for(i=1; i<PWM_CH; i++)
                {
                    DELAY_US(2000);
                 //   (*ePWM[i]).TBPRDHR = PeriodFine; //In Q16 format
                  // EPwm2Regs.TBPHS.bit.TBPHSHR = PeriodFine;  // 750 for  minimum voltage  phase shift 695=100

                }
            }
            else
            {
                //
                // No high-resolution movement on TBPRDHR.
                //
                for(i=1; i<PWM_CH; i++)
                {
                    (*ePWM[i]).TBPRDHR = 0;
                }
            }


            //
            status = SFO(); // in background, MEP calibration module
                            // continuously updates MEP_ScaleFactor

            if(status == SFO_ERROR)
            {
                error();   // SFO function returns 2 if an error occurs & # of
                           // MEP steps/coarse step exceeds maximum of 255.
            }
        } // end PeriodFine for loop
    } // end infinite for loop



}





__interrupt void adcA1ISR(void)
{
    //
    // Add the latest result to the buffer
    // ADCRESULT0 is the result register of SOC0
    adcAResults[index++] = AdcaResultRegs.ADCRESULT0;
    adcAResults2 = AdcaResultRegs.ADCRESULT0;

    //
    // Set the bufferFull flag if the buffer is full
    //
    if(RESULTS_BUFFER_SIZE <= index)
    {
        index = 0;
        bufferFull = 1;
    }

    //
    // Clear the interrupt flag
    //
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    //
    // Check if overflow has occurred
    //
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    //
    // Acknowledge the interrupt
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//
// End of File
//



//
// error - Halt debugger when error occurs
//
void error (void)
{
    ESTOP0;         // Stop here and handle error
}

//
// End of file
//
