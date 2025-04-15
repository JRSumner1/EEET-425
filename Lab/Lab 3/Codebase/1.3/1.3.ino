// AnalogReadSerial: public domain code
// Reads analog input on pin 0, prints to console.
void setup() // runs once when you press reset
{
  Serial.begin(9600);
}

void loop()
{
  int sensorValue = analogRead(A0);
  Serial.println(sensorValue);
  delay(1);
}
