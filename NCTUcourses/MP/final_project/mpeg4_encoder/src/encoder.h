#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "xvid.h"
#include "portab.h"
#include "global.h"
#include "image/image.h"
#include "utils/ratecontrol.h"

#define TOOSMALL_LIMIT 3		/* skip blocks having a coefficient sum below this value */

/*****************************************************************************
 * Constants
 ****************************************************************************/

/* Quatization type */
#define H263_QUANT	0
#define MPEG4_QUANT	1

/* Indicates no quantizer changes in INTRA_Q/INTER_Q modes */
#define NO_CHANGE 64

/*****************************************************************************
 * Types
 ****************************************************************************/

typedef int bool;

typedef enum
{
	I_VOP = 0,
	P_VOP = 1,
	B_VOP = 2
}
VOP_TYPE;

/*****************************************************************************
 * Structures
 ****************************************************************************/

typedef struct
{
	uint32_t width;
	uint32_t height;

	uint32_t edged_width;
	uint32_t edged_height;
	uint32_t mb_width;
	uint32_t mb_height;

	/* frame rate increment & base */
	uint32_t fincr;
	uint32_t fbase;

	/* rounding type; alternate 0-1 after each interframe */
	/* 1 <= fixed_code <= 4
	   automatically adjusted using motion vector statistics inside
	 */

	/* vars that not "quite" frame independant */
	uint32_t m_quant_type;
	uint32_t m_rounding_type;
	uint32_t m_fcode;

	HINTINFO *hint;

	uint32_t m_seconds;
	uint32_t m_ticks;

}
MBParam;


typedef struct
{
	uint32_t quant;
	uint32_t motion_flags;
	uint32_t global_flags;

	VOP_TYPE coding_type;
	uint32_t rounding_type;
	uint32_t fcode;
	uint32_t bcode;

	uint32_t seconds;
	uint32_t ticks;

	IMAGE image;

	MACROBLOCK *mbs;

}
FRAMEINFO;

typedef struct
{
	int iTextBits;
	float fMvPrevSigma;
	int iMvSum;
	int iMvCount;
	int kblks;
	int mblks;
	int ublks;
}
Statistics;



typedef struct
{
	MBParam mbParam;

	int iFrameNum;
	int iMaxKeyInterval;
	int bitrate;

	/* images */

	FRAMEINFO *current;
	FRAMEINFO *reference;

#ifdef _DEBUG_PSNR
	IMAGE sOriginal;
#endif
	IMAGE vInterH;
	IMAGE vInterV;
	IMAGE vInterHV;

	Statistics sStat;
	RateControl rate_control;
}
Encoder;

/*****************************************************************************
 * Inline functions
 ****************************************************************************/

static __inline uint8_t
get_fcode(uint16_t sr)
{
	if (sr <= 16)
		return 1;

	else if (sr <= 32)
		return 2;

	else if (sr <= 64)
		return 3;

	else if (sr <= 128)
		return 4;

	else if (sr <= 256)
		return 5;

	else if (sr <= 512)
		return 6;

	else if (sr <= 1024)
		return 7;

	else
		return 0;
}


/*****************************************************************************
 * Prototypes
 ****************************************************************************/

void init_encoder(uint32_t cpu_flags);

int encoder_create(XVID_ENC_PARAM * pParam);
int encoder_destroy(Encoder * pEnc);
int encoder_encode(Encoder * pEnc,
				   XVID_ENC_FRAME * pFrame,
				   XVID_ENC_STATS * pResult);

int encoder_encode_bframes(Encoder * pEnc,
				   XVID_ENC_FRAME * pFrame,
				   XVID_ENC_STATS * pResult);

#endif
