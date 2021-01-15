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
#define PWM_CH_IBC            12        // # of PWM channels - 4 for IBC
#define PWM_CH_DAB_PRI        12        // # of PWM channels - 4
#define PWM_CH_DAB_SEC        12        // # of PWM channels - 4
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
//volatile struct EPWM_REGS *ePWM[PWM_CH_DAB_PRI] = {&EPwm3Regs, &EPwm3Regs , &EPwm4Regs, &EPwm4Regs};



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
    PeriodFine = 1;
    status = SFO_INCOMPLETE;

    //
    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;


    
    //
    // ePWM and HRPWM register initialization
    //
    for(i=1; i<12; i++)
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
        //
        // Sweep PeriodFine as a Q16 number from 0.2 - 0.999
        //
        for(PeriodFine = 0x3333; PeriodFine < 0xFFBF; PeriodFine++)
        {
            if(UpdateFine)
            {
                //
                // Because auto-conversion is enabled, the desired
                // fractional period must be written directly to the
                // TBPRDHR register in Q16 format
                // (lower 8-bits are ignored)
                //
                // EPwm1Regs.TBPRDHR = PeriodFine;
                //
                // The hardware will automatically scale
                // the fractional period by the MEP_ScaleFactor
                // in the HRMSTEP register (which is updated
                // by the SFO calibration software).
                //
                // Hardware conversion:
                // MEP delay movement = ((TBPRDHR(15:0) >> 8) *  HRMSTEP(7:0) +
                //                       0x80) >> 8
                //
                for(i=2; i<12; i++)
                {
                    (*ePWM[i]).TBPRDHR = PeriodFine; //In Q16 format
                }
            }
            else
            {
                //
                // No high-resolution movement on TBPRDHR.
                //
                for(i=1; i<12; i++)
                {
                    (*ePWM[i]).TBPRDHR = 0;
                }
            }

            //
            // Call the scale factor optimizer lib function SFO(0)
            // periodically to track for any change due to temp/voltage.
            // This function generates MEP_ScaleFactor by running the
            // MEP calibration module in the HRPWM logic. This scale
            // factor can be used for all HRPWM channels. HRMSTEP
            // register is automatically updated by the SFO function.
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

//
// configHRPWM - Configures all ePWM channels and sets up HRPWM
//                on ePWMxA channels &  ePWMxB channels
//
void configHRPWM(uint16_t period)
{
    uint16_t j,k;

    //
    // ePWM channel register configuration with HRPWM
    // ePWMxA toggle low/high with MEP control on Rising edge
    //
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   // Disable TBCLK within the EPWM
    EDIS;


    // IBC PWM INITIALIZATION /////



    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
    EPwm1Regs.TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
    EPwm1Regs.CMPA.bit.CMPA = period / 2;   // set duty 50% initially
    EPwm1Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
    EPwm1Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
    EPwm1Regs.CMPB.all |= 1;
  //  (*ePWM[1]).TBPHS.all = 0;
    EPwm1Regs.TBCTR = 0;
    EPwm1Regs.DBCTL.bit.HALFCYCLE =1;

    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                    // count mode
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 11;

    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
    EPwm1Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED



    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
    EPwm1Regs.AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
    EPwm1Regs.AQCTLB.bit.CBD = AQ_CLEAR;



    EPwm1Regs.HRCNFG.all = 0x0;
    EPwm1Regs.HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                    // both edges.
    EPwm1Regs.HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                     // HR control.
    EPwm1Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                     // and CTR = TBPRD
    EPwm1Regs.HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                     // both edges
    EPwm1Regs.HRCNFG.bit.CTLMODEB = 1;         // CMPBHR and TBPRDHR
                                                     // HR control
    EPwm1Regs.HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                     // and CTR = TBPRD
    EPwm1Regs.HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                               // HR period

    EPwm1Regs.HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                               // (required for updwn
                                               //  count HR control)
    EPwm1Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                               // period control.

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within
                                               // the EPWM
 //   EPwm1Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high

                                               // resolution phase to
                                               // start HR period



//PWM2

    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
    EPwm2Regs.TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
    EPwm2Regs.CMPA.bit.CMPA = period / 2;   // set duty 50% initially
    EPwm2Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
    EPwm2Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
    EPwm2Regs.CMPB.all |= 1;
//    (*ePWM[2]).TBPHS.all = 0;
    EPwm2Regs.TBCTR = 0;
    EPwm2Regs.DBCTL.bit.HALFCYCLE =1;
    EPwm2Regs.TBCTL.bit.PHSEN = 1;        // enable phase shift for ePWM2

    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                    // count mode
    EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
    EPwm2Regs.TBCTL.bit.FREE_SOFT = 11;


    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
    EPwm2Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED


    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
    EPwm2Regs.AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
    EPwm2Regs.AQCTLB.bit.CBD = AQ_CLEAR;

    EPwm2Regs.HRCNFG.all = 0x0;
    EPwm2Regs.HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                    // both edges.
    EPwm2Regs.HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                     // HR control.
    EPwm2Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                     // and CTR = TBPRD
    EPwm2Regs.HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                     // both edges
    EPwm2Regs.HRCNFG.bit.CTLMODEB = 1;         // CMPBHR and TBPRDHR
                                                    // HR control
    EPwm2Regs.HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                     // and CTR = TBPRD
    EPwm2Regs.HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                               // HR period

    EPwm2Regs.HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                               // (required for updwn
                                               //  count HR control)
    EPwm2Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                               // period control.
    EPwm2Regs.TBPHS.bit.TBPHS = 0; // 750 for  minimum voltage  phase shift 695=100
    EPwm2Regs.TBPHS.bit.TBPHSHR = 1; // 750 for  minimum voltage  phase shift 695=100

    EPwm2Regs.TRREM.bit.TRREM = 0; // 750 for  minimum voltage  phase shift 695=100


    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within

                                               // the EPWM
    EPwm2Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                               // resolution phase to
                                               // start HR period







    // DAB SECONDry PWM INITIALIZATION


    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
    EPwm3Regs.TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
    EPwm3Regs.CMPA.bit.CMPA = period / 2;   // set duty 50% initially
    EPwm3Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
    EPwm3Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
    EPwm3Regs.CMPB.all |= 1;
     //      (*ePWM[3]).TBPHS.all = 0;
    EPwm3Regs.TBCTR = 0;

    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                           // count mode
    EPwm3Regs.TBCTL.bit.SYNCOSEL = 0;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
    EPwm3Regs.TBCTL.bit.FREE_SOFT = 11;
    EPwm3Regs.TBCTL.bit.PHSEN = 1;        // enable phase shift for ePWM2
           //(*ePWM[k]).TBCTL.bit.PHSEN = 1;

    EPwm3Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
    EPwm3Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED


    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
    EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
    EPwm3Regs.AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
    EPwm3Regs.AQCTLB.bit.CBD = AQ_CLEAR;

    EPwm3Regs.HRCNFG.all = 0x0;
    EPwm3Regs.HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                            // both edges.
    EPwm3Regs.HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                            // HR control.
    EPwm3Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                            // and CTR = TBPRD
    EPwm3Regs.HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                            // both edges
    EPwm3Regs.HRCNFG.bit.CTLMODEB = HR_CMP;         // CMPBHR and TBPRDHR
                                                            // HR control
    EPwm3Regs.HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                            // and CTR = TBPRD
    EPwm3Regs.HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                                      // HR period

    EPwm3Regs.HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                                      // (required for updwn
                                                      //  count HR control)
    EPwm3Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                      // period control.
    EPwm3Regs.TBPHS.bit.TBPHS = 0; // 750 for  minimum voltage  phase shift 695=100

    EPwm3Regs.TRREM.bit.TRREM = 0; // 750 for  minimum voltage  phase shift 695=100

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within
                                                      // the EPWM
    EPwm3Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                                      // resolution phase to
                                                      // start HR period




     EPwm4Regs.TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
         //     (*ePWM[k]).TBCTL2.bit.PRDLDSYNC = 0x2;  // set Shadow load
     EPwm4Regs.TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
     EPwm4Regs.CMPA.bit.CMPA = period / 2;   // set duty 50% initially
     EPwm4Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
     EPwm4Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
     EPwm4Regs.CMPB.all |= 1;
   //           (*ePWM[4]).TBPHS.all = 0;
     EPwm4Regs.TBCTR = 0;

     EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                              // count mode
     EPwm4Regs.TBCTL.bit.SYNCOSEL = 0;
     EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
     EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
     EPwm4Regs.TBCTL.bit.FREE_SOFT = 11;
     EPwm4Regs.TBCTL.bit.PHSEN = 1;        // enable phase shift for ePWM2

     EPwm4Regs.DBCTL.bit.OUT_MODE = 2;   // ePWM1A = RED
     EPwm4Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED


     EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
     EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
     EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
     EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

     EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
     EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
     EPwm4Regs.AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
     EPwm4Regs.AQCTLB.bit.CBD = AQ_CLEAR;

      EPwm4Regs.HRCNFG.all = 0x0;
      EPwm4Regs.HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                               // both edges.
      EPwm4Regs.HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                               // HR control.
      EPwm4Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                               // and CTR = TBPRD
      EPwm4Regs.HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                               // both edges
      EPwm4Regs.HRCNFG.bit.CTLMODEB = 1;         // CMPBHR and TBPRDHR
                                                               // HR control
      EPwm4Regs.HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                               // and CTR = TBPRD
      EPwm4Regs.HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                                         // HR period

      EPwm4Regs.HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                                         // (required for updwn
                                                         //  count HR control)
      EPwm4Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                         // period control.
      EPwm4Regs.TBPHS.bit.TBPHS = 0; // 750 for  minimum voltage  phase shift 695=100
      EPwm4Regs.TRREM.bit.TRREM = 0; // 750 for  minimum voltage  phase shift 695=100

      CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within
                                                         // the EPWM
      EPwm4Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                                         // resolution phase to
                                                         // start HR period




      EPwm5Regs.TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
          //     (*ePWM[k]).TBCTL2.bit.PRDLDSYNC = 0x2;  // set Shadow load
      EPwm5Regs.TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
      EPwm5Regs.CMPA.bit.CMPA = period / 2;   // set duty 50% initially
      EPwm5Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
      EPwm5Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
      EPwm5Regs.CMPB.all |= 1;
    //           (*ePWM[4]).TBPHS.all = 0;
      EPwm5Regs.TBCTR = 0;

      EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                               // count mode
      EPwm5Regs.TBCTL.bit.SYNCOSEL = 0;
      EPwm5Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
      EPwm5Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
      EPwm5Regs.TBCTL.bit.FREE_SOFT = 11;
      EPwm5Regs.TBCTL.bit.PHSEN = 1;        // enable phase shift for ePWM2

      EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
      EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
      EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
      EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

      EPwm5Regs.AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
      EPwm5Regs.AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
      EPwm5Regs.AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
      EPwm5Regs.AQCTLB.bit.CBD = AQ_CLEAR;

       EPwm5Regs.HRCNFG.all = 0x0;
       EPwm5Regs.HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                                // both edges.
       EPwm5Regs.HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                                // HR control.
       EPwm5Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                                // and CTR = TBPRD
       EPwm5Regs.HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                                // both edges
       EPwm5Regs.HRCNFG.bit.CTLMODEB = 1;         // CMPBHR and TBPRDHR
                                                                // HR control
       EPwm5Regs.HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                                // and CTR = TBPRD
       EPwm5Regs.HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                                          // HR period

       EPwm5Regs.HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                                          // (required for updwn
                                                          //  count HR control)
       EPwm5Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                          // period control.
       EPwm5Regs.TBPHS.bit.TBPHS = 0; // 750 for  minimum voltage  phase shift 695=100
       EPwm5Regs.TRREM.bit.TRREM = 0; // 750 for  minimum voltage  phase shift 695=100

       CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within
                                                          // the EPWM
       EPwm5Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                                          // resolution phase to
                                                          // start HR period



       EPwm6Regs.TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
              //     (*ePWM[k]).TBCTL2.bit.PRDLDSYNC = 0x2;  // set Shadow load
          EPwm6Regs.TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
          EPwm6Regs.CMPA.bit.CMPA = period / 2;   // set duty 50% initially
          EPwm6Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
          EPwm6Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
          EPwm6Regs.CMPB.all |= 1;
        //           (*ePWM[4]).TBPHS.all = 0;
          EPwm6Regs.TBCTR = 0;

          EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                                   // count mode
          EPwm6Regs.TBCTL.bit.SYNCOSEL = 0;
          EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
          EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
          EPwm6Regs.TBCTL.bit.FREE_SOFT = 11;
          EPwm6Regs.TBCTL.bit.PHSEN = 1;        // enable phase shift for ePWM2

          EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
          EPwm6Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
          EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
          EPwm6Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

          EPwm6Regs.AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
          EPwm6Regs.AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
          EPwm6Regs.AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
          EPwm6Regs.AQCTLB.bit.CBD = AQ_CLEAR;

           EALLOW;
           EPwm6Regs.HRCNFG.all = 0x0;
           EPwm6Regs.HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                                    // both edges.
           EPwm6Regs.HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                                    // HR control.
           EPwm6Regs.HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                                    // and CTR = TBPRD
           EPwm6Regs.HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                                    // both edges
           EPwm6Regs.HRCNFG.bit.CTLMODEB = 1;         // CMPBHR and TBPRDHR
                                                                    // HR control
           EPwm6Regs.HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                                    // and CTR = TBPRD
           EPwm6Regs.HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                                              // HR period

           EPwm6Regs.HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                                              // (required for updwn
                                                              //  count HR control)
           EPwm6Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                              // period control.
           EPwm6Regs.TBPHS.bit.TBPHS = 0; // 750 for  minimum voltage  phase shift 695=100
           EPwm6Regs.TRREM.bit.TRREM = 0; // 750 for  minimum voltage  phase shift 695=100

           CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within
                                                              // the EPWM
           EPwm6Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                                              // resolution phase to
                                                              // start HR period





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
