/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: Low Pass
Filter model: Rectangular Window
Sampling Frequency: 20 Hz
Cut Frequency: 1.000000 Hz
Coefficents Quantization: 8-bit
***************************************************************/
#define Ntap 20

#define DCgain 1024

__int8 fir(__int8 NewSample) {
    __int8 FIRCoef[Ntap] = { 
           11,
           21,
           33,
           44,
           55,
           65,
           73,
           79,
           83,
           84,
           83,
           79,
           73,
           65,
           55,
           44,
           33,
           21,
           11,
            2
    };

    static __int8 x[Ntap]; //input samples
    __int16 y=0;            //output sample
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
