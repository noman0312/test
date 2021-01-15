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
extern void InitSysCtrl(void);
extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
extern void InitCpuTimers(void);
extern void ConfigCpuTimer(struct CPUTIMER_VARS *, float, float);
extern void Init_ADC_converter(void);
//
// Defines
//
#define RESULTS_BUFFER_SIZE     256

//
// Globals
//
uint16_t adcAResults[RESULTS_BUFFER_SIZE];   // Buffer for results
uint16_t index;                              // Index into result buffer
volatile uint16_t bufferFull;                // Flag to indicate buffer is full

//
// Function Prototypes
//
void initADC(void);
void initEPWM(void);
void initADCSOC(void);
__interrupt void adcA1ISR(void);
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


typedef enum
{
    //! sync pulse is generated by software
    EPWM_SYNC_OUT_PULSE_ON_SOFTWARE  = 0,
    //! sync pulse is passed from EPWMxSYNCIN
    EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN = 0,
    //! sync pulse is generated when time base counter equals zero
    EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO = 1,
    //! sync pulse is generated when time base counter equals compare B value.
    EPWM_SYNC_OUT_PULSE_ON_COUNTER_COMPARE_B = 2,
    //! sync pulse is disabled
    EPWM_SYNC_OUT_PULSE_DISABLED = 4,
    //! sync pulse is generated when time base counter equals compare D value.
    EPWM_SYNC_OUT_PULSE_ON_COUNTER_COMPARE_C = 5,
    //! sync pulse is disabled.
    EPWM_SYNC_OUT_PULSE_ON_COUNTER_COMPARE_D = 6
}EPWM_SyncOutPulseMode;


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
    PeriodFine = 0;
    status = SFO_INCOMPLETE;

    EALLOW;
    PieVectTable.ADCA1_INT = &adcA1ISR;     // Function for ADCA interrupt 1
 //   PieVectTable.TINT0 = &cpu_timer0_isr;
    EDIS;

    initADC();
   // InitCpuTimers();    // basic setup CPU Timer0, 1 and 2
    //ConfigCpuTimer(&CpuTimer0,150,100);

    //
    // Configure the ePWM
    //
    initEPWM();

    //
    // Setup the ADC for ePWM triggered conversions on channel 1
    //
    initADCSOC();


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

    for(index = 0; index < RESULTS_BUFFER_SIZE; index++)
     {
         adcAResults[index] = 0;
     }

     index = 0;
     bufferFull = 0;

     //
     // Enable PIE interrupt
     //
     PieCtrlRegs.PIEIER1.bit.INTx1 = 1;

     //
     // Sync ePWM
     //
     EALLOW;
     CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;

     //
     // Take conversions indefinitely in loop
     //

     /*      */


     CpuTimer0Regs.TCR.bit.TSS = 0;  // start timer0

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
                for(i=1; i<12; i++)
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


//EPWM1


        (*ePWM[1]).TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
        (*ePWM[1]).TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
        (*ePWM[1]).CMPA.bit.CMPA = period / 2;   // set duty 50% initially
        (*ePWM[1]).CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
        (*ePWM[1]).CMPB.bit.CMPB = period / 2;   // set duty 50% initially
        (*ePWM[1]).CMPB.all |= 1;
        (*ePWM[1]).TBPHS.all = 0;
        (*ePWM[1]).TBCTR = 0;
        (*ePWM[1]).DBCTL.bit.HALFCYCLE =1;

        (*ePWM[1]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                        // count mode
        (*ePWM[1]).TBCTL.bit.SYNCOSEL = 1;
        (*ePWM[1]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
        (*ePWM[1]).TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
        (*ePWM[1]).TBCTL.bit.FREE_SOFT = 11;

        (*ePWM[1]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
        (*ePWM[1]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
        (*ePWM[1]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
        (*ePWM[1]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;

        (*ePWM[1]).AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
        (*ePWM[1]).AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
        (*ePWM[1]).AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
        (*ePWM[1]).AQCTLB.bit.CBD = AQ_CLEAR;


        EALLOW;
        (*ePWM[1]).HRCNFG.all = 0x0;
        (*ePWM[1]).HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                        // both edges.
        (*ePWM[1]).HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                         // HR control.
        (*ePWM[1]).HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                         // and CTR = TBPRD
        (*ePWM[1]).HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                         // both edges
        (*ePWM[1]).HRCNFG.bit.CTLMODEB = HR_CMP;         // CMPBHR and TBPRDHR
                                                         // HR control
        (*ePWM[1]).HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                         // and CTR = TBPRD
        (*ePWM[1]).HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                                   // HR period

        (*ePWM[1]).HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                                   // (required for updwn
                                                   //  count HR control)
        (*ePWM[1]).HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                   // period control.

        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within
                                                   // the EPWM
        (*ePWM[1]).TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                                   // resolution phase to
                                                   // start HR period
        EDIS;



    // DAB PWM INITIALIZATION

    for(k=4; k<PWM_CH_DAB_SEC; k++)
       {
           (*ePWM[k]).TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
           (*ePWM[k]).TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
           (*ePWM[k]).CMPA.bit.CMPA = period / 2;   // set duty 50% initially
           (*ePWM[k]).CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
           (*ePWM[k]).CMPB.bit.CMPB = period / 2;   // set duty 50% initially
           (*ePWM[k]).CMPB.all |= 1;
           (*ePWM[k]).TBPHS.all = 0;
           (*ePWM[k]).TBCTR = 0;
           (*ePWM[k]).DBCTL.bit.HALFCYCLE =1;

           (*ePWM[k]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                           // count mode
           (*ePWM[k]).TBCTL.bit.SYNCOSEL = 1;
           (*ePWM[k]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
           (*ePWM[k]).TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
           (*ePWM[k]).TBCTL.bit.FREE_SOFT = 11;
           //(*ePWM[k]).TBCTL.bit.PHSEN = 1;

           (*ePWM[k]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
           (*ePWM[k]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
           (*ePWM[k]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
           (*ePWM[k]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;

           (*ePWM[k]).AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
           (*ePWM[k]).AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
           (*ePWM[k]).AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
           (*ePWM[k]).AQCTLB.bit.CBD = AQ_CLEAR;

           EALLOW;
           (*ePWM[k]).HRCNFG.all = 0x0;
           (*ePWM[k]).HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                            // both edges.
           (*ePWM[k]).HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                            // HR control.
           (*ePWM[k]).HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                            // and CTR = TBPRD
           (*ePWM[k]).HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                            // both edges
           (*ePWM[k]).HRCNFG.bit.CTLMODEB = HR_CMP;         // CMPBHR and TBPRDHR
                                                            // HR control
           (*ePWM[k]).HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                            // and CTR = TBPRD
           (*ePWM[k]).HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                                      // HR period

           (*ePWM[k]).HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                                      // (required for updwn
                                                      //  count HR control)
           (*ePWM[k]).HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                      // period control.

           CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Enable TBCLK within
                                                      // the EPWM
           (*ePWM[k]).TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                                      // resolution phase to
                                                      // start HR period
           EDIS;
       }


    for(k=8; k<PWM_CH_IBC; k++)
          {
              (*ePWM[k]).TBCTL.bit.PRDLD = TB_SHADOW;  // set Shadow load
         //     (*ePWM[k]).TBCTL2.bit.PRDLDSYNC = 0x2;  // set Shadow load
              (*ePWM[k]).TBPRD = period;               // PWM frequency = 1/(2*TBPRD)
              (*ePWM[k]).CMPA.bit.CMPA = period / 4;   // set duty 50% initially
              (*ePWM[k]).CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
              (*ePWM[k]).CMPB.bit.CMPB = period / 4;   // set duty 50% initially
              (*ePWM[k]).CMPB.all |= 1;
              (*ePWM[k]).TBPHS.all = 0;
              (*ePWM[k]).TBCTR = 0;

              (*ePWM[k]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Select up-down
                                                              // count mode
              (*ePWM[k]).TBCTL.bit.SYNCOSEL = 1;
              (*ePWM[k]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
              (*ePWM[k]).TBCTL.bit.CLKDIV = TB_DIV1;          // TBCLK = SYSCLKOUT
              (*ePWM[k]).TBCTL.bit.FREE_SOFT = 11;

              (*ePWM[k]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // LOAD CMPA on CTR = 0
              (*ePWM[k]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
              (*ePWM[k]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
              (*ePWM[k]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;

              (*ePWM[k]).AQCTLA.bit.CAU = AQ_CLEAR;// AQ_SET;             // PWM toggle high/low
              (*ePWM[k]).AQCTLA.bit.CAD = AQ_SET;//  AQ_CLEAR;
              (*ePWM[k]).AQCTLB.bit.CBU = AQ_SET;             // PWM toggle high/low
              (*ePWM[k]).AQCTLB.bit.CBD = AQ_CLEAR;

              EALLOW;
              (*ePWM[k]).HRCNFG.all = 0x0;
              (*ePWM[k]).HRCNFG.bit.EDGMODE = HR_BEP;          // MEP control on
                                                               // both edges.
              (*ePWM[k]).HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR
                                                               // HR control.
              (*ePWM[k]).HRCNFG.bit.HRLOAD = HR_CTR_ZERO_PRD;  // load on CTR = 0
                                                               // and CTR = TBPRD
              (*ePWM[k]).HRCNFG.bit.EDGMODEB = HR_BEP;         // MEP control on
                                                               // both edges
              (*ePWM[k]).HRCNFG.bit.CTLMODEB = HR_CMP;         // CMPBHR and TBPRDHR
                                                               // HR control
              (*ePWM[k]).HRCNFG.bit.HRLOADB = HR_CTR_ZERO_PRD; // load on CTR = 0
                                                               // and CTR = TBPRD
              (*ePWM[k]).HRCNFG.bit.AUTOCONV = 1;        // Enable autoconversion for
                                                         // HR period

              (*ePWM[k]).HRPCTL.bit.TBPHSHRLOADE = 1;    // Enable TBPHSHR sync
                                                         // (required for updwn
                                                         //  count HR control)
              (*ePWM[k]).HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                         // period control.

              CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Enable TBCLK within
                                                         // the EPWM
              (*ePWM[k]).TBCTL.bit.SWFSYNC = 1;          // Synchronize high
                                                         // resolution phase to
                                                         // start HR period
              EDIS;
          }
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

void initADC(void)
{
    //
    // Setup VREF as internal
    //
    SetVREF(ADC_ADCA, ADC_INTERNAL, ADC_VREF3P3);

    EALLOW;

    //
    // Set ADCCLK divider to /4
    //
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;

    //
    // Set pulse positions to late
    //
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //
    // Power up the ADC and then delay for 1 ms
    //
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    EDIS;

    DELAY_US(1000);
}

//
// initEPWM - Function to configure ePWM1 to generate the SOC.
//
void initEPWM(void)
{
    EALLOW;

    EPwm1Regs.ETSEL.bit.SOCAEN = 0;     // Disable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = 4;    // Select SOC on up-count
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

    EPwm1Regs.CMPA.bit.CMPA = 0x0800;   // Set compare A value to 2048 counts
    EPwm1Regs.TBPRD = 0x1000;           // Set period to 4096 counts

    EPwm1Regs.TBCTL.bit.CTRMODE = 3;    // Freeze counter

    EDIS;
}

//
// initADCSOC - Function to configure ADCA's SOC0 to be triggered by ePWM1.
//
void initADCSOC(void)
{
    //
    // Select the channels to convert and the end of conversion flag
    //
    EALLOW;

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 1;     // SOC0 will convert pin A1
                                           // 0:A0  1:A1  2:A2  3:A3
                                           // 4:A4   5:A5   6:A6   7:A7
                                           // 8:A8   9:A9   A:A10  B:A11
                                           // C:A12  D:A13  E:A14  F:A15
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; // End of SOC0 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure INT1 flag is cleared

    EDIS;
}

//
// adcA1ISR - ADC A Interrupt 1 ISR
//
__interrupt void adcA1ISR(void)
{
    //
    // Add the latest result to the buffer
    // ADCRESULT0 is the result register of SOC0
    adcAResults[index++] = AdcaResultRegs.ADCRESULT0;

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
