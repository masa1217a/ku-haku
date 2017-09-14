#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include<errno.h>
#include<softPwm.h> 
#include<time.h>
#include <lcd.h>
#include <bcm2835.h>
#include <mcp23017.h>
#include <math.h>

//#define mot1_seiten 0						// 脱水モーター　正転
//#define mot1_gyakuten 2					// 脱水モーター　逆転
//#define mot1_gyakuten 3					// 脱水モーター　停止
//#define mot2_seiten 21					// 減容モーター　正転
//#define mot2_gyakuten 22					// 減容モーター　逆転
//#define mot2_gyakuten 23					// 減容モーター　フリーラン

#define BUTTON1   	0						// スタートボタン
#define BUTTON2  	2						// ストップボタン
#define BUTTON3   	3						// 電源ボタン

#define	LED1	105							//LED　通常時
#define	LED2	106							//LED　管理時
#define SW1	108								// 脱水　電源
#define SW2	109								// 脱水　正/逆
#define SW3	110								// 減容　電源
#define SW4	111								// 減容　正/逆

#define	RED	204								//表示灯	赤
#define	YELLOW	203							//表示灯	黃
#define GREEN  202							//表示灯	緑
#define BUZZER  205 						//ブザー

#define SPEED 305							//速度センサ
#define PHOTO 307							//光電センサ　受光
#define KINSETU1  308 						// 近接センサ1
#define KINSETU2	309						// 近接センサ2
#define KINSETU3	310						// 近接センサ3
#define KINSETU4	311						// 近接センサ4
#define KINSETU5	312						// 近接センサ5

#define LOG_OK  0     						/* テスト関数戻り値(正常)*/
#define LOG_NG -1     					  	/* テスト関数戻り値(異常)*/
#define LOG_FILE     "LOG/log.txt"        	/* ログディレクトリ(通常)  */

#define	MOT_OFF	0
#define	MOT_For	1 							//Forwards正転
#define	MOT_Rev	2							//Reversal逆転

volatile unsigned long time_prev = 0, time_now;
unsigned long time_chat =500;
int btn1=0, btn2=0,sw1=0,pht=0;
int st=0, t1=0, t2=0, mode=1,state=0,dec_time=0,kenti=0,error=0,teisi=0,act=0;
int  fd_lcd=0,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
int mot_state = MOT_OFF;
int volt_distance(float volt);
double map(double v);

int adc01(void);
int adc02(void);
int ctrl_c(void);
int read_speed(void);

static int distance_adc01_ch0 = 0;
static int distance_adc01_ch1 = 0;
static int distance_adc01_ch2 = 0;
static int distance_adc01_ch3 = 0;
static int distance_adc01_ch4 = 0;
static int distance_adc01_ch5 = 0;
static int mot_speed = 0;

static double temp_adc02_ch0 = 0;
static double temp_adc02_ch1 = 0;
static double temp_adc02_ch2 = 0;
static double temp_adc02_ch3 = 0;

static FILE *log_file;        					/* ログ */

pthread_t normal;	 
pthread_t admin;	 
pthread_t photo;	
pthread_t MOT;

/*****************************************
*				外部割り込み				 *
*****************************************/
//シャットダウンボタン
void shutdown(void)
{
	WiringPiSetupGpio();
	bcm2835_close();
    system("shutdown -h now");
}

//一時停止ボタン
void stop(void){
	if(act==1){
		time_now = millis();
		if(time_now-time_prev > time_chat){
			st = 1;
			teisi=1;
			mot_state = MOT_OFF;
			printf("一時停止\n");
		}
	time_prev = time_now;
	}
}

/*****************************************
*			スレッド処理					 *
*****************************************/
//フォトトランジスタ
int thread_photo(void *ptr){
	int i=0;
	int sum=0;
	int	time_count=0;

	for(i=0; i<100; i++){
		sum += digitalRead( PHOTO );
	}
	
	if(sum >= 1)
		pht = 1;
	else
		pht = 0; 
	
	sum = 0;
	
	if(pht != 0)printf("物体検知まで待つ\n");
	while(pht != 0)	//pht:1=受光　　pht:0=物体検知
	{ 
		if(st  == 1 || kenti==1) break; 
		
		for(i=0; i<100; i++){
			sum += digitalRead( PHOTO );
		}
	
		if(sum >= 1)
			pht = 1;
		else
			pht = 0; 
			
		sum = 0;	
		delay(100);
	}
	
	if(st==0)kenti=1;
    time_count=0;
   	sum = 0;
 	if(pht==0 ) printf("物体検知\n");
 	if(pht==0 ) printf("物体がなくなるまで待つ\n");
 
	while(1){
		dec_time = 0;
		
		for(i=0; i<100; i++){
			sum += digitalRead( PHOTO );
		}
				
		if(sum >= 1)
			pht = 1;
		else
			pht = 0; 
		
		if(st  == 1) break; 
		
		if(pht==1) 
		{
			printf("カウント開始\n");
			time_count=0;
			while(pht == 1){
				time_count++;
				
				for(i=0; i<100; i++){
					sum += digitalRead( PHOTO );
				}
				
				if(sum >= 1)
					pht = 1;
				else
					pht = 0; 
					
				if(dec_time >= 5)
				{
					dec_time = 0;
					st = 1;
					kenti=0;
					teisi=0;
					mot_state = MOT_OFF;
					printf("終了\n");
				}
				else 
				{
					if(time_count>4) {
						dec_time++;
						printf("経過時間　＝　%d　秒 \n",dec_time);		
						time_count=0;			
					}
				}
				if(st==1) break;
				sum = 0;
				delay(100);
			}
		}
		sum = 0;
		delay(100);
	}
	return 0;
}

//モータースレッド
/*
int thread_MOT(void)
{
	digitalWrite(mot_seiten1,0);
	digitalWrite(mot_gyakuten1,0);

	softPwmWrite(pwm,mot_speed);

	for(;;){
		switch(mot_state){
			case MOT_OFF: //モータがOFFの場合
				digitalWrite(mot_gyakuten,0);			//mot_gyakutenが1だったら逆転動作する
										  				//0だったら動作を停止する
				digitalWrite(mot_seiten,0);   			///mot_gyakutenが1だったら正転動作する
				break;
			
			case MOT_For://モーターが正転の場合			
				digitalWrite(mot_gyakuten,0);
				usleep(50000);
				digitalWrite(mot_seiten,1);
				break;

			case MOT_Rev://モータが逆転の場合
				digitalWrite(mot_seiten,0);
				usleep(50000);
				digitalWrite(mot_gyakuten,1);
				break;
			
			default: 
				printf("デフォルトです\n");
				break;
		}
	}
}
*/

//速度センサスレッド
int thread_speed(void *ptr){

  int speed_count = 0;
  int sp_flag = 0;
  //int g_flg = 0;
  int gear_  = 21;
  
  int start, end ;
  
  //struct timeval s, e;
  //gettimeofday( &s, NULL);
  
  //int i;
  double ck_sec;
  
  /* Start main routine */
  printf("start\n");
   start = clock();
   //printf("%d\n", start);
  for(;;) {
    read_speed();
    usleep(100);
    //printf("%d\n",status_speed);
    
    /*
  *  status_speedについて
	 *      1 : 歯車の凸部分の検出
	 *      0 : 歯車の凹部分の検出
	 * 　凸凹は１セットで検出
     */
    if (status_speed == 1 && sp_flag == 0) {
		  //printf("%d\n",status_speed);
		  sp_flag = 1;
		  
	  }
	  
	  /*  
	   * ギアの歯の数分カウントしたらそこまでの 時間を算出する
	   */
	  if(status_speed == 0 && sp_flag == 1){
			 // printf("%d\n",status_speed);
			  
			  speed_count++;
			   printf("count : %d\n\n", speed_count);
			  
			  if( (speed_count % gear_ ) == 0 ){
				  end = clock();
				  ck_sec = (double)(end - start) / CLOCKS_PER_SEC;
				  //printf("end : %d\n", end);
				  printf("%.3f sec\n", ck_sec);
				  start = clock();
			  }
			  sp_flag = 0;
	  }
	  end = clock();
	  ck_sec = (double)(end - start) / CLOCKS_PER_SEC;
	  /*
	   * 歯車が３秒間止まっていたらモーターを停止させる
	   */
	   /*
	  if( sp_flag == 0 && ck_sec >= 3 ){
	  			LOG_PRINT("モーターに過負荷がかかっている可能性あり", LOG_NG);
				return -1;
		}
		if( sp_flag == 1 && ck_sec >= 3 ){
				LOG_PRINT("モーターに過負荷がかかっている可能性あり", LOG_NG);
				return -1;
		}
		 */ //usleep(1*100);
  }
  return 0;
}

//近接センサスレッド
int thread_kinsetu(void *ptr){
	
	while(1){
		kinsetu1=digitalRead(KINSETU1);
		kinsetu2=digitalRead(KINSETU2);
		kinsetu3=digitalRead(KINSETU3);
		kinsetu4=digitalRead(KINSETU4);
		kinsetu5=digitalRead(KINSETU5);				
		
		delay(100);			
	}
	return 0;
}

//通常モード
int thread_normal(void *ptr)
{
	int	time_count=0;
	act=0;
	st =0;	    
	mode=0;
	printf("通常モード\n");
    digitalWrite(LED1, 1);
    digitalWrite(LED2, 0); 
	while(1){
			if(error ==0){
				digitalWrite(RED, 1);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 1);
			}
			if(error == 1){
				digitalWrite(RED, 1);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 0);				
			}
			
		btn1=digitalRead(BUTTON1);
		btn2=digitalRead(BUTTON2);
		sw1=digitalRead(SW1);
		
		if(sw1 == 1)	state = 0;	
		else   state = 1;

/****************************************************************************/
/*							スタートボタン押した時の動作								*/
/****************************************************************************/
		if(btn2==0 && btn1==1) {							
			st =0;	    
			if(kinsetu1 == 0){
					printf("扉を閉めてください\n");
					LOG_PRINT("扉が開いている", LOG_NG);
					error=1;
					st=1;
					delay(600);
			}else LOG_PRINT("扉が閉まっている", LOG_OK);

			adc01();
			if(st==0&&teisi==0 && distance_adc01_ch0<20)
			{
				printf("エラー:スポンジの量が多いです\n");
				LOG_PRINT("投入口のスポンジの量が多い",　LOG_NG);
				error=1;
				st=1;
				delay(600);
			}else if(st==0&&teisi==0  &&  distance_adc01_ch0>40)
			{
				printf("エラー:スポンジを投入してください\n");
				LOG_PRINT("投入口にスポンジがない", LOG_NG);
				error=1;
				st=1;
				delay(600);
			}else LOG_PRINT("投入口のスポンジの量がちょうどいい", LOG_OK);

			if(st==0)  
			{
				printf("スタート\n");
				pthread_create( &photo, NULL, (void*(*)(void*))thread_photo, NULL);	//スレッド[pth]スタート
				LOG_PRINT("スタート", LOG_OK);
				error=0;
				act=1;
			}
			if(error==0){
				digitalWrite(RED, 0);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 1);
			}
			if(error == 1){
				digitalWrite(RED, 1);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 0);				
			}
			if(st==0 && state==0)	mot_state = MOT_For;
			if(st==0 && state==1)	mot_state = MOT_Rev;
			while(1){
				time_count++;
				
				if(time_count > 9){
					adc02();
					time_count = 0;
				}
				
				if(st  == 1) 
				{
					act=0;
					mot_state = MOT_OFF;
					break; 
				}
				if(kinsetu1  == 0) 
				{
					printf("扉を閉めてください\n");
					LOG_PRINT("扉が開いている", LOG_NG);
					error=1;
					st = 1;
					teisi=1;
					act=0;
					mot_state = MOT_OFF;
					break; 
				}
				delay(500); 
			}
		}
/****************************************************************************/
/*				(一時停止→スタートボタン)を長押しした時の動作							*/
/****************************************************************************/
		if(btn2==1) {										
			if(btn1 == 1){
				while(1){
					btn1=digitalRead(BUTTON1);
					btn2=digitalRead(BUTTON2);
					t1++;
					if(btn1==0 || btn2==0) 
					{
						t1=0;
						break;
					}
					if(t1 >= 50){
						t1=0;
						mode=2;								//スレッド[admin]スタート
						LOG_PRINT("管理者モードに移行", LOG_OK);
						break;
					}
					delay(100);
				}
			}
		}
		if(mode==2) break;
	}
    return 0;
}

//管理モード
int  thread_admin(void *ptr)
{
	st =0;	    
	mode=0;
	printf("管理者モード\n");
    digitalWrite(LED1, 0);
    digitalWrite(LED2, 1); 
    while(1){
		if(error==0){
			digitalWrite(RED, 1);
			digitalWrite(YELLOW, 0);
			digitalWrite(GREEN, 1);
		}
		if(error == 1){
			digitalWrite(RED, 1);
			digitalWrite(YELLOW, 0);
			digitalWrite(GREEN, 0);				
		}
		btn1=digitalRead(BUTTON1);
		btn2=digitalRead(BUTTON2);
		sw1=digitalRead(SW1);

		if(sw1 == 1)	state = 0;	
		else   state = 1;

		if(btn2==0 && btn1==1) {							//スタートボタン押した時の動作
			st =0;	    
			printf("スタート\n");
			LOG_PRINT("スタート", LOG_OK);
			if(error==0){
				digitalWrite(RED, 0);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 1);
			}
			if(error == 1){
				digitalWrite(RED, 1);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 0);				
			}
			adc01();
			if(st==0)  
			{
				act=1;
				error=0;
			}
			if(st==0 && state==0)	mot_state = MOT_For;
			if(st==0 && state==1)	mot_state = MOT_Rev;
			while(1){
				if(st  == 1) break; 
				adc01();
				delay(500);
			}
		}
		
		if(btn2==1) {									//(一時停止→スタートボタン)を長押しした時の動作
			if(btn1 == 1){
				while(1){
					btn1=digitalRead(BUTTON1);
					btn2=digitalRead(BUTTON2);
					t2++;
					if(btn1==0|| btn2==0) 
					{
						t2=0;
						break;
					}
					if(t2 >= 50){
						t2=0;
						mode=1;							//スレッド[normal]スタート
						LOG_PRINT("通常ノードに移行", LOG_OK);
						break;
					}
					delay(100);
				}
			}
		}
		
		if(mode==1) break;
	}
	return 0;
}

/*****************************************
 *								関数													*
 * ****************************************/
int volt_distance(float volt)
{
  int distance = 0;
  if(2.4 < volt){
    distance = 10;
  }else if((1.37 < volt) && (volt <= 2.4)){
    distance = 33.3 - 9.71 * volt;
  }else if((0.9 < volt) && (volt <= 1.37)){
    distance = 49.15 - 21.27 * volt;
  }else if((0.79 < volt) && (volt <= 0.9)){
    distance = 111.82 - 90.91 * volt;
  }else if((0.69 < volt) && (volt <= 0.79)){
    distance = 119 - 100 * volt;
  }else if((0.5 < volt) && (volt <= 0.69)){
    distance = 86.32 - 52.63 * volt;
  }else if((0.44 < volt) && (volt <= 0.5)){
    distance = 180 - 250 * volt;
  }else{
    distance = 80;
  }
 
  return distance;
}

double map(double v)
{
	return(v - 4095) * (1 - 100) / (5 - 4095) + 100; 
}
 
int adc01(void)
{

  int loop_count = 0;
  int anarog_ch0 = 0;
  int anarog_ch1 = 0;
  int anarog_ch2 = 0;
  int anarog_ch3 = 0;
  int anarog_ch4 = 0;
  int anarog_ch5 = 0;
  int anarog_ch6 = 0;
  //int anarog_ch7 = 0;
  float volt_ch0 = 0;
  float volt_ch1 = 0;
  float volt_ch2 = 0;
  float volt_ch3 = 0;
  float volt_ch4 = 0;
  float volt_ch5 = 0;
  //float volt_ch6 = 0;
  //float volt_ch7 = 0;


  for(loop_count=1; loop_count<=10; loop_count++){
  char out_ch0[] = { 0b00000110, 0b00000000, 0b00000000 };
  char ch0_data[] = { 0x00, 0x00, 0x00 };
  char out_ch1[] = { 0b00000110, 0b01000000, 0b00000000 };
  char ch1_data[] = { 0x00, 0x00, 0x00 };
  char out_ch2[] = { 0b00000110, 0b10000000, 0b00000000 };
  char ch2_data[] = { 0x00, 0x00, 0x00 }; 
  char out_ch3[] = { 0b00000110, 0b11000000, 0b00000000 };
  char ch3_data[] = { 0x00, 0x00, 0x00 };
  char out_ch4[] = { 0b00000111, 0b00000000, 0b00000000 };
  char ch4_data[] = { 0x00, 0x00, 0x00 };
  char out_ch5[] = { 0b00000111, 0b01000000, 0b00000000 };
  char ch5_data[] = { 0x00, 0x00, 0x00 };
  char out_ch6[] = { 0b00000111, 0b10000000, 0b00000000 };
  char ch6_data[] = { 0x00, 0x00, 0x00 };
  
  if(!bcm2835_init()) return 1;
         
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);         
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128); 
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                 
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

  bcm2835_spi_transfernb(out_ch0, ch0_data, 3);
  bcm2835_spi_transfernb(out_ch1, ch1_data, 3);
  bcm2835_spi_transfernb(out_ch2, ch2_data, 3);
  bcm2835_spi_transfernb(out_ch3, ch3_data, 3);
  bcm2835_spi_transfernb(out_ch4, ch4_data, 3);
  bcm2835_spi_transfernb(out_ch5, ch5_data, 3);
  bcm2835_spi_transfernb(out_ch6, ch6_data, 3);
  //printf("CH0:    %02X %02X %02X\n", ch0_data[0], ch0_data[1], ch0_data[2]);
  //printf("CH6:    %02X %02X %02X\n", ch6_data[0], ch6_data[1], ch6_data[2]);
  anarog_ch0 = 16*16*ch0_data[1] + ch0_data[2];
  anarog_ch1 = 16*16*ch1_data[1] + ch1_data[2];
  anarog_ch2 = 16*16*ch2_data[1] + ch2_data[2];
  anarog_ch3 = 16*16*ch3_data[1] + ch3_data[2];
  anarog_ch4 = 16*16*ch4_data[1] + ch4_data[2];
  anarog_ch5 = 16*16*ch5_data[1] + ch5_data[2];
  anarog_ch6 = 16*16*ch6_data[1] + ch6_data[2];
  //printf("CH0 Anarog = %d\n", anarog_ch0);
  volt_ch0 = 3.3 / 4095 * anarog_ch0;
  volt_ch1 = 3.3 / 4095 * anarog_ch1;
  volt_ch2 = 3.3 / 4095 * anarog_ch2;
  volt_ch3 = 3.3 / 4095 * anarog_ch3;
  volt_ch4 = 3.3 / 4095 * anarog_ch4;
  volt_ch5 = 3.3 / 4095 * anarog_ch5;
  //printf("CH0 Volt = %4.2f\n", volt_ch0);
  distance_adc01_ch0 += volt_distance(volt_ch0);
  distance_adc01_ch1 += volt_distance(volt_ch1);
  distance_adc01_ch2 += volt_distance(volt_ch2);
  distance_adc01_ch3 += volt_distance(volt_ch3);
  distance_adc01_ch4 += volt_distance(volt_ch4);
  distance_adc01_ch5 += volt_distance(volt_ch5);
  //printf("CH0 Distance %d cm\n", distance_adc01_ch0); 

  bcm2835_spi_end();
  bcm2835_close();

  //sleep(1);
  }
  distance_adc01_ch0 = distance_adc01_ch0 / loop_count;
  distance_adc01_ch1 = distance_adc01_ch1 / loop_count;
  distance_adc01_ch2 = distance_adc01_ch2 / loop_count;
  distance_adc01_ch3 = distance_adc01_ch3 / loop_count;
  distance_adc01_ch4 = distance_adc01_ch4 / loop_count;
  distance_adc01_ch5 = distance_adc01_ch5 / loop_count;
  mot_speed = map(anarog_ch6);
  
  printf("ADC01 CH0 Distance %d cm\n", distance_adc01_ch0);
  //printf("ADC01 CH1 Distance %d cm\n", distance_adc01_ch1);
  //printf("ADC01 CH2 Distance %d cm\n", distance_adc01_ch2);
  //printf("ADC01 CH3 Distance %d cm\n", distance_adc01_ch3);
  //printf("ADC01 CH4 Distance %d cm\n", distance_adc01_ch4);
 //printf("ADC01 CH5 Distance %d cm\n", distance_adc01_ch5);
  //printf("ADC01 CH6 Dassui Moter Speed %d \n", mot_speed);
  
  return 0;
}

int adc02(void)
{
  int loop_count = 0;
  int anarog_ch0 = 0;
  int anarog_ch1 = 0;
  int anarog_ch2 = 0;
  int anarog_ch3 = 0;
  //int anarog_ch4 = 0;
  //int anarog_ch5 = 0;
  //int anarog_ch6 = 0;
  //int anarog_ch7 = 0;
  double volt_ch0 = 0;
  double volt_ch1 = 0;
  double volt_ch2 = 0;
  double volt_ch3 = 0;
  //double volt_ch4 = 0;
  //double volt_ch5 = 0;
  //double volt_ch6 = 0;
  //double volt_ch7 = 0;
  double log_temp_ch0 = 0;
  double log_temp_ch1 = 0;
  double log_temp_ch2 = 0;
  double log_temp_ch3 = 0;

  for(loop_count=1; loop_count<=10; loop_count++){
  char out_ch0[] = { 0b00000110, 0b00000000, 0b00000000 };
  char ch0_data[] = { 0x00, 0x00, 0x00 };
  char out_ch1[] = { 0b00000110, 0b01000000, 0b00000000 };
  char ch1_data[] = { 0x00, 0x00, 0x00 };
  char out_ch2[] = { 0b00000110, 0b10000000, 0b00000000 };
  char ch2_data[] = { 0x00, 0x00, 0x00 }; 
  char out_ch3[] = { 0b00000110, 0b11000000, 0b00000000 };
  char ch3_data[] = { 0x00, 0x00, 0x00 };
  char out_ch4[] = { 0b00000111, 0b00000000, 0b00000000 };
  char ch4_data[] = { 0x00, 0x00, 0x00 };
  char out_ch5[] = { 0b00000111, 0b01000000, 0b00000000 };
  char ch5_data[] = { 0x00, 0x00, 0x00 };
  char out_ch6[] = { 0b00000111, 0b10000000, 0b00000000 };
  char ch6_data[] = { 0x00, 0x00, 0x00 };
  char out_ch7[] = { 0b00000111, 0b11000000, 0b00000000 };
  char ch7_data[] = { 0x00, 0x00, 0x00 };
  
  if(!bcm2835_init()) return 1;
         
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);         
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128); 
  bcm2835_spi_chipSelect(BCM2835_SPI_CS1);                 
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);

  bcm2835_spi_transfernb(out_ch0, ch0_data, 3);
  bcm2835_spi_transfernb(out_ch1, ch1_data, 3);
  bcm2835_spi_transfernb(out_ch2, ch2_data, 3);
  bcm2835_spi_transfernb(out_ch3, ch3_data, 3);
  bcm2835_spi_transfernb(out_ch4, ch4_data, 3);
  bcm2835_spi_transfernb(out_ch5, ch5_data, 3);
  bcm2835_spi_transfernb(out_ch6, ch6_data, 3);
  bcm2835_spi_transfernb(out_ch7, ch7_data, 3);

  anarog_ch0 = 16*16*ch0_data[1] + ch0_data[2];
  anarog_ch1 = 16*16*ch1_data[1] + ch1_data[2];
  anarog_ch2 = 16*16*ch2_data[1] + ch2_data[2];
  anarog_ch3 = 16*16*ch3_data[1] + ch3_data[2];
 // anarog_ch4 = 16*16*ch4_data[1] + ch4_data[2];
  //anarog_ch5 = 16*16*ch5_data[1] + ch5_data[2];
  //anarog_ch6 = 16*16*ch6_data[1] + ch6_data[2];
 // anarog_ch7 = 16*16*ch7_data[1] + ch7_data[2];

  //printf("CH0 Anarog = %d\n", anarog_ch0);

  volt_ch0 = 3.3 / 4095 * anarog_ch0;
  volt_ch1 = 3.3 / 4095 * anarog_ch1;
  volt_ch2 = 3.3 / 4095 * anarog_ch2;
  volt_ch3 = 3.3 / 4095 * anarog_ch3;
  //volt_ch4 = 3.3 / 4095 * anarog_ch4;
  //volt_ch5 = 3.3 / 4095 * anarog_ch5;
  //volt_ch6 = 3.3 / 4095 * anarog_ch6;
  //volt_ch7 = 3.3 / 4095 * anarog_ch7;

  //printf("CH0 Volt = %g\n", volt_ch0);
  
  log_temp_ch0 = (volt_ch0 * 9100) / (3.3 - volt_ch0);
  log_temp_ch1 = (volt_ch1 * 9100) / (3.3 - volt_ch1);
  log_temp_ch2 = (volt_ch2 * 9100) / (3.3 - volt_ch2);
  log_temp_ch3 = (volt_ch3 * 9100) / (3.3 - volt_ch3);
  temp_adc02_ch0 += 1 / ( log(log_temp_ch0 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;
  temp_adc02_ch1 += 1 / ( log(log_temp_ch1 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;
  temp_adc02_ch2 += 1 / ( log(log_temp_ch2 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;
  temp_adc02_ch3 += 1 / ( log(log_temp_ch3 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;

  //printf("CH0 %g\n", temp_adc02_ch0);

  bcm2835_spi_end();
  bcm2835_close();

  //sleep(1);
  }
  temp_adc02_ch0 = temp_adc02_ch0 / loop_count;
  temp_adc02_ch1 = temp_adc02_ch1 / loop_count;
  temp_adc02_ch2 = temp_adc02_ch2 / loop_count;
  temp_adc02_ch3 = temp_adc02_ch3 / loop_count;
   printf("CH0 Temprature %g ℃\n", temp_adc02_ch0);
  //printf("ADC02 CH0 Temprature %g do\n", temp_adc02_ch0);
  //printf("ADC02 CH1 Temprature %g do\n", temp_adc02_ch1);
  //printf("ADC02 CH2 Temprature %g do\n", temp_adc02_ch2);
 // printf("ADC02 CH3 Temprature %g do\n", temp_adc02_ch3);
  
  return 0;
}

int read_speed(void){
  /* Read Current Switch Status */
  int i;
  int sum = 0;
  //float devi;
  
  for(i=0; i<10; i++){
		sum += digitalRead( SPEED );
	}
	//printf("%d\n",sum);
	//sum = sum / i; 
	
	if(sum >= 4)
		status_speed = 1;
	else
		status_speed = 0; 
		
		return 0;
}

/*
        ログ
        log_txt : ログに記載する内容
        log_status : 現状(エラーか正常か)
*/


void LOG_PRINT(char log_txt[256], int log_status )
{

        time_t timer;
        struct tm *date;
        char log_str[256], log_err[20];

        /* 時間取得 */
        timer = time(NULL);
        date = localtime(&timer);
        strftime(log_str, sizeof(log_str), "[%Y/%x %H:%M:%S] ", date);

        if ((log_file = fopen(LOG_FILE, "a")) == NULL) {
                printf("***file open error!!***\n");
                exit(EXIT_FAILURE);        /* エラーの場合は通常、異常終了する */
        }

        if(log_status == LOG_OK ){
                log_err = "[complete]"
        }else{
                log_err = "[  error ]"
        }

        /* 文字列結合 */
        strcat(log_str, log_err );  // エラー
        strcat(log_str,log_txt );   // 場所

        fputs(log_str, log_file);
        fclose(log_file);

        return;

}

/*****************************************
*							メイン処理												*
*****************************************/
int main(int argc, char **argv) {


	if (mcp23017Setup(100,0x21) == -1){			//mcp23017Setup(65以上の任意の数字,MCPのアドレス)
        printf("Setup Fail\n");
        exit(1);
    }

	if (mcp23017Setup(200,0x24) == -1){
        printf("Setup Fail\n");
        exit(1);
	}
	
	if (mcp23017Setup(300,0x20) == -1){
        printf("Setup Fail\n");
        exit(1);
	}

    fd_lcd = lcdInit(4,20,8,206,207,208,209,210,211,212,213,214,215);			//LCDの設定
    //lcdInit(行,列,ビット数,RS,E,D0,D1,D2,D3,D4,D5,D6,D7);
    
	if (wiringPiSetup() == -1){
        printf("Setup Fail\n");
        exit(1);
	}     
	
	/**********I/O設定**********/
    pinMode(BUTTON1, INPUT);
	wiringPiISR( BUTTON2, INT_EDGE_RISING, stop );		   //一時停止ボタンの外部割り込み設定てきなやつ
	wiringPiISR( BUTTON3, INT_EDGE_RISING, shutdown );		   //一時停止ボタンの外部割り込み設定てきなやつ
    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);
    pinMode(SW3, INPUT);
    pinMode(SW4, INPUT); 
    pinMode(PHOTO, INPUT); 
    pinMode(SPEED, INPUT);
    pinMode(KINSETU1, INPUT); 
    pinMode(KINSETU2, INPUT); 
    pinMode(KINSETU3, INPUT); 
    pinMode(KINSETU4, INPUT); 
    pinMode(KINSETU5, INPUT); 
         
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(YELLOW, OUTPUT);   
    pinMode(GREEN, OUTPUT); 
    pinMode(BUZZER, OUTPUT); 
   // pinMode(mot1_seiten,OUTPUT);
	//pinMode(mot1_gyakuten,OUTPUT);
    //pinMode(mot2_seiten,OUTPUT);
	//pinMode(mot2_gyakuten,OUTPUT);
    /*****************************/
    
 	printf("準備完了\n");    
    digitalWrite(LED1, 1);
    digitalWrite(LED2, 0);   
	lcdClear(fd_lcd);
	lcdPosition(fd_lcd,0,0);
	lcdPuts(fd_lcd,"start");
	
	//pthread_create(&MOT,NULL, (void*(*)(void*))thread_MOT, NULL);
	pthread_create(&MOT,NULL, (void*(*)(void*))thread_kinsetu, NULL);
	pthread_create(&MOT,NULL, (void*(*)(void*))thread_speed, NULL);
	
	while(1){
		switch(mode){
		case 1:
			pthread_create( &normal, NULL, (void*(*)(void*))thread_normal, NULL);	//スレッド[normal]スタート		
			break;
			
		case 2:
			pthread_create( &admin, NULL, (void*(*)(void*))thread_admin, NULL);		//スレッド[admin]スタート		
			break;
		}
		delay(500);
	}

   return 0;
}
