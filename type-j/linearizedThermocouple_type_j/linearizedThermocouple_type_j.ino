#include <SPI.h>
#include "Adafruit_MAX31855.h"

#define DO   12
#define CS   11
#define CLK  10
Adafruit_MAX31855 thermocouple(CLK, CS, DO);

void setup() {
  Serial.begin(9600);
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
}
void loop() {
  // Initialize variables.
  int i = 0; // Counter for arrays
  double internalTemp = thermocouple.readInternal(); // Read the internal temperature of the MAX31855.
  double rawTemp = thermocouple.readCelsius(); // Read the temperature of the thermocouple. This temp is compensated for cold junction temperature.
  double thermocoupleVoltage = 0;
  double internalVoltage = 0;
  double correctedTemp = 0;

  // Check to make sure thermocouple is working correctly.
  if (isnan(rawTemp)) {
    Serial.println("Something wrong with thermocouple!");
  }
  else {
    // Steps 1 & 2. Subtract cold junction temperature from the raw thermocouple temperature.
    thermocoupleVoltage = (rawTemp - internalTemp) * 0.057953; // C * mv/C = mV

    // Step 3. Calculate the cold junction equivalent thermocouple voltage.

    if (internalTemp >= -210.00 && internalTemp < 760.00) { //for temperatures between -210C to +760C use appropriate NIST coefficients
      // Coefficients and equations available from http://srdata.nist.gov/its90/download/type_j.tab

      double c[] = {0.000000000000E+00, 0.503811878150E-01, 0.304758369300E-04, -0.856810657200E-07, 0.132281952950E-09, -0.170529583370E-12, 0.209480906970E-15, -0.125383953360E-18, 0.156317256970E-22};

      // Count the the number of coefficients.
      int cLength = sizeof(c) / sizeof(c[0]);


      // From NIST: E = sum(i=0 to n) c_i t^i.
      // This loop sums up the c_i t^i components.
      for (i = 0; i < cLength; i++) {
        internalVoltage += c[i] * pow(internalTemp, i);
      }
    }
    else if (internalTemp <= 1200.00) { // for temperatures between 760C and 1200C.
      double c[] = {0.296456256810E+03, -0.149761277860E+01, 0.317871039240E-02, -0.318476867010E-05, 0.157208190040E-08, -0.306913690560E-12};

      // Count the number of coefficients.
      int cLength = sizeof(c) / sizeof(c[0]);

      // Below 0 degrees Celsius, the NIST formula is simpler and has no exponential components: E = sum(i=0 to n) c_i t^i
      for (i = 0; i < cLength; i++) {
        internalVoltage += c[i] * pow(internalTemp, i) ;
      }
    } else { // NIST only has data for J-type thermocouples from -210C to +1200C. If the temperature is not in that range, set temp to impossible value.
      // Error handling should be improved.
      Serial.print("Temperature is out of range. This should never happen.");
      internalVoltage = NAN;
    }

    // Step 4. Add the cold junction equivalent thermocouple voltage calculated in step 3 to the thermocouple voltage calculated in step 2.
    double totalVoltage = thermocoupleVoltage + internalVoltage;

    // Step 5. Use the result of step 4 and the NIST voltage-to-temperature (inverse) coefficients to calculate the cold junction compensated, linearized temperature value.
    // The equation is in the form correctedTemp = d_0 + d_1*E + d_2*E^2 + ... + d_n*E^n, where E is the totalVoltage in mV and correctedTemp is in degrees C.
    // NIST uses different coefficients for different temperature subranges: (-210 to 0C), (0 to 760C) and (760 to 1200C).
    if (totalVoltage < 0) { // Temperature is between -210C and 0C.
      double d[] = {0.0000000E+00, 1.9528268E+01, -1.2286185E+00, -1.0752178E+00, -5.9086933E-01, -1.7256713E-01, -2.8131513E-02, -2.3963370E-03, -8.3823321E-05};

      int dLength = sizeof(d) / sizeof(d[0]);
      for (i = 0; i < dLength; i++) {
        correctedTemp += d[i] * pow(totalVoltage, i);
      }
    }

    else if (totalVoltage < 42.919) { // Temperature is between 0C and 760C.
      double d[] = {0.000000E+00, 1.978425E+01, -2.001204E-01, 1.036969E-02, -2.549687E-04, 3.585153E-06, -5.344285E-08, 5.099890E-10, 0.000000E+00};
      int dLength = sizeof(d) / sizeof(d[0]);
      for (i = 0; i < dLength; i++) {
        correctedTemp += d[i] * pow(totalVoltage, i);
      }
    }
    else if (totalVoltage < 69.553 ) { // Temperature is between 500C and 1372C.
      double d[] = { -3.11358187E+03, 3.00543684E+02, -9.94773230E+00, 1.70276630E-01, -1.43033468E-03, 4.73886084E-06, 0.00000000E+00, 0.00000000E+00, 0.00000000E+00};
      int dLength = sizeof(d) / sizeof(d[0]);
      for (i = 0; i < dLength; i++) {
        correctedTemp += d[i] * pow(totalVoltage, i);
      }
    } else { // NIST only has data for J-type thermocouples from -210C to +1200C. If the temperature is not in that range, set temp to impossible value.
      // Error handling should be improved.
      Serial.print("Temperature is out of range. This should never happen.");
      correctedTemp = NAN;
    }

    Serial.print("Corrected Temp = ");
    Serial.println(correctedTemp, 5);
    Serial.print("Raw Temp = ");
    Serial.println(rawTemp, 5);
    Serial.print("Internal temp = ")
    Serial.println(internalTemp, 5);
    Serial.println("");

  }

  delay(1000);

}
