#include "../encoder.h"
#include "../prediction/mbprediction.h"
#include "../global.h"
#include "motion.h"
#include "sad.h"

static int32_t lambda_vec16[32] =	/* rounded values for lambda param for weight of motion bits as in modified H.26L */
{ 0, (int) (1.00235 + 0.5), (int) (1.15582 + 0.5), (int) (1.31976 + 0.5),
		(int) (1.49591 + 0.5), (int) (1.68601 + 0.5),
	(int) (1.89187 + 0.5), (int) (2.11542 + 0.5), (int) (2.35878 + 0.5),
		(int) (2.62429 + 0.5), (int) (2.91455 + 0.5),
	(int) (3.23253 + 0.5), (int) (3.58158 + 0.5), (int) (3.96555 + 0.5),
		(int) (4.38887 + 0.5), (int) (4.85673 + 0.5),
	(int) (5.37519 + 0.5), (int) (5.95144 + 0.5), (int) (6.59408 + 0.5),
		(int) (7.31349 + 0.5), (int) (8.12242 + 0.5),
	(int) (9.03669 + 0.5), (int) (10.0763 + 0.5), (int) (11.2669 + 0.5),
		(int) (12.6426 + 0.5), (int) (14.2493 + 0.5),
	(int) (16.1512 + 0.5), (int) (18.442 + 0.5), (int) (21.2656 + 0.5),
		(int) (24.8580 + 0.5), (int) (29.6436 + 0.5),
	(int) (36.4949 + 0.5)
};

static int32_t *lambda_vec8 = lambda_vec16;	/* same table for INTER and INTER4V for now */



/* mv.length table */
static const uint32_t mvtab[33] = {
	1, 2, 3, 4, 6, 7, 7, 7,
	9, 9, 9, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10,
	10, 11, 11, 11, 11, 11, 11, 12, 12
};


static __inline uint32_t
mv_bits(int32_t component,
		const uint32_t iFcode)
{
	if (component == 0)
		return 1;

	if (component < 0)
		component = -component;

	if (iFcode == 1) {
		if (component > 32)
			component = 32;

		return mvtab[component] + 1;
	}

	component += (1 << (iFcode - 1)) - 1;
	component >>= (iFcode - 1);

	if (component > 32)
		component = 32;

	return mvtab[component] + 1 + iFcode - 1;
}


static __inline uint32_t
calc_delta_16(const int32_t dx,
			  const int32_t dy,
			  const uint32_t iFcode,
			  const uint32_t iQuant)
{
	return NEIGH_TEND_16X16 * lambda_vec16[iQuant] * (mv_bits(dx, iFcode) +
													  mv_bits(dy, iFcode));
}

static __inline uint32_t
calc_delta_8(const int32_t dx,
			 const int32_t dy,
			 const uint32_t iFcode,
			 const uint32_t iQuant)
{
	return NEIGH_TEND_8X8 * lambda_vec8[iQuant] * (mv_bits(dx, iFcode) +
												   mv_bits(dy, iFcode));
}
 

/****************************************************************************
 **    Parallel code by gary
 ****************************************************************************/

int
MotionEstimationMB(MBParam * const pParam,
				   FRAMEINFO * const current,
				   FRAMEINFO * const reference,
				   int32_t x,
				   int32_t y)
{
	const uint32_t iWcount = pParam->mb_width;
	const uint32_t iHcount = pParam->mb_height;
	MACROBLOCK *const pMBs = current->mbs;
	MACROBLOCK *const prevMBs = reference->mbs;
	const IMAGE *const pCurrent = &current->image;
	const IMAGE *const pRef = &reference->image;

	static const VECTOR zeroMV = { 0, 0 };
	VECTOR predMV;

	VECTOR pmv;

	MACROBLOCK *pMB;

	if (sadInit)
		(*sadInit) ();

	pMB = &pMBs[x + y * iWcount];

	predMV = get_pmv2(pMBs, pParam->mb_width, 0, x, y, 0);
    
	pMB->sad16 = SEARCH16_INT(pRef->y, pCurrent, 
						      x, y, predMV.x, predMV.y, predMV.x, predMV.y, 
						      current->motion_flags, current->quant,
						      current->fcode, pParam, pMBs, prevMBs, &pMB->mv16,
						      &pMB->pmvs[0]); 

    if (0 < (pMB->sad16 - MV16_INTER_BIAS)) {
			int32_t deviation;

			deviation =
				dev16(pCurrent->y + x * 16 + y * 16 * pParam->edged_width,
						  pParam->edged_width);

			if (deviation < (pMB->sad16 - MV16_INTER_BIAS)) {
				pMB->mode = MODE_INTRA;
				pMB->mv16 = pMB->mvs[0] = pMB->mvs[1] = pMB->mvs[2] =
					pMB->mvs[3] = zeroMV;
				pMB->sad16 = pMB->sad8[0] = pMB->sad8[1] = pMB->sad8[2] =
					pMB->sad8[3] = 0;

				return ME_INTRA;  // intra coding,
			}
	}

	pmv = pMB->pmvs[0];

	if (current->global_flags & XVID_INTER4V) {
		if ((!(current->global_flags & XVID_LUMIMASKING) || pMB->dquant == NO_CHANGE)) {
				
			int32_t sad8 = IMV16X16 * current->quant;

			if (sad8 < pMB->sad16) {
				sad8 += pMB->sad8[0] =
							SEARCH8_INT(pRef->y,
									    pCurrent, 2 * x, 2 * y, 
									    pMB->mv16.x, pMB->mv16.y, predMV.x, predMV.y, 
									    current->motion_flags,
								    	current->quant, current->fcode, pParam,
									    pMBs, prevMBs, &pMB->mvs[0],
								    	&pMB->pmvs[0]);
			}

			if (sad8 < pMB->sad16) {
				predMV = get_pmv2(pMBs, pParam->mb_width, 0, x, y, 1);
				
				sad8 += pMB->sad8[1] =
							SEARCH8_INT(pRef->y,
									    pCurrent, 2 * x + 1, 2 * y,
									    pMB->mv16.x, pMB->mv16.y, predMV.x, predMV.y, 
								    	current->motion_flags,
									    current->quant, current->fcode, pParam,
								    	pMBs, prevMBs, &pMB->mvs[1],
								    	&pMB->pmvs[1]);
			}

			if (sad8 < pMB->sad16) {
				predMV = get_pmv2(pMBs, pParam->mb_width, 0, x, y, 2); 
				
				sad8 += pMB->sad8[2] =
							SEARCH8_INT(pRef->y,
									    pCurrent, 2 * x, 2 * y + 1, 
									    pMB->mv16.x, pMB->mv16.y, predMV.x, predMV.y, 
								    	current->motion_flags,
								        current->quant, current->fcode, pParam,
									    pMBs, prevMBs, &pMB->mvs[2],
								    	&pMB->pmvs[2]);
			}

			if (sad8 < pMB->sad16) {
				predMV = get_pmv2(pMBs, pParam->mb_width, 0, x, y, 3);  

				sad8 += pMB->sad8[3] =
							SEARCH8_INT(pRef->y,
									    pCurrent, 2 * x + 1, 2 * y + 1,
								    	pMB->mv16.x, pMB->mv16.y, predMV.x, predMV.y, 
								    	current->motion_flags, 
								    	current->quant, current->fcode, pParam, 
								    	pMBs, prevMBs,
								    	&pMB->mvs[3], 
								    	&pMB->pmvs[3]);
			}
					
			// decide: MODE_INTER or MODE_INTER4V
			//  mpeg4:   if (sad8 < pMB->sad16 - nb/2+1) use_inter4v
			//

			if (sad8 < pMB->sad16) {
				pMB->mode = MODE_INTER4V;
				pMB->sad8[0] *= 4;
				pMB->sad8[1] *= 4;
				pMB->sad8[2] *= 4;
				pMB->sad8[3] *= 4;
				return ME_INTPXL;
			}

		}
	}

	pMB->mode = MODE_INTER;
	pMB->pmvs[0] = pmv;	// pMB->pmvs[1] = pMB->pmvs[2] = pMB->pmvs[3]  are not needed for INTER 
	pMB->mvs[0] = pMB->mvs[1] = pMB->mvs[2] = pMB->mvs[3] = pMB->mv16;
	pMB->sad8[0] = pMB->sad8[1] = pMB->sad8[2] = pMB->sad8[3] =
	pMB->sad16;

	return ME_HALFPXL;
}

// Parallel code. by gary
#define CHECK_MV16_ZERO_INT {\
  if ( (0 <= max_dx) && (0 >= min_dx) \
    && (0 <= max_dy) && (0 >= min_dy) ) \
  { \
    iSAD = sad16( cur, get_ref_INT(pRef, x, y, 16, 0, 0 , iEdgedWidth), iEdgedWidth, MV_MAX_ERROR); \
    iSAD += calc_delta_16(-center_x, -center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=0; currMV->y=0; }  }	\
}

//Parallel code. by gary
#define NOCHECK_MV16_CANDIDATE_INT(X,Y) { \
    iSAD = sad16( cur, get_ref_INT(pRef, x, y, 16, X, Y, iEdgedWidth),iEdgedWidth, iMinSAD); \
    iSAD += calc_delta_16((X) - center_x, (Y) - center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); } \
}


#define CHECK_MV16_CANDIDATE(X,Y) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad16( cur, get_ref(pRef, pRefH, pRefV, pRefHV, x, y, 16, X, Y, iEdgedWidth),iEdgedWidth, iMinSAD); \
    iSAD += calc_delta_16((X) - center_x, (Y) - center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); } } \
}

//Parallel code. by gary
#define CHECK_MV16_CANDIDATE_INT(X,Y) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad16( cur, get_ref_INT(pRef, x, y, 16, X, Y, iEdgedWidth),iEdgedWidth, iMinSAD); \
    iSAD += calc_delta_16((X) - center_x, (Y) - center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); } } \
}

// for halfpiexl mode.
// Parallel code.  by gary
#define CHECK_MV16_CANDIDATE_HP(X,Y) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad16( cur, get_ref_HP(pRef, pRefH, pRefV, pRefHV, X- center_x, Y- center_y),iEdgedWidth, iMinSAD); \
    iSAD += calc_delta_16((X) - center_x, (Y) - center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); } } \
}

// Parallel code.  by gary
#define CHECK_MV16_CANDIDATE_DIR_INT(X,Y,D) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad16( cur, get_ref_INT(pRef, x, y, 16, X, Y, iEdgedWidth),iEdgedWidth, iMinSAD); \
    iSAD += calc_delta_16((X) - center_x, (Y) - center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); iDirection=(D); } } \
}

// Parallel code. by gary
#define CHECK_MV16_CANDIDATE_FOUND_INT(X,Y,D) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad16( cur, get_ref_INT(pRef, x, y, 16, X, Y, iEdgedWidth),iEdgedWidth, iMinSAD); \
    iSAD += calc_delta_16((X) - center_x, (Y) - center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); iDirection=(D); iFound=0; } } \
}


//Parallel code. by gary
#define CHECK_MV8_ZERO_INT {\
  iSAD = sad8( cur, get_ref_INT(pRef, x, y, 8, 0, 0 , iEdgedWidth), iEdgedWidth); \
  iSAD += calc_delta_8(-center_x, -center_y, (uint8_t)iFcode, iQuant);\
  if (iSAD < iMinSAD) \
  { iMinSAD=iSAD; currMV->x=0; currMV->y=0; } \
}

//Parallel code. by gary
#define NOCHECK_MV8_CANDIDATE_INT(X,Y) \
  { \
    iSAD = sad8( cur, get_ref_INT(pRef, x, y, 8, (X), (Y), iEdgedWidth),iEdgedWidth); \
    iSAD += calc_delta_8((X)-center_x, (Y)-center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); } \
}

#define CHECK_MV8_CANDIDATE(X,Y) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad8( cur, get_ref(pRef, pRefH, pRefV, pRefHV, x, y, 8, (X), (Y), iEdgedWidth),iEdgedWidth); \
    iSAD += calc_delta_8((X)-center_x, (Y)-center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); } } \
}

//Parallel code. by gary
#define CHECK_MV8_CANDIDATE_INT(X,Y) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad8( cur, get_ref_INT(pRef, x, y, 8, (X), (Y), iEdgedWidth),iEdgedWidth); \
    iSAD += calc_delta_8((X)-center_x, (Y)-center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); } } \
}

//Parallel code.  by gary
#define CHECK_MV8_CANDIDATE_DIR_INT(X,Y,D) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad8( cur, get_ref_INT(pRef, x, y, 8, (X), (Y), iEdgedWidth),iEdgedWidth); \
    iSAD += calc_delta_8((X)-center_x, (Y)-center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); iDirection=(D); } } \
}

//Parallel code.  by gary
#define CHECK_MV8_CANDIDATE_FOUND_INT(X,Y,D) { \
  if ( ((X) <= max_dx) && ((X) >= min_dx) \
    && ((Y) <= max_dy) && ((Y) >= min_dy) ) \
  { \
    iSAD = sad8( cur, get_ref_INT(pRef, x, y, 8, (X), (Y), iEdgedWidth),iEdgedWidth); \
    iSAD += calc_delta_8((X)-center_x, (Y)-center_y, (uint8_t)iFcode, iQuant);\
    if (iSAD < iMinSAD) \
    {  iMinSAD=iSAD; currMV->x=(X); currMV->y=(Y); iDirection=(D); iFound=0; } } \
}


//Parallel code. by gary
int32_t
Diamond16_MainSearch_INT(const uint8_t * const pRef,
					     const uint8_t * const cur,
					     const int x,
					     const int y,
					     const int start_x,
					     const int start_y,
					     int iMinSAD,
					     VECTOR * const currMV,
					     const int center_x,
					     const int center_y,
					     const int32_t min_dx,
					     const int32_t max_dx,
					     const int32_t min_dy,
					     const int32_t max_dy,
					     const int32_t iEdgedWidth,
					     const int32_t iDiamondSize,
					     const int32_t iFcode,
					     const int32_t iQuant,
					     int iFound)
{
/* Do a diamond search around given starting point, return SAD of best */

	int32_t iDirection = 0;
	int32_t iDirectionBackup;
	int32_t iSAD;
	VECTOR backupMV;

	backupMV.x = start_x;
	backupMV.y = start_y;

/* It's one search with full Diamond pattern, and only 3 of 4 for all following diamonds */

	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize, backupMV.y, 1);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize, backupMV.y, 2);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x, backupMV.y - iDiamondSize, 3);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x, backupMV.y + iDiamondSize, 4);

	if (iDirection)	{	
		while (!iFound) {
			iFound = 1;
			backupMV = *currMV;
			iDirectionBackup = iDirection;

			if (iDirectionBackup != 2)
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y, 1);
			if (iDirectionBackup != 1)
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y, 2);
			if (iDirectionBackup != 4)
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x,
										       backupMV.y - iDiamondSize, 3);
			if (iDirectionBackup != 3)
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x,
										       backupMV.y + iDiamondSize, 4);
		}
	} else {
		currMV->x = start_x;
		currMV->y = start_y;
	}
	return iMinSAD;
}


//Parallel code. by gary
int32_t
Square16_MainSearch_INT(const uint8_t * const pRef,
					    const uint8_t * const cur,
					    const int x,
					    const int y,
					    const int start_x,
					    const int start_y,
					    int iMinSAD,
					    VECTOR * const currMV,
					    const int center_x,
					    const int center_y,
					    const int32_t min_dx,
					    const int32_t max_dx,
					    const int32_t min_dy,
					    const int32_t max_dy,
					    const int32_t iEdgedWidth,
					    const int32_t iDiamondSize,
					    const int32_t iFcode,
					    const int32_t iQuant,
					    int iFound)
{
/* Do a square search around given starting point, return SAD of best */

	int32_t iDirection = 0;
	int32_t iSAD;
	VECTOR backupMV;

	backupMV.x = start_x;
	backupMV.y = start_y;

/* It's one search with full square pattern, and new parts for all following diamonds */

/*   new direction are extra, so 1-4 is normal diamond
      537
      1*2
      648  
*/

	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize, backupMV.y, 1);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize, backupMV.y, 2);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x, backupMV.y - iDiamondSize, 3);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x, backupMV.y + iDiamondSize, 4);

	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize,
							     backupMV.y - iDiamondSize, 5);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize,
							     backupMV.y + iDiamondSize, 6);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize,
							     backupMV.y - iDiamondSize, 7);
	CHECK_MV16_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize,
							     backupMV.y + iDiamondSize, 8);


	if (iDirection)	{
		while (!iFound) {
			iFound = 1;
			backupMV = *currMV;

			switch (iDirection) {
			case 1:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y, 1);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y - iDiamondSize, 5);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y - iDiamondSize, 7);
				break;
			case 2:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										       2);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y + iDiamondSize, 6);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y + iDiamondSize, 8);
				break;

			case 3:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										       4);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y - iDiamondSize, 7);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y + iDiamondSize, 8);
				break;

			case 4:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
										       3);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y - iDiamondSize, 5);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y + iDiamondSize, 6);
				break;

			case 5:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize, backupMV.y,
										       1);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
										       3);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y - iDiamondSize, 5);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y + iDiamondSize, 6);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y - iDiamondSize, 7);
				break;

			case 6:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										       2);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
										       3);

				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y - iDiamondSize, 5);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y + iDiamondSize, 6);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y + iDiamondSize, 8);

				break;

			case 7:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y, 1);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										       4);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y - iDiamondSize, 5);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y - iDiamondSize, 7);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y + iDiamondSize, 8);
				break;

			case 8:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										       2);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										       4);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y + iDiamondSize, 6);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y - iDiamondSize, 7);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y + iDiamondSize, 8);
				break;
			default:
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize, backupMV.y,
										       1);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										       2);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
										       3);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										       4);

				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y - iDiamondSize, 5);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										       backupMV.y + iDiamondSize, 6);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y - iDiamondSize, 7);
				CHECK_MV16_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										       backupMV.y + iDiamondSize, 8);
				break;
			}
		}
	} else {
		currMV->x = start_x;
		currMV->y = start_y;
	}
	return iMinSAD;
}

//Parallel code. by gary
int32_t
AdvDiamond16_MainSearch_INT(const uint8_t * const pRef,
						    const uint8_t * const cur,
						    const int x,
						    const int y,
						    const int start_xi,
						    const int start_yi,
						    int iMinSAD,
						    VECTOR * const currMV,
						    const int center_x,
						    const int center_y,
						    const int32_t min_dx,
						    const int32_t max_dx,
						    const int32_t min_dy,
						    const int32_t max_dy,
						    const int32_t iEdgedWidth,
						    const int32_t iDiamondSize,
						    const int32_t iFcode,
						    const int32_t iQuant,
						    int iDirection)
{

	int32_t iSAD;
	int start_x = start_xi, start_y = start_yi;

/* directions: 1 - left (x-1); 2 - right (x+1), 4 - up (y-1); 8 - down (y+1) */

	if (iDirection) {
		CHECK_MV16_CANDIDATE_INT(start_x - iDiamondSize, start_y);
		CHECK_MV16_CANDIDATE_INT(start_x + iDiamondSize, start_y);
		CHECK_MV16_CANDIDATE_INT(start_x, start_y - iDiamondSize);
		CHECK_MV16_CANDIDATE_INT(start_x, start_y + iDiamondSize);
	} else {
		int bDirection = 1 + 2 + 4 + 8;

		do {
			iDirection = 0;
			if (bDirection & 1)	/*we only want to check left if we came from the right (our last motion was to the left, up-left or down-left) */
				CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize, start_y, 1);

			if (bDirection & 2)
				CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize, start_y, 2);

			if (bDirection & 4)
				CHECK_MV16_CANDIDATE_DIR_INT(start_x, start_y - iDiamondSize, 4);

			if (bDirection & 8)
				CHECK_MV16_CANDIDATE_DIR_INT(start_x, start_y + iDiamondSize, 8);

			/* now we're doing diagonal checks near our candidate */

			if (iDirection)		/*checking if anything found */
			{
				bDirection = iDirection;
				iDirection = 0;
				start_x = currMV->x;
				start_y = currMV->y;
				if (bDirection & 3)	/*our candidate is left or right */
				{
					CHECK_MV16_CANDIDATE_DIR_INT(start_x, start_y + iDiamondSize, 8);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x, start_y - iDiamondSize, 4);
				} else			/* what remains here is up or down */
				{
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize, start_y, 2);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize, start_y, 1);
				}

				if (iDirection) {
					bDirection += iDirection;
					start_x = currMV->x;
					start_y = currMV->y;
				}
			} else				/*about to quit, eh? not so fast.... */
			{
				switch (bDirection) {
				case 2:
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y - iDiamondSize, 2 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y + iDiamondSize, 2 + 8);
					break;
				case 1:
	
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y - iDiamondSize, 1 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y + iDiamondSize, 1 + 8);
					break;
				case 2 + 4:
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y - iDiamondSize, 1 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y - iDiamondSize, 2 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y + iDiamondSize, 2 + 8);
					break;
				case 4:
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y - iDiamondSize, 2 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y - iDiamondSize, 1 + 4);
					break;
				case 8:
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y + iDiamondSize, 2 + 8);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y + iDiamondSize, 1 + 8);
					break;
				case 1 + 4:
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y + iDiamondSize, 1 + 8);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y - iDiamondSize, 1 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y - iDiamondSize, 2 + 4);
					break;
				case 2 + 8:
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y - iDiamondSize, 1 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y + iDiamondSize, 1 + 8);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y + iDiamondSize, 2 + 8);
					break;
				case 1 + 8:
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y - iDiamondSize, 2 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y + iDiamondSize, 2 + 8);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y + iDiamondSize, 1 + 8);
					break;
				default:		/*1+2+4+8 == we didn't find anything at all */
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y - iDiamondSize, 1 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											 start_y + iDiamondSize, 1 + 8);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y - iDiamondSize, 2 + 4);
					CHECK_MV16_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											 start_y + iDiamondSize, 2 + 8);
					break;
				}
				if (!iDirection)
					break;		/*ok, the end. really */
				else {
					bDirection = iDirection;
					start_x = currMV->x;
					start_y = currMV->y;
				}
			}
		}
		while (1);				/*forever */
	}

	return iMinSAD;
}

//Parallel code.  by gary
int32_t
AdvDiamond8_MainSearch_INT(const uint8_t * const pRef,
					       const uint8_t * const cur,
					       const int x,
					       const int y,
					       const int start_xi,
					       const int start_yi,
					       int iMinSAD,
					       VECTOR * const currMV,
					       const int center_x,
					       const int center_y,
					       const int32_t min_dx,
					       const int32_t max_dx,
					       const int32_t min_dy,
					       const int32_t max_dy,
					       const int32_t iEdgedWidth,
					       const int32_t iDiamondSize,
					       const int32_t iFcode,
					       const int32_t iQuant,
					       int iDirection)
{

	int32_t iSAD;
	int start_x = start_xi, start_y = start_yi;

/* directions: 1 - left (x-1); 2 - right (x+1), 4 - up (y-1); 8 - down (y+1) */

	if (iDirection) {
		CHECK_MV8_CANDIDATE_INT(start_x - iDiamondSize, start_y);
		CHECK_MV8_CANDIDATE_INT(start_x + iDiamondSize, start_y);
		CHECK_MV8_CANDIDATE_INT(start_x, start_y - iDiamondSize);
		CHECK_MV8_CANDIDATE_INT(start_x, start_y + iDiamondSize);
	} else {
		int bDirection = 1 + 2 + 4 + 8;

		do {
			iDirection = 0;
			if (bDirection & 1)	/*we only want to check left if we came from the right (our last motion was to the left, up-left or down-left) */
				CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize, start_y, 1);

			if (bDirection & 2)
				CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize, start_y, 2);

			if (bDirection & 4)
				CHECK_MV8_CANDIDATE_DIR_INT(start_x, start_y - iDiamondSize, 4);

			if (bDirection & 8)
				CHECK_MV8_CANDIDATE_DIR_INT(start_x, start_y + iDiamondSize, 8);

			/* now we're doing diagonal checks near our candidate */

			if (iDirection)		/*checking if anything found */
			{
				bDirection = iDirection;
				iDirection = 0;
				start_x = currMV->x;
				start_y = currMV->y;
				if (bDirection & 3)	/*our candidate is left or right */
				{
					CHECK_MV8_CANDIDATE_DIR_INT(start_x, start_y + iDiamondSize, 8);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x, start_y - iDiamondSize, 4);
				} else			/* what remains here is up or down */
				{
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize, start_y, 2);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize, start_y, 1);
				}

				if (iDirection) {
					bDirection += iDirection;
					start_x = currMV->x;
					start_y = currMV->y;
				}
			} else				/*about to quit, eh? not so fast.... */
			{
				switch (bDirection) {
				case 2:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y - iDiamondSize, 2 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y + iDiamondSize, 2 + 8);
					break;
				case 1:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y - iDiamondSize, 1 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y + iDiamondSize, 1 + 8);
					break;
				case 2 + 4:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y - iDiamondSize, 1 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y - iDiamondSize, 2 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y + iDiamondSize, 2 + 8);
					break;
				case 4:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y - iDiamondSize, 2 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y - iDiamondSize, 1 + 4);
					break;
				case 8:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y + iDiamondSize, 2 + 8);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y + iDiamondSize, 1 + 8);
					break;
				case 1 + 4:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y + iDiamondSize, 1 + 8);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y - iDiamondSize, 1 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y - iDiamondSize, 2 + 4);
					break;
				case 2 + 8:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y - iDiamondSize, 1 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y + iDiamondSize, 1 + 8);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y + iDiamondSize, 2 + 8);
					break;
				case 1 + 8:
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y - iDiamondSize, 2 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y + iDiamondSize, 2 + 8);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y + iDiamondSize, 1 + 8);
					break;
				default:		/*1+2+4+8 == we didn't find anything at all */
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y - iDiamondSize, 1 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x - iDiamondSize,
											start_y + iDiamondSize, 1 + 8);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y - iDiamondSize, 2 + 4);
					CHECK_MV8_CANDIDATE_DIR_INT(start_x + iDiamondSize,
											start_y + iDiamondSize, 2 + 8);
					break;
				}
				if (!(iDirection))
					break;		/*ok, the end. really */
				else {
					bDirection = iDirection;
					start_x = currMV->x;
					start_y = currMV->y;
				}
			}
		}
		while (1);				/*forever */
	}
	return iMinSAD;
}

Halfpel8_RefineFuncPtr Halfpel8_Refine;

int32_t
Halfpel16_Refine(const uint8_t * const pRef,
				 const uint8_t * const pRefH,
				 const uint8_t * const pRefV,
				 const uint8_t * const pRefHV,
				 const uint8_t * const cur,
				 const int x,
				 const int y,
				 VECTOR * const currMV,
				 int32_t iMinSAD,
			   const int center_x,
			   const int center_y,
				 const int32_t min_dx,
				 const int32_t max_dx,
				 const int32_t min_dy,
				 const int32_t max_dy,
				 const int32_t iFcode,
				 const int32_t iQuant,
				 const int32_t iEdgedWidth)
{
/* Do a half-pel refinement (or rather a "smallest possible amount" refinement) */

	int32_t iSAD;
	VECTOR backupMV = *currMV;

	CHECK_MV16_CANDIDATE(backupMV.x - 1, backupMV.y - 1);
	CHECK_MV16_CANDIDATE(backupMV.x, backupMV.y - 1);
	CHECK_MV16_CANDIDATE(backupMV.x + 1, backupMV.y - 1);
	CHECK_MV16_CANDIDATE(backupMV.x - 1, backupMV.y);
	CHECK_MV16_CANDIDATE(backupMV.x + 1, backupMV.y);
	CHECK_MV16_CANDIDATE(backupMV.x - 1, backupMV.y + 1);
	CHECK_MV16_CANDIDATE(backupMV.x, backupMV.y + 1);
	CHECK_MV16_CANDIDATE(backupMV.x + 1, backupMV.y + 1);

	return iMinSAD;
}


/********************************************************************************
 ** Parallel code. by gary
 ********************************************************************************/
int32_t
PMVfastSearch16_INT(const uint8_t * const pRef,
			 	   const IMAGE * const pCur,
				   const int x,
				   const int y,
				   const int start_x,	/* start is searched first, so it should contain the most */
				   const int start_y,   /* likely motion vector for this block */
				   const int center_x,	/* center is from where length of MVs is measured */
				   const int center_y,
				   const uint32_t MotionFlags,
				   const uint32_t iQuant,
				   const uint32_t iFcode,
				   const MBParam * const pParam,
				   const MACROBLOCK * const pMBs,
				   const MACROBLOCK * const prevMBs,
				   VECTOR * const currMV,
				   VECTOR * const currPMV)
{
	const uint32_t iWcount = pParam->mb_width;
	const int32_t iWidth = pParam->width;
	const int32_t iHeight = pParam->height;
	const int32_t iEdgedWidth = pParam->edged_width;

	const uint8_t *cur = pCur->y + x * 16 + y * 16 * iEdgedWidth;

	int32_t iDiamondSize;

	int32_t min_dx;
	int32_t max_dx;
	int32_t min_dy;
	int32_t max_dy;

	int32_t iFound;

	VECTOR newMV;
	VECTOR backupMV;			/* just for PMVFAST */

	VECTOR pmv[4];
	int32_t psad[4];

	MainSearch16FuncPtr_INT MainSearchPtr;

	const MACROBLOCK *const prevMB = prevMBs + x + y * iWcount;

	int32_t threshA, threshB;
	int32_t bPredEq;
	int32_t iMinSAD, iSAD;

/* Get maximum range */
	get_range(&min_dx, &max_dx, &min_dy, &max_dy, x, y, 16, iWidth, iHeight, iFcode);
/* we work with abs. MVs, not relative to prediction, so get_range is called relative to 0,0 */

    min_dx = EVEN(min_dx);
	max_dx = EVEN(max_dx);
	min_dy = EVEN(min_dy);
	max_dy = EVEN(max_dy);

	/* because we might use something like IF (dx>max_dx) THEN dx=max_dx; */
	bPredEq = get_pmvdata2(pMBs, iWcount, 0, x, y, 0, pmv, psad);

	if ((x == 0) && (y == 0)) {
		threshA = 512;
		threshB = 1024;
	} else {
		threshA = psad[0];
		threshB = threshA + 256;
		if (threshA < 512)
			threshA = 512;
		if (threshA > 1024)
			threshA = 1024;
		if (threshB > 1792)
			threshB = 1792;
	}

	iFound = 0;

/* Step 4: Calculate SAD around the Median prediction. 
   MinSAD=SAD 
   If Motion Vector equal to Previous frame motion vector 
   and MinSAD<PrevFrmSAD goto Step 10. 
   If SAD<=256 goto Step 10. 
*/

	currMV->x = EVEN(start_x);		
	currMV->y = EVEN(start_y);		

	if (currMV->x > max_dx) {
		currMV->x = max_dx;
	}
	if (currMV->x < min_dx) {
		currMV->x = min_dx;
	}
	if (currMV->y > max_dy) {
		currMV->y = max_dy;
	}
	if (currMV->y < min_dy) {
		currMV->y = min_dy;
	}

	iMinSAD =
		sad16(cur,
			  get_ref_mv_INT(pRef, x, y, 16, currMV,
						 iEdgedWidth), iEdgedWidth, MV_MAX_ERROR);
	iMinSAD +=
		calc_delta_16(currMV->x - center_x, currMV->y - center_y,
					  (uint8_t) iFcode, iQuant);

	if ((iMinSAD < 256) ||
		((MVequal(*currMV, prevMB->mvs[0])) &&
		 ((int32_t) iMinSAD < prevMB->sad16))) {
		if (iMinSAD < (int)(2 * iQuant))	/* high chances for SKIP-mode */
		{
			if (!MVzero(*currMV)) {
				iMinSAD += MV16_00_BIAS;
				CHECK_MV16_ZERO_INT;	// (0,0) saves space for letterboxed pictures
				iMinSAD -= MV16_00_BIAS;
			}
		}

		goto PMVfast16_Terminate_without_Refine_INT;
	}

/* Step 2 (lazy eval): Calculate Distance= |MedianMVX| + |MedianMVY| where MedianMV is the motion 
   vector of the median. 
   If PredEq=1 and MVpredicted = Previous Frame MV, set Found=2  
*/

	if ((bPredEq) && (MVequal(pmv[0], prevMB->mvs[0])))
		iFound = 2;

/* Step 3 (lazy eval): If Distance>0 or thresb<1536 or PredEq=1 Select small Diamond Search. 
   Otherwise select large Diamond Search. 
*/

	if ((!MVzero(pmv[0])) || (threshB < 1536) || (bPredEq))
		iDiamondSize = 1;		/* halfpel! */
	else
		iDiamondSize = 2;		/* halfpel! */

	if (!(MotionFlags & PMV_HALFPELDIAMOND16))
		iDiamondSize *= 2;

/* 
   Step 5: Calculate SAD for motion vectors taken from left block, top, top-right, and Previous frame block. 
   Also calculate (0,0) but do not subtract offset. 
   Let MinSAD be the smallest SAD up to this point. 
   If MV is (0,0) subtract offset. 
*/

/* (0,0) is always possible */

	if (!MVzero(pmv[0]))
	{
		CHECK_MV16_ZERO_INT;
	}

/* previous frame MV is always possible */

	if (!MVzero(prevMB->mvs[0]))
		if (!MVequal(prevMB->mvs[0], pmv[0]))
		{
			int32_t mvs_x = EVEN(prevMB->mvs[0].x);
			int32_t mvs_y = EVEN(prevMB->mvs[0].y);
            CHECK_MV16_CANDIDATE_INT(mvs_x, mvs_y);
		}

/* left neighbour, if allowed */

	if (!MVzero(pmv[1]))
		if (!MVequal(pmv[1], prevMB->mvs[0]))
			if (!MVequal(pmv[1], pmv[0])) {
				pmv[1].x = EVEN(pmv[1].x);
				pmv[1].y = EVEN(pmv[1].y);
				CHECK_MV16_CANDIDATE_INT(pmv[1].x, pmv[1].y);
			}
/* top neighbour, if allowed */
	if (!MVzero(pmv[2]))
		if (!MVequal(pmv[2], prevMB->mvs[0]))
			if (!MVequal(pmv[2], pmv[0]))
				if (!MVequal(pmv[2], pmv[1])) 
				{
				    pmv[2].x = EVEN(pmv[2].x);
				    pmv[2].y = EVEN(pmv[2].y);
					CHECK_MV16_CANDIDATE_INT(pmv[2].x, pmv[2].y);

/* top right neighbour, if allowed */
					if (!MVzero(pmv[3]))
						if (!MVequal(pmv[3], prevMB->mvs[0]))
							if (!MVequal(pmv[3], pmv[0]))
								if (!MVequal(pmv[3], pmv[1]))
									if (!MVequal(pmv[3], pmv[2])) 
									{
										pmv[3].x = EVEN(pmv[3].x);
										pmv[3].y = EVEN(pmv[3].y);
										CHECK_MV16_CANDIDATE_INT(pmv[3].x,pmv[3].y);
									}
				}

	if ((MVzero(*currMV)) &&
		(!MVzero(pmv[0])) /* && (iMinSAD <= iQuant * 96) */ )
		iMinSAD -= MV16_00_BIAS;


/* Step 6: If MinSAD <= thresa goto Step 10. 
   If Motion Vector equal to Previous frame motion vector and MinSAD<PrevFrmSAD goto Step 10. 
*/

	if ((iMinSAD <= threshA) ||
		(MVequal(*currMV, prevMB->mvs[0]) &&
		 ((int32_t) iMinSAD < prevMB->sad16))) {
			goto PMVfast16_Terminate_without_Refine_INT;
	}

/************ (Diamond Search)  **************/
/* 
   Step 7: Perform Diamond search, with either the small or large diamond. 
   If Found=2 only examine one Diamond pattern, and afterwards goto step 10 
   Step 8: If small diamond, iterate small diamond search pattern until motion vector lies in the center of the diamond. 
   If center then goto step 10. 
   Step 9: If large diamond, iterate large diamond search pattern until motion vector lies in the center. 
   Refine by using small diamond and goto step 10. 
*/

	if (MotionFlags & PMV_USESQUARES16)
		MainSearchPtr = Square16_MainSearch_INT;
	else if (MotionFlags & PMV_ADVANCEDDIAMOND16)
		MainSearchPtr = AdvDiamond16_MainSearch_INT;
	else
		MainSearchPtr = Diamond16_MainSearch_INT;

	backupMV = *currMV;			/* save best prediction, actually only for EXTSEARCH */


/* default: use best prediction as starting point for one call of PMVfast_MainSearch */
	iSAD =
		(*MainSearchPtr) (pRef, cur, x, y, 
						  currMV->x, currMV->y, iMinSAD, &newMV, center_x, center_y, 
						  min_dx, max_dx,
						  min_dy, max_dy, iEdgedWidth, iDiamondSize, iFcode,
						  iQuant, iFound);

	if (iSAD < iMinSAD) {
		*currMV = newMV;
		iMinSAD = iSAD;
	}

	if (MotionFlags & PMV_EXTSEARCH16) {
/* extended: search (up to) two more times: orignal prediction and (0,0) */

		if (!(MVequal(pmv[0], backupMV))) {
			iSAD =
				(*MainSearchPtr) (pRef,cur, x, y,
								  center_x, center_y, iMinSAD, &newMV, center_x, center_y,
								  min_dx, max_dx, min_dy, max_dy, iEdgedWidth,
								  iDiamondSize, iFcode, iQuant, iFound);

			if (iSAD < iMinSAD) {
				*currMV = newMV;
				iMinSAD = iSAD;
			}
		}

		if ((!(MVzero(pmv[0]))) && (!(MVzero(backupMV)))) {
			iSAD =
				(*MainSearchPtr) (pRef, cur, x, y, 0, 0,
								  iMinSAD, &newMV, center_x, center_y, 
								  min_dx, max_dx, min_dy, max_dy, 
								  iEdgedWidth, iDiamondSize, iFcode,
								  iQuant, iFound);

			if (iSAD < iMinSAD) {
				*currMV = newMV;
				iMinSAD = iSAD;
			}
		}
	}

/* 
   Step 10:  The motion vector is chosen according to the block corresponding to MinSAD.
*/

  PMVfast16_Terminate_without_Refine_INT:
	currPMV->x = currMV->x - center_x;
	currPMV->y = currMV->y - center_y;
	return iMinSAD;
}


//Parallel code .by gary
int32_t
Diamond8_MainSearch_INT(const uint8_t * const pRef,
					    const uint8_t * const cur,
					    const int x,
					    const int y,
					    const int32_t start_x,
					    const int32_t start_y,
					    int32_t iMinSAD,
					    VECTOR * const currMV,
					    const int center_x,
					    const int center_y,
					    const int32_t min_dx,
					    const int32_t max_dx,
					    const int32_t min_dy,
					    const int32_t max_dy,
					    const int32_t iEdgedWidth,
					    const int32_t iDiamondSize,
					    const int32_t iFcode,
					    const int32_t iQuant,
					    int iFound)
{
/* Do a diamond search around given starting point, return SAD of best */

	int32_t iDirection = 0;
	int32_t iDirectionBackup;
	int32_t iSAD;
	VECTOR backupMV;

	backupMV.x = start_x;
	backupMV.y = start_y;

/* It's one search with full Diamond pattern, and only 3 of 4 for all following diamonds */

	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize, backupMV.y, 1);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize, backupMV.y, 2);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x, backupMV.y - iDiamondSize, 3);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x, backupMV.y + iDiamondSize, 4);

	if (iDirection)	{
		while (!iFound) {
			iFound = 1;
			backupMV = *currMV;	/* since iDirection!=0, this is well defined! */
			iDirectionBackup = iDirection;

			if (iDirectionBackup != 2)
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y, 1);
			if (iDirectionBackup != 1)
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										      backupMV.y, 2);
			if (iDirectionBackup != 4)
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x,
										      backupMV.y - iDiamondSize, 3);
			if (iDirectionBackup != 3)
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x,
										      backupMV.y + iDiamondSize, 4);
		}
	} else {
		currMV->x = start_x;
		currMV->y = start_y;
	}
	return iMinSAD;
}


//Parallel code .by gary
int32_t
Square8_MainSearch_INT(const uint8_t * const pRef,
				       const uint8_t * const cur,
				       const int x,
				       const int y,
				       const int32_t start_x,
				       const int32_t start_y,
				       int32_t iMinSAD,
				       VECTOR * const currMV,
				       const int center_x,
				       const int center_y,
				       const int32_t min_dx,
				       const int32_t max_dx,
				       const int32_t min_dy,
				       const int32_t max_dy,
				       const int32_t iEdgedWidth,
				       const int32_t iDiamondSize,
				       const int32_t iFcode,
				       const int32_t iQuant,
				       int iFound)
{
/* Do a square search around given starting point, return SAD of best */

	int32_t iDirection = 0;
	int32_t iSAD;
	VECTOR backupMV;

	backupMV.x = start_x;
	backupMV.y = start_y;

/* It's one search with full square pattern, and new parts for all following diamonds */

/*   new direction are extra, so 1-4 is normal diamond
      537
      1*2
      648  
*/

	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize, backupMV.y, 1);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize, backupMV.y, 2);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x, backupMV.y - iDiamondSize, 3);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x, backupMV.y + iDiamondSize, 4);

	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize,
							    backupMV.y - iDiamondSize, 5);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x - iDiamondSize,
							    backupMV.y + iDiamondSize, 6);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize,
							    backupMV.y - iDiamondSize, 7);
	CHECK_MV8_CANDIDATE_DIR_INT(backupMV.x + iDiamondSize,
							    backupMV.y + iDiamondSize, 8);


	if (iDirection)	{
		while (!iFound) {
			iFound = 1;
			backupMV = *currMV;

			switch (iDirection) {
			case 1:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y, 1);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y - iDiamondSize, 5);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										      backupMV.y - iDiamondSize, 7);
				break;
			case 2:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										      2);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y + iDiamondSize, 6);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										      backupMV.y + iDiamondSize, 8);
				break;

			case 3:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										      4);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										      backupMV.y - iDiamondSize, 7);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										      backupMV.y + iDiamondSize, 8);
				break;

			case 4:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
									     	  3);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y - iDiamondSize, 5);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y + iDiamondSize, 6);
				break;

			case 5:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize, backupMV.y,
										      1);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
										      3);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y - iDiamondSize, 5);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										      backupMV.y + iDiamondSize, 6);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										      backupMV.y - iDiamondSize, 7);
				break;

			case 6:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										      2);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
										      3);

				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										 backupMV.y - iDiamondSize, 5);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										 backupMV.y + iDiamondSize, 6);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										 backupMV.y + iDiamondSize, 8);

				break;

			case 7:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										   backupMV.y, 1);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										 4);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										 backupMV.y - iDiamondSize, 5);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										 backupMV.y - iDiamondSize, 7);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										 backupMV.y + iDiamondSize, 8);
				break;

			case 8:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										 2);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										 4);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										 backupMV.y + iDiamondSize, 6);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										 backupMV.y - iDiamondSize, 7);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										 backupMV.y + iDiamondSize, 8);
				break;
			default:
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize, backupMV.y,
										 1);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize, backupMV.y,
										 2);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y - iDiamondSize,
										 3);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x, backupMV.y + iDiamondSize,
										 4);

				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										 backupMV.y - iDiamondSize, 5);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x - iDiamondSize,
										 backupMV.y + iDiamondSize, 6);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										 backupMV.y - iDiamondSize, 7);
				CHECK_MV8_CANDIDATE_FOUND_INT(backupMV.x + iDiamondSize,
										 backupMV.y + iDiamondSize, 8);
				break;
			}
		}
	} else {
		currMV->x = start_x;
		currMV->y = start_y;
	}
	return iMinSAD;
}


int32_t
Halfpel8_Refine_c(const uint8_t * const pRef,
				  const uint8_t * const pRefH,
				  const uint8_t * const pRefV,
				  const uint8_t * const pRefHV,
				  const uint8_t * const cur,
				  const int x,
				  const int y,
				  VECTOR * const currMV,
				  int32_t iMinSAD,
				  const int center_x,
				  const int center_y,
				  const int32_t min_dx,
				  const int32_t max_dx,
				  const int32_t min_dy,
				  const int32_t max_dy,
				  const int32_t iFcode,
				  const int32_t iQuant,
				  const int32_t iEdgedWidth)
{
/* Do a half-pel refinement (or rather a "smallest possible amount" refinement) */

	int32_t iSAD;
	VECTOR backupMV = *currMV;

	CHECK_MV8_CANDIDATE(backupMV.x - 1, backupMV.y - 1);
	CHECK_MV8_CANDIDATE(backupMV.x, backupMV.y - 1);
	CHECK_MV8_CANDIDATE(backupMV.x + 1, backupMV.y - 1);
	CHECK_MV8_CANDIDATE(backupMV.x - 1, backupMV.y);
	CHECK_MV8_CANDIDATE(backupMV.x + 1, backupMV.y);
	CHECK_MV8_CANDIDATE(backupMV.x - 1, backupMV.y + 1);
	CHECK_MV8_CANDIDATE(backupMV.x, backupMV.y + 1);
	CHECK_MV8_CANDIDATE(backupMV.x + 1, backupMV.y + 1);

	return iMinSAD;
}

//Parallel code .by gary
int32_t
Halfpel8_Refine_c_INT(const uint8_t * const pRef,
				      const uint8_t * const cur,
				      const int x,
				      const int y,
				      VECTOR * const currMV,
				      int32_t iMinSAD,
				      const int center_x,
				      const int center_y,
				      const int32_t min_dx,
				      const int32_t max_dx,
				      const int32_t min_dy,
				      const int32_t max_dy,
				      const int32_t iFcode,
				      const int32_t iQuant,
				      const int32_t iEdgedWidth)
{
/* Do a half-pel refinement (or rather a "smallest possible amount" refinement) */

	int32_t iSAD;
	VECTOR backupMV = *currMV;

	CHECK_MV8_CANDIDATE_INT(backupMV.x - 1, backupMV.y - 1);
	CHECK_MV8_CANDIDATE_INT(backupMV.x, backupMV.y - 1);
	CHECK_MV8_CANDIDATE_INT(backupMV.x + 1, backupMV.y - 1);
	CHECK_MV8_CANDIDATE_INT(backupMV.x - 1, backupMV.y);
	CHECK_MV8_CANDIDATE_INT(backupMV.x + 1, backupMV.y);
	CHECK_MV8_CANDIDATE_INT(backupMV.x - 1, backupMV.y + 1);
	CHECK_MV8_CANDIDATE_INT(backupMV.x, backupMV.y + 1);
	CHECK_MV8_CANDIDATE_INT(backupMV.x + 1, backupMV.y + 1);

	return iMinSAD;
}


/********************************************************************************
 ** Parallel code. by gary
 ********************************************************************************/
int32_t
PMVfastSearch8_INT(const uint8_t * const pRef,
			       const IMAGE * const pCur,
			       const int x,
			       const int y,
			       const int start_x,
			       const int start_y,
			       const int center_x,
				   const int center_y,
			       const uint32_t MotionFlags,
			       const uint32_t iQuant,
			       const uint32_t iFcode,
			       const MBParam * const pParam,
			       const MACROBLOCK * const pMBs,
			       const MACROBLOCK * const prevMBs,
			       VECTOR * const currMV,
			       VECTOR * const currPMV)
{
	const uint32_t iWcount = pParam->mb_width;
	const int32_t iWidth = pParam->width;
	const int32_t iHeight = pParam->height;
	const int32_t iEdgedWidth = pParam->edged_width;

	const uint8_t *cur = pCur->y + x * 8 + y * 8 * iEdgedWidth;

	int32_t iDiamondSize;

	int32_t min_dx;
	int32_t max_dx;
	int32_t min_dy;
	int32_t max_dy;

	VECTOR pmv[4];
	int32_t psad[4];
	VECTOR newMV;
	VECTOR backupMV;
	VECTOR startMV;

/*  const MACROBLOCK * const pMB = pMBs + (x>>1) + (y>>1) * iWcount; */
	const MACROBLOCK *const prevMB = prevMBs + (x >> 1) + (y >> 1) * iWcount;

	int32_t threshA, threshB;
	int32_t iFound, bPredEq;
	int32_t iMinSAD, iSAD;

	int32_t iSubBlock = (y & 1) + (y & 1) + (x & 1);

	MainSearch8FuncPtr_INT MainSearchPtr;

	/* Init variables */
	startMV.x = start_x;
	startMV.y = start_y;

	/* Get maximum range */
	get_range(&min_dx, &max_dx, &min_dy, &max_dy, x, y, 8, iWidth, iHeight,
			  iFcode);

	if (!(MotionFlags & PMV_HALFPELDIAMOND8)) {
		min_dx = EVEN(min_dx);
		max_dx = EVEN(max_dx);
		min_dy = EVEN(min_dy);
		max_dy = EVEN(max_dy);
	}

	/* because we might use IF (dx>max_dx) THEN dx=max_dx; */
	bPredEq = get_pmvdata2(pMBs, iWcount, 0, (x >> 1), (y >> 1), iSubBlock, pmv, psad);

	if ((x == 0) && (y == 0)) {
		threshA = 512 / 4;
		threshB = 1024 / 4;

	} else {
		threshA = psad[0] / 4;	/* good estimate? */
		threshB = threshA + 256 / 4;
		if (threshA < 512 / 4)
			threshA = 512 / 4;
		if (threshA > 1024 / 4)
			threshA = 1024 / 4;
		if (threshB > 1792 / 4)
			threshB = 1792 / 4;
	}

	iFound = 0;

/* Prepare for main loop  */

    if (MotionFlags & PMV_USESQUARES8)
	{
      MainSearchPtr = Square8_MainSearch_INT;
	}
    else
	{
		if (MotionFlags & PMV_ADVANCEDDIAMOND8){
			MainSearchPtr = AdvDiamond8_MainSearch_INT;
		}
		else{
			MainSearchPtr = Diamond8_MainSearch_INT;
		}
	}

	*currMV = startMV;

	iMinSAD =
		sad8(cur,
			 get_ref_mv_INT(pRef, x, y, 8, currMV,iEdgedWidth), iEdgedWidth);
	iMinSAD +=
		calc_delta_8(currMV->x - center_x, currMV->y - center_y,
					 (uint8_t) iFcode, iQuant);

	if ((iMinSAD < 256 / 4) || ((MVequal(*currMV, prevMB->mvs[iSubBlock])) 
		&& ((int32_t) iMinSAD < prevMB->sad8[iSubBlock]))) {
			goto PMVfast8_Terminate_without_Refine_INT;
	}

/* Step 2 (lazy eval): Calculate Distance= |MedianMVX| + |MedianMVY| where MedianMV is the motion 
   vector of the median. 
   If PredEq=1 and MVpredicted = Previous Frame MV, set Found=2  
*/

	if ((bPredEq) && (MVequal(pmv[0], prevMB->mvs[iSubBlock])))
		iFound = 2;

/* Step 3 (lazy eval): If Distance>0 or thresb<1536 or PredEq=1 Select small Diamond Search. 
   Otherwise select large Diamond Search. 
*/

	if ((!MVzero(pmv[0])) || (threshB < 1536 / 4) || (bPredEq))
		iDiamondSize = 1;		/* 1 halfpel! */
	else
		iDiamondSize = 2;		/* 2 halfpel = 1 full pixel! */

	if (!(MotionFlags & PMV_HALFPELDIAMOND8))
		iDiamondSize *= 2;

/* 
   Step 5: Calculate SAD for motion vectors taken from left block, top, top-right, and Previous frame block. 
   Also calculate (0,0) but do not subtract offset. 
   Let MinSAD be the smallest SAD up to this point. 
   If MV is (0,0) subtract offset. 
*/

/* the median prediction might be even better than mv16 */

	if (!MVequal(pmv[0], startMV))
		CHECK_MV8_CANDIDATE_INT(center_x, center_y);

/* (0,0) if needed */
	if (!MVzero(pmv[0]))
		if (!MVzero(startMV))
			CHECK_MV8_ZERO_INT;

/* previous frame MV if needed */
	if (!MVzero(prevMB->mvs[iSubBlock]))
		if (!MVequal(prevMB->mvs[iSubBlock], startMV))
			if (!MVequal(prevMB->mvs[iSubBlock], pmv[0]))
				CHECK_MV8_CANDIDATE_INT(EVEN(prevMB->mvs[iSubBlock].x),
									    EVEN(prevMB->mvs[iSubBlock].y));

	if ((iMinSAD <= threshA) ||
		(MVequal(*currMV, prevMB->mvs[iSubBlock]) &&
		 ((int32_t) iMinSAD < prevMB->sad8[iSubBlock]))) {
			goto PMVfast8_Terminate_without_Refine_INT;
	}

/* left neighbour, if allowed and needed */
	if (!MVzero(pmv[1]))
		if (!MVequal(pmv[1], startMV))
			if (!MVequal(pmv[1], prevMB->mvs[iSubBlock]))
				if (!MVequal(pmv[1], pmv[0])) {
					pmv[1].x = EVEN(pmv[1].x);
					pmv[1].y = EVEN(pmv[1].y);
					CHECK_MV8_CANDIDATE_INT(pmv[1].x, pmv[1].y);
				}
/* top neighbour, if allowed and needed */
	if (!MVzero(pmv[2]))
		if (!MVequal(pmv[2], startMV))
			if (!MVequal(pmv[2], prevMB->mvs[iSubBlock]))
				if (!MVequal(pmv[2], pmv[0]))
					if (!MVequal(pmv[2], pmv[1])) 
					{
						pmv[2].x = EVEN(pmv[2].x);
						pmv[2].y = EVEN(pmv[2].y);
						CHECK_MV8_CANDIDATE_INT(pmv[2].x, pmv[2].y);

/* top right neighbour, if allowed and needed */
						if (!MVzero(pmv[3]))
							if (!MVequal(pmv[3], startMV))
								if (!MVequal(pmv[3], prevMB->mvs[iSubBlock]))
									if (!MVequal(pmv[3], pmv[0]))
										if (!MVequal(pmv[3], pmv[1]))
											if (!MVequal(pmv[3], pmv[2])) 
											{
												pmv[3].x = EVEN(pmv[3].x);
												pmv[3].y = EVEN(pmv[3].y);
												CHECK_MV8_CANDIDATE_INT(pmv[3].x,pmv[3].y);
											}
					}

	if ((MVzero(*currMV)) &&
		(!MVzero(pmv[0])) /* && (iMinSAD <= iQuant * 96) */ )
		iMinSAD -= MV8_00_BIAS;


/* Step 6: If MinSAD <= thresa goto Step 10. 
   If Motion Vector equal to Previous frame motion vector and MinSAD<PrevFrmSAD goto Step 10. 
*/

	if ((iMinSAD <= threshA) ||
		(MVequal(*currMV, prevMB->mvs[iSubBlock]) &&
		 ((int32_t) iMinSAD < prevMB->sad8[iSubBlock]))) {
			goto PMVfast8_Terminate_without_Refine_INT;
	}

/************ (Diamond Search)  **************/
/* 
   Step 7: Perform Diamond search, with either the small or large diamond. 
   If Found=2 only examine one Diamond pattern, and afterwards goto step 10 
   Step 8: If small diamond, iterate small diamond search pattern until motion vector lies in the center of the diamond. 
   If center then goto step 10. 
   Step 9: If large diamond, iterate large diamond search pattern until motion vector lies in the center. 
   Refine by using small diamond and goto step 10. 
*/

	backupMV = *currMV;			/* save best prediction, actually only for EXTSEARCH */

/* default: use best prediction as starting point for one call of PMVfast_MainSearch */
	iSAD =
		(*MainSearchPtr) (pRef, cur, x, y, currMV->x,
						  currMV->y, iMinSAD, &newMV, center_x, center_y, min_dx, max_dx,
						  min_dy, max_dy, iEdgedWidth, iDiamondSize, iFcode,
						  iQuant, iFound);

	if (iSAD < iMinSAD) {
		*currMV = newMV;
		iMinSAD = iSAD;
	}

	if (MotionFlags & PMV_EXTSEARCH8) {
/* extended: search (up to) two more times: orignal prediction and (0,0) */

		if (!(MVequal(pmv[0], backupMV))) {
			iSAD =
				(*MainSearchPtr) (pRef,cur, x, y,
								  pmv[0].x, pmv[0].y, iMinSAD, &newMV, center_x, center_y,
								  min_dx, max_dx, min_dy, max_dy, iEdgedWidth,
								  iDiamondSize, iFcode, iQuant, iFound);

			if (iSAD < iMinSAD) {
				*currMV = newMV;
				iMinSAD = iSAD;
			}
		}

		if ((!(MVzero(pmv[0]))) && (!(MVzero(backupMV)))) {
			iSAD =
				(*MainSearchPtr) (pRef, cur, x, y, 0, 0,
								  iMinSAD, &newMV, center_x, center_y, min_dx, max_dx, min_dy,
								  max_dy, iEdgedWidth, iDiamondSize, iFcode,
								  iQuant, iFound);

			if (iSAD < iMinSAD) {
				*currMV = newMV;
				iMinSAD = iSAD;
			}
		}
	}

/* Step 10: The motion vector is chosen according to the block corresponding to MinSAD.
   By performing an optional local half-pixel search, we can refine this result even further.
*/

  PMVfast8_Terminate_without_Refine_INT:
	currPMV->x = currMV->x - center_x;
	currPMV->y = currMV->y - center_y;

	return iMinSAD;
}

/*****************************************************************************************
 ** Parallel Code . by gary
 *****************************************************************************************/
int32_t
Halfpel_8point(const uint8_t * const pRef,
			   const uint8_t * const pRefH,
			   const uint8_t * const pRefV,
			   const uint8_t * const pRefHV,
			   const uint8_t * const cur,
			   const int x,
			   const int y,
			   VECTOR * const currMV,
			   int32_t iMinSAD,
			   const int center_x,
			   const int center_y,
			   const int32_t iFcode,
			   const int32_t iQuant,
			   const int32_t iEdgedWidth,
			   const MBParam * const pParam)
{
	int32_t min_dx;
	int32_t max_dx;
	int32_t min_dy;
	int32_t max_dy;
	int32_t iSAD;

	const int32_t iWidth = pParam->width;
	const int32_t iHeight = pParam->height;

	get_range(&min_dx, &max_dx, &min_dy, &max_dy, x, y, 16, iWidth, iHeight, iFcode);

	CHECK_MV16_CANDIDATE_HP(center_x-1 ,center_y-1);
	CHECK_MV16_CANDIDATE_HP(center_x-1 ,center_y);
	CHECK_MV16_CANDIDATE_HP(center_x-1 ,center_y+1);
	CHECK_MV16_CANDIDATE_HP(center_x ,center_y-1);
	CHECK_MV16_CANDIDATE_HP(center_x ,center_y+1);
	CHECK_MV16_CANDIDATE_HP(center_x+1 ,center_y-1);
	CHECK_MV16_CANDIDATE_HP(center_x+1 ,center_y);
	CHECK_MV16_CANDIDATE_HP(center_x+1 ,center_y+1);

	return iMinSAD;
}
