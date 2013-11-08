#ifndef _ENCODER_CBP_H_
#define _ENCODER_CBP_H_

#include "../portab.h"

/*****************************************************************************
 * Function type
 ****************************************************************************/

typedef uint32_t(cbpFunc) (const int16_t * codes);

typedef cbpFunc *cbpFuncPtr;

typedef uint32_t(single_cbpFunc) (const int16_t * codes,int32_t cbp,const int32_t i); //by gary

typedef single_cbpFunc *single_cbpFuncPtr; //by gary
/*****************************************************************************
 * Global Function pointer
 ****************************************************************************/

extern cbpFuncPtr calc_cbp;

extern single_cbpFuncPtr single_calc_cbp; //by gary

/*****************************************************************************
 * Prototypes
 ****************************************************************************/

extern cbpFunc calc_cbp_c;

extern single_cbpFunc single_calc_cbp_c; //by gary

#endif /* _ENCODER_CBP_H_ */
