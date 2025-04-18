// Breathing Rate Detection System -- Final Integration
//
// Pieced together from code created by: Clark Hochgraf and David Orlicki Oct 18, 2018
// Modified by: Mark Thompson April 2020 to integrate MATLAB read and Write 
//              and integrate the system

#include <MsTimer2.h>
#include <SPI.h>

const int TSAMP_MSEC = 100;
const int NUM_SAMPLES = 3600;
const int NUM_SUBSAMPLES = 160;
const int DAC0 = 3, DAC1 = 4, DAC2 = 5, LM61 = A0, VDITH = A1;
const int V_REF = 5.0;
const int SPKR = 12; // d12  PB4

volatile boolean sampleFlag = false;

const long DATA_FXPT = 1000; // Scale value to convert from float to fixed
const float INV_FXPT = 1.0 / DATA_FXPT; // division slow: precalculate

int nSmpl = 1, sample;

float xv, yv, yLF, yMF, yHF, stdLF, stdMF, stdHF;
float printArray[9];
int numValues = 0;

int loopTick = 0;
bool statsReset;
bool isToneEn = false;

unsigned long startUsec, endUsec, execUsec;

//  Define a structure to hold statistics values for each filter band
struct stats_t
{
  int tick = 1;
  float mean, var, stdev;
} statsLF, statsMF, statsHF;

//**********************************************************************
void setup()
{
  configureArduino();
  Serial.begin(115200);delay(5);

   //Handshake with MATLAB 
  Serial.println(F("%Arduino Ready"));
  while (Serial.read() != 'g'); // spin

  MsTimer2::set(TSAMP_MSEC, ISR_Sample); // Set sample msec, ISR name
  MsTimer2::start(); // start running the Timer  
}

//**********************************************************************
void loop()
{
  // syncSample();  // Wait for the interupt when actually reading ADC data
  // Breathing Rate Detection
  // Declare variables
  float readValue, floatOutput;  //  Input data from ADC after dither averaging or from MATLAB
  long fxdInputValue, lpfInput, lpfOutput;  
  long eqOutput;  //  Equalizer output
  int alarmCode;  //  Alarm code

  // ******************************************************************
  //  When finding the impulse responses of the filters use this as an input
  //  Create a Delta function in time with the first sample a 1 and all others 0
  // xv = (loopTick == 0) ? 1.0 : 0.0; // impulse test input

  //  Use this when the test vector generator is used as an input
  xv = testVector();

  //  Read input value in ADC counts  -- Get simulated data from MATLAB
  //readValue = ReadFromMATLAB();

  //  Read input value from ADC using Dithering, and averaging
  readValue = analogReadDitherAve();

  //  Convert the floating point number to a fixed point value.  First
  //  scale the floating point value by a number to increase its resolution
  //  (use DATA_FXPT).  Then round the value and truncate to a fixed point
  //  INT datatype
  fxdInputValue = long(DATA_FXPT * readValue + 0.5);

  //  Execute the equalizer
  eqOutput = EqualizerFIR(fxdInputValue);
  
  //  Execute the noise filter.  
  // eqOutput = NoiseFilter( eqOutput, loopTick );

  //  Convert the output of the equalizer by scaling floating point
  // xv = float(eqOutput) * INV_FXPT;

  // Uncomment this when measuring execution times
  startUsec = micros();

  // Filter bank
  yLF = IIR_LPF(xv); // second order systems cascade
  yMF = IIR_BPF(xv);  
  yHF = IIR_HPF(xv);

  // Determine which filter is active and its standard deviation
  statsReset = ((statsLF.tick % 100) == 0);
  getStats(yLF, statsLF, statsReset);
  stdLF = statsLF.stdev;

  statsReset = ((statsMF.tick % 100) == 0);
  getStats(yMF, statsMF, statsReset);
  stdMF = statsMF.stdev;

  statsReset = ((statsHF.tick % 100) == 0);
  getStats(yHF, statsHF, statsReset);
  stdHF = statsHF.stdev;

  // Uncomment this when measuring execution times
  endUsec = micros();
  execUsec = execUsec + (endUsec-startUsec);

  //  Call the alarm check function to determine what breathing range 
  // alarmCode = AlarmCheck( stdLF, stdMF, stdHF );

  //  Call the alarm function to turn on or off the tone
  // setAlarm(alarmCode, isToneEn );

  printArray[0] = loopTick;  //  Sample Number
  printArray[1] = xv;        //  Input Data
  printArray[2] = yLF;       //  Raw Low Pass Filter
  printArray[3] = yMF;       //  Raw Bandpass Filter
  printArray[4] = yHF;       //  Raw High Pass Filter
  printArray[5] = stdLF;     //  StdDev Low Pass Filter
  printArray[6] = stdMF;     //  StdDev Bandpass Filter
  printArray[7] = stdHF;     //  StdDev High Pass Filter
  // printArray[8] = float(alarmCode);

  numValues = 8;  // The number of columns to be sent to the serial monitor (or MATLAB)
  WriteToSerial( numValues, printArray );  //  Write to the serial monitor (or MATLAB)

  if (++loopTick >= NUM_SAMPLES){
    Serial.print("Average execution time (uSec) = ");
    Serial.println( float(execUsec)/NUM_SAMPLES );
    while(true); // spin forever
  }

} // loop()

//**********************************************************************
int AlarmCheck( float stdLF, float stdMF, float stdHF)
{


//  Your alarm check logic code will go here.

  
//return alarmCode;

}  // end AlarmCheck

//**********************************************************************
long EqualizerFIR(long xInput)
{
  int i;
  long yN = 0; //  Current output
  const int equalizerLength = 4;
  static long xN[equalizerLength] = {0};
  long h[] = { 1, 1, -1, -1 };

  for (i = equalizerLength - 1; i >= 1; i--)
  {
    xN[i] = xN[i - 1];
  }

  xN[0] = xInput;

  for ( i = 0; i <= equalizerLength - 1; i++)
  {
    yN += h[i] * xN[i];
  }

  if (loopTick < equalizerLength)
  {
    return 0;
  }
  else
  {
    return yN;
  }
}

//**********************************************************************
float IIR_LPF(float xv)
{
  const int numStages = 3;
  static float G[numStages];
  static float b[numStages][3];
  static float a[numStages][3];
  
  int stage;
  int i;
  static float xM0[numStages] = {0.0}, xM1[numStages] = {0.0}, xM2[numStages] = {0.0};
  static float yM0[numStages] = {0.0}, yM1[numStages] = {0.0}, yM2[numStages] = {0.0};
  
  float yv = 0.0;
  unsigned long startTime;
  
  G[0] = 0.0054630;
  b[0][0] = 1.0000000; b[0][1] = 0.9990472; b[0][2]= 0.0000000;
  a[0][0] = 1.0000000; a[0][1] =  -0.9554256; a[0][2] =  0.0000000;
  G[1] = 0.0054630;
  b[1][0] = 1.0000000; b[1][1] = 2.0015407; b[1][2]= 1.0015416;
  a[1][0] = 1.0000000; a[1][1] =  -1.9217194; a[1][2] =  0.9289864;
  G[2] = 0.0054630;
  b[2][0] = 1.0000000; b[2][1] = 1.9994122; b[2][2]= 0.9994131;
  a[2][0] = 1.0000000; a[2][1] =  -1.9562202; a[2][2] =  0.9723269;
  
  for (i =0; i<numStages; i++)
    {
      yM2[i] = yM1[i]; yM1[i] = yM0[i];  xM2[i] = xM1[i]; xM1[i] = xM0[i], xM0[i] = G[i]*xv;
      yv = -a[i][2]*yM2[i] - a[i][1]*yM1[i] + b[i][2]*xM2[i] + b[i][1]*xM1[i] + b[i][0]*xM0[i];
      yM0[i] = yv;
      xv = yv;
    }
  
  return yv;
}

//**********************************************************************
float IIR_BPF(float xv)
{
  const int numStages = 5;
  static float G[numStages];
  static float b[numStages][3];
  static float a[numStages][3];
  
  int stage;
  int i;
  static float xM0[numStages] = {0.0}, xM1[numStages] = {0.0}, xM2[numStages] = {0.0};
  static float yM0[numStages] = {0.0}, yM1[numStages] = {0.0}, yM2[numStages] = {0.0};
  
  float yv = 0.0;
  unsigned long startTime;

  G[0] = 0.1005883;
  b[0][0] = 1.0000000; b[0][1] = -0.0002347; b[0][2]= -0.9992038;
  a[0][0] = 1.0000000; a[0][1] =  -1.7797494; a[0][2] =  0.8889605;
  G[1] = 0.1005883;
  b[1][0] = 1.0000000; b[1][1] = 2.0008350; b[1][2]= 1.0008352;
  a[1][0] = 1.0000000; a[1][1] =  -1.8483239; a[1][2] =  0.8984289;
  G[2] = 0.1005883;
  b[2][0] = 1.0000000; b[2][1] = 1.9996805; b[2][2]= 0.9996808;
  a[2][0] = 1.0000000; a[2][1] =  -1.9241023; a[2][2] =  0.9473604;
  G[3] = 0.1005883;
  b[3][0] = 1.0000000; b[3][1] = -2.0004549; b[3][2]= 1.0004550;
  a[3][0] = 1.0000000; a[3][1] =  -1.7805039; a[3][2] =  0.9515924;
  G[4] = 0.1005883;
  b[4][0] = 1.0000000; b[4][1] = -1.9998259; b[4][2]= 0.9998260;
  a[4][0] = 1.0000000; a[4][1] =  -1.9696094; a[4][2] =  0.9850368;
  
  for (i =0; i<numStages; i++)
    {
      yM2[i] = yM1[i]; yM1[i] = yM0[i]; xM2[i] = xM1[i]; xM1[i] = xM0[i], xM0[i] = G[i]*xv;
      yv = -a[i][2]*yM2[i] - a[i][1]*yM1[i] + b[i][2]*xM2[i] + b[i][1]*xM1[i] + b[i][0]*xM0[i];
      yM0[i] = yv;
      xv = yv;
    }
  
  return yv;
}

//**********************************************************************
float IIR_HPF(float xv)
{
  const int numStages = 3;
  static float G[numStages];
  static float b[numStages][3];
  static float a[numStages][3];
  
  int stage;
  int i;
  static float xM0[numStages] = {0.0}, xM1[numStages] = {0.0}, xM2[numStages] = {0.0};
  static float yM0[numStages] = {0.0}, yM1[numStages] = {0.0}, yM2[numStages] = {0.0};
  
  float yv = 0.0;
  unsigned long startTime;
  
  G[0] = 0.7527548;
  b[0][0] = 1.0000000; b[0][1] = -1.0012325; b[0][2]= 0.0000000;
  a[0][0] = 1.0000000; a[0][1] =  -0.2605136; a[0][2] =  0.0000000;
  G[1] = 0.7527548;
  b[1][0] = 1.0000000; b[1][1] = -1.9980085; b[1][2]= 0.9980100;
  a[1][0] = 1.0000000; a[1][1] =  -1.3350294; a[1][2] =  0.6145423;
  G[2] = 0.7527548;
  b[2][0] = 1.0000000; b[2][1] = -2.0007590; b[2][2]= 1.0007605;
  a[2][0] = 1.0000000; a[2][1] =  -1.7555162; a[2][2] =  0.9156503;
  
  for (i = 0; i < numStages; i++)
    {
      yM2[i] = yM1[i]; yM1[i] = yM0[i]; xM2[i] = xM1[i]; xM1[i] = xM0[i], xM0[i] = G[i]*xv;
      yv = -a[i][2]*yM2[i] - a[i][1]*yM1[i] + b[i][2]*xM2[i] + b[i][1]*xM1[i] + b[i][0]*xM0[i];
      yM0[i] = yv;
      xv = yv;
    }
  
  return yv;
}

//**********************************************************************
void getStats(float xv, stats_t &s, bool reset)
{
  float oldMean, oldVar;
  
  if (reset == true)
  {
    s.stdev = sqrt(s.var/s.tick);
    s.tick = 1;
    s.mean = xv;
    s.var = 0.0;  
  }
  else
  {
    oldMean = s.mean;
    s.mean = oldMean + (xv - oldMean)/(s.tick+1);
    oldVar = s.var; 
    s.var = oldVar + (xv - oldMean)*(xv - s.mean);      
  }
  s.tick++;  
}

//**********************************************************************
float analogReadDitherAve(void)
{ 
  float sum = 0.0;
  for (int i = 0; i < NUM_SUBSAMPLES; i++)
  {
    digitalWrite(DAC0, (i & B00000001)); // LSB bit mask
    digitalWrite(DAC1, (i & B00000010));
    digitalWrite(DAC2, (i & B00000100)); // MSB bit mask
    sum += analogRead(LM61);
  }
  return sum/NUM_SUBSAMPLES; // averaged subsamples 
}

//**********************************************************************
void setAlarm(int aCode, boolean isToneEn)
{

// Your alarm code goes here

} // setBreathRateAlarm()

//**********************************************************************
float testVector(void)
{
  // Variable rate sinusoidal input
  // Specify segment frequencies in bpm.
  // Test each frequency for nominally 60 seconds.
  // Adjust segment intervals for nearest integer cycle count.
    
  const int NUM_BAND = 6;
  const float CAL_FBPM = 10.0, CAL_AMP = 2.0; 
  
  const float FBPM[NUM_BAND] = {5.0, 10.0, 15.0, 20.0, 30.0, 70.0}; // LPF test
  static float bandAmp[NUM_BAND] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

  //  Determine the number of samples (around 600 ) that will give you an even number
  //  of full cycles of the sinewave.  This is done to avoid a large discontinuity 
  //  between bands.  This forces the sinewave in each band to end near a value of zer
  
  static int bandTick = int(int(FBPM[0]+0.5)*(600/FBPM[0]));
  static int simTick = 0, band = 0;
  static float Fc = FBPM[0]/600, cycleAmp = bandAmp[0];

  //for (int i = 0; i < NUM_BAND; i++) bandAmp[i] = CAL_AMP*(CAL_FBPM/FBPM[i]);  

  //  Check to see if the simulation tick has exceeded the number of tick in each band.
  //  If it has then switch to the next frequency (band) again computing how many
  //  ticks to go through to end up at the end of a cycle.
  
  if ((simTick >= bandTick) && (FBPM[band] > 0.0))
  {

    //  The simTick got to the end of the band cycle.  Go to the next frequency
    simTick = 0;
    band++;
    Fc = FBPM[band]/600.0;
    cycleAmp = bandAmp[band];
    bandTick = int(int(FBPM[band]+0.5)*(600/FBPM[band]));
  }
 
  float degC = 0.0; // DC offset
  degC += cycleAmp*sin(TWO_PI*Fc*simTick++);  
  //degC += 1.0*(tick/100.0); // drift: degC / 10sec
  //degC += 0.1*((random(0,101)-50.0)/29.0); // stdev scaled from 1.0
  return degC;
}

//**********************************************************************
void configureArduino(void)
{
  pinMode(DAC0,OUTPUT); digitalWrite(DAC0,LOW);
  pinMode(DAC1,OUTPUT); digitalWrite(DAC1,LOW);
  pinMode(DAC2,OUTPUT); digitalWrite(DAC2,LOW);

  pinMode(SPKR, OUTPUT); digitalWrite(SPKR,LOW);


  analogReference(INTERNAL); // DEFAULT, INTERNAL
  analogRead(LM61); // read and discard to prime ADC registers
  Serial.begin(115200); // 11 char/msec 
}

//**********************************************************************
void WriteToSerial( int numValues, float dataArray[] )
{

  int index=0; 
  for (index = 0; index < numValues; index++)
  {
    if (index >0)
    {
      Serial.print('\t');
    }
      Serial.print(dataArray[index], DEC);
  }

  Serial.print('\n');
  delay(20);

}  // end WriteToMATLAB

//**********************************************************************
float ReadFromMATLAB()
{
  int charCount;
  bool readComplete = false;
  char inputString[80], inChar;


  // Wait for the serial port

  readComplete = false;
  charCount = 0;
  while ( !readComplete )
  {
    while ( Serial.available() <= 0);
    inChar = Serial.read();

    if ( inChar == '\n' )
    {
      readComplete = true;
    }
    else
    {
      inputString[charCount++] = inChar;
    }
  }
  inputString[charCount] = 0;
  return atof(inputString);

} // end ReadFromMATLAB

//**********************************************************************
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