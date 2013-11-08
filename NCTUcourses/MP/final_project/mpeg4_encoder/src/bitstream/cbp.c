#include "../portab.h"
#include "cbp.h"

/*****************************************************************************
 * Global function pointer
 ****************************************************************************/

cbpFuncPtr calc_cbp;
single_cbpFuncPtr single_calc_cbp; //by gary

/*****************************************************************************
 * Functions
 ****************************************************************************/

/*
 * Returns a field of bits that indicates non zero ac blocks
 * for this macro block
 */


uint32_t
calc_cbp_c(const int16_t codes[6 * 64])
{
	uint32_t i, j;
	uint32_t cbp = 0;

	for (i = 0; i < 6; i++) 
	{
		for (j = 1; j < 61; j += 4) 
		{
			if (codes[i * 64 + j] | codes[i * 64 + j + 1] |
				codes[i * 64 + j + 2] | codes[i * 64 + j + 3]) 
			{
				cbp |= 1 << (5 - i);
				break;
			}
		}

		if (codes[i * 64 + j] | codes[i * 64 + j + 1] | codes[i * 64 + j + 2])
			cbp |= 1 << (5 - i);

	}

	return cbp;

}


// modify by gary
uint32_t
single_calc_cbp_c(const int16_t codes[64],int32_t cbp,const int32_t i)
{
	uint32_t j;


	for (j = 1; j < 61; j += 4) 
	{
			if (codes[j] | codes[j + 1] | codes[j + 2] | codes[j + 3]) 
			{
				cbp |= 1 << (5 - i);
				break;
			}
	}

	if (codes[j] | codes[j + 1] | codes[j + 2])
	{
		cbp |= 1 << (5 - i);
	}

	return cbp;

}

