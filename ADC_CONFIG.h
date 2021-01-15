//
// initADC - Function to configure and power up ADCA.
//
void initADC(void)
{
    //
    // Setup VREF as internal
    //
    SetVREF(ADC_ADCA, ADC_INTERNAL, ADC_VREF3P3);
    SetVREF(ADC_ADCB, ADC_INTERNAL, ADC_VREF3P3);


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

    EPwm1Regs.ETSEL.bit.SOCAEN = 0;     // Disable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = 4;    // Select SOC on up-count
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;     // Generate pulse on 1st event

   // EPwm1Regs.CMPA.bit.CMPA = 0x0800;   // Set compare A value to 2048 counts
    //EPwm1Regs.TBPRD = 0x1000;           // Set period to 4096 counts

    //EPwm1Regs.TBCTL.bit.CTRMODE = 3;    // Freeze counter

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

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 6;     // SOC0 will convert pin A1
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA




//    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 5;     // SOC0 will convert pin A1
//   AdcaRegs.ADCSOC0CTL.bit.ACQPS = 9;     // Sample window is 10 SYSCLK cycles
//    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;   // Trigger on ePWM1 SOCA






    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; // End of SOC0 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure INT1 flag is cleared

    EDIS;
}

//
// adcA1ISR - ADC A Interrupt 1 ISR
//
