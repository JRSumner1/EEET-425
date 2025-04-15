// OPEN A NEW SKETCH WINDOW IN ARDUINO
// CLICK IN THIS BOX, CTL-A, CTL-C (Copy code)
// CLICK IN SKETCH, CTL-A, CTL-V (Paste code into sketch)

// file: Lab7_FIR_Section_02_Base_Code.ino

// created:  Clark Hochgraf/David Orlicki October 17, 2017
// modified: David Orlicki October 26, 2018
// modified: David Orlicki October 25, 2019
// modified: Mark Thompson 3/21/2021 -- Added some comments, Added MATLAB handshake

//-----------------------------------------------------------------
// Create the sync based FIR filters indicated below.
//   Specify fractional corner frequency Fc.
//   Specify max kernel index M. (kernel lenght M+1, 0..M).
//   Use the companion sketch to create specified FIR kernels.
// Show the impulse response for each case.
// Show the FFT based frequency response for each case.
// Show the 10% - 90% transition BW(s) for each case.
// Use testVector() function to measure amplitude gain(s)
//   at frequencies +/- 5 BPM from each cut frequency.

// Filter specifications
// BPF: low 15 bpm, high = 50 bpm, transition BW = 10 bpm
// BSF: low 15 bpm, high = 50 bpm, transition BW = 10 bpm

const int TSAMP_MSEC = 5, DATA_LEN = 1200, DATA_FXPT = 100;
int tick, xv, yv;
float sample;
unsigned long execUsec;

//*******************************************************************
void setup()
{
  Serial.begin(115200); delay(20);
 
  // Handshake with MATLAB      
  Serial.println(F("%Arduino Ready"));
  while (Serial.read() != 'g'); // spin
}

//*******************************************************************
void loop()
{
//  sample = (tick == 0) ? 1000.0 : 0;
  sample = testVector();
  xv = int(DATA_FXPT*sample+0.5); // rounded, fixed point
  int yvLPF, yvHPF;
  execUsec = micros();
  
  // Combine the LPF and HPF appropriately to create a BPF 
  // and then a BSF.  See diagrams for assistance
  
  yvLPF = FIR_A(xv); // LPF
  yvHPF = FIR_B(xv); // HPF
  yv = FIR_B(yvLPF); // BPF
//  yv = yvLPF + yvHPF; // BSF
  execUsec = micros()-execUsec;
  
  displayData(tick,xv,yv);

  if (++tick >= DATA_LEN) // post processing
  {
    Serial.print(F("\ncalculation msec/sample = "));
    Serial.println(0.001*execUsec,3);
    while (true); // freeze process (spin forever)
  }
} // loop()

//*******************************************************************
int FIR_STUB(int x) {return(x);} // timing measurement stub

//*******************************************************************
int FIR_A(int x)
//
// FIR_A is a generic function that implements convolution of the input value x
// with the impulse response that is declared below.  The student will
// compute the impulse response in MATLAB using the FIR_Designer tool
// and use the C header that is created to replace the code below
//
{   
  // Filter type: LPF_BPF
  const int Fc_bpm = 50;
  // LPF FIR Filter Coefficients MFILT = 61, Fc = 50
  const int HFXPT = 4096, MFILT = 61;
  int h[] = {0, 2, 4, 5, 5, 4, 0, -6, -12, -17, -17, -12, 0, 17,
  35, 47, 47, 32, 0, -43, -87, -117, -119, -82, 0, 122, 270, 424,
  558, 649, 681, 649, 558, 424, 270, 122, 0, -82, -119, -117, -87, -43,
  0, 32, 47, 47, 35, 17, 0, -12, -17, -17, -12, -6, 0, 4,
  5, 5, 4, 2, 0};

//  // Filter type: LPF_BSF
//  const int Fc_bpm = 15;
//  // LPF FIR Filter Coefficients MFILT = 61, Fc = 15
//  const int HFXPT = 2048, MFILT = 61;
//  int h[] = {-2, -2, -2, -2, -2, -3, -3, -3, -2, -1, 0, 2, 5, 8,
//  12, 17, 23, 29, 36, 43, 51, 59, 67, 75, 82, 88, 94, 98,
//  102, 104, 104, 104, 102, 98, 94, 88, 82, 75, 67, 59, 51, 43,
//  36, 29, 23, 17, 12, 8, 5, 2, 0, -1, -2, -3, -3, -3,
//  -2, -2, -2, -2, -2};

  const float INV_HFXPT = 1.0/HFXPT;
  static int xv[MFILT] = {0};
  long hv, accum = 0;
  
  // Right shift old xv values. Install new x in xv[0];
  for (int i = (MFILT-1); i > 0; i--) xv[i] = xv[i-1]; xv[0] = x;

  // h[]*x[] overlap multiply-accumumlate
  for (int i = 0; i < MFILT; i++)
  {
    hv = h[i]; // create 32 bit space
    accum += hv*xv[MFILT-1-i];
  }
  return (accum*INV_HFXPT);
}

//*******************************************************************
int FIR_B(int x)
//
// FIR_b is another generic function that implements convolution of the input value x
// with the impulse response that is declared below.  The student will
// compute the impulse response in MATLAB using the FIR_Designer tool
// and use the C header that is created to replace the code below
//
{
  // Filter type: HPF_BPF
  const int Fc_bpm = 15;
  // HPF FIR Filter Coefficients MFILT = 61, Fc = 15
  const int HFXPT = 2048, MFILT = 61;
  int h[] = {2, 2, 2, 2, 2, 3, 3, 3, 2, 1, 0, -2, -5, -8,
  -12, -17, -23, -29, -36, -43, -51, -59, -67, -75, -82, -88, -94, -98,
  -102, -104, 1944, -104, -102, -98, -94, -88, -82, -75, -67, -59, -51, -43,
  -36, -29, -23, -17, -12, -8, -5, -2, 0, 1, 2, 3, 3, 3,
  2, 2, 2, 2, 2};
  
//  // Filter type: HPF_BSF
//  const int Fc_bpm = 50;
//  // HPF FIR Filter Coefficients MFILT = 61, Fc = 50
//  const int HFXPT = 4096, MFILT = 61;
//  int h[] = {0, -2, -4, -5, -5, -4, 0, 6, 12, 17, 17, 12, 0, -17,
//  -35, -47, -47, -32, 0, 43, 87, 117, 119, 82, 0, -122, -270, -424,
//  -558, -649, 3415, -649, -558, -424, -270, -122, 0, 82, 119, 117, 87, 43,
//  0, -32, -47, -47, -35, -17, 0, 12, 17, 17, 12, 6, 0, -4,
//  -5, -5, -4, -2, 0};

  const float INV_HFXPT = 1.0/HFXPT;
  static int xv[MFILT] = {0};
  long hv, accum = 0;
  
  // Right shift old xv values. Install new x in xv[0];
  for (int i = (MFILT-1); i > 0; i--) xv[i] = xv[i-1]; xv[0] = x;

  // h[]*x[] overlap multiply-accumumlate
  for (int i = 0; i < MFILT; i++)
  {
    hv = h[i]; // create 32 bit space
    accum += hv*xv[MFILT-1-i];
  }
  return (accum*INV_HFXPT);
}

//*******************************************************************
void displayData(int n, int x, int y)
{
  if (n == 0) Serial.print(F("\nn\tx\ty\n"));
  Serial.print(n); Serial.print('\t');
  Serial.print(x); Serial.print('\t');
  Serial.print(y); Serial.print('\n');
}

//*******************************************************************
float testVector(void)
{
  // Variable rate periodic input
  // Specify segment amplitude, bpm rate, interval seconds.
  // Intervals trimmed for nearest cycle ending zero crossing.

  const float AMP1 = 2.0, BPM1 = 10.0, TSEC1 = 24.0; 
  const float AMP2 = 2.0, BPM2 = 30.0, TSEC2 = 24.0; 
  const float AMP3 = 2.0, BPM3 = 50.0, TSEC3 = 24.0; 
    
  static int simTick, xt1, xt2, breathsPerInterval;
  static float cycleAmp, fracFreq, fracFreq1, fracFreq2, fracFreq3;
  float secPerBreath;
  
  if (tick == 0) // map full test amplitudes, frequencies and durations
  {
    fracFreq1 = BPM1/600; 
    secPerBreath = 60.0/BPM1;
    breathsPerInterval = int(TSEC1/secPerBreath+0.5)+1;
    xt1 = 10*breathsPerInterval*secPerBreath;
    
    fracFreq2 = BPM2/600;
    secPerBreath = 60.0/BPM2;
    breathsPerInterval = int(TSEC2/secPerBreath+0.5)+1;
    xt2 = xt1 + 10*breathsPerInterval*secPerBreath;
    
    fracFreq3 = BPM3/600; 
    secPerBreath = 60.0/BPM3;

    simTick = 0; cycleAmp = AMP1; fracFreq = fracFreq1; // interval 1
  }
  else if (tick == xt1) // transition to interval 2
  {
    simTick = 0; cycleAmp = AMP2; fracFreq = fracFreq2;
  }
  else if (tick == xt2) // transition to interval 3
  {
    simTick = 0; cycleAmp = AMP3; fracFreq = fracFreq3;
  }
  
  // Run breathing simulation parameterized by interval
  float degC = 5.0; // DC offset
  degC += 0.0*tick/100.0; // drift
  degC += cycleAmp*sin((fracFreq*simTick++)*TWO_PI);  
  degC += 0.0*((random(0,11)/10.0-0.5)/0.29); 
  return degC;
}
