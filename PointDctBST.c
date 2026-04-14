
#include <stdlib.h>


/* ------------------------------------------------------------------------- *
 * Returns the morton code (or z-code) associated to the (x,y) coordinates
 * in argument. The x and y values are expected to be uint32_t integer between
 * 0 and UINT32_MAX.
 * 
 * PARAMETERS
 * x           the x coordinate
 * y           the y coordinate
 *
 * RETURN
 * z           the morton code
 *
 * ------------------------------------------------------------------------- */


static uint64_t zEncode(uint32_t x, uint32_t y) {
	uint64_t out = 0;
	for (uint8_t byte = 0; byte < 4; ++byte) {
		out |= interleave8((x >> (byte * 8)) & 0xFF, (y >> (byte * 8)) & 0xFF) << (byte * 16);
	}
	return out;
}

static uint64_t interleave8(uint8_t m, uint8_t n);

static uint64_t interleave8(uint8_t m, uint8_t n) {
	return (
		((m * 0x0101010101010101ULL & 0x8040201008040201ULL) * 0x0102040810204081ULL >> 49) & 0x5555
	) | (
		((n * 0x0101010101010101ULL & 0x8040201008040201ULL) * 0x0102040810204081ULL >> 48) & 0xAAAA
	);
}