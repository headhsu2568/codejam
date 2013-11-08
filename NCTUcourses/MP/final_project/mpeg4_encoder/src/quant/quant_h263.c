#include "quant_h263.h"

#include "../bitstream/zigzag.h"

/*	mutliply+shift division table
*/

#define SCALEBITS	16
#define FIX(X)		((1L << SCALEBITS) / (X) + 1)

static const uint32_t multipliers[32] = {
	0, FIX(2), FIX(4), FIX(6),
	FIX(8), FIX(10), FIX(12), FIX(14),
	FIX(16), FIX(18), FIX(20), FIX(22),
	FIX(24), FIX(26), FIX(28), FIX(30),
	FIX(32), FIX(34), FIX(36), FIX(38),
	FIX(40), FIX(42), FIX(44), FIX(46),
	FIX(48), FIX(50), FIX(52), FIX(54),
	FIX(56), FIX(58), FIX(60), FIX(62)
};

// Optimization. Look up the table method. by gary
#define FIX_DC(X) ((1L << SCALEBITS) / (X))

static const uint32_t dc_multipliers[47] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	FIX_DC(8), FIX_DC(9), FIX_DC(10), FIX_DC(11), FIX_DC(12), FIX_DC(13), FIX_DC(14), FIX_DC(15), 
	FIX_DC(16), FIX_DC(17), FIX_DC(18), FIX_DC(19), FIX_DC(20), FIX_DC(21), FIX_DC(22), FIX_DC(23),
	FIX_DC(24), FIX_DC(25), FIX_DC(26), FIX_DC(27), FIX_DC(28), FIX_DC(29), FIX_DC(30), FIX_DC(31),
	FIX_DC(32), FIX_DC(33), FIX_DC(34), FIX_DC(35), FIX_DC(36), FIX_DC(37), FIX_DC(38), FIX_DC(39),
	FIX_DC(40), FIX_DC(41), FIX_DC(42), FIX_DC(43), FIX_DC(44), FIX_DC(45), FIX_DC(46)
};



#define DIV_DIV(a, b) ((a)>0) ? ((a)+((b)>>1))/(b) : ((a)-((b)>>1))/(b)

/* function pointers */
quanth263_intraFuncPtr quant_intra;
dquanth263_intraFuncPtr dequant_intra;

quanth263_interFuncPtr quant_inter;
dequanth263_interFuncPtr dequant_inter;



/*	quantize intra-block
*/


void
quant_intra_c(int16_t * coeff,
			  int8_t * last_address,
			  uint8_t * DCT_flag,
			  const int16_t * data,
			  const uint32_t quant,
			  const uint32_t dcscalar)
{
	const uint32_t mult = multipliers[quant];
	const uint16_t quant_m_2 = (uint16_t)(quant << 1);
	uint32_t i;

    *last_address = 0;  //last non-zero method .by gary
	*DCT_flag = 0; //zero skip method .by gary

	// Optimization. Look up the table method. by gary
	coeff[0] = (int16_t)((data[0] * dc_multipliers[dcscalar]) >> SCALEBITS);

    if(coeff[0] != 0)
	{
        *DCT_flag = *DCT_flag | (1);  //zero skip method .by gary
	}

	for (i = 1; i < 64; i++) {
		int16_t acLevel = data[i];

		if (acLevel < 0) {
			acLevel = -acLevel;
			if (acLevel < quant_m_2) {
				coeff[i] = 0;
				continue;
			}
			acLevel = (int16_t)(((uint32_t)acLevel * mult) >> SCALEBITS);
			coeff[i] = -acLevel;
		} else {
			if (acLevel < quant_m_2) {
				coeff[i] = 0;
				continue;
			}
			acLevel = (int16_t)(((uint32_t)acLevel * mult) >> SCALEBITS);
			coeff[i] = acLevel;
		}

		if(coeff[i] != 0)
		{
			*last_address = i;  //last non-zero method .by gary
			*DCT_flag = *DCT_flag | (1<< (i>>3));  //zero skip method .by gary
		}
	}
}


/*	quantize inter-block
*/

uint32_t
quant_inter_c(int16_t * coeff,
			  int8_t * last_address,
			  uint8_t * DCT_flag,
			  const int16_t * data,
			  const uint32_t quant)
{
	const uint32_t mult = multipliers[quant];
	const uint16_t quant_m_2 = (uint16_t)(quant << 1);
	const uint16_t quant_d_2 = (uint16_t)(quant >> 1);
	int sum = 0;
	uint32_t i;
	int16_t acLevel;
	int8_t address;     //last non-zero method .by gary

    *DCT_flag = 0; //zero skip method .by gary

	*last_address = 0;  //last non-zero method .by gary

	for (i = 0; i < 64; i++) {
		address = (char)scan_tables[0][i];        //last non-zero method .by gary
		acLevel = data[address];    //last non-zero method .by gary

		if (acLevel < 0) {
			acLevel = (-acLevel) - quant_d_2;
			if (acLevel < quant_m_2) {
				coeff[address] = 0;
				continue;
			}

			acLevel = (int16_t)(((uint32_t)acLevel * mult) >> SCALEBITS);
			sum += acLevel;		/* sum += |acLevel| */
			coeff[address] = -acLevel;
		} else {
			acLevel -= quant_d_2;
			if (acLevel < quant_m_2) {
				coeff[address] = 0;
				continue;
			}
			acLevel = (int16_t)(((uint32_t)acLevel * mult) >> SCALEBITS);
			sum += acLevel;
			coeff[address] = acLevel;
		}

		if(coeff[address] != 0)
		{
			*last_address = i;  //last non-zero method .by gary
			*DCT_flag = *DCT_flag | (1<< (address>>3));  //zero skip method .by gary
		}
	}

	return sum;
}


/*	dequantize intra-block & clamp to [-2048,2047]
*/

void
dequant_intra_c(int16_t * data,
				int8_t last_address,
				const int16_t * coeff,
				const uint32_t quant,
				const uint32_t dcscalar)
{
	const int32_t quant_m_2 = quant << 1;
	const int32_t quant_add = (quant & 1 ? quant : quant - 1);
	int32_t i;
	int32_t acLevel;

	int8_t address;     //last non-zero method .by gary

	data[0] = (int16_t)(coeff[0] * dcscalar);
	if (data[0] < -2048) {
		data[0] = -2048;
	} else if (data[0] > 2047) {
		data[0] = 2047;
	}


	for (i = 1; i < last_address+1; i++) {
		address = (char)scan_tables[0][i];        //last non-zero method .by gary
		acLevel = coeff[address];

		if (acLevel == 0) {
			data[address] = 0;
		} else if (acLevel < 0) {
			acLevel = quant_m_2 * -acLevel + quant_add;
			data[address] = (acLevel <= 2048 ? -acLevel : -2048);
		} else					/*  if (acLevel > 0) { */
		{
			acLevel = quant_m_2 * acLevel + quant_add;
			data[address] = (acLevel <= 2047 ? acLevel : 2047);
		}
	}

	//last non-zero method .by gary
	for(i = last_address+1; i < 64 ;i++)
	{   
		data[scan_tables[0][i]] = 0;
	}
}



/* dequantize inter-block & clamp to [-2048,2047]
*/

void
dequant_inter_c(int16_t * data,
				int8_t last_address,
				const int16_t * coeff,
				const uint32_t quant)
{
	const uint16_t quant_m_2 = (uint16_t)(quant << 1);
	const uint16_t quant_add = (uint16_t)(quant & 1 ? quant : quant - 1);
	int32_t i;
	int16_t acLevel;
	int8_t address;     //last non-zero method .by gary

	for (i = 0; i < last_address+1; i++) {
		address = (char)scan_tables[0][i];        //last non-zero method .by gary
		acLevel = coeff[address];    //last non-zero method .by gary

		if (acLevel == 0) {
			data[address] = 0;
		} else if (acLevel < 0) {
			acLevel = acLevel * quant_m_2 - quant_add;
			data[address] = (acLevel >= -2048 ? acLevel : -2048);
		} else					/* if (acLevel > 0) */
		{
			acLevel = acLevel * quant_m_2 + quant_add;
			data[address] = (acLevel <= 2047 ? acLevel : 2047);
		}
	}

	//last non-zero method .by gary
	for(i = last_address+1; i < 64 ;i++)
	{   
		data[scan_tables[0][i]] = 0;
	}
}
