//
// configHRPWM - Configures all ePWM channels and sets up HRPWM
//                on ePWMxA channels &  ePWMxB channels
//



void configHRPWM(uint16_t period)
{

        CpuSysRegs.PCLKCR2.bit.EPWM1=1;
        CpuSysRegs.PCLKCR2.bit.EPWM2=1;
        CpuSysRegs.PCLKCR2.bit.EPWM3=1;

        EALLOW;
        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;   // Disable TBCLK within the EPWM


        EPwm1Regs.ETSEL.bit.SOCAEN = 1;     // Disable SOC on A group
        EPwm1Regs.ETSEL.bit.SOCASEL = 4;    // Select SOC on up-count
        EPwm1Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event


        EPwm1Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
        EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
        EPwm1Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
        EPwm1Regs.AQCTLA.all = 0x0009;      // set ePWM1A on CMPA up//0x0006//0x0009
        EPwm1Regs.AQCTLB.all = 0x0006;      // clear ePWM1B on CMPA up
        EPwm1Regs.DBCTL.bit.HALFCYCLE =1;

        // set   ePWM1B on CMPA down

        EPwm1Regs.TBPRD = period;           // 1KHz - PWM signal
        EPwm1Regs.CMPA.bit.CMPA  = period/2;
        EPwm1Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
        EPwm1Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
        //dead time
        //    EPwm1Regs.DBRED = DT_R;              // 53 nanoseconds delay
        //   EPwm1Regs.DBFED = DT_F;              // for rising and falling edge
        EPwm1Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
        EPwm1Regs.DBCTL.bit.POLSEL = 2;     // S3=1 inverted signal at ePWM1B
        EPwm1Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED

        //phase shift enable
        EPwm1Regs.TBCTL.bit.SYNCOSEL = 3;
        EPwm1Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high

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
   //     EPwm1Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                   // period control.

//        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within
                                                   // the EPWM
     //   EPwm1Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high

                                                   // resolution phase to
                                                   // start HR period






        EPwm2Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
        EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
        EPwm2Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
        EPwm2Regs.AQCTLA.all = 0x0006;      // set ePWM1A on CMPA up
        EPwm2Regs.AQCTLB.all = 0x0009;      // clear ePWM1B on CMPA up
        EPwm2Regs.TBCTL.bit.PRDLD= 0;          // Shadow select


                          // set   ePWM1B on CMPA down
        EPwm2Regs.TBPRD = period;           // 1KHz - PWM signal
        EPwm2Regs.CMPA.bit.CMPA  = period/2;
        EPwm2Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
        EPwm2Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
        //Dead time
        //     EPwm2Regs.DBRED = DT_R;              // 53 nanoseconds delay
        //      EPwm2Regs.DBFED = DT_F;              // for rising and falling edge
            EPwm2Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
            EPwm2Regs.DBCTL.bit.POLSEL = 2;     // S3=1 inverted signal at ePWM1B
           EPwm2Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED
        //
        EPwm2Regs.TBCTL.bit.PHSEN = 1;       // enable phase shift for ePWM3
        EPwm1Regs.TBCTL.bit.PHSDIR= 1;          // Phase Direction Bit
//        EPwm2Regs.TBCTL.bit.PHSDIR = 0; // Count up after sync
        EPwm2Regs.TBPHS.bit.TBPHS = 0;  // 750 for  minimum voltage  phase shift 695=100
        EPwm2Regs.TBPHS.bit.TBPHSHR = 0x00FF;  // 750 for  minimum voltage  phase shift 695=100
        EPwm2Regs.TRREM.bit.TRREM = 0XFFF;  // 750 for  minimum voltage  phase shift 695=100
        EPwm2Regs.TBCTL.bit.SYNCOSEL = 0;



          EPwm2Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high...


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
//          EPwm2Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                     // period control.










        EPwm3Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
        EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
        EPwm3Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
        EPwm3Regs.AQCTLA.all = 0x0006;      // set ePWM1A on CMPA up
        EPwm3Regs.AQCTLB.all = 0x0009;      // clear ePWM1B on CMPA up
        EPwm3Regs.DBCTL.bit.HALFCYCLE =1;

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
        EPwm3Regs.TBPHS.bit.TBPHS = 0;  // 750 for  minimum voltage  phase shift 695=100
        EPwm3Regs.TBPHS.bit.TBPHSHR = 0xFFFF;  // 750 for  minimum voltage  phase shift 695=100

        EPwm3Regs.TBCTL.bit.SYNCOSEL = 0;
        EPwm3Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high





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
           EPwm4Regs.TBPHS.bit.TBPHS = 0;  // 750 for  minimum voltage  phase shift 695=100
           EPwm4Regs.TBPHS.bit.TBPHSHR = 0;  // 750 for  minimum voltage  phase shift 695=100
           EPwm4Regs.TBCTL.bit.SYNCOSEL = 0;
        EPwm4Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high

//EPWM-5


        EPwm5Regs.TBCTL.bit.CLKDIV =  0;    // CLKDIV = 1
        EPwm5Regs.TBCTL.bit.HSPCLKDIV = 0;  // HSPCLKDIV = 1
        EPwm5Regs.TBCTL.bit.CTRMODE = 2;    // up - down mode
        EPwm5Regs.AQCTLA.all = 0x0006;      // set ePWM1A on CMPA up
        EPwm5Regs.AQCTLB.all = 0x0009;      // clear ePWM1B on CMPA up
        EPwm5Regs.TBCTL.bit.PRDLD= 0;          // Shadow select


                          // set   ePWM1B on CMPA down
        EPwm5Regs.TBPRD = period;           // 1KHz - PWM signal
        EPwm5Regs.CMPA.bit.CMPA  = period/2;
        EPwm5Regs.CMPA.bit.CMPAHR = (1 << 8);   // initialize HRPWM extension
        EPwm5Regs.CMPB.bit.CMPB = period / 2;   // set duty 50% initially
        //Dead time
        //     EPwm5Regs.DBRED = DT_R;              // 53 nanoseconds delay
        //      EPwm5Regs.DBFED = DT_F;              // for rising and falling edge
            EPwm5Regs.DBCTL.bit.OUT_MODE = 3;   // ePWM1A = RED
            EPwm5Regs.DBCTL.bit.POLSEL = 2;     // S3=1 inverted signal at ePWM1B
           EPwm5Regs.DBCTL.bit.IN_MODE = 0;    // ePWM1A = source for RED & FED
        //
        EPwm5Regs.TBCTL.bit.PHSEN = 1;       // enable phase shift for ePWM3
        EPwm1Regs.TBCTL.bit.PHSDIR= 1;          // Phase Direction Bit
//        EPwm5Regs.TBCTL.bit.PHSDIR = 0; // Count up after sync
        EPwm5Regs.TBPHS.bit.TBPHS = 0;  // 750 for  minimum voltage  phase shift 695=100
        EPwm5Regs.TBPHS.bit.TBPHSHR = 0x00FF;  // 750 for  minimum voltage  phase shift 695=100
        EPwm5Regs.TRREM.bit.TRREM = 0XFFF;  // 750 for  minimum voltage  phase shift 695=100
        EPwm5Regs.TBCTL.bit.SYNCOSEL = 0;



          EPwm5Regs.TBCTL.bit.SWFSYNC = 1;          // Synchronize high...


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
//          EPwm5Regs.HRPCTL.bit.HRPE = 1;            // Turn on high-resolution
                                                     // period control.








        CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // Enable TBCLK within





}
