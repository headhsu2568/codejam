#ifndef _MOTION_H_
#define _MOTION_H_

#include "../portab.h"
#include "../global.h"

/* hard coded motion search parameters for motion_est */

/* very large value */
#define MV_MAX_ERROR	(4096 * 256)

/* stop search if sdelta < THRESHOLD */
#define MV16_THRESHOLD	192
#define MV8_THRESHOLD	56

#define NEIGH_MOVE_THRESH 0	
/* how much a block's MV must differ from his neighbour  */
/* to be search for INTER4V. The more, the faster... */

/* sad16(0,0) bias; mpeg4 spec suggests nb/2+1 */
/* nb  = vop pixels * 2^(bpp-8) */
#define MV16_00_BIAS	(128+1)
#define MV8_00_BIAS	(0)

/* INTER bias for INTER/INTRA decision; mpeg4 spec suggests 2*nb */
#define MV16_INTER_BIAS	512

/* Parameters which control inter/inter4v decision */
#define IMV16X16			5

/* vector map (vlc delta size) smoother parameters */
#define NEIGH_TEND_16X16	2
#define NEIGH_TEND_8X8		2

/* fast ((A)/2)*2 */
#define EVEN(A)		(((A)<0?(A)+1:(A)) & ~1)

#define MVzero(A) ( ((A).x)==(0) && ((A).y)==(0) )
#define MVequal(A,B) ( ((A).x)==((B).x) && ((A).y)==((B).y) )

/* default methods of Search, will be changed to function variable */

//Parallel code .by gary
#ifndef SEARCH16_INT
#define SEARCH16_INT	PMVfastSearch16_INT
#endif

//Parallel code .by gary
#ifndef SEARCH8_INT
#define SEARCH8_INT	PMVfastSearch8_INT
#endif

//Parallel code .by gary
#define ME_INTRA 1
#define ME_INTPXL 2
#define ME_HALFPXL 3

/*
 * Calculate the min/max range (in halfpixels)
 * relative to the _MACROBLOCK_ position
 */

static void __inline
get_range(int32_t * const min_dx,
		  int32_t * const max_dx,
		  int32_t * const min_dy,
		  int32_t * const max_dy,
		  const uint32_t x,
		  const uint32_t y,
		  const uint32_t block_sz,	/* block dimension, 8 or 16 */

		  const uint32_t width,
		  const uint32_t height,
		  const uint32_t fcode)
{

	const int search_range = 32 << (fcode - 1);
	const int high = search_range - 1;
	const int low = -search_range;

	/* convert full-pixel measurements to half pixel */
	const int hp_width = 2 * width;
	const int hp_height = 2 * height;
	const int hp_edge = 2 * block_sz;

	/* we need _right end_ of block, not x-coordinate */
	const int hp_x = 2 * (x) * block_sz;

	/* same for _bottom end_ */
	const int hp_y = 2 * (y) * block_sz;

	*max_dx = MIN(high, hp_width - hp_x);
	*max_dy = MIN(high, hp_height - hp_y);
	*min_dx = MAX(low, -(hp_edge + hp_x));
	*min_dy = MAX(low, -(hp_edge + hp_y));

}


/*
 * getref: calculate reference image pointer 
 * the decision to use interpolation h/v/hv or the normal image is
 * based on dx & dy.
 */

static __inline const uint8_t *
get_ref(const uint8_t * const refn,
		const uint8_t * const refh,
		const uint8_t * const refv,
		const uint8_t * const refhv,
		const uint32_t x,
		const uint32_t y,
		const uint32_t block,	/* block dimension, 8 or 16 */

		const int32_t dx,
		const int32_t dy,
		const uint32_t stride)
{


	switch (((dx & 1) << 1) + (dy & 1)) {	/* ((dx%2)?2:0)+((dy%2)?1:0) */
	case 0:
		return refn + (int) ((x * block + dx / 2) + (y * block + dy / 2) * stride);
	case 1:
		return refv + (int) ((x * block + dx / 2) + (y * block +
											  (dy - 1) / 2) * stride);
	case 2:
		return refh + (int) ((x * block + (dx - 1) / 2) + (y * block +
													dy / 2) * stride);
	default:
	case 3:
		return refhv + (int) ((x * block + (dx - 1) / 2) + (y * block +
													 (dy - 1) / 2) * stride);
	}

}

//Parallel code .by gary
static __inline const uint8_t *
get_ref_INT(const uint8_t * const refn,
		    const uint32_t x,
		    const uint32_t y,
		    const uint32_t block,	/* block dimension, 8 or 16 */

		    const int32_t dx,
		    const int32_t dy,
		    const uint32_t stride)
{
		return refn + (int) ((x * block + dx / 2) + (y * block + dy / 2) * stride);
}

// for HalfPixel mode.
//Parallel code .by gary
static __inline const uint8_t *
get_ref_HP(const uint8_t * const refn,
	       const uint8_t * const refh,
		   const uint8_t * const refv,
		   const uint8_t * const refhv,

		   const int32_t dx,
		   const int32_t dy)
{


	switch (((dx & 1) << 1) + (dy & 1)) {	/* ((dx%2)?2:0)+((dy%2)?1:0) */
	case 0:
		return refn ;  // never happen.
	case 1:
		return refv + (int)(dy==-1 ? 0:17);
	case 2:
		return refh + (int)(dx==-1 ? 0:1);
	default:
	case 3:
		return refhv + (int)(dy==-1 ? (dx==-1 ? 0:1):(dx==-1 ? 17:18));
	}

}


/* This is somehow a copy of get_ref, but with MV instead of X,Y */

static __inline const uint8_t *
get_ref_mv(const uint8_t * const refn,
		   const uint8_t * const refh,
		   const uint8_t * const refv,
		   const uint8_t * const refhv,
		   const uint32_t x,
		   const uint32_t y,
		   const uint32_t block,	/* block dimension, 8 or 16 */

		   const VECTOR * mv,	/* measured in half-pel! */

		   const uint32_t stride)
{

	switch ((((mv->x) & 1) << 1) + ((mv->y) & 1)) {
	case 0:
		return refn + (int) ((x * block + (mv->x) / 2) + (y * block +
												   (mv->y) / 2) * stride);
	case 1:
		return refv + (int) ((x * block + (mv->x) / 2) + (y * block +
												   ((mv->y) - 1) / 2) * stride);
	case 2:
		return refh + (int) ((x * block + ((mv->x) - 1) / 2) + (y * block +
														 (mv->y) / 2) * stride);
	default:
	case 3:
		return refhv + (int) ((x * block + ((mv->x) - 1) / 2) + (y * block +
														  ((mv->y) -
														   1) / 2) * stride);
	}

}

//Parallel code . by gary
static __inline const uint8_t *
get_ref_mv_INT(const uint8_t * const refn,
		       const uint32_t x,
		       const uint32_t y,
		       const uint32_t block,	/* block dimension, 8 or 16 */

		       const VECTOR * mv,	/* measured in half-pel! */

		       const uint32_t stride)
{
		return refn + (int) ((x * block + (mv->x) / 2) + (y * block +
												   (mv->y) / 2) * stride);
}


/* prototypes for MainSearch functions, i.e. Diamondsearch, FullSearch or whatever */

//Parallel code .by gary
typedef int32_t(MainSearch16Func_INT) (const uint8_t * const pRef,
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
								       int iFound);

typedef MainSearch16Func_INT *MainSearch16FuncPtr_INT;

//Parallel code .by gary
typedef int32_t(MainSearch8Func_INT) (const uint8_t * const pRef,
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
								      int iFound);

typedef MainSearch8Func_INT *MainSearch8FuncPtr_INT;


//Parallel code .by gary
typedef int32_t(Search16Func_INT) (	const uint8_t * const pRef,
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
								    VECTOR * const currPMV);

typedef Search16Func_INT *Search16FuncPtr_INT;

//Parallel code .by gary
typedef int32_t(Search8Func_INT) (	const uint8_t * const pRef,
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
								    VECTOR * const currPMV);

typedef Search8Func_INT *Search8FuncPtr_INT;


//Parallel code .by gary
Search16Func_INT PMVfastSearch16_INT;

Search8Func_INT	PMVfastSearch8_INT;


//Parallel code .by gary
int
MotionEstimationMB(MBParam * const pParam,
				   FRAMEINFO * const current,
				   FRAMEINFO * const reference,
				   int32_t x,
				   int32_t y);

typedef int32_t(Halfpel8_RefineFunc) (const uint8_t * const pRef,
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
				      const int32_t iEdgedWidth);

typedef Halfpel8_RefineFunc *Halfpel8_RefineFuncPtr;
extern Halfpel8_RefineFuncPtr Halfpel8_Refine;
Halfpel8_RefineFunc Halfpel8_Refine_c;

//Parallel code .by gary
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
			   const MBParam * const pParam);

//by gary
void
compensate8x8_halfpel(int16_t * const dct_codes,
					  uint8_t * const cur,
					  const uint8_t * const ref,
					  const uint8_t * const refh,
					  const uint8_t * const refv,
					  const uint8_t * const refhv,
					  const uint32_t x,
					  const uint32_t y,
					  const int32_t dx,
					  const int dy,
					  const uint32_t stride);
#endif							/* _MOTION_H_ */
