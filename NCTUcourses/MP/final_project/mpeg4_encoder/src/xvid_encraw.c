#include "xvid.h"

/*****************************************************************************
 *                            For portable
 ****************************************************************************/
#define _MSC_VER
#if defined(_MSC_VER)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* global state */
struct timespec start_time;                                 
struct timespec end_time;                                 

#define ARG_INPUTFILE "./input_data"  //for input address
#define ARG_OUTPUTFILE "./output_data"        //for ouput address
#define MEMORY_INFO "./memory_info.txt" //for memory controller

#define FILE_SIZE 91238400   //Picture Size
                  
FILE *out_file = NULL;      //for output file
FILE *info_ptr = NULL;      //for memory controller
int mem_state =0;           //for memory controller
#endif

//for memory controller
typedef struct _mem_info mem_info;
struct _mem_info
{
	int size;
	unsigned char top;
	unsigned char down;
	unsigned char * address;
	mem_info* next_info;
};

mem_info * free_ptr = NULL;     //for memory controller 
mem_info * used_ptr = NULL;     //for memory controller
static unsigned char divide_num = 0;   //for memory controller

// for memory allocation (1.5M start)
#define MEM_ADD 0x150000

// for input file (9.5M start)
#define INPUT_ADD 0x950000

// for output file (46M start)
#define OUTPUT_ADD 0x2E00000

// for memory allocation size
#define MALLOC_SIZE 1024*1024*8

static unsigned char * mem_add = NULL;          //for my_malloc
static unsigned char * input_add = NULL;        //for input file
static unsigned char * output_add = NULL;       //for output file
static unsigned char * output_add_temp = NULL;  //for output file counter
static unsigned int output_size = 0;            //the size of output file
static int malloc_total =0;                     //size of memory allocation

		

/*****************************************************************************
 *                     Command line global variables
 ****************************************************************************/

static int   ARG_FRAMENO = 60;        //the number of frame
//static int   ARG_BITRATE = 6400000;
static int   ARG_BITRATE = 6400000;
static int   ARG_QUANTI = 0;    // is quant != 0, use a fixed quant (and ignore bitrate)
static int   ARG_QUALITY = 6;   // decide motion and general, range between 0 - 6
static int   ARG_MINQUANT = 1;
static int   ARG_MAXQUANT = 31;
static float ARG_FRAMERATE = 30.00f;
static int   ARG_NUMBEROFP = 14; // number of P between
static int   XDIM = 704;
static int   YDIM = 576;

/*****************************************************************************
 *                            Quality presets
 ****************************************************************************/

static int const motion_presets[7] = {
	0,                                                        /* Q 0 */
	PMV_EARLYSTOP16,                                          /* Q 1 */
	PMV_EARLYSTOP16,                                          /* Q 2 */
	PMV_EARLYSTOP16 | PMV_HALFPELREFINE16,                    /* Q 3 */
	PMV_EARLYSTOP16 | PMV_HALFPELREFINE16,                    /* Q 4 */
	PMV_EARLYSTOP16 | PMV_HALFPELREFINE16 | PMV_EARLYSTOP8 |  /* Q 5 */
	PMV_HALFPELREFINE8,
	PMV_EARLYSTOP16 | PMV_HALFPELREFINE16 | PMV_EXTSEARCH16 | /* Q 6 */
	PMV_USESQUARES16 | PMV_EARLYSTOP8 | PMV_HALFPELREFINE8
};

static int const general_presets[7] = {
	XVID_H263QUANT,	                              /* Q 0 */
	XVID_MPEGQUANT,                               /* Q 1 */
	XVID_H263QUANT,                               /* Q 2 */
	XVID_H263QUANT | XVID_HALFPEL,                /* Q 3 */
	XVID_H263QUANT | XVID_HALFPEL | XVID_INTER4V, /* Q 4 */
	XVID_H263QUANT | XVID_HALFPEL | XVID_INTER4V, /* Q 5 */
	XVID_H263QUANT | XVID_HALFPEL | XVID_INTER4V  /* Q 6 */
};

/*****************************************************************************
 *                     Macro function
 ****************************************************************************/

#define IMAGE_SIZE(x,y) ((x)*(y)*3/2)

#define MAX(A,B) ( ((A)>(B)) ? (A) : (B) )

#define SMALL_EPS 1e-10

#define SWAP(a) ( (((a)&0x000000ff)<<24) | (((a)&0x0000ff00)<<8) | \
                  (((a)&0x00ff0000)>>8)  | (((a)&0xff000000)>>24) )

/****************************************************************************
 *                     Nasty global vars ;-)
 ***************************************************************************/

static int i,filenr = 0;

/* Internal structures (handles) for encoding and decoding */
static void *enc_handle = NULL;

/*****************************************************************************
 *               Local prototypes
 ****************************************************************************/

/* Encoder related functions */
static int enc_init(void);
static int enc_stop();
static int enc_main(unsigned char* image, unsigned char* bitstream,
					long *streamlength, long* frametype);

/*****************************************************************************
 *               Main function
 ****************************************************************************/

int main(void)
{

	unsigned char *mp4_buffer = NULL;
	unsigned char *in_buffer = NULL;
	unsigned char *out_buffer = NULL;
  
	int status;
	long frame_type; 
	long m4v_size;
							
/*****************************************************************************
 *                            Arguments checking
 ****************************************************************************/

	if (XDIM <= 0 || XDIM >= 2048 || YDIM <=0 || YDIM >= 2048 ) {
		my_printf("Trying to retreive width and height from PGM header\n");
	}

	my_malloc_init(); // initial memory allocation. by gary
	input_file();   // it give input address,or open input file. by gary
	output_file();  // it give output address, or open output file . by gary

	/* now we know the sizes, so allocate memory */

	in_buffer = (unsigned char *) my_malloc(IMAGE_SIZE(XDIM,YDIM));
	if (!in_buffer)
		goto free_all_memory;

	/* this should really be enough memory ! */
	mp4_buffer = (unsigned char *) my_malloc(IMAGE_SIZE(XDIM,YDIM)*2);
	if (!mp4_buffer)
		goto free_all_memory;	

	clock_gettime(CLOCK_REALTIME, &start_time);    
/*****************************************************************************
 *                            XviD PART  Start
 ****************************************************************************/


	status = enc_init();
	if (status)    
	{ 
		my_printf("Encore INIT problem, return value %d\n", status);
		goto release_all;
	}

/*****************************************************************************
 *                       Encoding loop
 ****************************************************************************/


	do {

			/* read raw data (YUV-format) */

		my_read(in_buffer, 1, IMAGE_SIZE(XDIM, YDIM), input_add);
        input_add = input_add + IMAGE_SIZE(XDIM, YDIM);

/*****************************************************************************
 *                       Encode and decode this frame
 ****************************************************************************/

		status = enc_main(in_buffer, mp4_buffer,
						  &m4v_size, &frame_type);

		output_size += m4v_size;

		my_printf("Frame %5d: intra %1d, size=%6dbytes\n",
			   (int)filenr, (int)frame_type, (int)m4v_size);

/*****************************************************************************
 *                       Save stream to file
 ****************************************************************************/

		/* Write mp4 data */
		my_write(mp4_buffer, m4v_size, 1, output_add_temp);
		output_add_temp = output_add_temp + m4v_size;

		filenr++;

	} while ( filenr < ARG_FRAMENO );

	
      
/*****************************************************************************
 *         Calculate totals and averages for output, print results
 ****************************************************************************/

	clock_gettime(CLOCK_REALTIME, &end_time);    
	printf("sizeof(start_time.tv_sec):%d, sizeof(start_time.tv_nsec):%d\n", sizeof(start_time.tv_sec), sizeof(start_time.tv_nsec));
	printf("s_time.tv_sec:%d, s_time.tv_nsec:%d\n", start_time.tv_sec, start_time.tv_nsec);
	printf("e_time.tv_sec:%d, e_time.tv_nsec:%d\n", end_time.tv_sec, end_time.tv_nsec);
		double execution_time = (double)end_time.tv_sec + (double)end_time.tv_nsec/1000000000.0 
		- (double)start_time.tv_sec - (double)start_time.tv_nsec/1000000000.0;
	printf("diff_time:%.4f(s)\n", execution_time);

	my_memcpy(output_add,&output_size,4);
	finish_file();

	output_size    /= filenr;

	my_printf("Avg: filesize %7d bytes\n",(int)output_size);

/*****************************************************************************
 *                            XviD PART  Stop
 ****************************************************************************/

 release_all:

	if (enc_handle)
	{	
		status = enc_stop();
		if (status)    
			my_printf("Encore RELEASE problem return value %d\n", status);
	}

 free_all_memory:
	my_free(mp4_buffer);
	my_free(in_buffer);

	return 0;

}

/*****************************************************************************
 *     Routines for encoding: init encoder, frame step, release encoder
 ****************************************************************************/

#define FRAMERATE_INCR 1001

/* Initialize encoder for first use, pass all needed parameters to the codec */
static int enc_init(void)
{
	int xerr;
	
	XVID_INIT_PARAM xinit;
	XVID_ENC_PARAM xparam;

	xinit.cpu_flags = XVID_CPU_FORCE;

	xvid_init(NULL, 0, &xinit, NULL);

	xparam.width = XDIM;
	xparam.height = YDIM;
	if ((ARG_FRAMERATE - (int)ARG_FRAMERATE) < SMALL_EPS)
	{
		xparam.fincr = 1;
		xparam.fbase = (int)ARG_FRAMERATE;
	}
	else
	{
		xparam.fincr = FRAMERATE_INCR;
		xparam.fbase = (int)(FRAMERATE_INCR * ARG_FRAMERATE);
	}
	xparam.rc_reaction_delay_factor = 16;
    xparam.rc_averaging_period = 100;
    xparam.rc_buffer = 10;
	xparam.rc_bitrate = ARG_BITRATE; 
	xparam.min_quantizer = ARG_MINQUANT;
	xparam.max_quantizer = ARG_MAXQUANT;
	xparam.max_key_interval = (int)ARG_FRAMERATE*10;

	/* I use a small value here, since will not encode whole movies, but short clips */

	xerr = xvid_encore(NULL, XVID_ENC_CREATE, &xparam, NULL);
	enc_handle=xparam.handle;

	return xerr;
}

static int enc_stop()
{
	int xerr;

	xerr = xvid_encore(enc_handle, XVID_ENC_DESTROY, NULL, NULL);
	return xerr;

}

static int enc_main(unsigned char* image, unsigned char* bitstream,
					long *streamlength, long *frametype)
{
	int xerr;

	XVID_ENC_FRAME xframe;
	XVID_ENC_STATS xstats;

	xframe.bitstream = bitstream;
	xframe.length = -1; 	/* this is written by the routine */

	xframe.image = image;
	xframe.colorspace = XVID_CSP_I420;	/* defined in <xvid.h> */

    /* let the codec decide between I-frame (1) and P-frame (0) */
    if(filenr % (ARG_NUMBEROFP + 1) == 0)
	{
		xframe.intra = 1;
	}
	else
	{
		xframe.intra = 0;
	}

	xframe.quant = ARG_QUANTI;	/* is quant != 0, use a fixed quant (and ignore bitrate) */

	xframe.motion = motion_presets[ARG_QUALITY];
	xframe.general = general_presets[ARG_QUALITY];
	xframe.quant_intra_matrix = xframe.quant_inter_matrix = NULL;

	xerr = xvid_encore(enc_handle, XVID_ENC_ENCODE, &xframe, &xstats);

	/*
	 * This is statictical data, e.g. for 2-pass. If you are not
	 * interested in any of this, you can use NULL instead of &xstats
	 */
	*frametype = xframe.intra;
	*streamlength = xframe.length;

	return xerr;
}

//************************************************************
//    memory controller by gary
//************************************************************


unsigned char * my_malloc(int size)
{
	unsigned char check = 0;
	unsigned char timer = 1;
	mem_info * temp_ptr = free_ptr;
	mem_info * add_one;

    do{
		if(temp_ptr->size >= size+16)
		{
			add_one = (mem_info *)(temp_ptr->address + (temp_ptr->size -size-16));
			add_one->size = size;
			add_one->top = divide_num;
			add_one->down = temp_ptr->down;
			add_one->address = (unsigned char *)add_one+16;
			add_one->next_info = used_ptr;
			used_ptr = add_one;
			temp_ptr->down = divide_num++;
			temp_ptr->size = temp_ptr->size - (size+16);
			check = 1;
			break;
		}
		temp_ptr = temp_ptr->next_info;
	}while(temp_ptr != NULL);

#if defined(_MSC_VER)
	fprintf(info_ptr,"** State %d **\n ",mem_state++);
	fprintf(info_ptr,"     Used state \n");
	fprintf(info_ptr," ---------------------------------------------------------------------\n");
	temp_ptr = used_ptr;
	while(temp_ptr != NULL)
	{
		fprintf(info_ptr,"%2d. Size = %8d , top = %3d , down = %3d , start address = %#08X \n",timer++,temp_ptr->size,temp_ptr->top,temp_ptr->down,temp_ptr->address);
        temp_ptr = temp_ptr->next_info;
	}
    fprintf(info_ptr,"\n");

	timer=1;
	temp_ptr = free_ptr;
	fprintf(info_ptr,"      Free state \n");
	fprintf(info_ptr," ---------------------------------------------------------------------\n");
	while(temp_ptr != NULL)
	{
		fprintf(info_ptr,"%2d. Size = %8d , top = %3d , down = %3d , start address = %#08X \n",timer++,temp_ptr->size,temp_ptr->top,temp_ptr->down,temp_ptr->address);
        temp_ptr = temp_ptr->next_info;
	}
    fprintf(info_ptr,"\n\n\n");
#endif

	if(check == 1)
	{
		malloc_total = malloc_total + size;
		return add_one->address;
	}
	else
	{
		return 0;
	}
}

void my_free(void * ptr)
{
	unsigned char check = 0;
	unsigned char timer = 1;
	mem_info * temp_ptr1 = used_ptr;
	mem_info * temp_ptr2 = temp_ptr1->next_info;
	mem_info * temp_ptr3 = free_ptr;

	if(temp_ptr1->address <= (unsigned char *)ptr && temp_ptr1->address+temp_ptr1->size >= (unsigned char *)ptr)
	{
		used_ptr = temp_ptr2;
		temp_ptr2 = temp_ptr1;
		check = 1;
	}
	else
	{
	    do
		{
			if(temp_ptr2->address <= (unsigned char *)ptr && temp_ptr2->address+temp_ptr2->size >= (unsigned char *)ptr)
			{
				temp_ptr1->next_info = temp_ptr2->next_info;
				check = 1;
			    break;
			}
		    temp_ptr1 = temp_ptr2;
		    temp_ptr2 = temp_ptr2->next_info;
		}while(temp_ptr2 != NULL);
	}

    malloc_total = malloc_total - temp_ptr2->size;

	if(check == 1)
	{

		temp_ptr1 = free_ptr;
		do
		{
			if(temp_ptr2->top == temp_ptr3->down)
			{
				temp_ptr3->down = temp_ptr2->down;
				temp_ptr3->size = temp_ptr3->size+temp_ptr2->size + 16;
				
				if(temp_ptr1 != temp_ptr3)
				{
					temp_ptr1->next_info = temp_ptr3->next_info;
				}
				else
				{
					free_ptr = temp_ptr3->next_info;
				}

				temp_ptr2  = temp_ptr3;

				if(free_ptr == NULL)
				{
					break;
				}

				temp_ptr1 = temp_ptr3 = free_ptr;
				continue;
			}

			if(temp_ptr2->down == temp_ptr3->top)
			{
				temp_ptr2->down = temp_ptr3->down;
				temp_ptr2->size = temp_ptr2->size+temp_ptr3->size+16;
                
				if(temp_ptr1 != temp_ptr3)
				{
					temp_ptr1->next_info = temp_ptr3->next_info;
				}
				else
				{
					free_ptr = temp_ptr3->next_info;
				}

				if(free_ptr == NULL)
				{
					break;
				}

				temp_ptr1 = temp_ptr3 = free_ptr;
				continue;
			}

			temp_ptr1 = temp_ptr3;
			temp_ptr3 = temp_ptr3->next_info;

		}while(temp_ptr3 != NULL);

		temp_ptr2->next_info = NULL;
		if(free_ptr != NULL)
		{
		    temp_ptr1->next_info = temp_ptr2;
		}
		else
		{
			free_ptr = temp_ptr2;
		}
	}
	else
	{
		my_printf("Error!! my_free() can't find block \n");
	}

#if defined(_MSC_VER)
	fprintf(info_ptr,"** State %d **\n ",mem_state++);
	fprintf(info_ptr,"     Used state \n");
	fprintf(info_ptr," ---------------------------------------------------------------------\n");
	temp_ptr1 = used_ptr;
	while(temp_ptr1 != NULL)
	{
		fprintf(info_ptr,"%2d. Size = %8d , top = %3d , down = %3d , start address = %#08X \n",timer++,temp_ptr1->size,temp_ptr1->top,temp_ptr1->down,temp_ptr1->address);
        temp_ptr1 = temp_ptr1->next_info;
	}
	fprintf(info_ptr,"\n");

	timer=1;
	temp_ptr1 = free_ptr;
	fprintf(info_ptr,"      Free state \n");
	fprintf(info_ptr," ---------------------------------------------------------------------\n");
	while(temp_ptr1 != NULL)
	{
		fprintf(info_ptr,"%2d. Size = %8d , top = %3d , down = %3d , start address = %#08X \n",timer++,temp_ptr1->size,temp_ptr1->top,temp_ptr1->down,temp_ptr1->address);
		temp_ptr1 = temp_ptr1->next_info;
	}
    fprintf(info_ptr,"\n\n\n");
#endif

}

void my_malloc_init(void)
{
	unsigned char check = 0;
	unsigned char timer = 1;
	mem_info * temp_ptr ;
    mem_info *first;

#if defined(_MSC_VER)    
   
	info_ptr = fopen(MEMORY_INFO,"wb");
	if(info_ptr == NULL)
	{
		    my_printf("Error opening MEMORY information file %s\n", MEMORY_INFO);
			exit(0);
	}

    mem_add = (unsigned char *)	malloc(MALLOC_SIZE);  //5M
#else
    mem_add = (unsigned char *)MEM_ADD ;
#endif

	first = (mem_info *)mem_add;

	first->size = MALLOC_SIZE-16;
    first->top = divide_num++;
	first->down = divide_num++;
    first->address = mem_add+16;
	first->next_info = NULL;
	free_ptr = first;

#if defined(_MSC_VER)
	fprintf(info_ptr,"** State %d **\n ",mem_state++);
	fprintf(info_ptr,"     Used state \n");
	fprintf(info_ptr," ---------------------------------------------------------------------\n");
	temp_ptr = used_ptr;
	while(temp_ptr != NULL)
	{
		fprintf(info_ptr,"%2d. Size = %8d , top = %3d , down = %3d , start address = %#08X \n",timer++,temp_ptr->size,temp_ptr->top,temp_ptr->down,temp_ptr->address);
        temp_ptr = temp_ptr->next_info;
	}
    fprintf(info_ptr,"\n");

	timer=1;
	temp_ptr = free_ptr;
	fprintf(info_ptr,"      Free state \n");
	fprintf(info_ptr," ---------------------------------------------------------------------\n");
	while(temp_ptr != NULL)
	{
		fprintf(info_ptr,"%2d. Size = %8d , top = %3d , down = %3d , start address = %#08X \n",timer++,temp_ptr->size,temp_ptr->top,temp_ptr->down,temp_ptr->address);
        temp_ptr = temp_ptr->next_info;
	}
    fprintf(info_ptr,"\n\n\n");
#endif
}

void my_read(void *target,int size,int n,unsigned char * file)
{
	int x = size*n;
    my_memcpy(target,file,x);
}

void my_write(void *Source,int size,int n,unsigned char *file)
{
	int x = size*n;
	my_memcpy(file,Source,x);
}

void my_memcpy(void *target,void *source,int size)
{
	int i;
	unsigned char * target_ptr = target;
	unsigned char * source_ptr = source;
    for(i=0 ; i<size ; i++)
	{
		*(target_ptr+i) = *(source_ptr+i);
	}
}

void * my_memset(void *buffer,int c,int count)
{
	int i;
	unsigned char * temp = buffer;
	for(i=0;i<count;i++)
	{
		temp[i]=c;
	}
	return buffer;
}
//************************************************************
//    print information by gary
//************************************************************

void my_printf(char *format, ...)
{
#if defined(_MSC_VER)
	#include <stdio.h>
    #include <stdarg.h>

	va_list args;
    va_start(args, format);
	vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
#else

#endif
}

//************************************************************
//    I/O by gary
//************************************************************

void input_file(void)
{
#if defined(_MSC_VER)
	FILE *in_file;

	in_file = fopen(ARG_INPUTFILE, "rb");
	if (in_file == NULL) 
	{
			my_printf("Error opening input file %s\n", ARG_INPUTFILE);
			exit(0);
	}

	input_add = (unsigned char *) malloc(FILE_SIZE);
	if (!input_add)
	{
        my_printf("Error!! in input_add \n");
	    exit(0);	
	}
	fread(input_add, FILE_SIZE, 1, in_file);
	fclose(in_file);
#else
	input_add = (unsigned char *)INPUT_ADD ;
#endif
}

void output_file(void)
{
#if defined(_MSC_VER)
	out_file = fopen(ARG_OUTPUTFILE, "wb");
	if (out_file == NULL) 
	{
			my_printf("Error opening out_file file %s\n", ARG_OUTPUTFILE);
			exit(0);
	}

	output_add = (unsigned char *) malloc(ARG_FRAMENO*XDIM*YDIM);
	output_add_temp = output_add+4;
	if (!output_add)
	{
        my_printf("Error!! in output_add \n");
	    exit(0);	
	}
#else
	output_add = (unsigned char *)OUTPUT_ADD ;
	output_add_temp = (unsigned char *)OUTPUT_ADD + 4;
#endif
}

void finish_file(void)
{
#if defined(_MSC_VER)
	fwrite(output_add+4,output_size,1,out_file);
	fclose(out_file);
#else
#endif
}

//************************************************************
//    Math function by gary
//************************************************************
float my_sqrt(float value)
{
	float p0,p,d;
    float TOL=0.0000000001 ;
    int i=1;
	int N=25;

	p0 = 1.0 ;

	do
	{
		p = p0 - ((p0*p0 - value)/(p0*2)) ;
               
        if((p-p0)<0)
		{

			d = -(p-p0) ;
		}
        else
		{
            d =  (p-p0) ;
		}

        i++ ;
        p0 = p ;
	}while( (i-1)< N && d > TOL) ;

	return p0;
}
