#include "xvid.h"
#include "decoder.h"
#include "encoder.h"
#include "bitstream/cbp.h"
#include "dct/idct.h"
#include "dct/fdct.h"
#include "image/colorspace.h"
#include "image/interpolate8x8.h"
#include "utils/mem_transfer.h"
#include "quant/quant_h263.h"
#include "motion/motion.h"
#include "motion/sad.h"
#include "bitstream/mbcoding.h"

/*****************************************************************************
 * XviD Init Entry point
 *
 * Well this function initialize all internal function pointers according
 * to the CPU features forced by the library client or autodetected (depending
 * on the XVID_CPU_FORCE flag). It also initializes vlc coding tables and all
 * image colorspace transformation tables.
 * 
 * Returned value : XVID_ERR_OK
 *                  + API_VERSION in the input XVID_INIT_PARAM structure
 *                  + core build  "   "    "       "               "
 *
 ****************************************************************************/

int
xvid_init(void *handle,
		  int opt,
		  void *param1,
		  void *param2)
{
	XVID_INIT_PARAM *init_param;

	init_param = (XVID_INIT_PARAM *) param1;

	/* Inform the client the API version */
	init_param->api_version = API_VERSION;

	/* Inform the client the core build - unused because we're still alpha */
	init_param->core_build = 1000;

	/* Fixed Point Forward/Inverse DCT transformations */
	fdct = fdct_int32;
	idct = idct_int32;

	/* Only needed on PPC Altivec archs */
	sadInit = 0;

	/* Quantization functions */
	quant_intra   = quant_intra_c;
	dequant_intra = dequant_intra_c;
	quant_inter   = quant_inter_c;
	dequant_inter = dequant_inter_c;

	/* Block transfer related functions */
	transfer_8to16copy = transfer_8to16copy_c;
	transfer_16to8copy = transfer_16to8copy_c;
	transfer_8to16sub  = transfer_8to16sub_c;
	transfer_8to16sub2 = transfer_8to16sub2_c;
	transfer_16to8add  = transfer_16to8add_c;
	transfer8x8_copy   = transfer8x8_copy_c;

	/* Image interpolation related functions */
	interpolate8x8_halfpel_h  = interpolate8x8_halfpel_h_c;
	interpolate8x8_halfpel_v  = interpolate8x8_halfpel_v_c;
	interpolate8x8_halfpel_hv = interpolate8x8_halfpel_hv_c;

	/* All colorspace transformation functions User Format->YV12 */
	yuv_to_yv12    = yuv_to_yv12_c;

	/* Functions used in motion estimation algorithms */
	calc_cbp = calc_cbp_c;
	single_calc_cbp = single_calc_cbp_c; //by gary
	sad16    = sad16_c;
	sad8     = sad8_c;
	dev16    = dev16_c;
	
	Halfpel8_Refine = Halfpel8_Refine_c;

	return XVID_ERR_OK;
}

/*****************************************************************************
 * XviD Native encoder entry point
 *
 * This function is just a wrapper to all the option cases.
 *
 * Returned values : XVID_ERR_FAIL when opt is invalid
 *                   else returns the wrapped function result
 *
 ****************************************************************************/

int
xvid_encore(void *handle,
			int opt,
			void *param1,
			void *param2)
{
	switch (opt) {
	case XVID_ENC_ENCODE:
		return encoder_encode((Encoder *) handle, (XVID_ENC_FRAME *) param1,
							  (XVID_ENC_STATS *) param2);

	case XVID_ENC_CREATE:
		return encoder_create((XVID_ENC_PARAM *) param1);

	case XVID_ENC_DESTROY:
		return encoder_destroy((Encoder *) handle);

	default:
		return XVID_ERR_FAIL;
	}
}
