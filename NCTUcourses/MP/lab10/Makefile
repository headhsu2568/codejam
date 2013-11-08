CXXFLAG64 = -O0 -gencode=arch=compute_11,code=\"sm_11,compute_11\"  -gencode=arch=compute_20,code=\"sm_20,compute_20\" --compiler-options -fno-strict-aliasing  -I. -I/usr/local/cuda/include -I/root/NVIDIA_GPU_Computing_SDK/C/common/inc -I/root/NVIDIA_GPU_Computing_SDK/shared/inc -DUNIX 
LDFLAG64 = -fPIC -L/usr/local/cuda/lib64 -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -L/usr/local/cuda/lib64 -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -lcutil_x86_64 -lshrutil_x86_64

CXXFLAG = -O0 -gencode=arch=compute_11,code=\"sm_11,compute_11\"  -gencode=arch=compute_20,code=\"sm_20,compute_20\" --compiler-options -fno-strict-aliasing  -I. -I/usr/local/cuda/include -I/root/NVIDIA_GPU_Computing_SDK/C/common/inc -I/root/NVIDIA_GPU_Computing_SDK/shared/inc -DUNIX 
LDFLAG = -fPIC -L/usr/local/cuda/lib -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -L/usr/local/cuda/lib -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -lcutil_i386 -lshrutil_i386

CFLAGS_INIT = -g -O0 
LDFLAGS_INIT = -g -lpthread -lrt -fopenmp

all:									
	nvcc $(CXXFLAG) -c -o main.o main.cu
	g++ -o main *.o  $(LDFLAG) 
	g++ $(CFLAGS_INIT) -c -o main_init.o main.cpp
	g++ -o main_init *.o  $(LDFLAGS_INIT) 
			
clean:
	rm -rf *.o main	
	rm -rf *.log	

run:
	./main 1
