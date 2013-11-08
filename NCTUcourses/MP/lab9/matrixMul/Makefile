CFLAGS_PT = -g -O0 
LDFLAGS_PT = -g -lpthread -lrt -fopenmp

CXXFLAG_CU = -O0 -gencode=arch=compute_10,code=\"sm_10,compute_10\"  -gencode=arch=compute_20,code=\"sm_20,compute_20\" --compiler-options -fno-strict-aliasing  -I. -I/usr/local/cuda/include -I/root/NVIDIA_GPU_Computing_SDK/C/common/inc -I/root/NVIDIA_GPU_Computing_SDK/shared/inc -DUNIX 
LDFLAG_CU = -fPIC -L/usr/local/cuda/lib -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -L/usr/local/cuda/lib -L/root/NVIDIA_GPU_Computing_SDK/C/lib -L/root/NVIDIA_GPU_Computing_SDK/C/common/lib/linux -L/root/NVIDIA_GPU_Computing_SDK/shared/lib -lcudart -lcutil_i386 -lshrutil_i386

all:									
	make pt
	make cu

pt:
	g++ $(CFLAGS_PT) -c -o main_pt.o main.cpp
	g++ -o main_pt.exe *.o  $(LDFLAGS_PT) 

cu:
	nvcc $(CXXFLAG_CU) -c -o main_cu.o main.cu
	g++ -o main_cu.exe *.o  $(LDFLAG_CU) 

run:
	make runpt
	make runcu

runpt:
	./main_pt.exe 1000 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 100 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 10 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 2 1000  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 1 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 1000 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 100 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 10 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 2 100  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 1 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 1000 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 100 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 10 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 2 10  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 1 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 1000 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 100 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 10 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 2 2  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_pt.exe 1 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g

runcu:
	./main_cu.exe 1000 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 100 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 10 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 2 1000  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 1 1000 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 1000 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 100 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 10 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 2 100  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 1 100 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 1000 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 100 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 10 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 2 10  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 1 10 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 1000 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 100 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 10 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 2 2  | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g
	./main_cu.exe 1 2 | head -n 2 | tail -n 1 | sed -e s/Program.runs.in.//g | sed -e s/.secs//g

clean:
	rm -rf *.exe 
	rm -rf *.o main
	rm -rf *.log
