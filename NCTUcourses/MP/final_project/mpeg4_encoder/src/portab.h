#ifndef _PORTAB_H_
#define _PORTAB_H_

/*****************************************************************************
 *  Common things
 ****************************************************************************/

/* Debug level masks */
#define DPRINTF_ERROR		0x00000001
#define DPRINTF_STARTCODE	0x00000002
#define DPRINTF_HEADER		0x00000004
#define DPRINTF_TIMECODE	0x00000008
#define DPRINTF_MB			0x00000010
#define DPRINTF_COEFF		0x00000020
#define DPRINTF_MV			0x00000040
#define DPRINTF_DEBUG		0x80000000

/* debug level for this library */
#define DPRINTF_LEVEL		0

/* Buffer size for msvc implementation because it outputs to DebugOutput */
#define DPRINTF_BUF_SZ  1024

/*****************************************************************************
 *  Types used in XviD sources
 ****************************************************************************/

/*----------------------------------------------------------------------------
 | For MSVC
 *---------------------------------------------------------------------------*/

#    define int8_t   char
#    define uint8_t  unsigned char
#    define int16_t  short
#    define uint16_t unsigned short
#    define int32_t  int
#    define uint32_t unsigned int

#if defined(_MSC_VER)
    #if defined(_TI_C6X)
        #    define int64_t  long
        #    define uint64_t unsigned long
        #    define size_t   unsigned int
    #else
        #    define int64_t  __int64
        #    define uint64_t unsigned __int64
        #    define size_t   unsigned int
    #endif
#else
#    define int64_t  long long
#    define uint64_t unsigned long long
#    define size_t   unsigned int
#endif

/*****************************************************************************
 *  Some things that are only architecture dependant
 ****************************************************************************/

#    define CACHE_LINE  16
#    define ptr_t uint32_t



#define BSWAP(a) \
            ((a) = (((a) & 0xff) << 24)  | (((a) & 0xff00) << 8) | \
                   (((a) >> 8) & 0xff00) | (((a) >> 24) & 0xff))


#define DECLARE_ALIGNED_MATRIX(name,sizex,sizey,type,alignment) \
                type name[(sizex)*(sizey)]

#endif
