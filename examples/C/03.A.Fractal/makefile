CC=cc
LD=$(CC)
CCOPT=

%.o : %.c
	$(CC) -c $(CCOPT) -o $@ $<

program : release

# test target too

release   : CCOPT += -O3 -DNDEBUG
debug     : CCOPT += -g -DDEBUG
debug_test: CCOPT += -g -DDEBUG

release    : main.o      image.o fractal.o
	$(LD) -o $@ $^

debug      : main.o      image.o fractal.o
	$(LD) -o $@ $^

debug_test : test_main.o image.o fractal.o
	$(LD) -o $@ $^

clean :
	- rm *.o
	- rm *.hdrdep.bak

cleanall : clean
	- rm release debug debug_test
	- echo > makefile.hdrdep

mrproper : cleanall

# Advanced feature: #include file dependencies
# Run `make depends` before `make all`

depends :
	makedepend -f makefile.hdrdep *.c

include makefile.hdrdep
