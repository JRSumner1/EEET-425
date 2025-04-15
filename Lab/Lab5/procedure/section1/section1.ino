// OPEN NEW ARDUINO SKETCH.
// CLICK IN THIS TEXT BOX. CTRL-A, CTRL-C.
// CLICK IN SKETCH. CTRL-A, CTRL-V.

// file: Datatypes.ino
// created by: Clark Hochgraf   Sept 15, 2015
// modified by: David Orlicki   Sept 1, 2017
// modified by: Clark Hochgraf  Sept 28, 2019 -- added number arrays
// purpose: Illustration of datatype effect on adding numbers

#define DATATYPE byte // change declaration to byte, int, long, float

DATATYPE av, bv, yv; // uses #define value to change datatype for each variable

// change declaration to byte, int, long, float
DATATYPE avl[16] = {100, 200, 20000, 100, 100, 100, 255, 255, 25500, 20000, 20000, 20000, 20000, 1000000000, 2000000000, 2000000000};
DATATYPE bvl[16] = {100, 200, 20000, 155, 156, 157, 100, 10,  25500, 12767, 12768, 12769, 12770, 1000000000, 147483649,  147483649};

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Lab 3 datatypes190928"));
  Serial.println(F("\nEnter 'g' to go ....."));
  while (Serial.read() != 'g'); // spin until 'g' entry

  Serial.println("Data type is byte"); // byte, int, long, float
  av = 100;
  bv = 100;
  yv = av + bv;
  Serial.println("Add two numbers (100+100)");
  Serial.print("a = "); Serial.print(av); Serial.print("\t"); Serial.print("\t");
  Serial.print("b = "); Serial.print(bv); Serial.print("\t"); Serial.print("\t");
  Serial.print("y = a+b = "); Serial.println(yv);


  av = 200;
  bv = 200;
  yv = av + bv;
  Serial.println("\nAdd two numbers (200+200)");
  Serial.print("a = "); Serial.print(av); Serial.print("\t"); Serial.print("\t");
  Serial.print("b = "); Serial.print(bv); Serial.print("\t"); Serial.print("\t");
  Serial.print("y = a+b = "); Serial.println(yv);


  av = 20000;
  bv = 20000;
  yv = av + bv;
  Serial.println("\nAdd two numbers (20,000+20,000)");
  Serial.print("a = "); Serial.print(av); Serial.print("\t"); Serial.print("\t");
  Serial.print("b = "); Serial.print(bv); Serial.print("\t"); Serial.print("\t");
  Serial.print("y = a+b = "); Serial.println(yv);
}

void loop() {
  Serial.println("\nAdd numbers from array");

  // TODO: Add a for-loop to run through all the 'a' and 'b' values in the
  // arrays 'avl' and 'bvl'. Start at index 0 and run through all 16 values in the array.
  // Inside the for-loop:
  // 1) set av and bv to values picked from 'avl' and 'bvl'
  // 2) perform the addition
  // 3) print the a and b values and the result yv

  // INSERT YOUR CODE HERE




  // END INSERTION OF CODE
  while (true) {}; // spin forever
}
