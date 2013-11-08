GCC=gcc
OUTPUT=mpeg4_encoder

all :
	${GCC} -O2 -ggdb -fpack-struct ../src/bitstream/bitstream.c ../src/bitstream/cbp.c \
	../src/bitstream/mbcoding.c ../src/dct/fdct.c ../src/dct/idct.c ../src/image/colorspace.c \
	../src/image/image.c ../src/image/interpolate8x8.c ../src/motion/motion_comp.c ../src/motion/motion_est.c \
	../src/motion/sad.c ../src/prediction/mbprediction.c ../src/quant/quant_h263.c \
	../src/quant/quant_matrix.c ../src/utils/mem_align.c \
	../src/utils/mem_transfer.c ../src/utils/ratecontrol.c ../src/xvid_encraw.c ../src/encoder.c \
	../src/xvid.c -lm -lc -lrt -o ${OUTPUT} 

run:
	rm output_data -f
	./${OUTPUT}
clean:
	rm -f output_data ${OUTPUT} memory_info.txt
diff:
	diff -s golden_data output_data
