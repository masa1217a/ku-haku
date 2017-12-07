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

#include "ketugou.h"

int btn1=0;
int btn2=0,sw1=0,sw2=0,sw3=0,sw4=0,shuttdown=0;
int st=0, t1=0, t2=0, mode=1,error=0,teisi=0,d_teisi=0,d_end = 0,act=0;
int fd_lcd=0,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
int d_power,g_power,d_state,g_state;
int mot_state = MOT_OFF, mot_state2=MOT_OFF;

int sel_sen = 0;
int KOUDEN=0;
int motor1 = 0;
int motor2 = 0;
int flg_manpai = 0;

double dry_secA   = 0;
double dry_secB   = 0;
double crash_secA = 0;
double crash_secB = 0;

char LOG_FILE[100] =  "/home/pi/LOG/log.txt";        /* ログディレクトリ(通常)  */
FILE *log_file;        /* 通常ログ */

extern int mot1_F;                                             // 脱水モーター　正転
extern int mot1_R;                                             // 脱水モーター　逆転
extern int mot1_STOP;                                     // 脱水モーター　停止
extern int mot2_F;                                             // 減容モーター　正転
extern int mot2_R;                                             // 減容モーター　逆転
extern int mot2_STOP;                                     // 減容モーター　停止
extern int MOT_Temp;                                        //温度
extern int mot_clean_sec;
extern int mot_format_sec;
///////////////////////////
/* 透過型光電センサ */
extern int LIGHT;                                              // I2Cチェック用LED
extern int PHOTO1;                                             // 光電センサ　受光 脱水部
extern int PHOTO2;                                             // 光電センサ　受光　減容部
extern int FlgKouden;                                        // 正常運転以外で停止した場合１で保存される(非常停止、停止)
///////////////////////////
/* 速度センサ */
extern int SPEED1;                                             //速度センサ
extern int SPEED2;                                             //速度センサ
extern int SPEED3;                                             //速度センサ
extern int SPEED4;                                             //速度センサ
extern int GEAR_DRY;                                        //刀の枚数
extern int GEAR_CRASH;
extern int time_sp;                                          // 詰まり検知
////////////////////////////
/* 近接センサ */
extern int KINSETU1;                                       // 近接センサ1　投入部
extern int KINSETU2;                                       // 近接センサ2　ドッキング部
extern int KINSETU3;                                       // 近接センサ3　屑箱
////////////////////////////
/* 表示灯 */
extern int GREEN;                                                //表示灯   緑
extern int YELLOW;                                             //表示灯   黃
extern int RED;                                                     //表示灯   赤
extern int BUZZER;                                             //ブザー
////////////////////////////
/* 操作パネルボタン */
extern int BUTTON1;                                          // スタートボタン
extern int BUTTON2;                                          // ストップボタン
extern int BUTTON3;                                          // 電源ボタン
///////////////////////////
/* 操作パネルＬＥＤ */
extern int LED1;                                                  //LED　通常時
extern int LED2;                                                  //LED　管理時
////////////////////////////
/* 管理パネル */
extern int SW1;                                                     // 脱水　電源
extern int SW2;                                                     // 脱水　正/逆
extern int SW3;                                                     // 減容　電源
extern int SW4;                                                     // 減容　正/逆
////////////////////////////

//int flg_manpai=0;

/*
    速度センサスレッド
    スレッドを開始する前にsel_senという変数にピンの入力を行う
    ex)
    sel_sen = SPEED1;
    pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL);
*/

//double dry_secA   = 0;
//double dry_secB   = 0;
//double crash_secA = 0;
//double crash_secB = 0;

int thread_speed(void *ptr){

  int speed_count = 0;      // 歯の数を数える変数
  int ct_sp = 0;
  int sp_flag = 0;          // 連続で同じ条件に入らないようにする
  int start, end ;          // 時間計測

  int gpio_speed = sel_sen; // gpioピンの格納
  int gear_;                // 刃の枚数

  int flg_sec = 0;
  // 時間    ////////////
  double ck_sec = 0;
  //////////////////////

  //　ギアの枚数を変更する
  if(gpio_speed = SPEED1){
    gear_ = GEAR_DRY;
    printf("脱水Ａ：");
  }
  if(gpio_speed = SPEED2){
    gear_ = GEAR_DRY;
    printf("脱水B：");
  }
  if(gpio_speed = SPEED3){
    gear_ = GEAR_CRASH;
    printf("減容Ａ：");
  }
  if(gpio_speed = SPEED4){
    gear_ = GEAR_CRASH;
    printf("減容Ｂ：");
  }

  //struct timeval s, e;
  //gettimeofday( &s, NULL);

  //int i;

  /* Start main routine */
  printf("start\n");
   start = millis();
   //printf("%d\n", start);
  for(;;) {
    if(st  == 1)break;

    if(flg_sec == 1 && d_teisi == 1){
        while(d_teisi){
            if(st == 1) break;
            delay(200);
        }
    }

    read_speed(gpio_speed);
    usleep(100);
    //printf("%d\n",status_speed);

    /*
     *  status_speedについて
     *      1 : 歯車の凸部分の検出
     *      0 : 歯車の凹部分の検出
     *  凸凹は１セットで検出
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
                end = millis();
                ck_sec = (double)(end - start) / 1000;
                //printf("end : %d\n", end);
                  if(gpio_speed = SPEED1){
                    printf("脱水Ａ：");
                    dry_secA = ck_sec;
                  }if(gpio_speed = SPEED2){
                    printf("脱水B：");
                    dry_secB = ck_sec;
                  }if(gpio_speed = SPEED3){
                    printf("減容Ａ：");
                    crash_secA = ck_sec;
                  }if(gpio_speed = SPEED4){
                    printf("減容Ｂ：");
                    crash_secB = ck_sec;
                  }
                ct_sp++;
                printf("%.3f sec\n", ck_sec);
                start = millis();
            }
            sp_flag = 0;
    }
    end = millis();
    ck_sec = (double)(end - start) / 1000;

    if(gpio_speed = SPEED1)
      dry_secA   = ck_sec;
    if(gpio_speed = SPEED2)
      dry_secB   = ck_sec;
    if(gpio_speed = SPEED3)
      crash_secA = ck_sec;
    if(gpio_speed = SPEED4)
      crash_secB = ck_sec;

    if(flg_manpai==1) break;
  }
  return 0;
}
