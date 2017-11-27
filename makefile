CC = gcc
LDLIBS = -lwiringPi -lwiringPiDev  -pthread -l bcm2835 -lm
OBJGROUP = ketugouX.o   #ここにファイルを追加する

ketugouX:$(OBJGROUP)
	$(CC) -o ketugouX $(OBJGROUP) $(LDLIBS)

clean:
	$(RM) *.o
	$(RM) ketugouX
