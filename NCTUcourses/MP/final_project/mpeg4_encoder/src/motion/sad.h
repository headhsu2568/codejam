#ifndef _ENCODER_SAD_H_
#define _ENCODER_SAD_H_

#include "../portab.h"

typedef void (sadInitFunc) (void);
typedef sadInitFunc *sadInitFuncPtr;

extern sadInitFuncPtr sadInit;
sadInitFunc sadInit_altivec;


typedef uint32_t(sad16Func) (const uint8_t * const cur,
							 const uint8_t * const ref,
							 const uint32_t stride,
							 const uint32_t best_sad);
typedef sad16Func *sad16FuncPtr;
extern sad16FuncPtr sad16;
sad16Func sad16_c;


typedef uint32_t(sad8Func) (const uint8_t * const cur,
							const uint8_t * const ref,
							const uint32_t stride);
typedef sad8Func *sad8FuncPtr;
extern sad8FuncPtr sad8;
sad8Func sad8_c;


typedef uint32_t(dev16Func) (const uint8_t * const cur,
							 const uint32_t stride);
typedef dev16Func *dev16FuncPtr;
extern dev16FuncPtr dev16;
dev16Func dev16_c;

#endif							/* _ENCODER_SAD_H_ */
