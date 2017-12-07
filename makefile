CC = gcc
LDLIBS = -lwiringPi -lwiringPiDev  -pthread -l bcm2835 -lm
OBJGROUP = drin_crash.o th_kouden.o sys_format.o thread_speed.o #ここにファイルを追加する



ketugouX:$(OBJGROUP)
	$(CC) -o drin_crash $(OBJGROUP) $(LDLIBS)

clean:
	$(RM) *.o
	$(RM) ketugouX
