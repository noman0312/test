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

//
// Defines
//
#define PWM_CH            2        // # of PWM channels - 1
#define STATUS_SUCCESS    1
#define STATUS_FAIL       0

//
// Globals
//
uint16_t UpdateFine, PeriodFine, status;
int MEP_ScaleFactor; // Global variable used by the SFO library
                     // Result can be used for all HRPWM channels
                     // This variable is also copied to HRMSTEP
                     // register by SFO(0) function.

// Used by SFO library (ePWM[0] is a dummy value that isn't used)
volatile struct EPWM_REGS *ePWM[12] = {&EPwm1Regs, &EPwm1Regs , &EPwm2Regs, &EPwm2Regs,&EPwm3Regs, &EPwm3Regs , &EPwm4Regs, &EPwm4Regs,&EPwm5Regs, &EPwm5Regs , &EPwm6Regs, &EPwm6Regs};

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

    //
    // Initialize device clock and peripherals
    //
    InitSysCtrl();

    //
    // Initialize GPIO
    //
    InitGpio();
    initHRPWM1GPIO();

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

    //
    // Setup example variables
    //
    UpdateFine = 1;
    PeriodFine = 0;
    status = SFO_INCOMPLETE;

    //
    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;

    //
    // ePWM and HRPWM register initialization
    //

    configHRPWM(500);

    //
    // Calling SFO() updates the HRMSTEP register with calibrated MEP_ScaleFactor.
    // HRMSTEP must be populated with a scale factor value prior to enabling
    // high resolution period control.
    //
    while(1)
    {
                  // steps/coarse step exceeds maximum of 255.
    }


}


//
// configHRPWM - Configures all ePWM channels and sets up HRPWM
//                on ePWMxA channels &  ePWMxB channels
//
void configHRPWM(uint16_t period)
{


    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;   // Disable TBCLK within the EPWM



    EPwm1Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
            EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
            EPwm1Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
            EPwm1Regs.AQCTLA.all = 0x0009;      // set ePWM1A on CMPA up//0x0006//0x0009
            EPwm1Regs.AQCTLB.all = 0x0006;      // clear ePWM1B on CMPA up
            EPwm1Regs.DBCTL.bit.HALFCYCLE =1;

                                                    // set   ePWM1B on CMPA down

            EPwm1Regs.TBPRD = period;           // 1KHz - PWM signal
             EPwm1Regs.CMPA.bit.CMPA  = period/2;
              //dead time
           //    EPwm1Regs.DBRED = DT_R;              // 53 nanoseconds delay
            //   EPwm1Regs.DBFED = DT_F;              // for rising and falling edge
               EPwm1Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
               EPwm1Regs.DBCTL.bit.POLSEL = 2;     // S3=1 inverted signal at ePWM1B
               EPwm1Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED

               //phase shift enable
              EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;



              EPwm2Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
                      EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
                      EPwm2Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
                      EPwm2Regs.AQCTLA.all = 0x0006;      // set ePWM1A on CMPA up
                      EPwm2Regs.AQCTLB.all = 0x0009;      // clear ePWM1B on CMPA up
                      EPwm1Regs.DBCTL.bit.HALFCYCLE =1;

                                                              // set   ePWM1B on CMPA down
                      EPwm2Regs.TBPRD = period;           // 1KHz - PWM signal
                      EPwm2Regs.CMPA.bit.CMPA  = period/2;
                         //Dead time
                   //     EPwm2Regs.DBRED = DT_R;              // 53 nanoseconds delay
                   //      EPwm2Regs.DBFED = DT_F;              // for rising and falling edge
                         EPwm2Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
                         EPwm2Regs.DBCTL.bit.POLSEL = 2;     // S3=1 inverted signal at ePWM1B
                         EPwm2Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED
                         //
                             EPwm2Regs.TBCTL.bit.PHSEN = 1;       // enable phase shift for ePWM3
                             EPwm2Regs.TBCTL.bit.PHSDIR = 0; // Count up after sync
                             EPwm2Regs.TBPHS.bit.TBPHS = 100;  // 750 for  minimum voltage  phase shift 695=100
                             EPwm2Regs.TBPHS.bit.TBPHSHR = 100;  // 750 for  minimum voltage  phase shift 695=100
                             EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;
                        //     EPwm2Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high








                             EPwm3Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
                                     EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
                                     EPwm3Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
                                     EPwm3Regs.AQCTLA.all = 0x0006;      // set ePWM1A on CMPA up
                                     EPwm3Regs.AQCTLB.all = 0x0009;      // clear ePWM1B on CMPA up
                                     EPwm1Regs.DBCTL.bit.HALFCYCLE =1;

                                                                             // set   ePWM1B on CMPA down
                                     EPwm3Regs.TBPRD = period;           // 1KHz - PWM signal
                                     EPwm3Regs.CMPA.bit.CMPA  = period/2;
                                        //Dead time
                                  //     EPwm3Regs.DBRED = DT_R;              // 53 nanoseconds delay
                                  //      EPwm3Regs.DBFED = DT_F;              // for rising and falling edge
                                        EPwm3Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
                                        EPwm3Regs.DBCTL.bit.POLSEL = 2;     // S3=1 inverted signal at ePWM1B
                                        EPwm3Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED
                                        //
                                            EPwm3Regs.TBCTL.bit.PHSEN = 1;       // enable phase shift for ePWM3
                                            EPwm3Regs.TBCTL.bit.PHSDIR = 0; // Count up after sync
                                            EPwm3Regs.TBPHS.bit.TBPHS = 100;  // 750 for  minimum voltage  phase shift 695=100
                                            EPwm3Regs.TBPHS.bit.TBPHSHR = 100;  // 750 for  minimum voltage  phase shift 695=100

                                            EPwm3Regs.TBCTL.bit.SYNCOSEL = 0;
                                       //     EPwm3Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high





                                EPwm4Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
                                        EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
                                        EPwm4Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
                                        EPwm4Regs.AQCTLA.all = 0x0006;      // set ePWM1A on CMPA up
                                        EPwm4Regs.AQCTLB.all = 0x0009;      // clear ePWM1B on CMPA up
                                        EPwm1Regs.DBCTL.bit.HALFCYCLE =1;

                                                                                // set   ePWM1B on CMPA down
                                        EPwm4Regs.TBPRD = period;           // 1KHz - PWM signal
                                        EPwm4Regs.CMPA.bit.CMPA  = period/2;
                                           //Dead time
                                     //     EPwm4Regs.DBRED = DT_R;              // 53 nanoseconds delay
                                     //      EPwm4Regs.DBFED = DT_F;              // for rising and falling edge
                                           EPwm4Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
                                           EPwm4Regs.DBCTL.bit.POLSEL = 2;     // S3=1 inverted signal at ePWM1B
                                           EPwm4Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED
                                           //
                                               EPwm4Regs.TBCTL.bit.PHSEN = 1;       // enable phase shift for ePWM3
                                               EPwm4Regs.TBCTL.bit.PHSDIR = 0; // Count up after sync
                                               EPwm4Regs.TBPHS.bit.TBPHS = 100;  // 750 for  minimum voltage  phase shift 695=100
                                               EPwm4Regs.TBPHS.bit.TBPHSHR = 100;  // 750 for  minimum voltage  phase shift 695=100
                                               EPwm4Regs.TBCTL.bit.SYNCOSEL = 0;
                                          //     EPwm4Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high


                                            CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within





                                            EDIS;
}

//
// initHRPWM1GPIO - Initialize HRPWM1 GPIOs
//
void initHRPWM1GPIO(void)
{
    EALLOW;

    //
    // Disable internal pull-up for the selected output pins
    // for reduced power consumption
    // Pull-ups can be enabled or disabled by the user.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;    // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;    // Disable pull-up on GPIO1 (EPWM1B)

    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;    // Disable pull-up on GPIO2 (EPWM2A)
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;    // Disable pull-up on GPIO3 (EPWM2B)

    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;    // Disable pull-up on GPIO4 (EPWM3A)
    GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;    // Disable pull-up on GPIO5 (EPWM3B)

    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1;    // Disable pull-up on GPIO4 (EPWM3A)
    GpioCtrlRegs.GPAPUD.bit.GPIO7 = 1;    // Disable pull-up on GPIO5 (EPWM3B)

    GpioCtrlRegs.GPAPUD.bit.GPIO8 = 1;    // Disable pull-up on GPIO4 (EPWM3A)
    GpioCtrlRegs.GPAPUD.bit.GPIO9 = 1;    // Disable pull-up on GPIO5 (EPWM3B)

    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;    // Disable pull-up on GPIO4 (EPWM3A)
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 1;    // Disable pull-up on GPIO5 (EPWM3B)


    //
    // Configure EPWM-1 pins using GPIO regs
    // This specifies which of the possible GPIO pins will be EPWM1 functional
    // pins.
    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // Configure GPIO1 as EPWM1B

    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   // Configure GPIO3 as EPWM2B

    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;   // Configure GPIO4 as EPWM3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;   // Configure GPIO5 as EPWM3B

    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;   // Configure GPIO4 as EPWM3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;   // Configure GPIO5 as EPWM3B

    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 1;   // Configure GPIO4 as EPWM3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 1;   // Configure GPIO5 as EPWM3B

    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;   // Configure GPIO4 as EPWM3A
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 1;   // Configure GPIO5 as EPWM3B

    EDIS;
}

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
