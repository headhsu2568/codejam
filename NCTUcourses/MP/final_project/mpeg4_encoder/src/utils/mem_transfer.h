#ifndef _MEM_TRANSFER_H
#define _MEM_TRANSFER_H

/*****************************************************************************
 * transfer8to16 API
 ****************************************************************************/

typedef void (TRANSFER_8TO16COPY) (int16_t * const dst,
								   const uint8_t * const src,
								   uint32_t stride);

typedef TRANSFER_8TO16COPY *TRANSFER_8TO16COPY_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_8TO16COPY_PTR transfer_8to16copy;

/* Implemented functions */
TRANSFER_8TO16COPY transfer_8to16copy_c;

/*****************************************************************************
 * transfer16to8 API
 ****************************************************************************/

typedef void (TRANSFER_16TO8COPY) (uint8_t * const dst,
								   const int16_t * const src,
								   uint32_t stride);
typedef TRANSFER_16TO8COPY *TRANSFER_16TO8COPY_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_16TO8COPY_PTR transfer_16to8copy;

/* Implemented functions */
TRANSFER_16TO8COPY transfer_16to8copy_c;

/*****************************************************************************
 * transfer8to16 + substraction op API
 ****************************************************************************/

typedef void (TRANSFER_8TO16SUB) (int16_t * const dct,
								  uint8_t * const cur,
								  const uint8_t * ref,
								  const uint32_t stride);

typedef TRANSFER_8TO16SUB *TRANSFER_8TO16SUB_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_8TO16SUB_PTR transfer_8to16sub;

/* Implemented functions */
TRANSFER_8TO16SUB transfer_8to16sub_c;

/*****************************************************************************
 * transfer8to16 + substraction op API - Bidirectionnal Version
 ****************************************************************************/

typedef void (TRANSFER_8TO16SUB2) (int16_t * const dct,
								   uint8_t * const cur,
								   const uint8_t * ref1,
								   const uint8_t * ref2,
								   const uint32_t stride);

typedef TRANSFER_8TO16SUB2 *TRANSFER_8TO16SUB2_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_8TO16SUB2_PTR transfer_8to16sub2;

/* Implemented functions */
TRANSFER_8TO16SUB2 transfer_8to16sub2_c;


/*****************************************************************************
 * transfer16to8 + addition op API
 ****************************************************************************/

typedef void (TRANSFER_16TO8ADD) (uint8_t * const dst,
								  const int16_t * const src,
								  uint32_t stride);

typedef TRANSFER_16TO8ADD *TRANSFER_16TO8ADD_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_16TO8ADD_PTR transfer_16to8add;

/* Implemented functions */
TRANSFER_16TO8ADD transfer_16to8add_c;

/*****************************************************************************
 * transfer8to8 + no op
 ****************************************************************************/

typedef void (TRANSFER8X8_COPY) (uint8_t * const dst,
								 const uint8_t * const src,
								 const uint32_t stride);

typedef TRANSFER8X8_COPY *TRANSFER8X8_COPY_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER8X8_COPY_PTR transfer8x8_copy;

/* Implemented functions */
TRANSFER8X8_COPY transfer8x8_copy_c;

#endif
