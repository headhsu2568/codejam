#include <stdio.h>
#include <stdlib.h>
#include <cutil_inline.h>
#include <time.h>
#include <assert.h>
#include <sm_11_atomic_functions.h>
#include <shrUtils.h>


struct position_t
{
    int lock;
    int diff;
    int width;
    int height;
};

int frame_width = 1024;
int frame_height = 1024;

int body_width = 32;
int body_height = 32;

int* h_frame = NULL;
int* h_body = NULL;
int* h_diff = NULL;
position_t* h_position = NULL;

int* d_frame = NULL;
int* d_body = NULL;
int* d_diff = NULL;
position_t* d_position = NULL;

int LOOP_NUM = 1;

unsigned int seed = 0x1234567;

//timespec start_time;
//timespec end_time;

__global__ void body_track(int* frame, int frame_width, int frame_height,
        int* body, int body_width, int body_height, int* diff, position_t* pos)
{

};

unsigned int myrand(unsigned int *seed, unsigned int input)
{
    *seed ^= (*seed << 13) ^ (*seed >> 15) + input;
    *seed += (*seed << 17) ^ (*seed >> 14) ^ input;
    return *seed;
};

void sig_check()
{
    unsigned int sig = 0x1234567;
    for(int i = 0; i < frame_height; i++)
        for(int j = 0; j < frame_width; j++)
            myrand(&sig, h_diff[i*frame_width+j]);

    //myrand(&sig, h_position->height);
    //myrand(&sig, h_position->width);

    printf("Computed check sum signature:0x%08x\n", sig);
    if(sig == 0x17dd3971)
        printf("Result check by signature successful!!\n");
    else
        printf("Result check by signature failed!!\n");
}

void show_array(int* array, int width, int height)
{
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
            printf("%03d, ", array[i*width+j]);
        printf("\n");
    }
    printf("\n");
}

int main (int argc, char *argv[])
{
    // get the dimension of array
    assert(argc == 2);
    LOOP_NUM = atoi(argv[1]);

    printf("LOOP_NUM:%d\n", LOOP_NUM);

    // Allocate input vectors h_A and h_B in host memory
    h_frame = (int*)malloc(frame_width*frame_height*sizeof(int));
    h_body = (int*)malloc(body_width*body_height*sizeof(int));
    h_diff = (int*)malloc(frame_width*frame_height*sizeof(int));
    h_position = (position_t*)malloc(sizeof(position_t));
    assert(h_frame);
    assert(h_body);
    assert(h_diff);
    assert(h_position);

    // initial frame, body, diff
    for(int i = 0; i < frame_height; i++)
        for(int j = 0; j < frame_width; j++)
        {
            h_frame[i*frame_width+j] = myrand(&seed, i*j) & 0xff;
            h_diff[i*frame_width+j] = 0;
        }

    for(int i = 0; i < body_height; i++)
        for(int j = 0; j < body_width; j++)
        {
            h_body[i*body_width+j] = myrand(&seed, i*j) & 0xff;
        }

    h_position->lock = 0;
    h_position->diff = 0x7fffffff;
    h_position->width = -1;
    h_position->height = -1;

    //clock_gettime(CLOCK_REALTIME, &start_time);

    // Allocate vectors in device memory
    cutilSafeCall( cudaMalloc((void**)&d_frame, frame_width*frame_height*sizeof(int)) );
    cutilSafeCall( cudaMalloc((void**)&d_body, body_width*body_height*sizeof(int)) );
    cutilSafeCall( cudaMalloc((void**)&d_diff, frame_width*frame_height*sizeof(int)) );
    cutilSafeCall( cudaMalloc((void**)&d_position, sizeof(*h_position)) );

    // Copy vectors from host memory to device memory
    cutilSafeCall( cudaMemcpy(d_frame, h_frame, frame_width*frame_height*sizeof(int), cudaMemcpyHostToDevice) );
    cutilSafeCall( cudaMemcpy(d_body, h_body, body_width*body_height*sizeof(int), cudaMemcpyHostToDevice) );
    cutilSafeCall( cudaMemcpy(d_position, h_position, sizeof(*h_position), cudaMemcpyHostToDevice) );

    // Invoke kernel
    int threadsPerBlock = 256;
    int blocksPerGrid = (frame_height*frame_width + threadsPerBlock - 1) / threadsPerBlock;
    body_track<<<blocksPerGrid, threadsPerBlock>>>(d_frame, frame_width, frame_height, d_body, body_width, body_height, d_diff, d_position);
    cutilCheckMsg("kernel launch failure");

    // Copy result from device memory to host memory
    // h_C contains the result in host memory
    cutilSafeCall( cudaMemcpy(h_diff, d_diff, frame_width*frame_height*sizeof(int), cudaMemcpyDeviceToHost) );
    cutilSafeCall( cudaMemcpy(h_position, d_position, sizeof(*h_position), cudaMemcpyDeviceToHost) );

    //clock_gettime(CLOCK_REALTIME, &end_time);

    printf("position(%d,%d):%d\n", h_position->width, h_position->height, h_position->diff);
    //printf("sizeof(start_time.tv_sec):%d, sizeof(start_time.tv_nsec):%d\n", sizeof(start_time.tv_sec), sizeof(start_time.tv_nsec));
    //printf("s_time.tv_sec:%d, s_time.tv_nsec:%d\n", start_time.tv_sec, start_time.tv_nsec);
    //printf("e_time.tv_sec:%d, e_time.tv_nsec:%d\n", end_time.tv_sec, end_time.tv_nsec);
    //double execution_time = (double)end_time.tv_sec + (double)end_time.tv_nsec/1000000000.0
    //  - (double)start_time.tv_sec - (double)start_time.tv_nsec/1000000000.0;
    //printf("diff_time:%.4f(s)\n", execution_time);

    //show_array(h_frame, frame_width, frame_height);
    //show_array(h_body, body_width, body_height);
    //show_array(h_diff, frame_width, frame_height);

    sig_check();

    //cutilSafeCall( cudaThreadExit() );

    return 0;
}
