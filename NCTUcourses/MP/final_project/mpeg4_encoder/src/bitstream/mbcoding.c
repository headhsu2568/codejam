#include "../portab.h"
#include "bitstream.h"
#include "zigzag.h"
#include "mbcoding.h"

#define ABS(X) (((X)>0)?(X):-(X))
#define CLIP(X,A) (X > A) ? (A) : (X)

#define LEVELOFFSET 32

/*****************************************************************************
 * Local inlined functions for MB coding
 ****************************************************************************/

static __inline void
CodeVector(Bitstream * bs,
		   int32_t value,
		   int32_t f_code,
		   Statistics * pStat)
{

	const int scale_factor = 1 << (f_code - 1);
	const int cmp = scale_factor << 5;

	if (value < (-1 * cmp))
		value += 64 * scale_factor;

	if (value > (cmp - 1))
		value -= 64 * scale_factor;

	pStat->iMvSum += value * value;
	pStat->iMvCount++;

	if (value == 0) {
		BitstreamPutBits(bs, mb_motion_table[32].code,
						 mb_motion_table[32].len);
	} else {
		uint16_t length, code, mv_res, sign;

		length = 16 << f_code;
		f_code--;

		sign = (value < 0);

		if (value >= length)
			value -= 2 * length;
		else if (value < -length)
			value += 2 * length;

		if (sign)
			value = -value;

		value--;
		mv_res = value & ((1 << f_code) - 1);
		code = ((value - mv_res) >> f_code) + 1;

		if (sign)
			code = -code;

		code += 32;
		BitstreamPutBits(bs, mb_motion_table[code].code,
						 mb_motion_table[code].len);

		if (f_code)
			BitstreamPutBits(bs, mv_res, f_code);
	}

}

//*********************************************************************************************
//  for parallel program , by gary
//*********************************************************************************************
void
CodeVLCBlockIntra(const MACROBLOCK * pMB,
				  const int16_t qcoeff[64],
		          const uint16_t * zigzag,
				  const int32_t i, 
				  VLC * runlevel,
				  uint32_t *count,
				  int8_t last_address)
{
	int32_t timer=0;
	int32_t write_timer = 0;
    uint32_t abs_level, run, prev_run, code, len;
	int32_t level, prev_level;

	if (i < 4)
	{
		runlevel[write_timer].code = dcy_tab[qcoeff[0] + 255].code;
	    runlevel[write_timer++].len = dcy_tab[qcoeff[0] + 255].len;
		timer++;
	}
	else
	{
		runlevel[write_timer].code = dcc_tab[qcoeff[0] + 255].code;
	    runlevel[write_timer++].len = dcc_tab[qcoeff[0] + 255].len;
		timer++;
	}

	if (pMB->cbp & (1 << (5 - i)))
	{
		run = 0;

	    while (!(level = qcoeff[zigzag[timer++]]))
		{
			run++;
		}

	    prev_level = level;
	    prev_run   = run;
	    run = 0;

		if(last_address > 31)   //last non-zero method .by gary
		{
			while (timer < 64)
			{
				if ((level = qcoeff[zigzag[timer++]]) != 0)
				{
					abs_level = ABS(prev_level);
			        abs_level = abs_level < 64 ? abs_level : 0;
			        code = coeff_VLC[1][0][abs_level][prev_run].code;
			        len	= coeff_VLC[1][0][abs_level][prev_run].len;
			        if (len != 128)
					{
						code |= (prev_level < 0);
					}
			        else
					{
					    code = (ESCAPE3 << 21) | (prev_run << 14) | (1 << 13) | ((prev_level & 0xfff) << 1) | 1;
				        len  = 30;
					}

				    runlevel[write_timer].code = code;
				    runlevel[write_timer++].len = len;

			        prev_level = level;
			        prev_run   = run;
			        run = 0;
				}
		        else
				{
				    run++;
				}
			}
		}
		else
		{
			while (timer < 32)
			{
				if ((level = qcoeff[zigzag[timer++]]) != 0)
				{
					abs_level = ABS(prev_level);
			        abs_level = abs_level < 64 ? abs_level : 0;
			        code = coeff_VLC[1][0][abs_level][prev_run].code;
			        len	= coeff_VLC[1][0][abs_level][prev_run].len;
			        if (len != 128)
					{
						code |= (prev_level < 0);
					}
			        else
					{
					    code = (ESCAPE3 << 21) | (prev_run << 14) | (1 << 13) | ((prev_level & 0xfff) << 1) | 1;
				        len  = 30;
					}

				    runlevel[write_timer].code = code;
				    runlevel[write_timer++].len = len;

			        prev_level = level;
			        prev_run   = run;
			        run = 0;
				}
		        else
				{
				    run++;
				}
			}
		}

	    abs_level = ABS(prev_level);
	    abs_level = abs_level < 64 ? abs_level : 0;
	    code = coeff_VLC[1][1][abs_level][prev_run].code;
	    len	= coeff_VLC[1][1][abs_level][prev_run].len;
	    
		if (len != 128)
		{
			code |= (prev_level < 0);
		}
	    else
		{
			code = (ESCAPE3 << 21) | (1 << 20) | (prev_run << 14) | (1 << 13) | ((prev_level & 0xfff) << 1) | 1;
		    len  = 30;
		}
	    runlevel[write_timer].code = code;
		runlevel[write_timer++].len = len;
	}
	*count = write_timer;
}

void
WriteMBHeaderCoeff(const FRAMEINFO * frame,
				   const MACROBLOCK * pMB,
				   Bitstream * bs,
				   Statistics * pStat,
				   VLC *runlevel,
				   uint32_t *count)
{
	uint32_t i, mcbpc, cbpy, bits, timer;

	cbpy = pMB->cbp >> 2;

	/* write mcbpc */
	if (frame->coding_type == I_VOP) 
	{
		mcbpc = ((pMB->mode >> 1) & 3) | ((pMB->cbp & 3) << 2);
		BitstreamPutBits(bs, mcbpc_intra_tab[mcbpc].code,
						 mcbpc_intra_tab[mcbpc].len);
	} else 
	{
		mcbpc = (pMB->mode & 7) | ((pMB->cbp & 3) << 3);
		BitstreamPutBits(bs, mcbpc_inter_tab[mcbpc].code,
						 mcbpc_inter_tab[mcbpc].len);
	}

	/* ac prediction flag */
	if (pMB->acpred_directions[0])
		BitstreamPutBits(bs, 1, 1);
	else
		BitstreamPutBits(bs, 0, 1);

	/* write cbpy */
	BitstreamPutBits(bs, cbpy_tab[cbpy].code, cbpy_tab[cbpy].len);

	/* write dquant */
	if (pMB->mode == MODE_INTRA_Q)
		BitstreamPutBits(bs, pMB->dquant, 2);

	/* write interlacing */
	if (frame->global_flags & XVID_INTERLACING) 
	{
		BitstreamPutBit(bs, pMB->field_dct);
	}

	for (i = 0; i < 6; i++) 
	{
		timer = 0;
		BitstreamPutBits(bs, runlevel[i*64+timer].code,runlevel[i*64+timer].len);  //write DC
		timer++;

		if(count[i]>1)
		{
			bits = BitstreamPos(bs);
		}

		while(timer < count[i])
		{
			BitstreamPutBits(bs, runlevel[i*64+timer].code,runlevel[i*64+timer].len);  //write AC
			timer++;
		}

		if(count[i]>1)
		{
			bits = BitstreamPos(bs) - bits;
			pStat->iTextBits += bits;
		}
	}
}

void
CodeVLCBlockInter(const MACROBLOCK * pMB,
				  const int16_t qcoeff[64],
		          const uint16_t * zigzag,
				  const int32_t i, 
				  VLC * runlevel,
				  uint32_t *count,
				  int8_t last_address)
{
	int32_t timer=0;
	int32_t write_timer = 0;
    uint32_t run, prev_run, code, len;
	int32_t level, prev_level, level_shifted;

	if (pMB->cbp & (1 << (5 - i)))
	{
		run = 0;

		while (!(level = qcoeff[zigzag[timer++]]))
			run++;

		prev_level = level;
		prev_run   = run;
		run = 0;

		while (timer < last_address+1)  //last non-zero method .by gary
		{
			if ((level = qcoeff[zigzag[timer++]]) != 0)
			{
				level_shifted = prev_level + 32;
				if (!(level_shifted & -64))
				{
					code = coeff_VLC[0][0][level_shifted][prev_run].code;
					len	 = coeff_VLC[0][0][level_shifted][prev_run].len;
				}
				else
				{
					code = (ESCAPE3 << 21) | (prev_run << 14) | (1 << 13) | ((prev_level & 0xfff) << 1) | 1;
					len  = 30;
				}
				runlevel[write_timer].code = code;
				runlevel[write_timer++].len = len;
				prev_level = level;
				prev_run   = run;
				run = 0;
			}
			else
			{
				run++;
			}
		}

		level_shifted = prev_level + 32;
		if (!(level_shifted & -64))
		{
			code = coeff_VLC[0][1][level_shifted][prev_run].code;
			len	 = coeff_VLC[0][1][level_shifted][prev_run].len;
		}
		else
		{
			code = (ESCAPE3 << 21) | (1 << 20) | (prev_run << 14) | (1 << 13) | ((prev_level & 0xfff) << 1) | 1;
			len  = 30;
		}
		runlevel[write_timer].code = code;
	    runlevel[write_timer++].len = len;
	}
	*count = write_timer;
}

void
WriteMBHeaderCoeffPFrame(const FRAMEINFO * frame,
				         const MACROBLOCK * pMB,
				         Bitstream * bs,
				         Statistics * pStat,
				         VLC *runlevel,
				         uint32_t *count)
{
	int32_t i;
	uint32_t bits, mcbpc, cbpy, timer;

	mcbpc = (pMB->mode & 7) | ((pMB->cbp & 3) << 3);
	cbpy = 15 - (pMB->cbp >> 2);

	/* write mcbpc */
	BitstreamPutBits(bs, mcbpc_inter_tab[mcbpc].code,
					 mcbpc_inter_tab[mcbpc].len);

	/* write cbpy */
	BitstreamPutBits(bs, cbpy_tab[cbpy].code, cbpy_tab[cbpy].len);

	/* write dquant */
	if (pMB->mode == MODE_INTER_Q)
		BitstreamPutBits(bs, pMB->dquant, 2);

	/* interlacing */
	if (frame->global_flags & XVID_INTERLACING) {
		if (pMB->cbp) {
			BitstreamPutBit(bs, pMB->field_dct);
		}

		/* if inter block, write field ME flag */
		if (pMB->mode == MODE_INTER || pMB->mode == MODE_INTER_Q) {
			BitstreamPutBit(bs, pMB->field_pred);

			/* write field prediction references */
			if (pMB->field_pred) {
				BitstreamPutBit(bs, pMB->field_for_top);
				BitstreamPutBit(bs, pMB->field_for_bot);
			}
		}
	}
	/* code motion vector(s) */
	for (i = 0; i < (pMB->mode == MODE_INTER4V ? 4 : 1); i++) {
		CodeVector(bs, pMB->pmvs[i].x, frame->fcode, pStat);
		CodeVector(bs, pMB->pmvs[i].y, frame->fcode, pStat);
	}

	bits = BitstreamPos(bs);

	for (i = 0; i < 6; i++) 
	{
		timer = 0;

		while(timer < count[i])
		{
			BitstreamPutBits(bs, runlevel[i*64+timer].code,runlevel[i*64+timer].len);  //write AC
			timer++;
		}
	}

	bits = BitstreamPos(bs) - bits;
	pStat->iTextBits += bits;
}
/*****************************************************************************
 * Macro Block bitstream encoding functions
 ****************************************************************************/

void
MBSkip(Bitstream * bs)
{
	BitstreamPutBit(bs, 1);	/* not coded */
	return;
}

/*****************************************************************************
 * decoding stuff starts here
 ****************************************************************************/

/*
 * For IVOP addbits == 0
 * For PVOP addbits == fcode - 1
 * For BVOP addbits == max(fcode,bcode) - 1
 * returns true or false
 */

int 
check_resync_marker(Bitstream * bs, int addbits)
{
	uint32_t nbits;
	uint32_t code;
	uint32_t nbitsresyncmarker = NUMBITS_VP_RESYNC_MARKER + addbits;

	nbits = BitstreamNumBitsToByteAlign(bs);
	code = BitstreamShowBits(bs, nbits);

	if (code == (((uint32_t)1 << (nbits - 1)) - 1))
	{
		return BitstreamShowBitsFromByteAlign(bs, nbitsresyncmarker) == RESYNC_MARKER;
	}

	return 0;
}



int
get_mcbpc_intra(Bitstream * bs)
{

	uint32_t index;

	index = BitstreamShowBits(bs, 9);
	index >>= 3;

	BitstreamSkip(bs, mcbpc_intra_table[index].len);

	return mcbpc_intra_table[index].code;

}

int
get_mcbpc_inter(Bitstream * bs)
{

	uint32_t index;
	
	index = CLIP(BitstreamShowBits(bs, 9), 256);

	BitstreamSkip(bs, mcbpc_inter_table[index].len);

	return mcbpc_inter_table[index].code;

}

int
get_cbpy(Bitstream * bs,
		 int intra)
{

	int cbpy;
	uint32_t index = BitstreamShowBits(bs, 6);

	BitstreamSkip(bs, cbpy_table[index].len);
	cbpy = cbpy_table[index].code;

	if (!intra)
		cbpy = 15 - cbpy;

	return cbpy;

}

int
get_mv_data(Bitstream * bs)
{

	uint32_t index;

	if (BitstreamGetBit(bs))
		return 0;

	index = BitstreamShowBits(bs, 12);

	if (index >= 512) {
		index = (index >> 8) - 2;
		BitstreamSkip(bs, TMNMVtab0[index].len);
		return TMNMVtab0[index].code;
	}

	if (index >= 128) {
		index = (index >> 2) - 32;
		BitstreamSkip(bs, TMNMVtab1[index].len);
		return TMNMVtab1[index].code;
	}

	index -= 4;

	BitstreamSkip(bs, TMNMVtab2[index].len);
	return TMNMVtab2[index].code;

}

int
get_mv(Bitstream * bs,
	   int fcode)
{

	int data;
	int res;
	int mv;
	int scale_fac = 1 << (fcode - 1);

	data = get_mv_data(bs);

	if (scale_fac == 1 || data == 0)
		return data;

	res = BitstreamGetBits(bs, fcode - 1);
	mv = ((ABS(data) - 1) * scale_fac) + res + 1;

	return data < 0 ? -mv : mv;

}

int
get_dc_dif(Bitstream * bs,
		   uint32_t dc_size)
{

	int code = BitstreamGetBits(bs, dc_size);
	int msb = code >> (dc_size - 1);

	if (msb == 0)
		return (-1 * (code ^ ((1 << dc_size) - 1)));

	return code;

}

int
get_dc_size_lum(Bitstream * bs)
{

	int code, i;

	code = BitstreamShowBits(bs, 11);

	for (i = 11; i > 3; i--) {
		if (code == 1) {
			BitstreamSkip(bs, i);
			return i + 1;
		}
		code >>= 1;
	}

	BitstreamSkip(bs, dc_lum_tab[code].len);
	return dc_lum_tab[code].code;

}


int
get_dc_size_chrom(Bitstream * bs)
{

	uint32_t code, i;

	code = BitstreamShowBits(bs, 12);

	for (i = 12; i > 2; i--) {
		if (code == 1) {
			BitstreamSkip(bs, i);
			return i;
		}
		code >>= 1;
	}

	return 3 - BitstreamGetBits(bs, 2);

}

/*****************************************************************************
 * Local inlined function to "decode" written vlc codes
 ****************************************************************************/

static __inline int
get_coeff(Bitstream * bs,
		  int *run,
		  int *last,
		  int intra,
		  int short_video_header)
{

	uint32_t mode;
	int32_t level;
	REVERSE_EVENT *reverse_event;

	if (short_video_header)		/* inter-VLCs will be used for both intra and inter blocks */
		intra = 0;

	if (BitstreamShowBits(bs, 7) != ESCAPE) {
		reverse_event = &DCT3D[intra][BitstreamShowBits(bs, 12)];

		if ((level = reverse_event->event.level) == 0)
			goto error;

		*last = reverse_event->event.last;
		*run  = reverse_event->event.run;

		BitstreamSkip(bs, reverse_event->len);

		return BitstreamGetBits(bs, 1) ? -level : level;
	}

	BitstreamSkip(bs, 7);

	if (short_video_header) {
		/* escape mode 4 - H.263 type, only used if short_video_header = 1  */
		*last = BitstreamGetBit(bs);
		*run = BitstreamGetBits(bs, 6);
		level = BitstreamGetBits(bs, 8);

		return (level << 24) >> 24;
	}

	mode = BitstreamShowBits(bs, 2);

	if (mode < 3) {
		BitstreamSkip(bs, (mode == 2) ? 2 : 1);

		reverse_event = &DCT3D[intra][BitstreamShowBits(bs, 12)];

		if ((level = reverse_event->event.level) == 0)
			goto error;

		*last = reverse_event->event.last;
		*run  = reverse_event->event.run;

		BitstreamSkip(bs, reverse_event->len);

		if (mode < 2)			/* first escape mode, level is offset */
			level += max_level[intra][*last][*run];
		else					/* second escape mode, run is offset */
			*run += max_run[intra][*last][level] + 1;

		return BitstreamGetBits(bs, 1) ? -level : level;
	}

	/* third escape mode - fixed length codes */
	BitstreamSkip(bs, 2);
	*last = BitstreamGetBits(bs, 1);
	*run = BitstreamGetBits(bs, 6);
	BitstreamSkip(bs, 1);		/* marker */
	level = BitstreamGetBits(bs, 12);
	BitstreamSkip(bs, 1);		/* marker */

	return (level << 20) >> 20;

  error:
	*run = VLC_ERROR;
	return 0;
}

/*****************************************************************************
 * MB reading functions
 ****************************************************************************/

void
get_intra_block(Bitstream * bs,
				int16_t * block,
				int direction,
				int coeff)
{

	const uint16_t *scan = scan_tables[direction];
	int level;
	int run;
	int last;

	do {
		level = get_coeff(bs, &run, &last, 1, 0);
		if (run == -1) {
			break;
		}
		coeff += run;
		block[scan[coeff]] = level;

		coeff++;
	} while (!last);

}

void
get_inter_block(Bitstream * bs,
				int16_t * block)
{

	const uint16_t *scan = scan_tables[0];
	int p;
	int level;
	int run;
	int last;

	p = 0;
	do {
		level = get_coeff(bs, &run, &last, 0, 0);
		if (run == -1) {
			break;
		}
		p += run;

		block[scan[p]] = level;

		p++;
	} while (!last);

}
