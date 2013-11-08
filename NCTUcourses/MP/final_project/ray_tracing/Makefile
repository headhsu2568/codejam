obj = c-ray-f.o
bin = c-ray-f

CC = g++
CFLAGS = -O3 -ffast-math

$(bin): $(obj)
	$(CC) -o $@ $(obj) -lm -lpthread -lrt -pg

.PHONY: clean
clean:
	rm -f $(obj) $(bin) *.ppm

.PHONY: install
install:
	cp $(bin) /usr/local/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f /usr/local/bin/$(bin)

run:
	./c-ray-f -r 10 -i sphfract -o test.ppm
