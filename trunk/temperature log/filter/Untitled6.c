/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: Low Pass
Filter model: Chebyshev
Filter order: 3
Sampling Frequency: 20 KHz
Cut Frequency: 0.010000 KHz
Pass band Ripple: 1.000000 dB
Coefficents Quantization: float

Z domain Zeros
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000

Z domain Poles
z = 0.998446 + j -0.000009
z = 0.999222 + j -0.003028
z = 0.999222 + j 0.003028
***************************************************************/
#define NCoef 3
float iir(float NewSample) {
    float ACoef[NCoef+1] = {
        0.00000000931329148875,
        0.00000002793987446626,
        0.00000002793987446626,
        0.00000000931329148875
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -2.99688763931455470000,
        2.99378750508951930000,
        -0.99689985056499619000
    };

    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}
