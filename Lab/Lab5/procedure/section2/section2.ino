// OPEN NEW ARDUINO SKETCH.
// CLICK IN THIS TEXT BOX. CTRL-A, CTRL-C.
// CLICK IN SKETCH. CTRL-A, CTRL-V.

// file: Roundoff_1.ino
// created by: Clark Hochgraf  Sept 15, 2015
// modified by: David Orlicki  Sept 1, 2017
// purpose: Illustration of roundoff error when using float

const int NUM_CALC = 1000;
float xv = 0.0, A, B, error, scale_factor;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Lab 3 P2 datatypes 190928"));
  Serial.println(F("\nEnter 'g' to go ....."));
  while (Serial.read() != 'g'); // spin until 'g' entry
  
  randomSeed(425);

  scale_factor = 1.0; // start with 1.0
  xv = 1.0;
  Serial.print("Adding random floating point numbers with scale factor ");
  Serial.println(scale_factor); 
  Serial.print("x value starts at x = ");
  Serial.println(xv,10);  // print to 10 decimal places
  for (int i = 1; i < NUM_CALC; i++)
  {
    A = random_float() * scale_factor;
    B = random_float() * scale_factor; 
    xv = xv+A;   // add A to x
    xv = xv+B;   // add B to x
    xv = xv-A;   // subtract A from x
    xv = xv-B;   // subtract B from x
  } // for
  
  error=(1.0-xv);
  
  // end result should just be the original value of x (i.e. 1.0)
  Serial.print("x value finishes at x = "); Serial.println(xv,10); 
  Serial.print("Total percent error after 1000 calculations = ");
  Serial.print(error*100.0,10); Serial.println(" %");
  Serial.print("Average percent error per addition or subtraction = ");
  Serial.print(error*100.0/(4.0*NUM_CALC),10); Serial.println(" %");
  Serial.println(); 
} // setup()

void loop(){} // spin forever

//*********************************************************************
float random_float() { return (random(2147483648)/2147483648.0); }
