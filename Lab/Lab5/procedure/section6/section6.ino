// OPEN A NEW SKETCH WINDOW IN ARDUINO
// CLICK IN THIS BOX, CTL-A, CTL-C (Copy code)
// CLICK IN SKETCH, CTL-A, CTL-V (Paste code into sketch)

// file: Convolution.ino
// created by: Clark Hochgraf Sept 15, 2015
// modified by: David Orlicki Sept 1, 2017
// purpose: Convolution implementation
// x data stored in float, 
// h impulse response stored in float
// y result stored in float

unsigned long startTime, endTime, execTime; // micros() casting
const int IMP_RESP_LEN = 30;
const int DATA_LEN = 100;

// initialize all arrays to zero
int xv[DATA_LEN]={0};    // convolution input:  x variable
long yv[DATA_LEN]={0};    // convolution output: y variable
long h[IMP_RESP_LEN]={0}; // filter impulse response 

void setup()
{
  Serial.begin(9600);
  Serial.println("x variable\nn\txv[n]");
  for (int i = 0; i < DATA_LEN; i++)
  {
    xv[i] = 5.0;
    Serial.print(i); Serial.print('\t');
    Serial.println(xv[i]);
  }

  Serial.println("\nh impulse response\nn\th[n]");
  for (int i = 0; i < IMP_RESP_LEN; i++)
  {
    h[i] = 100000.0/IMP_RESP_LEN;
    Serial.print(i); Serial.print('\t');
    Serial.println(h[i],3);
  }

  // perform sum of products
  // start convolution only where data is valid
  // first IMP_RESP_LEN-1 datapoints are not valid
  
  startTime = micros();
  for (int k = IMP_RESP_LEN-1; k < DATA_LEN; k++){
    for (int i = 0; i < IMP_RESP_LEN; i++){
      yv[k] = yv[k]+h[i]*xv[k-i];
    } 
  }
  execTime = micros()-startTime;
  
  Serial.println("\nN\txv\tyv");
  for (int i = 0; i < DATA_LEN; i++)
  {
    Serial.print(i); Serial.print('\t');
    Serial.print(xv[i],4); Serial.print('\t');
    // Serial.println(yv[i],4);

    Serial.print("final value of yv = ");
    Serial.println(yv[DATA_LEN-1]/100000.0);
  }

  Serial.println("\nFor given data type:");
  Serial.println("------------------------");
  Serial.print("Data length = ");
  Serial.println(DATA_LEN); 
  Serial.print("final value of yv = ");
  Serial.println(yv[DATA_LEN-1]);
  Serial.print("microseconds for each new datapoint = ");
  Serial.println(execTime/(DATA_LEN-(IMP_RESP_LEN-1)));
  Serial.print("Max update rate (Hz) = ");
  Serial.println(1000000/(execTime/(DATA_LEN-(IMP_RESP_LEN-1))));
}

void loop(){ } // spin forever
