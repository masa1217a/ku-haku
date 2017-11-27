CC := gcc
LDLIBS := -lwiringPi -lwiringPiDev  -pthread -l bcm2835 -lm

ketugouX:ketugouX.o

clean:
	$(RM) *.o
	$(RM) ketugouX
