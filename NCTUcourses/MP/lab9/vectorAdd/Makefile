CXXFLAG = -O0 -gencode=arch=compute_10,code=\"sm_10,compute_10\"  -gencode=arch=compute_20,code=\"sm_20,compute_20\" --compiler-options -fno-strict-aliasing  -I. -I/usr/local/cuda/include -I/root/NVIDIA_GPU_Computing_SDK/C/common/inc -I/root/NVIDIA_GPU_Computing_SDK/shared/inc -DUNIX 
LDFLAG = -fPIC -L/usr/local/cuda/lib -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -L/usr/local/cuda/lib -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -lcutil_i386 -lshrutil_i386

all:									
	nvcc $(CXXFLAG) -c -o main.o vectorAdd.cu
	
	g++ -o main.exe *.o  $(LDFLAG) 
			
clean:
	rm -rf *.o main	
	rm -rf *.log	

run:
	./main.exe 1000
