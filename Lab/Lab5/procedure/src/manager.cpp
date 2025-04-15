#include <manager.hpp>

const int TSAMP_MSEC = 100; // senor: 100, sim: 10
const int DAC0 = 3, DAC1 = 4, DAC2 = 5, LM61 = A0, VDITH = A1;
const int NUM_SUBSAMPLES = 160, NUM_SAMPLES = 256;

volatile boolean sampleFlag = false;
int tick = 0;
float sample, mean, stdev;

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
  return analogRead(VDITH);  
}

//*******************************************************************
float analogReadDitherAve(int index)
{

  float sum = 0.0;
  for (int i = 0; i < NUM_SUBSAMPLES; i++) {
      digitalWrite(DAC0, (i & B00000001)); // LSB bit mask
      digitalWrite(DAC1, (i & B00000010));
      digitalWrite(DAC2, (i & B00000100)); // MSB bit mask
      sum += analogRead(LM61);
  }
  
  return sum / NUM_SUBSAMPLES; // Return the average of subsamples
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