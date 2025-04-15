// OPEN NEW ARDUINO SKETCH.
// CLICK IN THIS TEXT BOX. CTRL-A, CTRL-C.
// CLICK IN SKETCH. CTRL-A, CTRL-V.

// Lab7_FIR_Section_03_Base_Code.ino

// created: Clark Hochgraf 2015
// modified: David Orlicki, Sept 14, 2017
// modified: David Orlicki, Oct 25, 2018
// modified: David Orlicki, Oct 25, 2019
// modified: Mark Thompson, 3/21/2021 -- Added MATLAB handshake

// Calculates running mean and standard deviation of samples.

const int NUM_SAMPLES = 1400;
int tick = 0;
float sample, stdev;

//*******************************************************************
void setup()
{ 
  Serial.begin(115200);
  
  // Handshake with MATLAB      
  Serial.println(F("%Arduino Ready"));
  while (Serial.read() != 'g'); // spin
  
} // setup()

//*******************************************************************
void loop()
{ 
  sample = testVector();
  recurseStats(sample, stdev, ((tick%100)==0));
  displayData(); delay(5);
  
  if (++tick >= NUM_SAMPLES) while(true); // spin forever
} // loop()

//*******************************************************************
void recurseStats(float smpl, float &stdevRet, bool reset)
{
  static int statTick = 0;
  static float mean, stdev=0.0, stdevSave;
  static float oldMean, runSumVar, oldRunSumVar;
  
  if (statTick%100 == 0)
  {
    stdevSave = stdev;

    statTick = 0; 
    mean = smpl;
    oldMean = smpl; 
    stdev = 0.0;
    runSumVar = 0.0;
    oldRunSumVar = 0.0;
  }
  else
  {
    mean = oldMean + (smpl - oldMean)/(statTick+1);
    runSumVar = oldRunSumVar + (smpl - oldMean)*(smpl - mean);
    stdev = sqrt(runSumVar/statTick);  
  }
  oldMean = mean; // Set up next recursion
  oldRunSumVar = runSumVar; 
  statTick++;

  stdevRet = stdevSave;
}
//*******************************************************************
void displayData(void)
{
  if (tick == 0) Serial.print("\nn\tsmpl\tstdev\n");
  Serial.print(tick);     Serial.print('\t');
  Serial.print(sample,3); Serial.print('\t');
  Serial.print(stdev,3);  Serial.print('\n');
}
//*******************************************************************
float testVector(void)
{
  // Variable rate periodic input
  // Specify segment amplitude, bpm rate, interval seconds.
  // Intervals trimmed for nearest cycle ending zero crossing.

  const float AMP1 = 5.0, BPM1 = 10.0, TSEC1 = 40.0; 
  const float AMP2 = 1.0, BPM2 = 30.0, TSEC2 = 40.0; 
  const float AMP3 = 3.0, BPM3 = 10.0, TSEC3 = 40.0; 
    
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
  else if (tick == xt1) { // transition to interval 2
    simTick = 0; cycleAmp = AMP2; fracFreq = fracFreq2;
  }
  else if (tick == xt2) { // transition to interval 3
    simTick = 0; cycleAmp = AMP3; fracFreq = fracFreq3;
  }
  
  // Run breathing simulation parameterized by interval
  float degC = 5.0; // DC offset
  degC += 0.0*tick/100.0; // drift
  degC += cycleAmp*sin((fracFreq*simTick++)*TWO_PI);  
  degC += 0.0*((random(0,11)/10.0-0.5)/0.29); 
  return degC;
}
