int nSmpl = 1, sample; // global variables
void setup()
{
 Serial.begin(9600);            // Set baudrate to 9600
 Serial.print("\nsmpl\tADC\n"); // column headers
}
void loop()
{
 sample = analogRead(A0);// Read ADC value on pin A0
 Serial.print(nSmpl);    // Print sample number
 Serial.print('\t');     // Print tab
 Serial.println(sample); // Print sample value
 ++nSmpl;                // Increment sample number
 delay(500);             // Delay of 500 milliseconds
}