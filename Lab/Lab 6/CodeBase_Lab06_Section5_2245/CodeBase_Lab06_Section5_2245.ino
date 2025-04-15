// OPEN A NEW SKETCH WINDOW IN ARDUINO
// Copy the code in this file into the new sketch

// file: CodeBase_Lab06_Section5_2245.ino

// created by: Clark Hochgraf/David Orlicki Oct 19, 2019
// Modified by: Mark Thompson 2/15/2020  Corrected the generation of the impulse
//              response so that even M can be handled correctly.  Added comments
// 				Mark Thompson 2/28/2025

#include <MsTimer2.h>

//  Define some constants as integers
//
//  TSAMP_MSEC -- Sample time in milliseconds
//  DATA_LEN -- number of samples to compute
//  MFILT -- number of samples in the impulse response

const int TSAMP_MSEC = 10, DATA_LEN = 256, MFILT = 41;

//  Define some constants as LONG
//
//  DATA_FXPT -- Value to scale floating point data values by to create 
//        a fixed point number
//  HFXPT -- Value to scale the floating point impulse response values by
//        when making them fixed point

const long DATA_FXPT = 100, HFXPT = pow(2,12);

// Define variables
// sampleFlag -- Boolean set when it is time to sample
// fxptX -- Fixed point X value -- Input to the filter
// fxptY -- Fixed point Y value -- Output of the filter
// hfxpt[] -- Fixed point array holding the impulse response values
// fltX -- Floating point X value
// fltY -- Floating point Y value
// tick -- Sample number
// i  -- General index
// execUsec -- execution time in microseconds

volatile boolean sampleFlag = false;
int fxptX, fxptY, hfxpt[MFILT];
float fltX, fltY;
int tick = 0, i;
unsigned long execUsec;

//*******************************************************************
void setup()
{

  //  Set up the serial port and display a header
  
  Serial.begin(115200); delay(20);
 
  //  Declare the impulse response array hflt of length MFILT hflt as a float
  //  Declare the corner frequency of the filter
  
  float hflt[MFILT], Fc;

  //  Create a fixed and floating point version of the SINC impulse response
  // (filter kernel).  Return the impulse reponses in hflt and hfxpt.  Then 
  //  Display the kernel and its properties
   
  buildFixedPointKernelSinc(Fc = 0.05, hflt, hfxpt);
  displayKernel(hflt, hfxpt);

// Handshake with MATLAB
 
  Serial.println(F("%Arduino Ready"));
//  Serial.println(("\nEnter 'g' to go ..."));
  while (Serial.read() != 'g'); // spin  

  //  Set up the interrupt timer

  MsTimer2::set(TSAMP_MSEC, ISR_Sample); // Set sample msec, ISR name
  MsTimer2::start();
}

//*******************************************************************
void loop()
{

  //  Wait for an interrupt to occur and trigger a sample
  syncSample(); 

  //  Compute a single point from the simulated breath function, then 
  //  compute the next point out of the filter for the new point into the filter
  //  Pass the new input value and the fixed point filter kernel (impulse response)
  //  Return a value in fxptY.    Display the data output
  
  fxptX = simulatedBreathFxptDegC();    
  fxptY = fxptKernelFIR(fxptX, hfxpt);
  displayData(fxptX, fxptY);

  //  Test for having generated enough data, print the execution time, then just spin
  
  if (++tick >= DATA_LEN)
  {
    Serial.print("\npoint usec = "); Serial.println(execUsec/float(MFILT));
    while (true); // freeze process (spin forever)
  }
} // loop()

//*******************************************************************
int fxptKernelFIR(int xin, int h[])
{ 

  //  Declare a static buffer to store values as they come in.  The input
  //  value is xin
  
  static int x[MFILT] = {0};

  //  Timer for execution time
  execUsec = micros();
 

  // Shift the new sample into input buffer and discard the oldest sample.
  //  The newest value is x[0].  This puts the input buffer in time reversed order
  //  Putting these in time reversed order accomplishes the same thing as time
  //  reversing or flipping the impulse response.
  
  for (i = MFILT-1; i >= 1; i--)
  {
    x[i] = x[i-1];  // Shift the buffer to the "right"
    x[0] = xin;     //  Bring the newest sample in on the "left"
  }

  // MEASURE PER-POINT CONVOLUTION TIME FOR BOTH FLOAT AND LONG CASTS
  // JUSTIFY CAST CHOICE IN TERMS OF HFXPT AND INPUT VARIABLE SIZE BOUND

  //  Perform the convolution operation.  Multiply each value in the input buffer
  //  by each impulse response value with the same index and accumulate the
  //  sum.  Remember that the input is flipped in time so you don't need to flip the
  //  impulse response in time

  //  FLOAT Version of the convolution sum
  float yv = 0; for (i = 0; i < MFILT; i++) yv += float(h[i])*x[i];

  //  LONG version of the convolution sum
  // long yv = 0; for (i = 0; i < MFILT; i++) yv += long(h[i])*x[i];


  //  Scale down (divide) the output of the convolution by the fixed point scale value for
  //  impulse response.  This keeps the magnitude of the output from growing
  //  exponentially
  
  const float INV_HXPT = 1.0/HFXPT; // division slow: precalculate
  int yret = int(yv*INV_HXPT);

  //  If there haven't been enough samples to entirely fill the input buffer
  //  just return the input value itself.  This avoids the end effects
  if (tick < MFILT) yret = xin;

  //  Compute the execution time for the convolution routine.
  execUsec = micros()-execUsec; 

  
  return yret;
}
//*******************************************************************
void displayData(float X, float Y)
{
  if (tick == 0) Serial.print(F("\nn\tX\tY\n"));
  Serial.print(tick); Serial.print('\t');
  Serial.print(X); Serial.print('\t'); // fixed point input
  Serial.println(Y); // fixed point output
}
//*******************************************************************
float simulatedBreathFxptDegC(void) // handout starter code
{ 
  float degC = 0.0;

  float driftIntercept;
  float driftSlope;
  float samplesPerCycle;
  float tempP;
  float bpm;
  float sigma;
  
   //  Create a simulated breath output value using the equation
  //
  //  n is the sample number 
  //
  //  driftSlope is in degrees per sample
  //
  //  tempP is the peak temperature variation
  //
  //  samplesPerCycle is the period of the sinusoid of the breathing rate
  //  This translates to a breathing rate in bpm of
  //     bpm = 1/samplesPerCycle * 10Sec/Sample * 60 Sec/Min
  //
  //  randNoise -- a uniformly distributed number from +/- (.5/.29 ) so it has
  //      a standard deviation of 1 degree C
  //
  //  degC = driftIntercept + ( driftSlope * n ) + tempP * sin( 2pi * n / ticksPerCycle )
  //         + sigma * randNoise

  //  Replace these values with values that match your model
  
  driftIntercept = 50;
  driftSlope = 0.0;
  bpm = 12;
  tempP = 0.6;
  sigma = 0.05;
  
  samplesPerCycle = 10 * 60 / bpm;
  degC = driftIntercept; // driftIntercept value
  degC += driftSlope*(tick); // driftSlope in degC/sample
  degC += tempP*sin(TWO_PI*tick/samplesPerCycle);
  degC += sigma*((random(0,11)/10.0-0.5)/0.29); // scaled 1 degC  * sigma


  //  Convert the floating point temperature to a fixed point value by scaling
  //  the floating point number up by DATA_FXPT and rounding.  This
  //  gives the fixed point number better resolution.
  
  int fxptDeg = int(DATA_FXPT*degC + 0.5);
  return fxptDeg;
  
}
//*******************************************************************
void buildFixedPointKernelSinc(float Fc, float hflt[], int hfxp[])
{
  // Windowed Sinc Lowpass Filter Kernel
  // M even => M+1 elements: 0...M => kernel symmetric about M/2
  //
  //  First create a floating point version then convert it to fixed point by
  //  scaling and rounding
  
  float accum = 0.0, midSample;

  midSample = (float(MFILT)-1)/2 ;
 
  
  // Normalized floating point sinc
  //  Repeat for each value in the impulse response.  the 
  
  for (int i=0; i <= MFILT-1; i++)
  {
    //  Start by computing the angle of a point of the sine wave.  For 
    //  a corner frequency of Fc (which is expressed as a fraction of the 
    //  sample frequency).  
    
    
    hflt[i] = TWO_PI*Fc*(i-midSample);

    //  Check to see if the sample is in the middle.  If so sin(x)/x will evaluate
    //  to infinity and fail.  Set it to one considering l'Hopital's rule
    //  If not in the middle, then just compute sin(x)/x as normal
    
    if (i == midSample)
    {
      hflt[i] = 1.0; // L'Hopital's point repair
    }
    else 
    {
      hflt[i] = sin(hflt[i])/hflt[i]; // sinc
    }

    //  Create a value of a Hamming window and weight the value of the impulse
    //  response at that point by the window value.
    
    hflt[i] *= (0.54-0.46*cos(TWO_PI*i/(MFILT-1))); // window

    //  Sum up the values in the filter to determine the DC gain
    
    accum += hflt[i]; // raw windowed kernel sum
  }


  Serial.print(F("\nHFXPT = ")); Serial.print(HFXPT); 
  Serial.print(F(", DATA_FXPT = ")); Serial.println(DATA_FXPT); 

  //  Iterate through all values of the filter and divide each value by the 
  //  sum of all the values in the filter (the DC Gain) computed above.
  //  This will then normalize the DC gain of the filter to 1.
  
  for (i=0; i < MFILT; i++) hflt[i] = hflt[i]/accum; // normalize

  //  Iterate through all the values and sum them.  This is to make sure that
  //  the DC gain was really normalized to 1
  
  accum = 0.0; for (i=0; i < MFILT; i++) accum += hflt[i]; // kernel sum
  Serial.print(F("flt kernel sum = ")); Serial.println(accum,4); 
    
  // Fixed Point Scale the impulse response
  // Multiply each value in the impulse response by the fixed point
  // scaling value for the impulse response (HFXPT).  Round the value
  // considering the sign of the value
  
  for (int i=0; i < MFILT; i++)
  {
    hfxp[i] = int(HFXPT*hflt[i]+((hflt[i] > 0.0) ? 0.5 : -0.5));
  }

  //  Again compute the DC gain, but of the fixed point version by summing
  //  all the values in the impulse response
  
  accum = 0.0; for (int i=0; i < MFILT; i++) accum += hfxp[i];

  
  Serial.print(F("fxpt kernel sum = ")); Serial.println(accum,0);
  Serial.print(F("fxpt DC gain = ")); Serial.println(accum/HFXPT,4); 
}
//*******************************************************************
void displayKernel(float hflt[], int hfxpt[])
{
  Serial.println(F("\nn\thflt\thfltSc\thfxpt[n]"));
  for (int i=0; i < MFILT; i++)
  {
    Serial.print(i); Serial.print('\t');
    Serial.print(hflt[i],6); Serial.print('\t');
    Serial.print(HFXPT*hflt[i]); Serial.print('\t');
    Serial.println(hfxpt[i]);
    delay(10);
  }
}
//*******************************************************************
void syncSample(void)
{
  while (sampleFlag == false); // spin until ISR trigger
  sampleFlag = false;          // disarm flag: enforce dwell
}
//*******************************************************************
void ISR_Sample() { sampleFlag = true; }
