#include "../utils/mem_transfer.h"

typedef void (INTERPOLATE8X8) (uint8_t * const dst,
							   const uint8_t * const src,
							   const uint32_t stride,
							   const uint32_t rounding);
typedef INTERPOLATE8X8 *INTERPOLATE8X8_PTR;

extern INTERPOLATE8X8_PTR interpolate8x8_halfpel_h;
extern INTERPOLATE8X8_PTR interpolate8x8_halfpel_v;
extern INTERPOLATE8X8_PTR interpolate8x8_halfpel_hv;

INTERPOLATE8X8 interpolate8x8_halfpel_h_c;
INTERPOLATE8X8 interpolate8x8_halfpel_v_c;
INTERPOLATE8X8 interpolate8x8_halfpel_hv_c;

//Parallel code .by gary
void
OneMV_interpolate8x8_halfpel_h(uint8_t * const dst,
						       const uint8_t * const src,
						       const uint32_t stride,
						       const uint32_t rounding);

//Parallel code .by gary
void
OneMV_interpolate8x8_halfpel_v(uint8_t * const dst,
							   const uint8_t * const src,
							   const uint32_t stride,
							   const uint32_t rounding);

//Parallel code .by gary
void
OneMV_interpolate8x8_halfpel_hv(uint8_t * const dst,
							    const uint8_t * const src,
							    const uint32_t stride,
							    const uint32_t rounding);

void interpolate8x8_c(uint8_t * const dst,
					  const uint8_t * const src,
					  const uint32_t x,
					  const uint32_t y,
					  const uint32_t stride);

static __inline void
interpolate8x8_switch(uint8_t * const cur,
					  const uint8_t * const refn,
					  const uint32_t x,
					  const uint32_t y,
					  const int32_t dx,
					  const int32_t dy,
					  const uint32_t stride,
					  const uint32_t rounding)
{
	int32_t ddx, ddy;

	switch (((dx & 1) << 1) + (dy & 1))	/* ((dx%2)?2:0)+((dy%2)?1:0) */
	{
	case 0:
		ddx = dx / 2;
		ddy = dy / 2;
		transfer8x8_copy(cur + y * stride + x,
						 refn + (int)((y + ddy) * stride + x + ddx), stride);
		break;

	case 1:
		ddx = dx / 2;
		ddy = (dy - 1) / 2;
		interpolate8x8_halfpel_v(cur + y * stride + x,
								 refn + (int)((y + ddy) * stride + x + ddx), stride,
								 rounding);
		break;

	case 2:
		ddx = (dx - 1) / 2;
		ddy = dy / 2;
		interpolate8x8_halfpel_h(cur + y * stride + x,
								 refn + (int)((y + ddy) * stride + x + ddx), stride,
								 rounding);
		break;

	default:
		ddx = (dx - 1) / 2;
		ddy = (dy - 1) / 2;
		interpolate8x8_halfpel_hv(cur + y * stride + x,
								 refn + (int)((y + ddy) * stride + x + ddx), stride,
								  rounding);
		break;
	}
}

