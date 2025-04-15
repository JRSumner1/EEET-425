// OPEN NEW ARDUINO SKETCH.
// CLICK IN THIS TEXT BOX. CTRL-A, CTRL-C.
// CLICK IN SKETCH. CTRL-A, CTRL-V.

// Lab2_handout.ino
// created by: Clark Hochgraf 2015
// modified by: David Orlicki, October 1, 2017
// modified by: David Orlicki, September 7, 2019
// modified by: Mark Thompson, October 15, 2020

#include <MsTimer2.h>
#include <SPI.h>

const int TSAMP_MSEC = 100;
const int NUM_SUBSAMPLES = 160, NUM_SAMPLES = 256;
const int DAC0 = 3, DAC1 = 4, DAC2 = 5, LM61 = A0, VDITH = A1;
const int V_REF = 5.0;

int tick = 0;

volatile boolean sampleFlag = false;

const long DATA_FXPT = 1000; // Scale value to convert from float to fixed

const float INV_FXPT = 1.0 / DATA_FXPT; // division slow: precalculate

float eqOutputFlt = 0.0;

float sample, volts, degC;



int nSmpl = 1;

//**********************************************************************
void setup()
{

  configureArduino();
  Serial.println("%Arduino Ready");

// Wait until MATLAB sends a 'g' to start sending data
  while (Serial.read() != 'g'); // spin until 'g' entry  
  
  MsTimer2::set(TSAMP_MSEC, ISR_Sample); // Set sample msec, ISR name
  MsTimer2::start(); // start running the Timer   
  
} // end setup()  


////**********************************************************************
void loop()
{

  // Breathing Rate Detection

  // Declare variables


  long fxdInputValue, lpfInput, lpfOutput;
  long eqOutputFxd;

   syncSample();

  //  Read sample from ADC
  sample = analogReadDitherAve();

  // Convert the ADC value to temperature
  volts = (sample/1024)*V_REF;
  degC = 100 * (volts - 0.6);
 
  //  Convert the floating point number to a fixed point value.  First
  //  scale the floating point value by a number to increase its resolution
  //  (use DATA_FXPT).  Then round the value and truncate to a fixed point
  //  INT datatype

  fxdInputValue = long(DATA_FXPT * degC + 0.5);

  //  Execute the equalizer

  eqOutputFxd = Equalizer( fxdInputValue );

  // Scale the equalizer output 
  eqOutputFlt = float(eqOutputFxd) * INV_FXPT;

  
  displayData();
  if (++tick >= NUM_SAMPLES) while(true); // spin forever

} // loop()

//*******************************************************************
long Equalizer(long xInput )
{

  int i;
  long yN=0; //  Current output
  const int equalizerLength = 4;
  static long xN[equalizerLength] = {0};
  long h[] = { 1, 1, -1, -1 };  // Impulse response of the equalizer 
  
  //  Update the xN array

  for ( i = equalizerLength-1 ; i >= 1; i-- )
  {
    xN[i] = xN[i - 1];
  }

  xN[0] = xInput;

  //  Convolve the input with the impulse response

  for ( i = 0; i <= equalizerLength-1 ; i++)
  {
    yN += h[i] * xN[i];
  }

  if (tick < equalizerLength)
  {
    return 0;
  }
  else
  {
   return yN;
  }

}

//*******************************************************************
float analogReadDitherAve(void)
{ 
 
float sum = 0.0;
int index;
  for (int i = 0; i < NUM_SUBSAMPLES; i++)
  {
    index = i;
    digitalWrite(DAC0, (index & B00000001)); // LSB bit mask
    digitalWrite(DAC1, (index & B00000010));
    digitalWrite(DAC2, (index & B00000100)); // MSB bit mask
    sum += analogRead(LM61);
  }
  return sum/NUM_SUBSAMPLES; // averaged subsamples 

}

//*******************************************************************
void displayData(void)
{
  if (tick == 0) Serial.print("\nn\tsample\teqOut\n");
  Serial.print(tick);   Serial.print('\t');
  Serial.print(sample); Serial.print('\t');
  Serial.print(eqOutputFlt,5); Serial.print('\n');
  
} // end displayData


//*******************************************************************
void configureArduino(void)
{
  pinMode(DAC0,OUTPUT); digitalWrite(DAC0,LOW);
  pinMode(DAC1,OUTPUT); digitalWrite(DAC1,LOW);
  pinMode(DAC2,OUTPUT); digitalWrite(DAC2,LOW);
  analogReference(DEFAULT); // DEFAULT, INTERNAL
  analogRead(LM61); // read and discard to prime ADC registers
  Serial.begin(115200); // 11 char/msec 
}
//*******************************************************************
void syncSample(void)
{
  while (sampleFlag == false); // spin until ISR trigger
  sampleFlag = false;          // disarm flag: enforce dwell  
}
//**********************************************************************
void ISR_Sample()
{
  sampleFlag = true;
}
