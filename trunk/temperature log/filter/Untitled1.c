/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: Low Pass
Filter model: Rectangular Window
Sampling Frequency: 20 Hz
Cut Frequency: 1.000000 Hz
Coefficents Quantization: 16-bit
***************************************************************/
#define Ntap 20

#define DCgain 262144

__int16 fir(__int16 NewSample) {
    __int16 FIRCoef[Ntap] = { 
         2923,
         5619,
         8487,
        11387,
        14170,
        16691,
        18812,
        20418,
        21419,
        21759,
        21419,
        20418,
        18812,
        16691,
        14170,
        11387,
         8487,
         5619,
         2923,
          521
    };

    static __int16 x[Ntap]; //input samples
    __int32 y=0;            //output sample
    int n;

    //shift the old samples
    for(n=Ntap-1; n>0; n--)
       x[n] = x[n-1];

    //Calculate the new output
    x[0] = NewSample;
    for(n=0; n<Ntap; n++)
        y += FIRCoef[n] * x[n];
    
    return y / DCgain;
}
