#ifndef SPIFFSCHECKSUM_HPP
#define SPIFFSCHECKSUM_HPP

struct {
	const char* name;
	uint32_t sum;
} static const SPIFFSChecksums[] = {
	{ "/spiffs/Disconnect.aac", 2191105},
	{ "/spiffs/Gasenje.aac", 2160857},
	{ "/spiffs/GasenjeBatt.aac", 2948739},
	{ "/spiffs/Intro2.aac", 3347904},
	{ "/spiffs/PairCancel.aac", 2188221},
	{ "/spiffs/PairFail.aac", 2162664},
	{ "/spiffs/PairStart.aac", 2197151},
	{ "/spiffs/PairSuccess.aac", 2200685},
};

#endif //SPIFFSCHECKSUM_HPP
