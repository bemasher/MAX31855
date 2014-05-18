#if (ARDUINO >= 100)
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

struct ConversionBits {
	unsigned short OpenCircuit      :1;
	unsigned short GNDShort         :1;
	unsigned short VCCShort         :1;
	unsigned short                  :1;
	uint16_t ColdJunctionTemp       :12;
	unsigned short Fault            :1;
	unsigned short                  :1;
	uint16_t ProbeTemp              :14;
};

union Conversion {
	uint32_t data;
	struct ConversionBits bits;
};

class MAX31855 {
	public:
		MAX31855(int8_t SCLK, int8_t CS, int8_t MISO);

		union Conversion conv;

		void NewConversion(void);

		double ProbeTemp(void);
		double ColdJunctionTemp(void);
		double Voltage(char type);

	private:
		int8_t sclk, cs, miso;

		int signExtend(int data, unsigned bits);
};
