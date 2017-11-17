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

#define STR_MAX 256
#define CONFIG_FILE "setting.txt"

/* モーター */
#define MOT_OFF 0
#define MOT_For 1                              //Forwards正転
#define MOT_Rev 2                              //Reversal逆転
#define MOT_Clean 3                         //詰まり検知後の動作
#define MOT_Format 4                      //初期チェック
#define MOT_For_check 5              //初期チェック

int mot1_F;                                             // 脱水モーター　正転
int mot1_R;                                             // 脱水モーター　逆転
int mot1_STOP;                                     // 脱水モーター　停止
int mot2_F;                                             // 減容モーター　正転
int mot2_R;                                             // 減容モーター　逆転
int mot2_STOP;                                     // 減容モーター　停止
int MOT_Temp;                                        //温度
int mot_clean_sec;
int mot_format_sec;
///////////////////////////
/* 透過型光電センサ */
int LIGHT;                                              // I2Cチェック用LED
int PHOTO1;                                             // 光電センサ　受光 脱水部
int PHOTO2;                                             // 光電センサ　受光　減容部
int photo_conf;                                         //
///////////////////////////
/* 速度センサ */
int SPEED1;                                             //速度センサ
int SPEED2;                                             //速度センサ
int SPEED3;                                             //速度センサ
int SPEED4;                                             //速度センサ
int GEAR_DRY;                                        //刀の枚数
int GEAR_CRASH;
int time_sp;                                          // 詰まり検知
////////////////////////////
/* 近接センサ */
int KINSETU1;                                       // 近接センサ1　投入部
int KINSETU2;                                       // 近接センサ2　ドッキング部
int KINSETU3;                                       // 近接センサ3　屑箱
////////////////////////////
/* 表示灯 */
int GREEN;                                                //表示灯   緑
int YELLOW;                                             //表示灯   黃
int RED;                                                     //表示灯   赤
int BUZZER;                                             //ブザー
////////////////////////////
/* 操作パネルボタン */
int BUTTON1;                                          // スタートボタン
int BUTTON2;                                          // ストップボタン
int BUTTON3;                                          // 電源ボタン
///////////////////////////
/* 操作パネルＬＥＤ */
int LED1;                                                  //LED　通常時
int LED2;                                                  //LED　管理時
////////////////////////////
/* 管理パネル */
int SW1;                                                     // 脱水　電源
int SW2;                                                     // 脱水　正/逆
int SW3;                                                     // 減容　電源
int SW4;                                                     // 減容　正/逆
////////////////////////////
/* ログ */
/* macros */
#define logN 256
#define LOG_OK  0                                /* テスト関数戻り値(正常)*/
#define LOG_NG -1                                /* テスト関数戻り値(異常)*/

char LOG_FILE[100] =  "/home/pi/LOG/log.txt";        /* ログディレクトリ(通常)  */
FILE *log_file;        /* 通常ログ */
////////////////////////////

volatile unsigned long time_prev = 0, time_now;
unsigned long time_chat =500;
int btn1=0, btn2=0,sw1=0,sw2=0,sw3=0,sw4=0,shuttdown=0;
int st=0, t1=0, t2=0, mode=1,error=0,teisi=0,d_teisi=0,d_end = 0,act=0;
int fd_lcd=0,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
int d_power,g_power,d_state,g_state;
int mot_state = MOT_OFF, mot_state2=MOT_OFF;
int volt_distance(float volt);
double map(double v);

double dry_sec   = 0;
double crash_sec = 0;

int sel_sen = 0;
int KOUDEN=0;

int adc01(void);
int adc02(void);
int read_speed(int gpio_speed);
int lcd(void);

static int distance_adc01_ch0 = 0;                  //測距：脱水投入口
static int distance_adc01_ch1 = 0;                  //測距：脱水投入口
static int distance_adc01_ch2 = 0;                  //測距：減容貯蓄部
static int distance_adc01_ch3 = 0;                  //測距：減容貯蓄部
static int distance_adc01_ch4 = 0;                  //測距：屑箱
static int distance_adc01_ch5 = 0;                  //測距：屑箱

static double temp_adc02_ch0 = 0;                   //温度：脱水モーター
static double temp_adc02_ch1 = 0;                   //温度：脱水モーター
static double temp_adc02_ch2 = 0;                   //温度：減容モーター
static double temp_adc02_ch3 = 0;                   //温度：減容モーター

pthread_t normal;
pthread_t admin;
pthread_t th;
pthread_t th_sp;
pthread_t th_ph;

void LOG_PRINT(char log_txt[256], int log_status );
void IOsetting(void);

/*****************************************
*                           スレッド処理                                          *
*****************************************/
//光電スレッド
int thread_photo(void *ptr){
    int time_count = 0;
    int wonda = 0;                          // ピン番号格納
    int pht = 0;
    int dec_time = 0;                       //
    int kouden_num = 0;                     // 光電センサが脱水部か減容部か（1→脱水部　0→減容部）

    if(KOUDEN == PHOTO1)    {
        wonda = PHOTO1;                       // 脱水部
        kouden_num = 1;
        printf("光電：脱水部");
    }else{
        wonda = PHOTO2;                       // 減容部
        kouden_num = 0;
        printf("光電：減容部");
    }

    pht = digitalRead(wonda);
    // printf("kouden No: %d\n",kouden_num);

    if(pht == 1)printf("物体検知まで待つ\n");
    while(pht == 1) //pht:1=受光　　pht:0=物体検知
    {
      // 停止ボタンで動作を止める
      if(st == 1) break;
      pht = digitalRead(wonda);            // 光電読み込み
      delay(50);
    }

    time_count=0;
    if(pht==0 ) printf("光電センサが物体検知\n
                        物体がなくなるまで待つ\n");

    while(1){
        dec_time = 0;

        if(kouden_num  == 1)  pht=digitalRead( PHOTO1 );
        else  pht=digitalRead( PHOTO2 );

        if(st  == 1) break;

        if(kouden_num == 1 && d_end == 1)break;

        if(kouden_num == 1 && d_teisi == 1){
                while(d_teisi){
                        if(st == 1) break;
                        delay(200);
                }
        }

        if(kouden_num == 1 && mot_state == MOT_Clean){
                while(MOT_Clean){
                        if(st == 1) break;
                        delay(200);
                }
        }

        if(pht==1)
        {
            printf("カウント開始\n");
            time_count=0;
            while(pht == 1){
                time_count++;

                if(kouden_num == 1 && d_teisi == 1){
                    while(d_teisi){
                        if(st == 1) break;
                    }
                }

                if(kouden_num   ==  1)  pht=digitalRead( PHOTO1 );
                else  pht=digitalRead( PHOTO2 );

                if(dec_time >= 5)
                {
                    if(kouden_num   == 1){
                        dec_time = 0;
                        mot_state = MOT_OFF;
                        d_end = 1;
                        printf("脱水終了\n");
                    }else{
                        d_teisi = 0;
                        d_end = 0;
                        dec_time = 0;
                        st = 1;
                        kenti1=0;
                        kenti2=0;
                        teisi=0;
                        mot_state  = MOT_OFF;
                        mot_state2 = MOT_OFF;
                        printf("終了\n");
                    }
                }
                else
                {
                    if(time_count>19) {
                        dec_time++;
                        printf("経過時間　＝　%d　秒 \n",dec_time);
                        time_count=0;
                    }
                }
                if(st==1) break;
                if(kouden_num   == 1 && d_end == 1 ) break;

                delay(50);
            }
        }
        delay(50);
    }
    return 0;
}
