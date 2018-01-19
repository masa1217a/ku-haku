CC = gcc
LDLIBS = -lwiringPi -lwiringPiDev  -pthread -l bcm2835 -lm
OBJGROUP = ketugouX.o thread_kouden.o sys_format.o thread_speed.o log.o thread_motor.o setting.o adc.o clock.o#ここにファイルを追加する

ketugouX:$(OBJGROUP)
	$(CC) -o ketugouX $(OBJGROUP) $(LDLIBS)
	$(RM) *.o

clean:
	$(RM) *.o
	$(RM) ketugouX
