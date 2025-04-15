// OPEN A NEW SKETCH WINDOW IN ARDUINO
// CLICK IN THIS BOX, CTL-A, CTL-C (Copy code)
// CLICK IN SKETCH, CTL-A, CTL-V (Paste code into sketch)

// file: Lab7_FIR_kernel_maker 191025.ino

// created:  David Orlicki October 18, 2017
// modified: David Orlicki October 25, 2018
// modified: David Orlicki October 25, 2019
// modified: Mark Thompson March 20, 2021 
//           Added comments and modified the SINC function display
//           function to make copying to MATLAB easier


// MFILT is the kernel length.  Must be an odd number
const int MFILT = 81; // MFILT points from 0...MFILT-1

// FC_BPM is the corner frequency of the filter in breaths per minute
// MOD_BPM is not used
const int FC_BPM = 50, MOD_BPM = 0;

//FC is the normalized corner frequency.  Normalized to the sample rate
const float FC = FC_BPM/600.0; // fs = 10 Hz -> 600 bpm

//FM is the normalized MOD frequency.  Normalized to the sample rate
const float FM = MOD_BPM/600.0;

//HFXPT is the scale value to convert to a fixed point kernel
const long HFXPT = 10000;


//  Storage for the kernel values
int h[MFILT] = {0};
int hFilt[MFILT] = {0};

//*********************************************************************
void setup()
{
  Serial.begin(115200);delay(20);
// Handshake with User      
  Serial.println(F("%Arduino Ready Press 'g' to go\n"));
  while (Serial.read() != 'g'); // spin


  
  Serial.println(F("FIR kernel header maker 191025"));
  
  buildKernelSincLPF(FC, h);
  displayHeaderSinc(h, "LPF");
  displayKernel(h);

  spectralInvertKernel(h, hFilt);
  displayHeaderSinc(hFilt, "HPF-Spectral Invert");
  displayKernel(hFilt);
}

//*********************************************************************
void loop(){}

//*******************************************************************
void buildKernelSincLPF(float Fc, int h[])
//
// This function computes a Windowed SINC lowpass filter.
//  
//  The code operates by first building a floating point 
//  version of the filter and while doing so summing
//  all the kernel values.  Then the filter is created again
//  using floating point numbers, but the floating point values are
//  converted to fixed point at the end by dividing by the sum
//  of the kernel values to set the filter gain to 1.0 and then
//  scaling by HFXPT to convert the numbers to fixed point.
//
//  Windowed Sinc Lowpass Filter Kernel, M odd
{
 
  float hv, accumRaw = 0.0;

  // This part of the code builds the filter once and finds the filter gain
  //
  //  Iterate from i = 0 to MFILT-1.  MFILT/2 is the center point
   
  for (int i=0; i < MFILT; i++)
  {

    //  hv is the argument of the SINC function.  See Page 285 in Smith
    //  It is a function of the normalized corner frequency of the filter.
     
    hv = TWO_PI*Fc*(i-MFILT/2);

    // if the point is the center point then the argument hv will be 0 and
    // dividing by zero is a problem, so correct for this point.  In the limit
    // sin(0)/0 will be 1.0 so set the point to 1.0
    
    if (i == (MFILT/2)) hv = 1.0; // L'Hopital point repair for the center point

    // Otherwise compute SINC normally.  SINC(hv) = sin(hv)/hv
    
    else hv = sin(hv)/hv;

    //  Now compute the value of the Hamming Window a this point and
    //  multiply the kernel value by this window value
    
    hv *= 0.54-0.46*cos(TWO_PI*i/(MFILT-1)); 

    //  Add upt the values of each point in the kernel to find the gain
    //  of the filter.  This will be used later to scale the kernel values
    
    accumRaw += hv; // floating point sinc DC gain normalizer
  }

  //  Now build the filter again, but scale each value in the kernel by the gain
  //  and the value of HFXPT and convert the values to fixed point.  See comments above
  //  to determine what each line below is doing.
  
  for (int i=0; i < MFILT; i++)
  {
    hv = TWO_PI*Fc*(i-MFILT/2);
    if (i == (MFILT/2)) hv = 1.0; // L'Hopital point repair
    else hv = sin(hv)/hv; // sinc
    hv *= 0.54-0.46*cos(TWO_PI*i/(MFILT-1)); // window

    //  Normalize by the filter gain and scale for fixed point
    
    h[i] = int(HFXPT*hv/accumRaw + ((hv > 0.0) ? 0.5 : -0.5)); // round, fix point
  }
}
//**********************************************************************
void spectralInvertKernel(int h[], int hsi[])
{
  // YOUR CODE GOES HERE. Create HPF by spectral inversion per Smith.
  // The input argument is h[] and the output argument (the kernel for 
  // the HPF) is hsi[]
  for (int i = 0; i < MFILT; i++)
  {
    if (i == MFILT/2)
    {
      // Center tap: 1 - hLPF(center)  in fixed-point
      hsi[i] = HFXPT - h[i];
    }
    else
    {
      // All other taps: just flip sign
      hsi[i] = -h[i];
    }
  }
}
//*********************************************************************
void displayKernel(int h[])
{
//  for (int i = 0; i < MFILT; i++)
//  {
//    if (i == 0) Serial.println(F("\nh[n]"));
//        Serial.println(h[i]);
//  }


  Serial.print(F("\n\nh[n] to copy to MATLAB\n\t"));
  for (int i = 0; i < (MFILT-1); i++)
  {
    Serial.print(h[i]);Serial.print(",");
  }
  Serial.print(h[MFILT-1]);Serial.print("\n");

  
}
//*********************************************************************
void displayHeaderSinc(int h[], String Ftype)
{ 
  // console output in C declaration format
  
  Serial.print(F("\n// Filter type: ")); Serial.print(Ftype);
  Serial.print(F("\nconst int Fc_bpm = ")); Serial.print(FC_BPM);
  Serial.println(';');
  Serial.print(F("const int HFXPT = ")); Serial.print(HFXPT);
  Serial.print(F(", MFILT = ")); Serial.print(MFILT);
  Serial.println(';');
   
  const int NCOL = 14;
  Serial.print(F("int h[] = {"));
  Serial.print(h[0]); Serial.print(","); 
  for (int i = 1; i < (MFILT-1); i++)
  {
    Serial.print(h[i]);
    if ((i%NCOL) < (NCOL-1)) Serial.print(",");
    else if (i == MFILT-2) Serial.print(",");
    else Serial.print(",\n\t");
  }
  Serial.print(h[MFILT-1]);
  Serial.println("};");
}
