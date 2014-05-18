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
	
	// For all 32 bits
	for(uint8_t idx = 0; idx < 32; idx++) {
		// Toggle clock
		digitalWrite(sclk, HIGH);
		// Shift in bit from MISO (MSB first)
		conv.data = (conv.data << 1) | digitalRead(miso);
		// Toggle clock
		digitalWrite(sclk, LOW);
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
	double sensitivityProbe;
	double sensitivityColdJunction;
	switch (type) {
		case 'K': sensitivityProbe = 41.276e-6;
			sensitivityColdJunction = 40.73e-6;
			break;
		case 'J': sensitivityProbe = 57.953e-6;
			sensitivityColdJunction = 40.73e-6;
			break;
		case 'N': sensitivityProbe = 36.256e-6;
			sensitivityColdJunction = 27.171e-6;
			break;
		case 'S': sensitivityProbe = 9.587e-6; 
			sensitivityColdJunction = 6.181e-6;
			break;
		case 'T': sensitivityProbe = 52.18e-6; 
			sensitivityColdJunction = 41.56e-6;
			break;
		case 'E': sensitivityProbe = 76.373e-6;
			sensitivityColdJunction = 44.123e-6;
			break;
		case 'R': sensitivityProbe = 10.506e-6;
			sensitivityColdJunction = 6.158e-6;
			break;
	}

	// Vout = (coeff V/C TC) * Tprobe - (coeff V/C TC - coeff V/C CJ)* Tcoldjunction
	return sensitivityProbe * ProbeTemp() - (sensitivityProbe - sensitivityColdJunction) * ColdJunctionTemp();
}

// Sign extension used for decoding temperature information of arbitrary bit length.
int MAX31855::signExtend(int data, unsigned bits) {
	// Mask only required bits
	data &= (1 << bits) - 1;
	
	// Extend sign bit
	int mask = 1 << (bits - 1);
	return (data ^ mask) - mask;
}
