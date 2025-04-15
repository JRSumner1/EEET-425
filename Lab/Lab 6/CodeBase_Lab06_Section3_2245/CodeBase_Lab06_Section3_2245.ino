// OPEN A NEW SKETCH WINDOW IN ARDUINO
// Copy the code in this file into the new sketch

// file: CodeBase_Lab06_Section3_2245.ino


// created by: Clark Hochgraf/David Orlicki Oct 19, 2019
// Modified by: Mark Thompson 2/27/2025

#include <MsTimer2.h>

//  Define some constants as integers
//
//  TSAMP_MSEC -- Sample time in milliseconds
//  DATA_LEN -- number of samples to compute
//  MFILT -- number of samples in the impulse response

const int TSAMP_MSEC = 10, DATA_LEN = 256, MFILT = 11;


//  Define some constants as LONG
//
//  DATA_FXPT -- Value to scale floating point data values by to create 
//        a fixed point number
//  HFXPT -- Value to scale the floating point impulse response values by
//        when making them fixed point

const long DATA_FXPT = 100;

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
int fxptX, fxptY;
float fltX, fltY, hflt[MFILT], Fc;
int tick = 0, i;
unsigned long execUsec;

//*******************************************************************
void setup()
{
	
  //  Set up the serial port and display a header
  
  Serial.begin(115200); delay(20);

  
  //  Call the function to create a moving average filter impulse response
   
//  buildKernelMAV(hflt);
  
  // Call the function to create a Windowed SINC filter impulse response
  
  buildKernelSinc(Fc = 0.05, hflt);
  
 displayKernel(hflt);

 
//  Handshake with MATLAB
  Serial.println(F("%Arduino Ready"));
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
  fxptY = floatKernelFIR(fxptX, hflt);
  displayData(fxptX, fxptY);

  //  Test for having generated enough data, print the execution time, then just spin
    
  if (++tick >= DATA_LEN)
  {
    //Serial.print("\nconvolution point usec = ");
    //Serial.println(execUsec/float(MFILT));
    while (true); // freeze process (spin forever)
  }
} // loop()

//*******************************************************************
void  buildKernelMAV(float hflt[])
{
	
//  Create a moving average filter.  The MAV filter is a constant values for 
//  each sample in the impulse response.  The value is 1/M, the number of samples
//  in the impulse response.  When all the values are summed the total is 1.
//  This makes the DC gain of the filter 1

  // DC gain = sum of kernel samples = MFILT*(1/MFILT) = 1.0
  float accum;
  for (int i = 0; i < MFILT; i++) hflt[i] = 1.0/MFILT;  
}
//*******************************************************************
void buildKernelSinc(float Fc, float hflt[])
{
  // Windowed Sinc Lowpass Filter Kernel
  // M even => M+1 elements: 0...M => kernel symmetric about M/2
  
  //  First create a floating point version then convert it to fixed point by
  //  scaling and rounding  
  float accum = 0.0, midSample;
  midSample = (float(MFILT)-1)/2 ;
  
  // Normalized floating point sinc
  //  Repeat for each value in the impulse response.
  
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
 
  //  Iterate through all values of the impulse response and sum the values.
  //  This value is the DC gain of the filter.
 
  accum = 0.0; for (int i=0; i < MFILT; i++) accum += hflt[i];
  
  //  Iterate through all values of the filter and divide each value by the 
  //  sum of all the values in the filter (the DC Gain) computed above.
  //  This will then normalize the DC gain of the filter to 1.
  
  // To Do -- Modify the line below by dividing each value in the kernel
  // by the accumulated sum of the kernel
  
  for (i=0; i < MFILT; i++) hflt[i] = hflt[i]/accum; // NORMALIZE GAIN HERE
  
  //  Iterate through all the values and sum them.  This is to make sure that
  //  the DC gain was really normalized to 1
  accum = 0.0; for (int i=0; i < MFILT; i++) accum += hflt[i];
  
  
  
  
  Serial.print(F("\nsinc kernel sum = ")); Serial.println(accum,3);
}
//*******************************************************************
void displayKernel(float hflt[])
{
  Serial.println(F("\nn\thflt"));
  for (int i=0; i < MFILT; i++)
  {
    Serial.print(i); Serial.print('\t');
    Serial.println(hflt[i],6);
    delay(10); // clear USART comm buffer
  }
}
//*******************************************************************
int floatKernelFIR(int xin, float h[])
{
	

  //  Declare a static buffer to store values as they come in.  The input
  //  value is xin	
  static int x[MFILT] = {0};
  
  //  Timer for execution time  
  execUsec = micros();
  
  
  //  Shift the new sample into the input buffer and discard the oldest sample.
  //  The newest value is x[0].  This puts the input buffer in time reversed order
  //  Putting these in time reversed order accomplishes the same thing as time
  //  reversing or flipping the impulse response.
  
  
  for (i = MFILT-1; i >= 1; i--)
  {	 
	x[i] = x[i-1];
	x[0] = xin;

  }
  
  //  Perform the convolution operation.  Multiply each value in the input buffer
  //  by each impulse response value with the same index and accumulate the
  //  sum.  Remember that the input is flipped in time so you don't need to flip the
  //  impulse response in time

  //  FLOAT Version of the convolution sum  
    
  float yv = 0.0;

  for (i = 0; i < MFILT; i++)
  {
	  yv += h[i]*x[i];
  }
  
  //  If there haven't been enough samples to entirely fill the input buffer
  //  just return the input value itself.  This avoids the end effects. 
  //  Comment select the following line in to remove the end effects 

  if (tick < MFILT) yv = xin;
   
  execUsec = micros()-execUsec;
  return int(yv);
}
//*******************************************************************
void displayData(float X, float Y)
{
  //if (tick == 0) Serial.print(F("\nn\tX\tY\n"));
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
  float tempPeak;
  float bpm;
  float sigma;
  
   //  Create a simulated breath output value using the equation
  //
  //  n is the sample number 
  //
  //  driftSlope is in degrees per sample
  //
  //  tempPeak is the peak temperature variation
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
  
  driftIntercept = 30;
  driftSlope = 0.0;
  bpm = 12;
  tempPeak = 0.6;
  sigma = 0.05;
  
  samplesPerCycle = 10 * 60 / bpm;
  degC = driftIntercept; // driftIntercept value
  degC += driftSlope*(tick); // driftSlope in degC/sample
  degC += tempPeak*sin(TWO_PI*tick/samplesPerCycle);
  degC += sigma*((random(0,11)/10.0-0.5)/0.29); // scaled 1 degC  * sigma


  //  Convert the floating point temperature to a fixed point value by scaling
  //  the floating point number up by DATA_FXPT and rounding.  This
  //  gives the fixed point number better resolution.
  
  int fxptDeg = int(DATA_FXPT*degC + 0.5);
  return fxptDeg;
  
}



//*******************************************************************
void syncSample(void)
{
  while (sampleFlag == false); // spin until ISR trigger
  sampleFlag = false;          // disarm flag: enforce dwell
}
//*******************************************************************
void ISR_Sample() { sampleFlag = true; }
