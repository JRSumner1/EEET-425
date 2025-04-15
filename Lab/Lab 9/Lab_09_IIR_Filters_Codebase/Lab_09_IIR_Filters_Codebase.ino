// OPEN NEW ARDUINO SKETCH.
// CLICK IN THIS TEXT BOX. USE Ctrl-A, Ctrl-C TO COPY ALL.
// CLICK IN SKETCH. USE Ctrl-A, CTRL-V TO PASTE. SAVE. RUN.

//  Modified 3/9/2020 by Mark Thompson to recursively operate over the stages
//      of the SOS filter.  Comments added

// Exercise_3_IIR_Filters_2195

// Variable Declarations

// TSAMP_MSEC -- Sample time for the sample interrupt
// DATA_LEN -- Number of samples of data to collect or compute

const int TSAMP_MSEC = 10;
const unsigned int DATA_LEN = 512; // 512 1200;

//  xv -- Input samples to the filter
//  yv -- Output samples from the filter
//  tick -- Sample counter
//  statRest -- Flag to reset the resettable stats computations
//  stat_t --  Structure to hold the output stats computations
//  mean -- running mean of the statistics
//  var -- running variance of the statistics
//  stdev -- running standard devation of the statistics
//  statsX -- Holds the stats for X
//  statsY -- Holds the stats for Y

float xv, yv, stdX, stdY;
int tick = 0;
bool statsReset;
struct stats_t
{
  int tick = 1;
  float mean, var, stdev;
} statsX, statsY;

//  execUsec -- holds the accumulated execution time value
unsigned long execUsec;

//*********************************************************************
void setup()
{
  Serial.begin(115200); delay(20);
  
 //Handshake with MATLAB 
  Serial.println(F("%Arduino Ready"));
  while (Serial.read() != 'g'); // spin   
}

//*********************************************************************
void loop()
{

  //  Select the input to the system.  Impulse function or from the test
  //  vector generator.

  // Impulse function input
  //  Create a Delta function in time with the first sample a 1 and all others 0
  xv = (tick == 0) ? 1.0 : 0.0; // impulse test input



  //  Create an input from the test vector generator
  //  xv = testVector();


  //  Compute the output of the filter with the direct form of the IIR filter
  yv = IIR_DIRECT(xv); // monolithic loop architecture

  //  Compute the output of the filter using the cascaded SOS sections
  //yv = IIR_SOS(xv); // second order systems cascade  


  //  Set the statsReset flag if the tick counter is at multiples of 100
  statsReset = (tick%100 == 0);

  //  Compute the latest output of the running stats for the output of the filters.
  //  Pass the entire set of output values, the latest stats structure and the reset flag
  getStats(yv, statsY, statsReset); stdY = statsY.stdev;


 //  Post it up on the console
  displayData();
  delay(20); // clear comm buffer: 11 char/msec @ 115200 baud


  //  If we've collected enough samples, just hang out
  if (++tick >= DATA_LEN) 
    {
//      Serial.print("\nExecution Time Per Point (uSec) -- \t");
//      Serial.print(execUsec/DATA_LEN);
    
      while(true); // spin
    }
  
} // loop()

//*******************************************************************************
float IIR_DIRECT(float xv)
{  

  //  Data to define the filter coefficients for the direct form IIR filter
  
  //BWRTH low, order 5, 12 BPM
  const int MFILT = 6;
  static float GAIN = 8.04236e-06;
  static float b[] = {0.1000000, 0.5000000, 1.0000000, 1.0000000, 0.5000000, 0.1000000};
  static float a[] = {1.0000000, -4.5934214, 8.4551152, -7.7949183, 3.5989028, -0.6656525};
  
  //---------------------------------------------------------------
  //  Two arrays to contain the input and output sequences in time
  static float xM[MFILT] = {0.0}, yM[MFILT] = {0.0}; 


 //  Shift the input and output values in time by one sample, then bring in the next sample 
  for (int i = MFILT-1; i > 0; i--) // shift x, y histories
  {
    yM[i] = yM[i-1];
    xM[i] = xM[i-1];
  } 
  xM[0] = GAIN*xv; // insert new input
  yM[0] = 0.0;     // init output accumulator

  //  Execute the IIR filter by multiplying each output historical value by the a ( the numerator) coefficients
  //  and each new input value by the b (the denominator) coefficients.
  
  for (int i = MFILT-1; i > 0; i--) yM[0] += ( -a[i]*yM[i] + b[i]*xM[i]);

  // Update the output with the newest value xM[0] times b[0]
  return (yM[0] += b[0]*xM[0]);
}
//*******************************************************************************
float IIR_SOS(float xv)
{  


  //  ***  Copy variable declarations from MATLAB generator to here  ****

  const int numStages = 3;
  static float G[numStages];
  static float b[numStages][3];
  static float a[numStages][3];

//  *** Stop copying MATLAB variable declarations here
  
  int stage;
  int i;
  static float xM0[numStages] = {0.0}, xM1[numStages] = {0.0}, xM2[numStages] = {0.0};
  static float yM0[numStages] = {0.0}, yM1[numStages] = {0.0}, yM2[numStages] = {0.0};
  
  float yv = 0.0;
  unsigned long startTime;



//  ***  Copy variable initialization code from MATLAB generator to here  ****

// CHEBY HIGH, order 5, R = 1.0, 25 BPM
G[0] = 0.8230265;
b[0][0] = 1.0000000; b[0][1] = -0.9990472; b[0][2]= 0.0000000;
a[0][0] = 1.0000000; a[0][1] =  -0.3747890; a[0][2] =  0.0000000;
G[1] = 0.8230265;
b[1][0] = 1.0000000; b[1][1] = -2.0015407; b[1][2]= 1.0015416;
a[1][0] = 1.0000000; a[1][1] =  -1.6209618; a[1][2] =  0.7573573;
G[2] = 0.8230265;
b[2][0] = 1.0000000; b[2][1] = -1.9994122; b[2][2]= 0.9994131;
a[2][0] = 1.0000000; a[2][1] =  -1.8868646; a[2][2] =  0.9542270;

//  **** Stop copying MATLAB code here  ****

  //  Initialize the timer for each point computed
  startTime = micros();


  //  Iterate over each second order stage.  For each stage shift the input data
  //  buffer ( x[kk] ) by one and the output data buffer by one ( y[k] ).  Then bring in 
  //  a new sample xv into the buffer;
  //
  //  Then execute the recusive filter on the buffer
  //
  //  y[k] = -a[2]*y[k-2] + -a[1]*y[k-1] + g*b[0]*x[k] + b[1]*x[k-1] + b[2]*x[k-2] 
  //
  //  Pass the output from this stage to the next stage by setting the input
  //  variable to the next stage x to the output of the current stage y
  //  
  //  Repeat this for each second order stage of the filter

  
  for (i =0; i<numStages; i++)
    {
      yM2[i] = yM1[i]; yM1[i] = yM0[i];  xM2[i] = xM1[i]; xM1[i] = xM0[i], xM0[i] = G[i]*xv;
      yv = -a[i][2]*yM2[i] - a[i][1]*yM1[i] + b[i][2]*xM2[i] + b[i][1]*xM1[i] + b[i][0]*xM0[i];
      yM0[i] = yv;
      xv = yv;
    }

  execUsec += micros()-startTime;
  
  return yv;
}
//*******************************************************************
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
//*************************************************************
float testVector(void)
{
  // Variable rate sinusoidal input
  // Specify segment frequencies in bpm.
  // Test each frequency for nominally 60 seconds.
  // Adjust segment intervals for nearest integer cycle count.
    
  const int NUM_BAND = 6;
  const float CAL_FBPM = 10.0, CAL_AMP = 2.0; 
  
  const float FBPM[NUM_BAND] = {10.0, 30.0, 0.0, 0.0, 0.0, 0.0}; // LPF test
  static float bandAmp[NUM_BAND] = {2.0, 2.1, 0.0, 0.0, 0.0, 0.0};

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
//*********************************************************************
void displayData()
{
  const int Nsig = 6; // 6 for impulse resp, 3 for temporal tests
  if (tick == 0)
  {
    Serial.print("\nsmpl\txv\tyv\n");
    //Serial.print("\nsmpl\txv\tyv\tstdY\n");
  }
  Serial.print(tick);   Serial.print('\t');
  Serial.print(xv);     Serial.print('\t');
  Serial.print(yv,6);   Serial.print('\n'); // 6: impulse resp, 2: sine tests
  //Serial.print(stdY);   Serial.print('\n');   
}
