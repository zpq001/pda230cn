/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: Low Pass
Filter model: Butterworth
Filter order: 2
Sampling Frequency: 1000 Hz
Cut Frequency: 10.000000 Hz
Coefficents Quantization: 8-bit

Z domain Zeros
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000

Z domain Poles
z = 0.955599 + j -0.042512
z = 0.955599 + j 0.042512
***************************************************************/
#define Ntap 8

#define DCgain 512

__int8 fir(__int8 NewSample) {
    __int8 FIRCoef[Ntap] = { 
           60,
           64,
           67,
           69,
           67,
           64,
           60,
           57
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
