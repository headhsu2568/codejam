#include "encoder.h"
#include "prediction/mbprediction.h"
#include "global.h"
#include "image/image.h"
#include "motion/motion.h"
#include "bitstream/cbp.h"
//#include "utils/mbfunctions.h"
#include "bitstream/bitstream.h"
#include "bitstream/mbcoding.h"
#include "utils/ratecontrol.h"
#include "bitstream/mbcoding.h"
#include "quant/quant_matrix.h"
#include "utils/mem_align.h"

//MBTransQuantIntra() function by gary
#include "utils/mem_transfer.h"
#include "dct/fdct.h"
#include "dct/idct.h"
#include "quant/quant_h263.h"

//prediction() function by gary
#include "prediction/mbprediction.h"

//MBCoding() function by gary
#include "bitstream/vlc_codes.h"
#include "bitstream/zigzag.h"

//MBMotionCompensation() function by gary
#include "motion/motion.h"
#include "image/interpolate8x8.h"

/*****************************************************************************
 * Local macros
 ****************************************************************************/

#define ENC_CHECK(X) if(!(X)) return XVID_ERR_FORMAT
#define SWAP(A,B)    { void * tmp = A; A = B; B = tmp; }

/*****************************************************************************
 * Local function prototypes
 ****************************************************************************/

static int FrameCodeI(Encoder * pEnc,
					  Bitstream * bs,
					  uint32_t * pBits);

static int FrameCodeP(Encoder * pEnc,
					  Bitstream * bs,
					  uint32_t * pBits,
					  bool vol_header);

#ifdef _DEBUG_PSNR
static float sum_psnr = 0.0;
#endif

/*****************************************************************************
 * Local data
 ****************************************************************************/

static int DQtab[4] = {
	-1, -2, 1, 2
};

static int iDQtab[5] = {
	1, 0, NO_CHANGE, 2, 3
};


static void __inline
image_null(IMAGE * image)
{
	image->y = image->u = image->v = NULL;
}


/*****************************************************************************
 * Encoder creation
 *
 * This function creates an Encoder instance, it allocates all necessary
 * image buffers (reference, current) and initialize the internal xvid
 * encoder paremeters according to the XVID_ENC_PARAM input parameter.
 *
 * The code seems to be very long but is very basic, mainly memory allocation
 * and cleaning code.
 *
 * Returned values :
 *    - XVID_ERR_OK     - no errors
 *    - XVID_ERR_MEMORY - the libc could not allocate memory, the function
 *                        cleans the structure before exiting.
 *                        pParam->handle is also set to NULL.
 *
 ****************************************************************************/

int
encoder_create(XVID_ENC_PARAM * pParam)
{
	Encoder *pEnc;
	int i;

	pParam->handle = NULL;

	ENC_CHECK(pParam);

	ENC_CHECK(pParam->width > 0 && pParam->width <= 1920);
	ENC_CHECK(pParam->height > 0 && pParam->height <= 1280);
	ENC_CHECK(!(pParam->width % 2));
	ENC_CHECK(!(pParam->height % 2));

	/* Fps */

	if (pParam->fincr <= 0 || pParam->fbase <= 0) {
		pParam->fincr = 1;
		pParam->fbase = 25;
	}

	/*
	 * Simplify the "fincr/fbase" fraction
	 * (neccessary, since windows supplies us with huge numbers)
	 */

	i = pParam->fincr;
	while (i > 1) {
		if (pParam->fincr % i == 0 && pParam->fbase % i == 0) {
			pParam->fincr /= i;
			pParam->fbase /= i;
			i = pParam->fincr;
			continue;
		}
		i--;
	}

	if (pParam->fbase > 65535) {
		float div = (float) pParam->fbase / 65535;

		pParam->fbase = (int) (pParam->fbase / div);
		pParam->fincr = (int) (pParam->fincr / div);
	}

	/* Bitrate allocator defaults */

	if (pParam->rc_bitrate <= 0)
		pParam->rc_bitrate = 900000;

	if (pParam->rc_reaction_delay_factor <= 0)
		pParam->rc_reaction_delay_factor = 16;

	if (pParam->rc_averaging_period <= 0)
		pParam->rc_averaging_period = 100;

	if (pParam->rc_buffer <= 0)
		pParam->rc_buffer = 100;

	/* Max and min quantizers */

	if ((pParam->min_quantizer <= 0) || (pParam->min_quantizer > 31))
		pParam->min_quantizer = 1;

	if ((pParam->max_quantizer <= 0) || (pParam->max_quantizer > 31))
		pParam->max_quantizer = 31;

	if (pParam->max_quantizer < pParam->min_quantizer)
		pParam->max_quantizer = pParam->min_quantizer;

	/* 1 keyframe each 10 seconds */

	if (pParam->max_key_interval <= 0)
		pParam->max_key_interval = 10 * pParam->fincr / pParam->fbase;

	pEnc = (Encoder *) xvid_malloc(sizeof(Encoder), CACHE_LINE);
	if (pEnc == NULL)
		return XVID_ERR_MEMORY;

	/* Zero the Encoder Structure */

	my_memset(pEnc, 0, sizeof(Encoder));

	/* Fill members of Encoder structure */

	pEnc->mbParam.width = pParam->width;
	pEnc->mbParam.height = pParam->height;

	pEnc->mbParam.mb_width = (pEnc->mbParam.width + 15) / 16;
	pEnc->mbParam.mb_height = (pEnc->mbParam.height + 15) / 16;

	pEnc->mbParam.edged_width = 16 * pEnc->mbParam.mb_width + 2 * EDGE_SIZE;
	pEnc->mbParam.edged_height = 16 * pEnc->mbParam.mb_height + 2 * EDGE_SIZE;

	pEnc->mbParam.fbase = pParam->fbase;
	pEnc->mbParam.fincr = pParam->fincr;

	pEnc->mbParam.m_quant_type = H263_QUANT;

	pEnc->sStat.fMvPrevSigma = -1;

	/* Fill rate control parameters */

	pEnc->bitrate = pParam->rc_bitrate;

	pEnc->iFrameNum = 0;
	pEnc->iMaxKeyInterval = pParam->max_key_interval;

	/* try to allocate frame memory */

	pEnc->current = xvid_malloc(sizeof(FRAMEINFO), CACHE_LINE);
	pEnc->reference = xvid_malloc(sizeof(FRAMEINFO), CACHE_LINE);

	if (pEnc->current == NULL || pEnc->reference == NULL)
		goto xvid_err_memory1;

	/* try to allocate mb memory */

	pEnc->current->mbs =
		xvid_malloc(sizeof(MACROBLOCK) * pEnc->mbParam.mb_width *
					pEnc->mbParam.mb_height, CACHE_LINE);
	pEnc->reference->mbs =
		xvid_malloc(sizeof(MACROBLOCK) * pEnc->mbParam.mb_width *
					pEnc->mbParam.mb_height, CACHE_LINE);

	if (pEnc->current->mbs == NULL || pEnc->reference->mbs == NULL)
		goto xvid_err_memory2;

	/* try to allocate image memory */

#ifdef _DEBUG_PSNR
	image_null(&pEnc->sOriginal);
#endif
	image_null(&pEnc->current->image);
	image_null(&pEnc->reference->image);
	image_null(&pEnc->vInterH);
	image_null(&pEnc->vInterV);
	image_null(&pEnc->vInterHV);

#ifdef _DEBUG_PSNR
	if (image_create
		(&pEnc->sOriginal, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
#endif
	if (image_create
		(&pEnc->current->image, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
	if (image_create
		(&pEnc->reference->image, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
	if (image_create
		(&pEnc->vInterH, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
	if (image_create
		(&pEnc->vInterV, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
	if (image_create
		(&pEnc->vInterHV, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;

	pParam->handle = (void *) pEnc;

	if (pParam->rc_bitrate) {
		RateControlInit(&pEnc->rate_control, pParam->rc_bitrate,
						pParam->rc_reaction_delay_factor,
						pParam->rc_averaging_period, pParam->rc_buffer,
						pParam->fbase * 1000 / pParam->fincr,
						pParam->max_quantizer, pParam->min_quantizer);
	}

	return XVID_ERR_OK;

	/*
	 * We handle all XVID_ERR_MEMORY here, this makes the code lighter
	 */

  xvid_err_memory3:
#ifdef _DEBUG_PSNR
	image_destroy(&pEnc->sOriginal, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
#endif

	image_destroy(&pEnc->current->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->reference->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterH, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterHV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);

  xvid_err_memory2:
	xvid_free(pEnc->current->mbs);
	xvid_free(pEnc->reference->mbs);

  xvid_err_memory1:
	xvid_free(pEnc->current);
	xvid_free(pEnc->reference);
	xvid_free(pEnc);

	pParam->handle = NULL;

	return XVID_ERR_MEMORY;
}

/*****************************************************************************
 * Encoder destruction
 *
 * This function destroy the entire encoder structure created by a previous
 * successful encoder_create call.
 *
 * Returned values (for now only one returned value) :
 *    - XVID_ERR_OK     - no errors
 *
 ****************************************************************************/

int
encoder_destroy(Encoder * pEnc)
{
	
	ENC_CHECK(pEnc);

	/* All images, reference, current etc ... */
	image_destroy(&pEnc->current->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->reference->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterH, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterHV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);

#ifdef _DEBUG_PSNR
	image_destroy(&pEnc->sOriginal, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
#endif

	/* Encoder structure */
	xvid_free(pEnc->current->mbs);
	xvid_free(pEnc->current);

	xvid_free(pEnc->reference->mbs);
	xvid_free(pEnc->reference);

	xvid_free(pEnc);

	return XVID_ERR_OK;
}


void inc_frame_num(Encoder * pEnc)
{
	pEnc->mbParam.m_ticks += pEnc->mbParam.fincr;
	pEnc->mbParam.m_seconds = pEnc->mbParam.m_ticks / pEnc->mbParam.fbase;
	pEnc->mbParam.m_ticks = pEnc->mbParam.m_ticks % pEnc->mbParam.fbase;
}

/*****************************************************************************
 * "original" IP frame encoder entry point
 *
 * Returned values :
 *    - XVID_ERR_OK     - no errors
 *    - XVID_ERR_FORMAT - the image subsystem reported the image had a wrong
 *                        format
 ****************************************************************************/

int
encoder_encode(Encoder * pEnc,
			   XVID_ENC_FRAME * pFrame,
			   XVID_ENC_STATS * pResult)
{
	Bitstream bs;
	uint32_t bits;
	unsigned short write_vol_header = 0;

#ifdef _DEBUG_PSNR
	float psnr;
#endif

	ENC_CHECK(pEnc);
	ENC_CHECK(pFrame);
	ENC_CHECK(pFrame->bitstream);
	ENC_CHECK(pFrame->image);

	SWAP(pEnc->current, pEnc->reference);

	pEnc->current->global_flags = pFrame->general;
	pEnc->current->motion_flags = pFrame->motion;
	pEnc->current->seconds = pEnc->mbParam.m_seconds;
	pEnc->current->ticks = pEnc->mbParam.m_ticks;
	pEnc->mbParam.hint = &pFrame->hint;

	// ** conv ** 
	yuv_to_yv12(pEnc->current->image.y, 
		        pEnc->current->image.u, 
				pEnc->current->image.v, 
				pFrame->image, 
				pEnc->mbParam.width, 
				pEnc->mbParam.height,
				pEnc->mbParam.edged_width);
	// ** stop conv **

#ifdef _DEBUG_PSNR
	image_copy(&pEnc->sOriginal, &pEnc->current->image,
			   pEnc->mbParam.edged_width, pEnc->mbParam.height);
#endif

	BitstreamInit(&bs, pFrame->bitstream, 0);

	if (pFrame->quant == 0) {
		pEnc->current->quant = RateControlGetQ(&pEnc->rate_control, 0);
	} else {
		pEnc->current->quant = pFrame->quant;
	}

	if (pEnc->mbParam.m_quant_type != H263_QUANT)
			write_vol_header = 1;
	pEnc->mbParam.m_quant_type = H263_QUANT;

	if (pFrame->intra == 1) {
		pFrame->intra = FrameCodeI(pEnc, &bs, &bits);
	} else {
		pFrame->intra = FrameCodeP(pEnc, &bs, &bits, write_vol_header);
	}

	BitstreamPutBits(&bs, 0xFFFF, 16);
	BitstreamPutBits(&bs, 0xFFFF, 16);
	BitstreamPad(&bs);
	pFrame->length = BitstreamLength(&bs);

	if (pResult) {
		pResult->quant = pEnc->current->quant;
		pResult->hlength = pFrame->length - (pEnc->sStat.iTextBits / 8);
		pResult->kblks = pEnc->sStat.kblks;
		pResult->mblks = pEnc->sStat.mblks;
		pResult->ublks = pEnc->sStat.ublks;
	}


	if (pFrame->quant == 0) {
		RateControlUpdate(&pEnc->rate_control, (short)pEnc->current->quant,
						  pFrame->length, pFrame->intra);
	}
#ifdef _DEBUG_PSNR
	psnr =
		image_psnr(&pEnc->sOriginal, &pEnc->current->image,
				   pEnc->mbParam.edged_width, pEnc->mbParam.width,
				   pEnc->mbParam.height);

	my_printf("PSNR: %f\n", psnr);
	sum_psnr += psnr;
	my_printf("Total PSNR: %f\n", sum_psnr);
#endif

	inc_frame_num(pEnc);
	pEnc->iFrameNum++;

	return XVID_ERR_OK;
}


static __inline void
CodeIntraMB(Encoder * pEnc,
			MACROBLOCK * pMB)
{

	pMB->mode = MODE_INTRA;

	/* zero mv statistics */
	pMB->mvs[0].x = pMB->mvs[1].x = pMB->mvs[2].x = pMB->mvs[3].x = 0;
	pMB->mvs[0].y = pMB->mvs[1].y = pMB->mvs[2].y = pMB->mvs[3].y = 0;
	pMB->sad8[0] = pMB->sad8[1] = pMB->sad8[2] = pMB->sad8[3] = 0;
	pMB->sad16 = 0;
	pMB->cbp = 0;

	if ((pEnc->current->global_flags & XVID_LUMIMASKING)) {
		if (pMB->dquant != NO_CHANGE) {
			pMB->mode = MODE_INTRA_Q;
			pEnc->current->quant += DQtab[pMB->dquant];

			if (pEnc->current->quant > 31)
				pEnc->current->quant = 31;
			if (pEnc->current->quant < 1)
				pEnc->current->quant = 1;
		}
	}

	pMB->quant = pEnc->current->quant;
}


static int
FrameCodeI(Encoder * pEnc,
		   Bitstream * bs,
		   uint32_t * pBits)
{
    // ** MBTransQuantIntra add **
	uint32_t stride = pEnc->mbParam.edged_width;
	uint32_t stride2 = stride / 2;
	uint32_t next_block = stride * 8;
	uint32_t iQuant = pEnc->current->quant;
	uint8_t *pY_Cur, *pU_Cur, *pV_Cur;
	IMAGE *pCurrent = &pEnc->current->image;
	// ** end **

	// ** MBPrediction add **
	int32_t iDcScaler;
	int16_t predictors[6][8];
	// ** end **

	// ** last non-zero method **
	int8_t last_address[6];
	// ** end **

	// ** prediction **
	int32_t sum_pred;    
	// ** end **

	// ** zero skip method **
	uint8_t DCT_flag[6];
	// ** end **

	DECLARE_ALIGNED_MATRIX(dct_codes, 6, 64, short, CACHE_LINE);
	DECLARE_ALIGNED_MATRIX(qcoeff, 6, 64, short, CACHE_LINE);
	DECLARE_ALIGNED_MATRIX(runlevel, 6, 64, VLC, CACHE_LINE);      //vlc
	DECLARE_ALIGNED_MATRIX(count, 6, 1, unsigned int, CACHE_LINE); //vlc

	unsigned short x, y;

	pEnc->iFrameNum = 0;
	pEnc->mbParam.m_rounding_type = 1;
	pEnc->current->rounding_type = pEnc->mbParam.m_rounding_type;
	pEnc->current->coding_type = I_VOP;

	BitstreamWriteVolHeader(bs, &pEnc->mbParam, pEnc->current);

	BitstreamWriteVopHeader(bs, &pEnc->mbParam, pEnc->current, 1);

	*pBits = BitstreamPos(bs);

	pEnc->sStat.iTextBits = 0;
	pEnc->sStat.kblks = pEnc->mbParam.mb_width * pEnc->mbParam.mb_height;
	pEnc->sStat.mblks = pEnc->sStat.ublks = 0;

	for (y = 0; y < pEnc->mbParam.mb_height; y++)
	{
		for (x = 0; x < pEnc->mbParam.mb_width; x++) 
		{
			MACROBLOCK *pMB =
				&pEnc->current->mbs[x + y * pEnc->mbParam.mb_width];

			CodeIntraMB(pEnc, pMB);

			iQuant = pEnc->current->quant;


            pY_Cur = pCurrent->y + (y << 4) * stride + (x << 4);
	        pU_Cur = pCurrent->u + (y << 3) * stride2 + (x << 3);
	        pV_Cur = pCurrent->v + (y << 3) * stride2 + (x << 3);

		// ** block0 **
            
			//** transfer **
			transfer_8to16copy(&dct_codes[0 * 64], pY_Cur, stride);
			//** stop transfer **

			iDcScaler = get_dc_scaler(iQuant, 1);

			//** dct **
		    fdct(&dct_codes[0 * 64]);
		    //** stop dct **

			//** quant **
			quant_intra(&qcoeff[0 * 64], &last_address[0], &DCT_flag[0], &dct_codes[0 * 64], iQuant, iDcScaler);
		    //** stop quant **

			//** iquant **
		    dequant_intra(&dct_codes[0 * 64], last_address[0], &qcoeff[0 * 64], iQuant, iDcScaler);
			//** stop iquant **

			//** idct **
		    idct(&dct_codes[0 * 64],last_address[0]>>3,DCT_flag[0]);
		    //** stop idct **

			//** transfer **
	        transfer_16to8copy(pY_Cur, &dct_codes[0 * 64], stride);
			//** stop transfer **

			//** prediction **
			predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 0, &qcoeff[0 * 64],
						           iQuant, iDcScaler, predictors[0], 0);

			sum_pred = calc_acdc(pMB, 0, &qcoeff[0 * 64], iDcScaler, predictors[0]);

			if(sum_pred < 0)
			{
				pMB->acpred_directions[0] = 0;
			}
			else
			{
				apply_acdc(pMB, 0, &qcoeff[0 * 64], predictors[0]);
			}
			//** end prediction **

			//** CBP **
			pMB->cbp = single_calc_cbp(&qcoeff[0 * 64],pMB->cbp,0);
            //** end CBP **

			// ** coding **
			CodeVLCBlockIntra(pMB,&qcoeff[0*64],scan_tables[pMB->acpred_directions[0]],0,&runlevel[0 * 64],&count[0],last_address[0]);
			// ** stop coding **
			
		// ** end block0 **

		// ** block1 **
            
			//** transfer **
			transfer_8to16copy(&dct_codes[1 * 64], pY_Cur + 8, stride);
			//** stop transfer **

			iDcScaler = get_dc_scaler(iQuant, 1);

			//** dct **
		    fdct(&dct_codes[1 * 64]);
		    //** stop dct **

			//** quant **
			quant_intra(&qcoeff[1 * 64], &last_address[1], &DCT_flag[1], &dct_codes[1 * 64], iQuant, iDcScaler);
		    //** stop quant **

			//** iquant **
		    dequant_intra(&dct_codes[1 * 64], last_address[1], &qcoeff[1 * 64], iQuant, iDcScaler);
			//** stop iquant **

			//** idct **
		    idct(&dct_codes[1 * 64],last_address[1]>>3,DCT_flag[1]);
		    //** stop idct **

			//** transfer **
	        transfer_16to8copy(pY_Cur+8, &dct_codes[1 * 64], stride);
			//** stop transfer **

			//** prediction **
            predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 1, &qcoeff[1 * 64],
						           iQuant, iDcScaler, predictors[1], 0);
			
			calc_acdc(pMB, 1, &qcoeff[1 * 64], iDcScaler, predictors[1]);

			if(sum_pred < 0)
			{
				pMB->acpred_directions[1] = 0;
			}
			else
			{				
				apply_acdc(pMB, 1, &qcoeff[1 * 64], predictors[1]);
			}
			//** end prediction **

			//** CBP **
			pMB->cbp = single_calc_cbp(&qcoeff[1 * 64],pMB->cbp,1);
            //** end CBP **

			// ** coding **
			CodeVLCBlockIntra(pMB,&qcoeff[1*64],scan_tables[pMB->acpred_directions[1]],1,&runlevel[1 * 64],&count[1],last_address[1]);
			// ** stop coding **
			
		// ** end block1 **

		// ** block2 **
            
			//** transfer **
			transfer_8to16copy(&dct_codes[2 * 64], pY_Cur + next_block, stride);
			//** stop transfer **

			iDcScaler = get_dc_scaler(iQuant, 1);

			//** dct **
		    fdct(&dct_codes[2 * 64]);
		    //** stop dct **

			//** quant **
			quant_intra(&qcoeff[2 * 64], &last_address[2], &DCT_flag[2], &dct_codes[2 * 64], iQuant, iDcScaler);
		    //** stop quant **

			//** iquant **
		    dequant_intra(&dct_codes[2 * 64], last_address[2], &qcoeff[2 * 64], iQuant, iDcScaler);
			//** stop iquant **

			//** idct **
		    idct(&dct_codes[2 * 64],last_address[2]>>3,DCT_flag[2]);
		    //** stop idct **

			//** transfer **
	        transfer_16to8copy(pY_Cur + next_block, &dct_codes[2 * 64], stride);
			//** stop transfer **

			//** prediction **
            predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 2, &qcoeff[2 * 64],
						           iQuant, iDcScaler, predictors[2], 0);

			calc_acdc(pMB, 2, &qcoeff[2 * 64], iDcScaler, predictors[2]);
			
			if(sum_pred < 0)
			{
				pMB->acpred_directions[2] = 0;
			}
			else
			{			
				apply_acdc(pMB, 2, &qcoeff[2 * 64], predictors[2]);
			}
			//** end prediction **

			//** CBP **
			pMB->cbp = single_calc_cbp(&qcoeff[2 * 64],pMB->cbp,2);
            //** end CBP **

			// ** coding **
			CodeVLCBlockIntra(pMB,&qcoeff[2*64],scan_tables[pMB->acpred_directions[2]],2,&runlevel[2 * 64],&count[2],last_address[2]);
			// ** stop coding **
			
		// ** end block2 **

		// ** block3 **
            
			//** transfer **
			transfer_8to16copy(&dct_codes[3 * 64], pY_Cur + next_block + 8, stride);
			//** stop transfer **

			iDcScaler = get_dc_scaler(iQuant, 1);

			//** dct **
		    fdct(&dct_codes[3 * 64]);
		    //** stop dct **

			//** quant **
		    quant_intra(&qcoeff[3 * 64], &last_address[3], &DCT_flag[3], &dct_codes[3 * 64], iQuant, iDcScaler);
		    //** stop quant **

			//** iquant **
		    dequant_intra(&dct_codes[3 * 64], last_address[3], &qcoeff[3 * 64], iQuant, iDcScaler);
			//** stop iquant **

			//** idct **
		    idct(&dct_codes[3 * 64],last_address[3]>>3,DCT_flag[3]);
		    //** stop idct **

			//** transfer **
	        transfer_16to8copy(pY_Cur + next_block + 8, &dct_codes[3 * 64], stride);
			//** stop transfer **

			//** prediction **
            predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 3, &qcoeff[3 * 64],
						           iQuant, iDcScaler, predictors[3], 0);

			calc_acdc(pMB, 3, &qcoeff[3 * 64], iDcScaler, predictors[3]);
			
			if(sum_pred < 0)
			{
				pMB->acpred_directions[3] = 0;
			}
			else
			{
				apply_acdc(pMB, 3, &qcoeff[3 * 64], predictors[3]);
			}
			//** end prediction **

			//** CBP **
			pMB->cbp = single_calc_cbp(&qcoeff[3 * 64],pMB->cbp,3);
            //** end CBP **

			// ** coding **
			CodeVLCBlockIntra(pMB,&qcoeff[3*64],scan_tables[pMB->acpred_directions[3]],3,&runlevel[3 * 64],&count[3],last_address[3]);
			// ** stop coding **
			
		// ** end block3 **

	    // ** block4 **
            
			//** transfer **
			transfer_8to16copy(&dct_codes[4 * 64], pU_Cur, stride2);
			//** stop transfer **

			iDcScaler = get_dc_scaler(iQuant, 0);

			//** dct **
		    fdct(&dct_codes[4 * 64]);
		    //** stop dct **

			//** quant **
			quant_intra(&qcoeff[4 * 64], &last_address[4], &DCT_flag[4], &dct_codes[4 * 64], iQuant, iDcScaler);
		    //** stop quant **

			//** iquant **
		    dequant_intra(&dct_codes[4 * 64], last_address[4], &qcoeff[4 * 64], iQuant, iDcScaler);
			//** stop iquant **

			//** idct **
		    idct(&dct_codes[4 * 64],last_address[4]>>3,DCT_flag[4]);
		    //** stop idct **

			//** transfer **
	        transfer_16to8copy(pU_Cur, &dct_codes[4 * 64], stride2);
			//** stop transfer **

			//** prediction **
            predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 4, &qcoeff[4 * 64],
						           iQuant, iDcScaler, predictors[4], 0);

			calc_acdc(pMB, 4, &qcoeff[4 * 64], iDcScaler, predictors[4]);
			
			if(sum_pred < 0)
			{
				pMB->acpred_directions[4] = 0;
			}
			else
			{			
			    apply_acdc(pMB, 4, &qcoeff[4 * 64], predictors[4]);
			}
			//** end prediction **

			//** CBP **
			pMB->cbp = single_calc_cbp(&qcoeff[4 * 64],pMB->cbp,4);
            //** end CBP **

			// ** coding **
			if (pEnc->current->global_flags & XVID_GREYSCALE)
			{	
				pMB->cbp &= 0x3C;		// keep only bits 5-2 
				qcoeff[4*64+0]=0;		// zero, because for INTRA MBs DC value is saved 
			}

			CodeVLCBlockIntra(pMB,&qcoeff[4*64],scan_tables[pMB->acpred_directions[4]],4,&runlevel[4 * 64],&count[4],last_address[4]);
			// ** stop coding **
		
		// ** end block4 **

		// ** block5 **
            
			//** transfer **
			transfer_8to16copy(&dct_codes[5 * 64], pV_Cur, stride2);
			//** stop transfer **

			iDcScaler = get_dc_scaler(iQuant, 0);

			//** dct **
		    fdct(&dct_codes[5 * 64]);
		    //** stop dct **

			//** quant **
			quant_intra(&qcoeff[5 * 64], &last_address[5], &DCT_flag[5], &dct_codes[5 * 64], iQuant, iDcScaler);
		    //** stop quant **

			//** iquant **
		    dequant_intra(&dct_codes[5 * 64], last_address[5], &qcoeff[5 * 64], iQuant, iDcScaler);
			//** stop iquant **

			//** idct **
		    idct(&dct_codes[5 * 64],last_address[5]>>3,DCT_flag[5]);
		    //** stop idct **

			//** transfer **
	        transfer_16to8copy(pV_Cur, &dct_codes[5 * 64], stride2);
			//** stop transfer **

			//** prediction **
            predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 5, &qcoeff[5 * 64],
						           iQuant, iDcScaler, predictors[5], 0);	
			
			calc_acdc(pMB, 5, &qcoeff[5 * 64], iDcScaler, predictors[5]);

			if(sum_pred < 0)
			{
				pMB->acpred_directions[5] = 0;
			}
			else
			{	
			    apply_acdc(pMB, 5, &qcoeff[5 * 64], predictors[5]);
			}
			//** end prediction **

			//** CBP **
			pMB->cbp = single_calc_cbp(&qcoeff[5 * 64],pMB->cbp,5);
            //** end CBP **

			// ** coding **
			if (pEnc->current->global_flags & XVID_GREYSCALE)
			{	
				qcoeff[5*64+0]=0;
			}

			CodeVLCBlockIntra(pMB,&qcoeff[5*64],scan_tables[pMB->acpred_directions[5]],5,&runlevel[5 * 64],&count[5],last_address[5]);
			// ** stop coding **

		// ** end block5 **

			// ** write header and coefficient **
			WriteMBHeaderCoeff(pEnc->current,pMB,bs,&pEnc->sStat,runlevel,count);
			// ** end to write header and coefficient **
		}
	}

	*pBits = BitstreamPos(bs) - *pBits;
	pEnc->sStat.fMvPrevSigma = -1;
	pEnc->sStat.iMvSum = 0;
	pEnc->sStat.iMvCount = 0;
	pEnc->mbParam.m_fcode = 2;

	return 1;					// intra 
}

#define INTRA_THRESHOLD 0.5

static int
FrameCodeP(Encoder * pEnc,
		   Bitstream * bs,
		   uint32_t * pBits,
		   bool vol_header)
{
    // ** MBTransQuantIntra and MBTransQuantInter add **
	uint32_t stride = pEnc->mbParam.edged_width;
	uint32_t stride2 = stride / 2;
	uint32_t next_block = stride * 8;
	uint32_t iQuant = pEnc->current->quant;
	uint8_t *pY_Cur, *pU_Cur, *pV_Cur;
	IMAGE *pCurrent = &pEnc->current->image;
	// ** end **

	// ** MBPrediction add **
	int32_t iDcScaler;
	int16_t predictors[6][8];
	// ** end **

	// ** comp **
	static const uint32_t roundtab[16] =
		{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2 };

	int32_t dx ;
	int32_t dy ;
	int32_t sum ;
	// ** end **

	// ** last non-zero method **
	int8_t last_address[6];
	// ** end **

	// ** prediction **
	int32_t sum_pred;    
	// ** end **

	// ** zero skip method **
	uint8_t DCT_flag[6];
	// ** end **

	// ** Optimization. Parametric regulation method. by gary **
	int32_t k1_param = 0x7333;  // e.g. k1 = 0.5 = 0.1 shift left 16 => 0x8000
	                            //   k1 = 0.35=> 0x5999
	                            //   k1 = 0.4 => 0x6666
	                            //   k1 = 0.45=> 0x7333
	                            //   k1 = 0.5 => 0x8000

	int16_t square = 8;   // 8*8(6bit) or 16*16(8bit)
	int32_t result_pr;
	int32_t i,j;
	// ** end **

	float fSigma;

	DECLARE_ALIGNED_MATRIX(dct_codes, 6, 64, short, CACHE_LINE);
	DECLARE_ALIGNED_MATRIX(qcoeff, 6, 64, short, CACHE_LINE);
	DECLARE_ALIGNED_MATRIX(runlevel, 6, 64, VLC, CACHE_LINE);      //vlc
	DECLARE_ALIGNED_MATRIX(count, 6, 1, unsigned int, CACHE_LINE); //vlc

	unsigned int x, y;
	int iSearchRange;
	int bIntra;
	
	/* IMAGE *pCurrent = &pEnc->current->image; */
	IMAGE *pRef = &pEnc->reference->image;

	// ** edges **
	image_setedges(pRef, pEnc->mbParam.edged_width, pEnc->mbParam.edged_height,
				   pEnc->mbParam.width, pEnc->mbParam.height);
	// ** stop edges **

	pEnc->mbParam.m_rounding_type = 1 - pEnc->mbParam.m_rounding_type;
	pEnc->current->rounding_type = pEnc->mbParam.m_rounding_type;
	pEnc->current->fcode = pEnc->mbParam.m_fcode;

	pEnc->current->coding_type = P_VOP;

	if (vol_header)
		BitstreamWriteVolHeader(bs, &pEnc->mbParam, pEnc->current);

	BitstreamWriteVopHeader(bs, &pEnc->mbParam, pEnc->current, 1);

	*pBits = BitstreamPos(bs);

	pEnc->sStat.iTextBits = 0;
	pEnc->sStat.iMvSum = 0;
	pEnc->sStat.iMvCount = 0;
	pEnc->sStat.kblks = pEnc->sStat.mblks = pEnc->sStat.ublks = 0;

	for (y = 0; y < pEnc->mbParam.mb_height; y++) {
		for (x = 0; x < pEnc->mbParam.mb_width; x++) {

			MACROBLOCK *pMB = &pEnc->current->mbs[x + y * pEnc->mbParam.mb_width];

			pY_Cur = pCurrent->y + (y << 4) * stride + (x << 4);
	        pU_Cur = pCurrent->u + (y << 3) * stride2 + (x << 3);
	        pV_Cur = pCurrent->v + (y << 3) * stride2 + (x << 3);

			bIntra = MotionEstimationMB(&pEnc->mbParam, pEnc->current, pEnc->reference,x,y);

			if(bIntra == ME_HALFPXL && pMB->sad16 > 256)
			{
				OneMV_image_interpolate(pRef,pMB->mv16.x+x,pMB->mv16.y+y, 
					                    &pEnc->vInterH, &pEnc->vInterV,
						                &pEnc->vInterHV, pEnc->mbParam.edged_width,
						                pEnc->mbParam.edged_height,
						                pEnc->current->rounding_type);

				pMB->sad16=Halfpel_8point(pEnc->reference->image.y, pEnc->vInterH.y, pEnc->vInterV.y, pEnc->vInterHV.y,pEnc->current->image.y,
					           x,y,&pMB->mv16,pMB->sad16,pMB->mv16.x,pMB->mv16.y,
							   pEnc->current->fcode, pEnc->current->quant,
							   pEnc->mbParam.edged_width, &pEnc->mbParam);

				pMB->mvs[0] = pMB->mvs[1] = pMB->mvs[2] = pMB->mvs[3] = pMB->mv16;
				pMB->sad8[0] = pMB->sad8[1] = pMB->sad8[2] = pMB->sad8[3] = pMB->sad16;
			}

			bIntra = (pMB->mode == MODE_INTRA) || (pMB->mode == MODE_INTRA_Q);

			if (!bIntra) {

				if ((pEnc->current->global_flags & XVID_LUMIMASKING)) {
					if (pMB->dquant != NO_CHANGE) {
						pMB->mode = MODE_INTER_Q;
						pEnc->current->quant += DQtab[pMB->dquant];
						if (pEnc->current->quant > 31)
							pEnc->current->quant = 31;
						else if (pEnc->current->quant < 1)
							pEnc->current->quant = 1;
					}
				}
				iQuant = pMB->quant = pEnc->current->quant;

				pMB->field_pred = 0;

				pMB->cbp = 0;

				result_pr = ( (k1_param * iQuant) << square )>>16;  //Parametric regulation method. by gary

            // ** block0 **

				// ** comp **
				if (pMB->mode == MODE_INTER || pMB->mode == MODE_INTER_Q) {
					    compensate8x8_halfpel(&dct_codes[0 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x, 
											  16 * y, 
											  pMB->mvs[0].x, 
											  pMB->mvs[0].y,
											  pEnc->mbParam.edged_width);
				}else
				{
						compensate8x8_halfpel(&dct_codes[0 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x, 
											  16 * y, 
											  pMB->mvs[0].x,
							                  pMB->mvs[0].y, 
											  pEnc->mbParam.edged_width);
				}
				// ** stop comp **

			   // ** Optimization. Parametric regulation method. by gary **
			   if (pMB->sad8[0] > result_pr)
			   {

				//** dct **
		        fdct(&dct_codes[0 * 64]);
		        //** stop dct **

			    //** quant **
				sum = quant_inter(&qcoeff[0 * 64], &last_address[0], &DCT_flag[0], &dct_codes[0 * 64], iQuant);
				//** stop quant **

				if ((sum >= TOOSMALL_LIMIT) || (qcoeff[0*64] != 0) || (qcoeff[0*64+1] != 0) || (qcoeff[0*64+8] != 0)) 
				{
					//** iquant **
					dequant_inter(&dct_codes[0 * 64], last_address[0], &qcoeff[0 * 64], iQuant);
				    //** stop iquant **

					pMB->cbp |= 1 << (5 - 0);

					//** idct **
					   //last non-zero method
					if(last_address[0] < 32)
					{
						idct(&dct_codes[0 * 64],3,DCT_flag[0]);
					}
					else
					{
						idct(&dct_codes[0 * 64],7,DCT_flag[0]);
					}
					//** stop idct **
				}

			   }
			   else
			   {
				   for(j=0 ; j<8 ; j++){
					   for(i=0 ; i<8 ; i++){
						   qcoeff[j*8+i] =0;
					   }
				   }
			   }
			   // ** stop. Parametric regulation method. by gary **

				//** transfer **
				if (pMB->cbp & 32)
					transfer_16to8add(pY_Cur, &dct_codes[0 * 64], stride);
				//** stop transfer **

				// ** coding **
			    CodeVLCBlockInter(pMB,&qcoeff[0*64],scan_tables[0],0,&runlevel[0 * 64],&count[0],last_address[0]);
			    // ** stop coding **

			// ** end block0 **

			// ** block1 **

				// ** comp **
				if (pMB->mode == MODE_INTER || pMB->mode == MODE_INTER_Q) {
					    compensate8x8_halfpel(&dct_codes[1 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x + 8, 
											  16 * y, 
											  pMB->mvs[0].x, 
											  pMB->mvs[0].y,
											  pEnc->mbParam.edged_width);
				}else
				{
						compensate8x8_halfpel(&dct_codes[1 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x + 8, 
											  16 * y, 
											  pMB->mvs[1].x,
							                  pMB->mvs[1].y, 
											  pEnc->mbParam.edged_width);
				}
				// ** stop comp **

			   // ** Optimization. Parametric regulation method. by gary **
			   if (pMB->sad8[1] > result_pr)
			   {

				//** dct **
		        fdct(&dct_codes[1 * 64]);
		        //** stop dct **

			    //** quant **
				sum = quant_inter(&qcoeff[1 * 64], &last_address[1], &DCT_flag[1], &dct_codes[1 * 64], iQuant);
				//** stop quant **

				if ((sum >= TOOSMALL_LIMIT) || (qcoeff[1*64] != 0) || (qcoeff[1*64+1] != 0) || (qcoeff[1*64+8] != 0)) 
				{
					//** iquant **
					dequant_inter(&dct_codes[1 * 64], last_address[1], &qcoeff[1 * 64], iQuant);
				    //** stop iquant **

					pMB->cbp |= 1 << (5 - 1);

					//** idct **
					   //last non-zero method
					if(last_address[1] < 32)
					{
						idct(&dct_codes[1 * 64],3,DCT_flag[1]);
					}
					else
					{
						idct(&dct_codes[1 * 64],7,DCT_flag[1]);
					}
					//** stop idct **
				}

			   }
			   else
			   {
				   for(j=0 ; j<8 ; j++){
					   for(i=0 ; i<8 ; i++){
						   qcoeff[j*8+i + 1*64] =0;
					   }
				   }
			   } 
			   // ** Stop. Parametric regulation method. by gary **

				//** transfer **
				if (pMB->cbp & 16)
				        transfer_16to8add(pY_Cur + 8, &dct_codes[1 * 64], stride);
				//** stop transfer **

				// ** coding **
			    CodeVLCBlockInter(pMB,&qcoeff[1*64],scan_tables[0],1,&runlevel[1 * 64],&count[1],last_address[1]);
			    // ** stop coding **

			// ** end block1 **

			// ** block2 **

				// ** comp **
				if (pMB->mode == MODE_INTER || pMB->mode == MODE_INTER_Q) {
					    compensate8x8_halfpel(&dct_codes[2 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x, 
											  16 * y + 8, 
											  pMB->mvs[0].x, 
											  pMB->mvs[0].y,
											  pEnc->mbParam.edged_width);
				}else
				{
						compensate8x8_halfpel(&dct_codes[2 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x, 
											  16 * y + 8, 
											  pMB->mvs[2].x,
							                  pMB->mvs[2].y, 
											  pEnc->mbParam.edged_width);
				}
				// ** stop comp **

			   // ** Optimization. Parametric regulation method. by gary **
			   if (pMB->sad8[2] > result_pr)
			   {

				//** dct **
		        fdct(&dct_codes[2 * 64]);
		        //** stop dct **

			    //** quant **
				sum = quant_inter(&qcoeff[2 * 64], &last_address[2], &DCT_flag[1], &dct_codes[2 * 64], iQuant);
				//** stop quant **

				if ((sum >= TOOSMALL_LIMIT) || (qcoeff[2*64] != 0) || (qcoeff[2*64+1] != 0) || (qcoeff[2*64+8] != 0)) 
				{
					//** iquant **
					dequant_inter(&dct_codes[2 * 64], last_address[2], &qcoeff[2 * 64], iQuant);
				    //** stop iquant **

					pMB->cbp |= 1 << (5 - 2);

					//** idct **
					   //last non-zero method
					if(last_address[2] < 32)
					{
						idct(&dct_codes[2 * 64],3,DCT_flag[2]);
					}
					else
					{
						idct(&dct_codes[2 * 64],7,DCT_flag[2]);
					}
					//** stop idct **
				}

			   }
			   else
			   {
				   for(j=0 ; j<8 ; j++){
					   for(i=0 ; i<8 ; i++){
						   qcoeff[j*8+i + 2*64] =0;
					   }
				   }
			   }
			   // ** Stop. Parametric regulation method. by gary **

				//** transfer **
				if (pMB->cbp & 8)
						transfer_16to8add(pY_Cur + next_block, &dct_codes[2 * 64], stride);
				//** stop transfer **

				// ** coding **
			    CodeVLCBlockInter(pMB,&qcoeff[2*64],scan_tables[0],2,&runlevel[2 * 64],&count[2],last_address[2]);
			    // ** stop coding **

			// ** end block2 **

			// ** block3 **

				// ** comp **
				if (pMB->mode == MODE_INTER || pMB->mode == MODE_INTER_Q) {
					    compensate8x8_halfpel(&dct_codes[3 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x + 8, 
											  16 * y + 8, 
											  pMB->mvs[0].x, 
											  pMB->mvs[0].y,
											  pEnc->mbParam.edged_width);
				}else
				{
						compensate8x8_halfpel(&dct_codes[3 * 64], 
							                  pEnc->current->image.y, 
											  pEnc->reference->image.y, 
											  pEnc->vInterH.y,
							                  pEnc->vInterV.y, 
											  pEnc->vInterHV.y, 
											  16 * x + 8, 
											  16 * y + 8, 
											  pMB->mvs[3].x,
							                  pMB->mvs[3].y, 
											  pEnc->mbParam.edged_width);
				}
				// ** stop comp **

			   // ** Optimization. Parametric regulation method. by gary **
			   if (pMB->sad8[3] > result_pr)
			   {

				//** dct **
		        fdct(&dct_codes[3 * 64]);
		        //** stop dct **

			    //** quant **
				sum = quant_inter(&qcoeff[3 * 64], &last_address[3], &DCT_flag[3], &dct_codes[3 * 64], iQuant);
				//** stop quant **

				if ((sum >= TOOSMALL_LIMIT) || (qcoeff[3*64] != 0) || (qcoeff[3*64+1] != 0) || (qcoeff[3*64+8] != 0)) 
				{
					//** iquant **
					dequant_inter(&dct_codes[3 * 64], last_address[3], &qcoeff[3 * 64], iQuant);
				    //** stop iquant **

					pMB->cbp |= 1 << (5 - 3);

					//** idct **
					   //last non-zero method
					if(last_address[3] < 32)
					{
						idct(&dct_codes[3 * 64],3,DCT_flag[3]);
					}
					else
					{
						idct(&dct_codes[3 * 64],7,DCT_flag[3]);
					}
					//** stop idct **
				}

			   }
			   else
			   {
				   for(j=0 ; j<8 ; j++){
					   for(i=0 ; i<8 ; i++){
						   qcoeff[j*8+i + 3*64] =0;
					   }
				   }
			   }
			   // ** Stop. Parametric regulation method. by gary **

				//** transfer **
				if (pMB->cbp & 4)
						transfer_16to8add(pY_Cur + next_block + 8, &dct_codes[3 * 64], stride);
				//** stop transfer **

				// ** coding **
			    CodeVLCBlockInter(pMB,&qcoeff[3*64],scan_tables[0],3,&runlevel[3 * 64],&count[3],last_address[3]);
			    // ** stop coding **

			// ** end block3 **

			// ** block4 **

				// ** Interpolation_swith (U) **
				if (pMB->mode == MODE_INTER || pMB->mode == MODE_INTER_Q) {
					dx = (pMB->mvs[0].x & 3) ? (pMB->mvs[0].x >> 1) | 1 : pMB->mvs[0].x / 2;
		            dy = (pMB->mvs[0].y & 3) ? (pMB->mvs[0].y >> 1) | 1 : pMB->mvs[0].y / 2;
				}else
				{                 // mode == MODE_INTER4V 
					sum = pMB->mvs[0].x + pMB->mvs[1].x + pMB->mvs[2].x + pMB->mvs[3].x;
	  	            dx = (sum ? SIGN(sum) *
			              (roundtab[ABS(sum) % 16] + (ABS(sum) / 16) * 2) : 0);

		            sum = pMB->mvs[0].y + pMB->mvs[1].y + pMB->mvs[2].y + pMB->mvs[3].y;
		            dy = (sum ? SIGN(sum) *
			              (roundtab[ABS(sum) % 16] + (ABS(sum) / 16) * 2) : 0);
				}
				interpolate8x8_switch(pEnc->vInterV.u, 
						              pEnc->reference->image.u, 
									  8 * x, 8 * y, dx, dy,
							          pEnc->mbParam.edged_width / 2, 
									  pEnc->current->rounding_type);
		        transfer_8to16sub(&dct_codes[4 * 64],
						           pEnc->current->image.u + 8 * y * pEnc->mbParam.edged_width / 2 + 8 * x,
						           pEnc->vInterV.u + 8 * y * pEnc->mbParam.edged_width / 2 + 8 * x,
						           pEnc->mbParam.edged_width / 2);
				// ** stop Interpolation_swith (U) **

                //** dct **
		        fdct(&dct_codes[4 * 64]);
		        //** stop dct **

			    //** quant **
				sum = quant_inter(&qcoeff[4 * 64], &last_address[4], &DCT_flag[4], &dct_codes[4 * 64], iQuant);
				//** stop quant **

				if ((sum >= TOOSMALL_LIMIT) || (qcoeff[4*64] != 0) || (qcoeff[4*64+1] != 0) || (qcoeff[4*64+8] != 0)) 
				{
					//** iquant **
					dequant_inter(&dct_codes[4 * 64], last_address[4], &qcoeff[4 * 64], iQuant);
				    //** stop iquant **

					pMB->cbp |= 1 << (5 - 4);

					//** idct **
					   //last non-zero method
					if(last_address[4] < 32)
					{
						idct(&dct_codes[4 * 64],3,DCT_flag[4]);
					}
					else
					{
						idct(&dct_codes[4 * 64],7,DCT_flag[4]);
					}
					//** stop idct **
				}

				//** transfer **
				if (pMB->cbp & 2)
						transfer_16to8add(pU_Cur, &dct_codes[4 * 64], stride2);
				//** stop transfer **

				// ** coding **
			    CodeVLCBlockInter(pMB,&qcoeff[4*64],scan_tables[0],4,&runlevel[4 * 64],&count[4],last_address[4]);
			    // ** stop coding **

			// ** end block4 **

			// ** block5 **
				// ** Interpolation_swith (V) **
				if (pMB->mode == MODE_INTER || pMB->mode == MODE_INTER_Q) {
					dx = (pMB->mvs[0].x & 3) ? (pMB->mvs[0].x >> 1) | 1 : pMB->mvs[0].x / 2;
		            dy = (pMB->mvs[0].y & 3) ? (pMB->mvs[0].y >> 1) | 1 : pMB->mvs[0].y / 2;
				}else
				{                 // mode == MODE_INTER4V 
					sum = pMB->mvs[0].x + pMB->mvs[1].x + pMB->mvs[2].x + pMB->mvs[3].x;
	  	            dx = (sum ? SIGN(sum) *
			              (roundtab[ABS(sum) % 16] + (ABS(sum) / 16) * 2) : 0);

		            sum = pMB->mvs[0].y + pMB->mvs[1].y + pMB->mvs[2].y + pMB->mvs[3].y;
		            dy = (sum ? SIGN(sum) *
			              (roundtab[ABS(sum) % 16] + (ABS(sum) / 16) * 2) : 0);
				}

		        interpolate8x8_switch(pEnc->vInterV.v,
						              pEnc->reference->image.v, 
								      8 * x, 8 * y, dx, dy,
						              pEnc->mbParam.edged_width / 2, 
									  pEnc->current->rounding_type);
	            transfer_8to16sub(&dct_codes[5 * 64],
					              pEnc->current->image.v + 8 * y * pEnc->mbParam.edged_width / 2 + 8 * x,
					              pEnc->vInterV.v + 8 * y * pEnc->mbParam.edged_width / 2 + 8 * x,
					           	  pEnc->mbParam.edged_width / 2);
				// ** stop Interpolation_swith (V) **

				//** dct **
		        fdct(&dct_codes[5 * 64]);
		        //** stop dct **

			    //** quant **
				sum = quant_inter(&qcoeff[5 * 64], &last_address[5], &DCT_flag[5], &dct_codes[5 * 64], iQuant);
				//** stop quant **

				if ((sum >= TOOSMALL_LIMIT) || (qcoeff[5*64] != 0) || (qcoeff[5*64+1] != 0) || (qcoeff[5*64+8] != 0)) 
				{
					//** iquant **
					dequant_inter(&dct_codes[5 * 64], last_address[5], &qcoeff[5 * 64], iQuant);
				    //** stop iquant **

					pMB->cbp |= 1 << (5 - 5);

					//** idct **
					   //last non-zero method
					if(last_address[5] < 32)
					{
						idct(&dct_codes[5 * 64],3,DCT_flag[5]);
					}
					else
					{
						idct(&dct_codes[5 * 64],7,DCT_flag[5]);
					}
					//** stop idct **
				}

				//** transfer **
				if (pMB->cbp & 1)
						transfer_16to8add(pV_Cur, &dct_codes[5 * 64], stride2);
				//** stop transfer **

				// ** coding **
			    CodeVLCBlockInter(pMB,&qcoeff[5*64],scan_tables[0],5,&runlevel[5 * 64],&count[5],last_address[5]);
			    // ** stop coding **

			// ** end block5 **

				if (pMB->cbp || pMB->mvs[0].x || pMB->mvs[0].y ||
					pMB->mvs[1].x || pMB->mvs[1].y || pMB->mvs[2].x ||
					pMB->mvs[2].y || pMB->mvs[3].x || pMB->mvs[3].y) {
					pEnc->sStat.mblks++;
				}  else {
					pEnc->sStat.ublks++;
				} 

				// Finished processing the MB, now check if to CODE or SKIP 

				if (pMB->cbp == 0 && pMB->mode == MODE_INTER && pMB->mvs[0].x == 0 &&
					pMB->mvs[0].y == 0) {

						MBSkip(bs);	// without B-frames, no precautions are needed 
				}
				else {
					if (pEnc->current->global_flags & XVID_GREYSCALE) {
						pMB->cbp &= 0x3C;		// keep only bits 5-2 
						qcoeff[4*64+0]=0;		// zero, because DC for INTRA MBs DC value is saved 
						qcoeff[5*64+0]=0;
					}
                    BitstreamPutBit(bs, 0);	/* coded */

					// ** write header and coefficient **
				    WriteMBHeaderCoeffPFrame(pEnc->current,pMB,bs,&pEnc->sStat,runlevel,count);
				    // ** end to write header and coefficient **
				}

			} else {  			
                CodeIntraMB(pEnc, pMB);

				iQuant = pEnc->current->quant;
		    // ** block0 **
            
			    //** transfer **
    			transfer_8to16copy(&dct_codes[0 * 64], pY_Cur, stride);
    			//** stop transfer **

	    		iDcScaler = get_dc_scaler(iQuant, 1);

		    	//** dct **
		        fdct(&dct_codes[0 * 64]);
		        //** stop dct **

			    //** quant **
			    quant_intra(&qcoeff[0 * 64], &last_address[0], &DCT_flag[0], &dct_codes[0 * 64], iQuant, iDcScaler);
		        //** stop quant **

			    //** iquant **
		        dequant_intra(&dct_codes[0 * 64], last_address[0], &qcoeff[0 * 64], iQuant, iDcScaler);
			    //** stop iquant **

			    //** idct **
		        idct(&dct_codes[0 * 64],last_address[0]>>3,DCT_flag[0]);
		        //** stop idct **

			    //** transfer **
	            transfer_16to8copy(pY_Cur, &dct_codes[0 * 64], stride);
			    //** stop transfer **

			    //** prediction **
			    predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 0, &qcoeff[0 * 64],
						           iQuant, iDcScaler, predictors[0], 0);

			    sum_pred = calc_acdc(pMB, 0, &qcoeff[0 * 64], iDcScaler, predictors[0]);

				if(sum_pred < 0)
				{
					pMB->acpred_directions[0] = 0;
				}
			    else
				{
			        apply_acdc(pMB, 0, &qcoeff[0 * 64], predictors[0]);
				}
			    //** end prediction **

			    //** CBP **
			    pMB->cbp = single_calc_cbp(&qcoeff[0 * 64],pMB->cbp,0);
                //** end CBP **

				// ** coding **
			    CodeVLCBlockIntra(pMB,&qcoeff[0*64],scan_tables[pMB->acpred_directions[0]],0,&runlevel[0 * 64],&count[0],last_address[0]);
			    // ** stop coding **
			
		    // ** end block0 **

		    // ** block1 **
            
			    //** transfer **
			    transfer_8to16copy(&dct_codes[1 * 64], pY_Cur + 8, stride);
			    //** stop transfer **

			    iDcScaler = get_dc_scaler(iQuant, 1);

			    //** dct **
		        fdct(&dct_codes[1 * 64]);
		        //** stop dct **

			    //** quant **
			    quant_intra(&qcoeff[1 * 64], &last_address[1], &DCT_flag[1], &dct_codes[1 * 64], iQuant, iDcScaler);
		        //** stop quant **

			    //** iquant **
		        dequant_intra(&dct_codes[1 * 64], last_address[1], &qcoeff[1 * 64], iQuant, iDcScaler);
			    //** stop iquant **

			    //** idct **
		        idct(&dct_codes[1 * 64],last_address[1]>>3,DCT_flag[1]);
		        //** stop idct **

			    //** transfer **
	            transfer_16to8copy(pY_Cur+8, &dct_codes[1 * 64], stride);
			    //** stop transfer **

			    //** prediction **
                predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 1, &qcoeff[1 * 64],
						           iQuant, iDcScaler, predictors[1], 0);
				
				calc_acdc(pMB, 1, &qcoeff[1 * 64], iDcScaler, predictors[1]);

				if(sum_pred < 0)
				{
					pMB->acpred_directions[1] = 0;
				}
			    else
				{	
			        apply_acdc(pMB, 1, &qcoeff[1 * 64], predictors[1]);
				}
			    //** end prediction **

			    //** CBP **
			    pMB->cbp = single_calc_cbp(&qcoeff[1 * 64],pMB->cbp,1);
                //** end CBP **

				// ** coding **
			    CodeVLCBlockIntra(pMB,&qcoeff[1*64],scan_tables[pMB->acpred_directions[1]],1,&runlevel[1 * 64],&count[1],last_address[1]);
			    // ** stop coding **

		    // ** end block1 **

		    // ** block2 **
            
			    //** transfer **
			    transfer_8to16copy(&dct_codes[2 * 64], pY_Cur + next_block, stride);
			    //** stop transfer **

			    iDcScaler = get_dc_scaler(iQuant, 1);

			    //** dct **
		        fdct(&dct_codes[2 * 64]);
		        //** stop dct **

			    //** quant **
			    quant_intra(&qcoeff[2 * 64], &last_address[2], &DCT_flag[2], &dct_codes[2 * 64], iQuant, iDcScaler);
		        //** stop quant **

			    //** iquant **
		        dequant_intra(&dct_codes[2 * 64], last_address[2], &qcoeff[2 * 64], iQuant, iDcScaler);
			    //** stop iquant **

			    //** idct **
		        idct(&dct_codes[2 * 64],last_address[2]>>3,DCT_flag[2]);
		        //** stop idct **

			    //** transfer **
	            transfer_16to8copy(pY_Cur + next_block, &dct_codes[2 * 64], stride);
			    //** stop transfer **

			    //** prediction **
				predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 2, &qcoeff[2 * 64],
						           iQuant, iDcScaler, predictors[2], 0);

				calc_acdc(pMB, 2, &qcoeff[2 * 64], iDcScaler, predictors[2]);
			
				if(sum_pred < 0)
				{
					pMB->acpred_directions[2] = 0;
				}
			    else
				{
			        apply_acdc(pMB, 2, &qcoeff[2 * 64], predictors[2]);
				}
			    //** end prediction **

			    //** CBP **
			    pMB->cbp = single_calc_cbp(&qcoeff[2 * 64],pMB->cbp,2);
                //** end CBP **

				// ** coding **
			    CodeVLCBlockIntra(pMB,&qcoeff[2*64],scan_tables[pMB->acpred_directions[2]],2,&runlevel[2 * 64],&count[2],last_address[2]);
			    // ** stop coding **
			
		    // ** end block2 **

		    // ** block3 **
            
			    //** transfer **
			    transfer_8to16copy(&dct_codes[3 * 64], pY_Cur + next_block + 8, stride);
			    //** stop transfer **

			    iDcScaler = get_dc_scaler(iQuant, 1);

			    //** dct **
		        fdct(&dct_codes[3 * 64]);
		        //** stop dct **

			    //** quant **
			    quant_intra(&qcoeff[3 * 64], &last_address[3], &DCT_flag[3], &dct_codes[3 * 64], iQuant, iDcScaler);
		        //** stop quant **

			    //** iquant **
		        dequant_intra(&dct_codes[3 * 64], last_address[3], &qcoeff[3 * 64], iQuant, iDcScaler);
			    //** stop iquant **

			    //** idct **
		        idct(&dct_codes[3 * 64],last_address[3]>>3,DCT_flag[3]);
		        //** stop idct **

			    //** transfer **
	            transfer_16to8copy(pY_Cur + next_block + 8, &dct_codes[3 * 64], stride);
			    //** stop transfer **

			    //** prediction **
				predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 3, &qcoeff[3 * 64],
						           iQuant, iDcScaler, predictors[3], 0);

				calc_acdc(pMB, 3, &qcoeff[3 * 64], iDcScaler, predictors[3]);

				if(sum_pred < 0)
				{
					pMB->acpred_directions[3] = 0;
				}
			    else
				{
		            apply_acdc(pMB, 3, &qcoeff[3 * 64], predictors[3]);
				}
			    //** end prediction **

			    //** CBP **
			    pMB->cbp = single_calc_cbp(&qcoeff[3 * 64],pMB->cbp,3);
                //** end CBP **

				// ** coding **
			    CodeVLCBlockIntra(pMB,&qcoeff[3*64],scan_tables[pMB->acpred_directions[3]],3,&runlevel[3 * 64],&count[3],last_address[3]);
			    // ** stop coding **
			
		    // ** end block3 **

	        // ** block4 **
            
			    //** transfer **
			    transfer_8to16copy(&dct_codes[4 * 64], pU_Cur, stride2);
			    //** stop transfer **

			    iDcScaler = get_dc_scaler(iQuant, 0);

			    //** dct **
		        fdct(&dct_codes[4 * 64]);
		        //** stop dct **

			    //** quant **
			    quant_intra(&qcoeff[4 * 64], &last_address[4], &DCT_flag[4], &dct_codes[4 * 64], iQuant, iDcScaler);
		        //** stop quant **

			    //** iquant **
		        dequant_intra(&dct_codes[4 * 64], last_address[4], &qcoeff[4 * 64], iQuant, iDcScaler);
			    //** stop iquant **

			    //** idct **
		        idct(&dct_codes[4 * 64],last_address[4]>>3,DCT_flag[4]);
		        //** stop idct **

			    //** transfer **
	            transfer_16to8copy(pU_Cur, &dct_codes[4 * 64], stride2);
			    //** stop transfer **

			    //** prediction **
				predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 4, &qcoeff[4 * 64],
						           iQuant, iDcScaler, predictors[4], 0);

				calc_acdc(pMB, 4, &qcoeff[4 * 64], iDcScaler, predictors[4]);

				if(sum_pred < 0)
				{
					pMB->acpred_directions[4] = 0;
				}
			    else
				{
			        apply_acdc(pMB, 4, &qcoeff[4 * 64], predictors[4]);
				}
			    //** end prediction **

			    //** CBP **
			    pMB->cbp = single_calc_cbp(&qcoeff[4 * 64],pMB->cbp,4);
                //** end CBP **

				// ** coding **
			    if (pEnc->current->global_flags & XVID_GREYSCALE)
				{
				    pMB->cbp &= 0x3C;		// keep only bits 5-2 
				    qcoeff[4*64+0]=0;		// zero, because for INTRA MBs DC value is saved 
				}

			    CodeVLCBlockIntra(pMB,&qcoeff[4*64],scan_tables[pMB->acpred_directions[4]],4,&runlevel[4 * 64],&count[4],last_address[4]);
			    // ** stop coding **
		
		    // ** end block4 **

		    // ** block5 **
            
			    //** transfer **
			    transfer_8to16copy(&dct_codes[5 * 64], pV_Cur, stride2);
			    //** stop transfer **

			    iDcScaler = get_dc_scaler(iQuant, 0);

			    //** dct **
		        fdct(&dct_codes[5 * 64]);
		        //** stop dct **

			    //** quant **
			    quant_intra(&qcoeff[5 * 64], &last_address[5],&DCT_flag[5], &dct_codes[5 * 64], iQuant, iDcScaler);
		        //** stop quant **

			    //** iquant **
		        dequant_intra(&dct_codes[5 * 64], last_address[5], &qcoeff[5 * 64], iQuant, iDcScaler);
			    //** stop iquant **

			    //** idct **
		        idct(&dct_codes[5 * 64],last_address[5]>>3,DCT_flag[5]);
		        //** stop idct **

			    //** transfer **
	            transfer_16to8copy(pV_Cur, &dct_codes[5 * 64], stride2);
			    //** stop transfer **

			    //** prediction **
				predict_acdc(pEnc->current->mbs, x, y, pEnc->mbParam.mb_width, 5, &qcoeff[5 * 64],
						           iQuant, iDcScaler, predictors[5], 0);

				calc_acdc(pMB, 5, &qcoeff[5 * 64], iDcScaler, predictors[5]);

				if(sum_pred < 0)
				{
					pMB->acpred_directions[5] = 0;
				}
			    else
				{
			        apply_acdc(pMB, 5, &qcoeff[5 * 64], predictors[5]);
				}
			    //** end prediction **

			    //** CBP **
			    pMB->cbp = single_calc_cbp(&qcoeff[5 * 64],pMB->cbp,5);
                //** end CBP **

				// ** coding **
			    if (pEnc->current->global_flags & XVID_GREYSCALE)
				{	
					qcoeff[5*64+0]=0;
				}

			    CodeVLCBlockIntra(pMB,&qcoeff[5*64],scan_tables[pMB->acpred_directions[5]],5,&runlevel[5 * 64],&count[5],last_address[5]);
			    // ** stop coding **

		    // ** end block5 **

				pEnc->sStat.kblks++;
				BitstreamPutBit(bs, 0);  //for P frame

				// ** write header and coefficient **
			    WriteMBHeaderCoeff(pEnc->current,pMB,bs,&pEnc->sStat,runlevel,count);
			    // ** end to write header and coefficient **

            }
		}
	}

	if (pEnc->sStat.iMvCount == 0)
		pEnc->sStat.iMvCount = 1;

	fSigma = (float) my_sqrt((float) pEnc->sStat.iMvSum / pEnc->sStat.iMvCount);

	iSearchRange = 1 << (3 + pEnc->mbParam.m_fcode);

	if ((fSigma > iSearchRange / 3)
		&& (pEnc->mbParam.m_fcode <= 3))	// maximum search range 128 
	{
		pEnc->mbParam.m_fcode++;
		iSearchRange *= 2;
	} else if ((fSigma < iSearchRange / 6)
			   && (pEnc->sStat.fMvPrevSigma >= 0)
			   && (pEnc->sStat.fMvPrevSigma < iSearchRange / 6)
			   && (pEnc->mbParam.m_fcode >= 2))	// minimum search range 16 
	{
		pEnc->mbParam.m_fcode--;
		iSearchRange /= 2;
	}

	pEnc->sStat.fMvPrevSigma = fSigma;

	*pBits = BitstreamPos(bs) - *pBits;

	return 0;					/* inter */

}
