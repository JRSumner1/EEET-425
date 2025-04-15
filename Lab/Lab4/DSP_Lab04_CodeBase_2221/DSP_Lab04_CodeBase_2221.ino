// OPEN NEW ARDUINO SKETCH.
// CLICK IN THIS TEXT BOX. CTRL-A, CTRL-C.
// CLICK IN SKETCH. CTRL-A, CTRL-V.

// Lab2_handout.ino
// created by: Clark Hochgraf 2015
// modified by: David Orlicki, October 1, 2017
// modified by: David Orlicki, September 7, 2019
// modified by: Mark Thompson, September 8, 2020

// Outputs 3 bit DAC resistor network drive.
// Reads/displays ADC samples of LM61 sensor output.
// Reads/displays averaged subsamples of LM61 sensor output.
// Reads/displays dithered-averaged subsamples of LM61 sensor output.
// Calculates running mean and standard deviation of samples.

#include <MsTimer2.h>
#include <SPI.h>

const int TSAMP_MSEC = 100; // senor: 100, sim: 10
const int DAC0 = 3, DAC1 = 4, DAC2 = 5, LM61 = A0, VDITH = A1;
const int NUM_SUBSAMPLES = 160, NUM_SAMPLES = 256;
  
volatile boolean sampleFlag = false;
int tick = 0;
float sample, mean, stdev;

//*******************************************************************
void setup()
{
  configureArduino();


  
// This line tells MATLAB that the Arduino is ready to accept data
  Serial.println("%Arduino Ready");
  
  //Serial.println("Enter 'g' to go .....");

  // Wait until MATLAB sends a 'g' to start sending data
  while (Serial.read() != 'g'); // spin until 'g' entry  
  
  MsTimer2::set(TSAMP_MSEC, ISR_Sample); // Set sample msec, ISR name
  MsTimer2::start(); // start running the Timer   
} // setup()

//*******************************************************************
void loop()
{
  syncSample(); 
  
 sample = analogRead(LM61);
//  sample = analogReadAve();
//  sample = testDAC(tick);
//  sample = testDAC(random(256));
  // sample = analogReadDitherAve();
// sample = 185-0*tick/32.0+2*sin((tick/32.0)*TWO_PI);

  // recurseStats1(sample, mean, stdev);
  // recurseStats2(sample, mean, stdev);
  
  displayData();
  if (++tick >= NUM_SAMPLES) while(true); // spin forever
} // loop()

//*******************************************************************
float analogReadAve(void)
{ 
  float sum = 0.0;
  for (int i = 0; i < NUM_SUBSAMPLES; i++) sum += analogRead(LM61);
  return sum/NUM_SUBSAMPLES; // averaged subsamples   
}

//*******************************************************************
float testDAC(int index)
{
  digitalWrite(DAC0, (index & B00000001)); // LSB bit mask
  digitalWrite(DAC1, (index & B00000010));
  digitalWrite(DAC2, (index & B00000100)); // MSB bit mask
  return analogRead(LM61);  
}

//*******************************************************************
float analogReadDitherAve(void)
{ 
  float sum = 0.0;
  for (int i = 0; i < NUM_SUBSAMPLES; i++) {
    sum += testDAC(i);
  }
  return sum/NUM_SUBSAMPLES; 
}

//*******************************************************************
void recurseStats1(float smpl, float &mean, float &stdev)
{
  // Smith algorithm adjusted for tick start at 0
  static float sum, sumSquares;
  
  if (tick == 0)
  {
    mean = smpl;
    sum = smpl;
    sumSquares = smpl*smpl;
  }
  else 
  {
   sum += smpl;
   sumSquares += smpl*smpl;
   mean = sum/(tick+1);
   float var = (sumSquares - (sum*sum)/(tick+1))/tick;
   stdev = sqrt(var);    
  }  
}

//*******************************************************************
void recurseStats2(float smpl, float &mean, float &stdev)
{
  // B. P. Welford algorithm adjusted for tick start at 0
  static float oldMean, runSumVar, oldRunSumVar;
  
  mean = 0.0;
  stdev = 0.0;
}

//*******************************************************************
void displayData(void)
{
  if (tick == 0) Serial.print("\nn\tsample\n");
  Serial.print(tick);   Serial.print('\t');
  Serial.print(sample); Serial.print('\n');
}

//*******************************************************************
void syncSample(void)
{
  while (sampleFlag == false); // spin until ISR trigger
  sampleFlag = false;          // disarm flag: enforce dwell  
}

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
void ISR_Sample()
{
  sampleFlag = true;
}
