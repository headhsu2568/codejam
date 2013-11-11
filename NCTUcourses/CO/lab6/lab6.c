#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct set_def{
    struct line_def* line;
};

struct line_def{
    char vbit;
    char rbit;
    int tag;
    //int* data;
};

int main(int argc,char** argv){
    if(argc!=11){
        printf("usage:-t <trace_file> -s <cache_size> -l <line_size> -a <associativity> -r <FIFO/LRU>");
        exit(0);
    }
    int i,j;
    char *trace_file;
    int cache_size;
    int line_size;
    int associativity;
    int replacement;
    for(i=1;i<11;i+=2){
        if(argv[i][0]=='-' && argv[i][1]=='t'){
            trace_file=argv[i+1];
            printf("Trace file:%s\n",trace_file);
        }
        else if(argv[i][0]=='-' && argv[i][1]=='s'){
            cache_size=atoi(argv[i+1]);
            printf("Cache size:%d Byte\n",cache_size);
        }
        else if(argv[i][0]=='-' && argv[i][1]=='l'){
            line_size=atoi(argv[i+1]);
            printf("Line size:%d Byte\n",line_size);
        }
        else if(argv[i][0]=='-' && argv[i][1]=='a'){
            associativity=atoi(argv[i+1]);
            associativity==1?printf("Associativity:direct-mapped\n"):printf("Associativity:%d-way\n",associativity);
        }
        else if(argv[i][0]=='-' && argv[i][1]=='r'){
            replacement=atoi(argv[i+1]);
            replacement==1?printf("Replacement:LRU\n"):printf("Replacement:FIFO\n");
        }
    }
    struct set_def* cache;
    const unsigned int set_num=(cache_size/line_size)/associativity;
    const unsigned int tag_mask=0xffffffff*line_size*set_num;
    const unsigned int index_mask=(set_num-1)*line_size;
    //printf("set_num:%d\n",set_num);
    cache=(struct set_def*)malloc(sizeof(struct set_def)*set_num);
    //initialize
    for(i=0;i<set_num;++i){
        cache[i].line=(struct line_def*)malloc(sizeof(struct line_def)*associativity);
        for(j=0;j<associativity;++j){
            cache[i].line[j].vbit=0;
            cache[i].line[j].rbit=0;
        }
    }
    //read file
    FILE* fp=fopen(trace_file,"r");
    long int miss_counter=0;
    long int hit_counter=0;
    while(1){
        char v_addr[12];
        int v_addr_int;
        int tag;
        int index;
        int cacheit=0;
        int k;
        fgets(v_addr,sizeof(v_addr),fp);
        if(feof(fp)){
            break;
        }
        v_addr_int=strtoul(v_addr,NULL,16);
        tag=v_addr_int/(line_size*set_num);
        index=(v_addr_int&index_mask)/line_size;
        //printf("v_addr:%stag:%.8x\nindex:%d\n",v_addr,tag,index);
        //check hit --FIFO version
        for(k=0;k<associativity && replacement==0;++k){
            if(cache[index].line[k].tag==tag && cache[index].line[k].vbit==1){
                cache[index].line[k].vbit=1;
                //printf("hit! : %s",v_addr);
                ++hit_counter;
                cacheit=associativity+2;
                break;
            }
        }
        //check hit --LRU version
        for(k=0;k<associativity && replacement==1;++k){
            if(cache[index].line[k].tag==tag && cache[index].line[k].vbit==1){
                cache[index].line[k].vbit=1;
                cacheit=cache[index].line[k].rbit;
                cache[index].line[k].rbit=associativity+1;
                //printf("hit! : %s",v_addr);
                ++hit_counter;
                break;
            }
        }
        //check valid bit=0
        for(k=0;k<associativity && cacheit==0;++k){
            if(cache[index].line[k].vbit==0){
                //set valid bit
                cache[index].line[k].vbit=1;
                //set reference bit
                cache[index].line[k].rbit=associativity+1;
                //set tag
                cache[index].line[k].tag=tag;
                //printf("compulsory miss! : %s",v_addr);
                ++miss_counter;
                cacheit=1;
                break;
            }
        }
        //valid bit=1,select replacement
        for(k=0;k<associativity && cacheit==0;++k){
            if(cache[index].line[k].rbit==1){
                cache[index].line[k].vbit=1;
                cache[index].line[k].rbit=associativity+1;
                cache[index].line[k].tag=tag;
                //printf("conflict miss! : %s",v_addr);
                ++miss_counter;
                cacheit=1;
                break;
            }
        }
        for(k=0;k<associativity;++k){
            if(cache[index].line[k].rbit>=cacheit){
                --cache[index].line[k].rbit;
            }
        }
    }
    printf("hit:%ld\nmiss:%ld\nmiss rate:%f%%\n",hit_counter,miss_counter,((float)miss_counter/(float)(miss_counter+hit_counter))*100);
    for(i=0;i<set_num;++i){
        printf("index:%2d\t",i);
        for(j=0;j<associativity;++j){
            printf("%2d  ",cache[i].line[j].rbit);
        }
        printf("\n");
    }
    return 0;
}
