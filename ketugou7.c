#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <lcd.h>
#include <bcm2835.h>
#include <mcp23017.h>
#include <math.h>
#include <string.h>

/* モーター */
#define mot1_F         23									// 脱水モーター　正転
#define mot1_R         24									// 脱水モーター　逆転
//#define mot1_STOP 25							// 脱水モーター　停止
//#define mot2_F 29									// 減容モーター　正転
//#define mot2_R 28									// 減容モーター　逆転
//#define mot2_STOP 27							// 減容モーター　停止

#define	MOT_OFF	0
#define	MOT_For	1 										//Forwards正転
#define	MOT_Rev	2											//Reversal逆転
#define MOT_Clean 3										//詰まり検知後の動作
#define MOT_Format 4									//初期チェック
#define MOT_Temp 70
///////////////////////////
/* 透過型光電センサ */
#define LIGHT       101										//	
#define PHOTO		102 									//光電センサ　受光
//#define PHOTO2		103 							//光電センサ　受光
///////////////////////////
/* 速度センサ */
#define SPEED      107  									//速度センサ
//#define SPEED      105 									//速度センサ
//#define SPEED      106  									//速度センサ
//#define SPEED      107  									//速度センサ
#define GEAR		2										//刃の枚数
////////////////////////////
/* 近接センサ */ 
#define KINSETU1   108 									// 近接センサ1　投入部
#define KINSETU2	108									// 近接センサ2　ドッキング部
#define KINSETU3	108									// 近接センサ3　屑箱
////////////////////////////
/* 表示灯 */
#define GREEN  114											//表示灯	緑
#define	YELLOW	113										//表示灯	黃
#define	RED	112												//表示灯	赤
#define BUZZER  111 										//ブザー
////////////////////////////
/* 操作パネルボタン */
#define BUTTON1   	0										// スタートボタン
#define BUTTON2  	2										// ストップボタン
#define BUTTON3   	3										// 電源ボタン
///////////////////////////
/* 操作パネルＬＥＤ */
#define	LED1	211												//LED　通常時
#define	LED2	210												//LED　管理時
////////////////////////////
/* 管理パネル */
#define SW1	212												// 脱水　電源
#define SW2	213												// 脱水　正/逆
#define SW3	214												// 減容　電源
#define SW4	215												// 減容　正/逆
////////////////////////////
/* ログ */
#define LOG_OK  0     										/* テスト関数戻り値(正常)*/
#define LOG_NG -1     					  					/* テスト関数戻り値(異常)*/

char LOG_FILE[100] =  "/home/pi/LOG/log.txt";        /* ログディレクトリ(通常)  */
/* macros */
#define logN 256
FILE *log_file;        /* 通常ログ */
////////////////////////////

volatile unsigned long time_prev = 0, time_now;
unsigned long time_chat =500;
int btn1=0, btn2=0,sw1=0,sw2=0,sw3=0,sw4=0,pht=0,shuttdown=0;
int st=0, t1=0, t2=0, mode=1,dec_time=0,kenti=0,error=0,teisi=0,act=0;
int fd_lcd=0,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
int d_power,g_power,d_state,g_state;
int mot_state = MOT_OFF;
int volt_distance(float volt);
double map(double v);
double ck_sec;

int adc01(void);
int adc02(void);
int ctrl_c(void);
int read_speed(void);
int lcd(void);
int clean(void);

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

pthread_t normal;	 
pthread_t admin;	 
pthread_t th;
pthread_t th_sp;
pthread_t th_ph;

void LOG_PRINT(char log_txt[256], int log_status );


/*****************************************
*							スレッド処理											*
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
int thread_MOT(void){

	int m_start, m_end;
	double m_sec;
	// フラグ
	int flg_c=0, flg_f=0;

	digitalWrite(mot1_F,0);
	digitalWrite(mot1_R,0);

	for(;;){
		switch(mot_state){
			case MOT_OFF: //モータがOFFの場合
				digitalWrite(mot1_R,0);//mot_gyakutenが1だったら逆転動作する
										  //				  0だったら動作を停止する
				digitalWrite(mot1_F,0);   ///mot_gyakutenが1だったら正転動作する
				break;
			
			case MOT_For://モーターが正転の場合			
				digitalWrite(mot1_R,0);
				usleep(50000);
				digitalWrite(mot1_F,1);
				break;

			case MOT_Rev://モータが逆転の場合
				digitalWrite(mot1_F,0);
				usleep(50000);
				digitalWrite(mot1_R,1);
				break;
				
			case MOT_Clean:
				
				if(flg_c == 0){
					m_start = clock();
					flg_c=1;
					printf("つまり処理　開始\n");
				}
				m_end = clock();
				m_sec = (double)(m_end - m_start) / CLOCKS_PER_SEC;
				if(m_sec <= 5){
					digitalWrite(mot1_F,0);
					digitalWrite(mot1_R,1);
				}else if(m_sec > 5 && m_sec <= 10){
					digitalWrite(mot1_F,1);
					digitalWrite(mot1_R,0);
				}else if(m_sec > 10 && m_sec <= 15){
					digitalWrite(mot1_F,0);
					digitalWrite(mot1_R,1);
				}else if(m_sec > 15 && m_sec <= 20){
					digitalWrite(mot1_F,1);
					digitalWrite(mot1_R,0);
				}else{
					printf("つまり処理 正常終了\n");
					mot_state = MOT_OFF;
					flg_c = 0;
				}

			break;
				
			case MOT_Format:
				
				if(flg_f == 0){
					m_start = clock();
					flg_f=1;
					printf("初期モーター駆動\n");
				}
				m_end = clock();
				m_sec = (double)(m_end - m_start) / CLOCKS_PER_SEC;

				if(m_sec <= 5){
					digitalWrite(mot1_F,1);
					digitalWrite(mot1_R,0);
				}else if(m_sec > 5 && m_sec <= 10){
					digitalWrite(mot1_F,0);
					digitalWrite(mot1_R,1);
				}else if(m_sec > 10 && m_sec <= 15){
					digitalWrite(mot1_F,1);
					digitalWrite(mot1_R,0);
				}else{
					mot_state = MOT_OFF;
					flg_f = 0;
					printf("モーター駆動　正常終了\n");
				}
			break;
			
			default: 
				printf("デフォルトです\n");
				break;
		}
	}
}

//速度センサスレッド
int thread_speed(void *ptr){

  int speed_count = 0;
  int sp_flag = 0;
  //int g_flg = 0;
  int gear_  = GEAR;
  
  int start, end ;
  
  //struct timeval s, e;
  //gettimeofday( &s, NULL);
  
  //int i;
  
  /* Start main routine */
  printf("start\n");
   start = clock();
   //printf("%d\n", start);
  for(;;) {
	 if(st  == 1) break; 
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
			   //printf("count : %d\n\n", speed_count);
			  
			  if( (speed_count % gear_ ) == 0 ){
				  end = clock();
				  ck_sec = (double)(end - start) / CLOCKS_PER_SEC;
				  //printf("end : %d\n", end);
				  //printf("%.3f sec\n", ck_sec);
				  start = clock();
			  }
			  sp_flag = 0;
	  }
	  end = clock();
	  ck_sec = (double)(end - start) / CLOCKS_PER_SEC;
  }
  return 0;
}

//近接センサスレッド
int thread_kinsetu(void *ptr){
	
	while(1){
		kinsetu1=digitalRead(KINSETU1);
		kinsetu2=digitalRead(KINSETU2);
		kinsetu3=digitalRead(KINSETU3);		
		
		delay(100);			
	}
	return 0;
}

/*****************************************
*               初期化処理                *
*****************************************/
int sys_format(void){
	st = 0;
	teisi = 0;
	// 初期化のフラグ
	int flg_1   = 0;
	int flg_2   = 0;
	int flg_3   = 0;
	int flg_4   = 0;
	int flg_5   = 0;
	int flg_6   = 0;
	int flg_7   = 0;
	int flg_8   = 0;
	int flg_9   = 0;
	int flg_end = 0;

	digitalWrite(RED,    0);
	digitalWrite(YELLOW, 1);
	digitalWrite(GREEN,  0);
	printf("---------初期モード開始---------\n\n");
	LOG_PRINT("---------初期モード開始---------", LOG_OK);
    digitalWrite(LED1,1);
    digitalWrite(LED2,0);
    
    lcdPosition(fd_lcd,0,0);	
    lcdPrintf (fd_lcd, "\xBD\xB2\xAF\xC1\xA6\xBD\xCD\xDE\xC3\x4F\x46\x46\xC6\xBC\xC3\xB8\xC0\xDE\xBB\xB2") ;		//スイッチヲスベテOFFニシテクダサイ
    while(1){
		if(shuttdown ==1) break;			
		int old_sw1 = sw1;	
		int old_sw2 = sw2;	
		int old_sw3 = sw3;	
		int old_sw4 = sw4;				
		
		sw1=digitalRead(SW1);
		sw2=digitalRead(SW2);
		sw3=digitalRead(SW3);
		sw4=digitalRead(SW4);
				
		if(sw1 == 0)	d_power= 0;				//脱水　電源
		else   d_power = 1;
		if(sw2 == 0)	d_state = 0;				//脱水　正/逆
		else   d_state = 1;
		if(sw3 == 0)	g_power = 0;				//減容　電源				
		else   g_power = 1;
		if(sw4 == 0)	g_state = 0;				//減容　正/逆	
		else   g_state = 1;

		if(sw1 != old_sw1) lcd();
		if(sw2 != old_sw2) lcd();
		if(sw3 != old_sw3) lcd();
		if(sw4 != old_sw4) lcd();
		
		if(sw1==0 && sw2==0 && sw3==0 && sw4==0)	break;	
    }
    
    lcdPosition(fd_lcd,0,0);	
	lcdPrintf (fd_lcd, "\xBE\xAF\xC4\xB1\xAF\xCC\xDF\xC1\xAD\xB3          ") ;		//セットアップチュウ    
    
    while(!flg_end){
	    while(1){
		    /* 1. 脱水部と減容部のドッキングがされているか  */
		    if(kinsetu1 == 0 ){
				printf("脱水部と減容部のドッキングがされていません\n");
				LOG_PRINT("ドッキングエラー", LOG_NG);
				error=7;
				lcd();
				flg_1 = 0;
				delay(600); 
				break;
			}else{
			 LOG_PRINT("ドッキング", LOG_OK);
			 flg_1 = 1;
			}
			printf("\rflg_1 = %d\n",  flg_1);

			/* 2.	屑箱が設置されているか                */
			if(kinsetu1 == 0 ){
				printf("屑箱を設置してください\n");
				LOG_PRINT("屑箱なし", LOG_NG);
				error=9;
				lcd();
				flg_2 = 0;
				delay(600); 
				break;
			}else{
			 LOG_PRINT("屑箱設置中", LOG_OK);
			 flg_2 = 1;
			}
			printf("flg_2 = %d\n",  flg_2);

			/* 3.	屑箱内にスポンジが残っていないか       */
			adc01();
			if(st==0&&teisi==0 && distance_adc01_ch0<20 )
			{
				printf("エラー:スポンジの量が多いです\n");
				LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
				error=4;
				lcd();
				flg_3 = 0;
				delay(600); 
				break;
			}else{
			 LOG_PRINT("投入口のスポンジの量がちょうどいい", LOG_OK);
			 flg_3 = 1;
			}
			printf("flg_3 = %d\n",  flg_3);

			/* 4.	脱水部投入扉が閉じているか */
			if(kinsetu1 == 0 ){
				printf("扉を閉めてください\n");
				LOG_PRINT("扉が開いている", LOG_NG);
				error=1;
				lcd();
				flg_4 = 0;
				delay(600); 
				break;
			}else{
			 LOG_PRINT("扉が閉まっている", LOG_OK);
			 flg_4 = 1;
			}
			printf("flg_4 = %d\n",  flg_4);

			/* 5.	脱水部にスポンジが残されていないか       */	
			adc01();
			if(st==0&&teisi==0 && distance_adc01_ch0<20 )
			{
				printf("エラー:スポンジの量が多いです\n");
				LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
				error=4;
				lcd();
				flg_5 = 0;
				delay(600); 
				break;
			}else{
				LOG_PRINT("投入口のスポンジの量がちょうどいい", LOG_OK);
				flg_5 = 1;
			}
			printf("flg_5 = %d\n",  flg_5);

			/* 6.	減容部にスポンジが残されていないか       */
			adc01();
			if(st==0&&teisi==0 && distance_adc01_ch0<20 ) 
			{
				printf("エラー:スポンジの量が多いです\n");
				LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
				error=4;
				lcd();
				flg_6= 0;
				delay(600); 
				break;
			}else{
			 LOG_PRINT("投入口のスポンジの量がちょうどいい", LOG_OK);
			 flg_6 = 1;
			}
			printf("flg_6 = %d\n",  flg_6);

			/* 7.	モータの温度が安定動作できる範囲であるか  */
			if( adc02() >= MOT_Temp  ){
				printf("エラー:異常な温度を検知\n");
				LOG_PRINT("異常な温度を検知", LOG_NG);
				error=10;
				lcd();
				delay(600); 
				break;
			}else{
			 LOG_PRINT("正常な温度を検知", LOG_OK);
			 flg_7 = 1;
			}
			printf("flg_7 = %d\n",  flg_7);

			/* 8.	脱水部と減容部の詰まり確認　・　スポンジ残ってないか確認　 */
			pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL);	//スレッド[speed]スタート				
			mot_state = MOT_Format;
				
			while(1){
				if(shuttdown ==1) break;
				if( ck_sec >= 3 ){
					mot_state = MOT_Clean;
					while(1){
						if(shuttdown ==1) break;
						if(ck_sec >= 3){
							flg_8 = 0;
							break;
						}else if(mot_state == MOT_OFF){
							flg_8 = 1;
							break;
						}
					}
				}else if(mot_state == MOT_OFF){
					flg_8 =1;
					break;
				}
			}
				
			if(  flg_8 == 1 )	LOG_PRINT("詰まりなし", LOG_OK);	
			else {
				error = 5;
				printf("エラー:詰まりを検知\n");
				LOG_PRINT("詰まりを検知", LOG_NG);	
			}	
			printf("flg_8 = %d\n\n",  flg_8);	

			pthread_detach(th_sp);
			

			/*9.スポンジ残ってないか確認*/
			pthread_create( &th_ph, NULL, (void*(*)(void*))thread_photo, NULL);	//スレッド[pth]スタート
			mot_state = MOT_For;	
			delay(5000);
			if(kenti == 1){
				lcdPrintf (fd_lcd, "\xD0\xBC\xAE\xD8\xC9\xBD\xCE\xDF\xDD\xBC\xDE\xA6\xBC\xAE\xD8\xC1\xAD\xA3") ;		//ミショリノスポンジヲショリチュウ
				error = 11;
				LOG_PRINT("スポンジ有り", LOG_OK);				
			}else{
				LOG_PRINT("スポンジ無し", LOG_OK);
				mot_state = MOT_OFF;
				flg_9 = 1;			
			}
			printf("flg_9 = %d\n\n",  flg_9);
			pthread_detach(th_ph);

			// 終了条件
			if(flg_1 == 1 && flg_2 == 1 && flg_3 == 1 && flg_4 == 1 &&
	    	   flg_5 == 1 && flg_6 == 1 && flg_7 == 1 && flg_8 == 1 && flg_9 == 1 ){
				flg_end = 1;
				error=0;
				delay(600);
				break;
			}
		}
		while(error > 0 ){
				if(shuttdown ==1) break;
				digitalWrite(RED,    1);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN,  0);
				//digitalWrite(BUZZER, 1);
				//モーターＯＦＦ
				//エラー処理
				adc01();
				if(kinsetu1 == 1 && flg_1 == 0) error = 0;
				else if(kinsetu1 == 1 && flg_1 ==1) error = 0;
				else if(st==0&&teisi==0 &&  flg_2 == 1 &&
					distance_adc01_ch0>20 && distance_adc01_ch0<40)
					error = 0;
				else if(kinsetu1 == 1 && flg_3 == 1) error = 0;
				else if(st==0&&teisi==0 && flg_4 == 1 &&
					distance_adc01_ch0>20 && distance_adc01_ch0<40)
					error = 0;
				else if(st==0&&teisi==0 &&  flg_5 == 1 && 
					distance_adc01_ch0>20 && distance_adc01_ch0<40)
					error = 0;
				else if( adc02() <= MOT_Temp  && flg_6 == 1 ) error = 0;
				//8.
				//else if
				//printf("flg_1 = %d\n",  flg_1);
				//printf("flg_2 = %d\n",  flg_2);
				//printf("flg_3 = %d\n",  flg_3);
				//printf("flg_4 = %d\n",  flg_4);
				//printf("flg_5 = %d\n",  flg_5);
				//printf("flg_6 = %d\n",  flg_6);
				//printf("flg_7 = %d\n",  flg_7);
				//printf("flg_8 = %d\n\n",  flg_8);
				delay(600);
			}

	}
	printf("---------初期モード終了---------\n");
	LOG_PRINT("---------初期モード終了---------", LOG_OK);
	return 0;
}

/*************************************************
        ログ
        log_txt : ログに記載する内容
        log_status : 現状(エラーか正常か)
            0 : 正常(LOG_OK)
            1 : 異常(LOG_NG)
*************************************************/
void LOG_PRINT(char log_txt[256], int log_status )
{

        time_t timer;
        struct tm *date;
        char log_str[logN];
        char log_readline[logN] = {'\0'};
        char log_cp[2];
       // int log_mon;

        /* 時間取得 */
        timer = time(NULL);
        date = localtime(&timer);
        strftime(log_str, sizeof(log_str), "[%Y/%x %H:%M:%S] ", date);
        
        //printf("%2d %2d\n", date->tm_mon + 1, date->tm_sec);
        /* ファイルのオープン */
            if ((log_file = fopen(LOG_FILE, "r")) == NULL) {
                fprintf(stderr, "%sのオープンに失敗しました.\n", LOG_FILE);
                exit(EXIT_FAILURE);
            }

            /* ファイルの終端まで文字を読み取り表示する */
            fgets(log_readline, logN, log_file);
                //printf("%s\n", log_readline);

            /* ファイルのクローズ */
            fclose(log_file);

        // ファイルの1行目の日付(月)を抽出
        strncpy(log_cp, log_readline+6, 2);
        // 数字に変換
       // log_mon = atof(log_cp);
        //printf("%s \n", log_cp);
        //printf("%d\n", log_mon);
        //printf("%d\n", date->tm_mon+1);

        /*
        // 毎月データを削除
        if((date->tm_mon+1) != log_mon){
                //strcat(log_file, "_%2d", date->tm_mon );
                if(remove(LOG_FILE) == 0){
                        printf("log.txtファイルを削除しました\n", LOG_FILE );
                }else{
                        printf("ファイル削除に失敗しました\n");
                }
        }*/

        // 月ごとにファイルを作る
        sprintf(LOG_FILE,"/home/pi/LOG/log_%4d_%d.txt", date->tm_year+1900, date->tm_mon+1);          

        // ファイルオープン
        if ((log_file = fopen(LOG_FILE, "a")) == NULL) {
                printf("***file open error!!***\n");
                printf("%s\n", LOG_FILE);
                exit(EXIT_FAILURE);        /* エラーの場合は通常、異常終了する */
        }

        if(log_status == LOG_OK ){
                                strcat(log_str, "[complete]");
        }else{
                                strcat(log_str,  "[  error ]");
        }

        /* 文字列結合 */
        strcat(log_str,log_txt );   // 場所
        strcat(log_str,"\n");

        fputs(log_str, log_file);
        fclose(log_file);

        return;

}

/*****************************************
*							外部割り込み											*
*****************************************/
//シャットダウンボタン
void shutdown(void)
{
	time_now = millis();
	if(time_now-time_prev > time_chat){
		printf("おわり\n");
		pthread_detach(th);
		pthread_detach(normal);
		pthread_detach(admin);
		st=1;
		shuttdown = 1;
		exit(1);
	}
	time_prev = time_now;	
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
*             			  モード処理        						          *
*****************************************/
// 通常モード
int thread_normal(void *ptr)
{
	int	time_count=0;
	act=0;
	st =0;	    
	teisi=0;
	mode=0;
	printf("通常モード\n");
	LOG_PRINT("---------通常モード開始---------", LOG_OK);
    digitalWrite(LED1, 1);
    digitalWrite(LED2, 0); 
	lcdPrintf (fd_lcd, "\xCC\xB8\xDB\xCA\xBE\xAF\xC1\xBC\xCF\xBC\xC0\xB6\x3F       ") ;		//フクロハセッチシマシタカ？
	while(1){
		
			if(error == 0){
				digitalWrite(RED, 0);
				digitalWrite(YELLOW, 1);
				digitalWrite(GREEN, 0);
			}
			if(error > 0){
				digitalWrite(RED, 1);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 0);				
			}
			
		int old_sw1 = sw1;	
		int old_sw2 = sw2;	
		int old_sw3 = sw3;	
		int old_sw4 = sw4;		
				
		btn1=digitalRead(BUTTON1);
		btn2=digitalRead(BUTTON2);		
		sw1=digitalRead(SW1);
		sw2=digitalRead(SW2);
		sw3=digitalRead(SW3);
		sw4=digitalRead(SW4);
				
		if(sw1 == 0)	d_power= 0;				//脱水　電源
		else   d_power = 1;
		if(sw2 == 0)	d_state = 0;				//脱水　正/逆
		else   d_state = 1;
		if(sw3 == 0)	g_power = 0;				//減容　電源				
		else   g_power = 1;
		if(sw4 == 0)	g_state = 0;				//減容　正/逆	
		else   g_state = 1;

		if(sw1 != old_sw1) lcd();
		if(sw2 != old_sw2) lcd();
		if(sw3 != old_sw3) lcd();
		if(sw4 != old_sw4) lcd();
				
/****************************************************************************/
/*							スタートボタン押した時の動作								*/
/****************************************************************************/
		if(btn2==0 && btn1==1) {							
			st =0;	    
			if(kinsetu1  == 0 || kinsetu2 == 0) {
					printf("扉を閉めてください\n");
					LOG_PRINT("扉が開いている", LOG_NG);
					error=1;
					lcd();
					st=1;
					delay(100);
			}else LOG_PRINT("扉が閉まっている", LOG_OK);

			adc01();
			if(st==0 && teisi==0 )
			{
				if(distance_adc01_ch0<20 || distance_adc01_ch1<20){
					printf("エラー:スポンジの量が多いです\n");
					LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
					error=4;
					lcd();
					st=1;
					delay(100);
				}
				else LOG_PRINT("投入口のスポンジの量がちょうどいい", LOG_OK);
			}


			if(st==0)  
			{
				printf("スタート\n");
				pthread_create( &th, NULL, (void*(*)(void*))thread_photo, NULL);	//スレッド[pth]スタート
				pthread_create(&th,NULL, (void*(*)(void*))thread_speed, NULL);	//スレッド[speed]スタート
				LOG_PRINT("スタート", LOG_OK);
				error=0;
				act=1;
			}
			if(error==0){
				digitalWrite(RED, 0);
				digitalWrite(YELLOW, 0);        
				digitalWrite(GREEN, 1);
			}
			if(error > 0){
				digitalWrite(RED, 1);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 0);				
			}
						
			if(st==0 && d_power== 1 && d_state==0)	mot_state = MOT_For;
			if(st==0 && d_power== 1 && d_state==1)	mot_state = MOT_Rev;
			
			while(1){
				if(st  == 1) 
				{
					act=0;
					mot_state = MOT_OFF;
					lcdPosition(fd_lcd,0,0);	
					lcdPrintf (fd_lcd, "\xCC\xB8\xDB\xCA\xBE\xAF\xC1\xBC\xCF\xBC\xC0\xB6\x3F       ") ;		//フクロハセッチシマシタカ？
					if(shuttdown == 1) lcdClear(fd_lcd);
					break; 
				}
				
				if(st==0){
					lcdPosition(fd_lcd,0,0);	
					lcdPrintf (fd_lcd, "\xBC\xAE\xD8\xBC\xC3\xB2\xCF\xBD          ") ;		//ショリシテイマス
				}
				
				time_count++;
				
				if(time_count > 9){
					adc02();
					time_count = 0;
				}
				
				if(kinsetu1  == 0 || kinsetu2 == 0) 
				{
					printf("扉を閉めてください\n");
					LOG_PRINT("扉が開いている", LOG_NG);
					error=2;
					lcd();
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
						LOG_PRINT("*****管理者モードに移行******", LOG_OK);
						break;
					}
					delay(100);
				}
			}
		}
		if(mode==2  || shuttdown==1) break;
	}
	LOG_PRINT("---------通常モード終了---------", LOG_OK);
    return 0;
}

//管理モード
int  thread_admin(void *ptr)
{
	st =0;	    
	act=0;
	teisi=0;
	mode=0;
	printf("管理者モード\n");
	LOG_PRINT("---------管理者モード開始---------", LOG_OK);
    digitalWrite(LED1, 0);
    digitalWrite(LED2, 1); 
    while(1){
lcdPosition(fd_lcd,0,0);	
lcdPrintf (fd_lcd, "\xD2\xDD\xC3\xC5\xDD\xBD\xD3\xB0\xC4\xDE \xC1\xAD\xB3\xB2  ") ;			//メンテナンスモード チュウイ
		
		if(error==0){
			digitalWrite(RED, 0);
			digitalWrite(YELLOW, 1);
			digitalWrite(GREEN, 0);
		}
		if(error > 0){
			digitalWrite(RED, 1);
			digitalWrite(YELLOW, 0);
			digitalWrite(GREEN, 0);				
		}
		
		btn1=digitalRead(BUTTON1);
		btn2=digitalRead(BUTTON2);		
		sw1=digitalRead(SW1);
		sw2=digitalRead(SW2);
		sw3=digitalRead(SW3);
		sw4=digitalRead(SW4);

		if(sw1 == 0)	d_power= 0;				//脱水　電源
		else   d_power = 1;
		if(sw2 == 0)	d_state = 0;				//脱水　正/逆
		else   d_state = 1;
		if(sw3 == 0)	g_power = 0;				//減容　電源				
		else   g_power = 1;
		if(sw4 == 0)	g_state = 0;				//減容　正/逆	
		else   g_state = 1;

		if(btn2==0 && btn1==1) {							//スタートボタン押した時の動作
			st =0;	    
			printf("スタート\n");
			LOG_PRINT("スタート", LOG_OK);
			if(error==0){
				digitalWrite(RED, 0);
				digitalWrite(YELLOW, 0);
				digitalWrite(GREEN, 1);
			}
			if(error > 0){
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
			if(st==0 && d_power== 1 && d_state==0)	mot_state = MOT_For;
			if(st==0 && d_power== 1 && d_state==1)	mot_state = MOT_Rev;
				
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
						LOG_PRINT("*****通常モードに移行*****", LOG_OK);
						break;
					}
					delay(100);
				}
			}
		}
		
		if(mode==1 || shuttdown==1) break;
	}
	LOG_PRINT("---------管理者モード終了---------", LOG_OK);
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
  printf("ADC01 CH1 Distance %d cm\n", distance_adc01_ch1);
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

int read_speed(void)
{
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

int lcd(void)
{
	if(sw1 == 1){
		lcdPosition(fd_lcd,8,2);	
		lcdPrintf (fd_lcd, "ON ") ;		
	}
	else {
		lcdPosition(fd_lcd,8,2);	
		lcdPrintf (fd_lcd, "OFF") ;
	}				
	
	if(sw3 == 1){
		lcdPosition(fd_lcd,8,3);	
		lcdPrintf (fd_lcd, "ON ") ;		
	}
	else {
		lcdPosition(fd_lcd,8,3);	
		lcdPrintf (fd_lcd, "OFF") ;
	}			

	if(sw2 == 0){
		lcdPosition(fd_lcd,15,2);	
		lcdPrintf (fd_lcd, "\xBE\xB2   ") ;					//セイ
	}
	else {
		lcdPosition(fd_lcd,15,2);	
		lcdPrintf (fd_lcd, "\xB7\xDE\xAC\xB8") ;		//ギャク
	}			
	
	if(sw4 == 0){
		lcdPosition(fd_lcd,15,3);	
		lcdPrintf (fd_lcd, "\xBE\xB2   ") ;					//セイ
	}
	else {
		lcdPosition(fd_lcd,15,3);	
		lcdPrintf (fd_lcd, "\xB7\xDE\xAC\xB8") ;		//ギャク
	}			

	if(error > 0){
		lcdPosition(fd_lcd,0,0);	
		lcdPrintf (fd_lcd, "\xB4\xD7\xB0No.%d            " ,error) ;	
	}

	return 0;
}

int clean(void)
{
	 	mot_state = MOT_Rev;
		delay(5000);
			
		mot_state = MOT_For;
		delay(5000);
	
		mot_state = MOT_Rev;
		delay(5000);
	
		mot_state = MOT_For;
		delay(5000);
		
		mot_state = MOT_OFF;
	
	return 0;
}
/*****************************************
*							メイン処理												*
*****************************************/
int main(int argc, char **argv) {


	if (mcp23017Setup(100,0x20) == -1){			//mcp23017Setup(65以上の任意の数字,MCPのアドレス)
        printf("Setup Fail\n");
        exit(1);
    }

	if (mcp23017Setup(200,0x24) == -1){
        printf("Setup Fail\n");
        exit(1);
	}

    fd_lcd = lcdInit(4,20,8,200,201,209,202,208,203,207,204,206,205);			//LCDの設定
    //lcdInit(行,列,ビット数,RS,E,D0,D1,D2,D3,D4,D5,D6,D7);
    
	if (wiringPiSetup() == -1){
        printf("Setup Fail\n");
        exit(1);
	}     
	
	/**********I/O設定**********/
    pinMode(BUTTON1, INPUT);
	wiringPiISR( BUTTON2, INT_EDGE_RISING, stop );		   //一時停止ボタンの外部割り込み設定てきなやつ
	wiringPiISR( BUTTON3, INT_EDGE_RISING, shutdown );		   //シャットダウンボタンの外部割り込み設定てきなやつ
    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);
    pinMode(SW3, INPUT);
    pinMode(SW4, INPUT); 
    pinMode(PHOTO, INPUT); 
    pinMode(SPEED, INPUT);
    pinMode(KINSETU1, INPUT); 
    pinMode(KINSETU2, INPUT); 
    pinMode(KINSETU3, INPUT); 
         
    pinMode(LIGHT,OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(YELLOW, OUTPUT);   
    pinMode(GREEN, OUTPUT); 
    pinMode(BUZZER, OUTPUT); 
   pinMode(mot1_F,OUTPUT);
   pinMode(mot1_R,OUTPUT);
    /*****************************/
    
 	printf("準備完了\n");    
 	digitalWrite(LIGHT, 1);
    digitalWrite(LED1, 1);
    digitalWrite(LED2, 0);   
	lcdPosition(fd_lcd,0,1);	
	lcdPrintf (fd_lcd, "       \xC3\xDE\xDD\xB9\xDE\xDD  \xD3\xB0\xC0" ,error) ;		
	lcdPosition(fd_lcd,0,2);	
	lcdPrintf (fd_lcd, "\xC0\xDE\xAF\xBD\xB2 : " ) ;		
	lcdPosition(fd_lcd,0,3);	
	lcdPrintf (fd_lcd, "\xB9\xDE\xDD\xD6\xB3 : " ) ;			
	
	pthread_create(&th,NULL, (void*(*)(void*))thread_MOT, NULL);
	pthread_create(&th,NULL, (void*(*)(void*))thread_kinsetu, NULL);
	lcd();
	
	if(sys_format() != 0) return -1;
	
	while(1){
		switch(mode){
		case 1:
			pthread_create( &normal, NULL, (void*(*)(void*))thread_normal, NULL);	//スレッド[normal]スタート		
			break;
			
		case 2:
			pthread_create( &admin, NULL, (void*(*)(void*))thread_admin, NULL);	//スレッド[admin]スタート		
			break;
					
		}
		delay(500);
		if(shuttdown > 0) break;
	}

    digitalWrite(LED1, 0);
    digitalWrite(LED2, 0);
    digitalWrite(RED, 0);
    digitalWrite(LIGHT, 0);    
    digitalWrite(YELLOW, 0);   
    digitalWrite(GREEN, 0); 
    digitalWrite(BUZZER, 0); 
   digitalWrite(mot1_F,0);
   digitalWrite(mot1_R,0);
	lcdClear(fd_lcd);

	delay(1000);
	
   return 0;
}
