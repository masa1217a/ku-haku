#include "ketugou.h"

double act_time;
static double start, end;

void ck_init(){
	act_time=0;
}
void ck_start()
{
	start = clock();
	while(1){
		end = millis();
	}
	printf("処理開始：%g\n", act_time);
}

void ck_end()
{
	end = millis();
	act_time = act_time + (end - start) / 100000;
	if(st == 1)
		printf("停止：%g\n", act_time);
	if(mot_state == MOT_Clean)
		printf("クリーンモード移行：%g\n", act_time);
	//else
		//printf("処理終了：%g\n",  act_time);
}
