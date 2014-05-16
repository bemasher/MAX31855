#include <MAX31855.h>

int thermoDO = 12;
int thermoCS = 10;
int thermoCLK = 13;

MAX31855 thermocouple(thermoCLK, thermoCS, thermoDO);

void setup() {
  Serial.begin(9600);
}

void loop() {
   thermocouple.NewConversion();
   
   Serial.print("Fault: ");
   Serial.println(thermocouple.conv.bits.Fault);
   
   Serial.print("Probe: ");
   Serial.println(thermocouple.ProbeTemp(), 2);
   
   Serial.print("Cold Junction: ");
   Serial.print(thermocouple.ColdJunctionTemp(), 2);
   
   Serial.println();
}
