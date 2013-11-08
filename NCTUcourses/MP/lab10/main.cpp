#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

struct position_t
{
    int width;
    int height;
    int diff;
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

unsigned int seed = 0x1234567;

//timespec start_time;
//timespec end_time;

void body_track(int* frame, int frame_width, int frame_height,
        int* body, int body_width, int body_height, int* diff, position_t* pos)
{
    for(int i = 0; i < (frame_height-body_height+1); i++)
    {
        for(int j = 0; j < (frame_width-body_width+1); j++)
        {
            int* sub_frame = frame + i*frame_width + j;
            int total_diff = 0;

            for(int k = 0; k < body_height; k++)
            {
                for(int l = 0; l < body_width; l++)
                {
                    int diff = sub_frame[k*frame_width+l] - body[k*body_width+l];
                    diff = diff < 0 ? diff * -1 : diff;
                    total_diff += diff;
                }
            }

            diff[i*frame_width+j] = total_diff;

            if(total_diff < pos->diff)
            {
                pos->diff = total_diff;
                pos->height = i;
                pos->width = j;
                pos->diff = total_diff;
            }
        }
    }
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

    h_position->width = -1;
    h_position->height = -1;
    h_position->diff = 0x7fffffff;

    //clock_gettime(CLOCK_REALTIME, &start_time);

    body_track(h_frame, frame_width, frame_height, h_body, body_width, body_height, h_diff, h_position);

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

    return 0;
}
