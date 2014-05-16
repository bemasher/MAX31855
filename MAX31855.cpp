#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include "MAX31855.h"

// Setup pin directions and store pin values.
MAX31855::MAX31855(int8_t SCLK, int8_t CS, int8_t MISO) {
	sclk = SCLK;
	cs = CS;
	miso = MISO;

	pinMode(cs, OUTPUT);
	pinMode(sclk, OUTPUT); 
	pinMode(miso, INPUT);

	digitalWrite(cs, HIGH);
}

// Shifts in and stores a new conversion.
void MAX31855::NewConversion(void) {
	// Enable chip select
	digitalWrite(cs, LOW);

	// Shift in 4 bytes (32 bits)
	for(int idx = 3; idx >= 0; idx--) {
		conv.data[idx] = shiftIn(miso, sclk, MSBFIRST);
	}

	// Disable chip select
	digitalWrite(cs, HIGH);
}

// Returns probe temperature in degrees Celsius for the current conversion.
double MAX31855::ProbeTemp(void) {
	return signExtend(conv.bits.ProbeTemp, 14) * 0.25;
}

// Returns cold junction temperature in degrees Celsius for the current conversion.
double MAX31855::ColdJunctionTemp(void) {
	return signExtend(conv.bits.ColdJunctionTemp, 12) * 0.0625;
}

// Calculate voltage for the given thermocouple type. The constants used are
// baked into each variant of the chip so you will need to make sure you have
// the right MAX31855 for the thermocouple you're using and specify the
// correct type for this function produce correct results.
double MAX31855::Voltage(char type) {
	double voltsPerDegC;
	switch (type) {
		case 'K': voltsPerDecC = 41.276e-6; break;
		case 'J': voltsPerDecC = 57.953e-6; break;
		case 'N': voltsPerDecC = 36.256e-6; break;
		case 'S': voltsPerDecC = 9.587e-6;  break;
		case 'T': voltsPerDecC = 52.18e-6;  break;
		case 'E': voltsPerDecC = 76.373e-6; break;
		case 'R': voltsPerDecC = 10.506e-6; break;
	}

	// Vout = (coeff V/C) * (Tprobe - Tcoldjunction)
	return voltsPerDegC * (ProbeTemp() - ColdJunctionTemp());
}

// Sign extension used for decoding temperature information of arbitrary bit length.
int MAX31855::signExtend(int data, unsigned bits) {
	int mask = 1 << (bits - 1);
	data &= (1 << bits) - 1;
	return (data ^ mask) - mask;
}
