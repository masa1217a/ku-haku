/* ketugou.h */
#ifndef KETUGOU_H
#define KETUGOU_H

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
#define Data_MAX 1024
#define NOTE_ 500
#define Setting_FILE "settings.txt"
#define Save_FILE "save.csv"
#define size  36

/* モーター */
#define MOT_OFF 0
#define MOT_For 1                              //Forwards正転
#define MOT_Rev 2                              //Reversal逆転
#define MOT_Clean 3                         //詰まり検知後の動作
#define MOT_Format 4                      //初期チェック
#define MOT_For_check 5              //初期チェック

#define time_chat 500

/*構造体宣言*/
// 初期化処理
typedef struct{
    char name[STR_MAX];    // センサなどの名前
    int  value;            // センサなどの値
    //char note[NOTE_];    // 備考
}Vector;

Vector vec[Data_MAX];

// 測距センサ
typedef struct{
  int ch0;
  int ch1;
  int ch2;
  int ch3;
  int ch4;
  int ch5;
}Dist;

Dist dist;

// 温度センサ
typedef struct{
  int dryA;
  int dryB;
  int crashA;
  int crashB;
}Temp;

Temp temp;

typedef struct{
  double dry_secA;
  double dry_secB;
  double crash_secA;
  double crash_secB;
}speed_;

speed_ sp;

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
int FlgKouden;                                        // 正常運転以外で停止した場合１で保存される(非常停止、停止)
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
////////////////////////////

extern pthread_t normal;
extern pthread_t admin;
extern pthread_t th;
extern pthread_t th_sp;
extern pthread_t th_ph;

int adc01(void);
int adc02(void);
int read_speed(int gpio_speed);
int lcd(void);
void LOG_PRINT(char log_txt[256], int log_status );
void IOsetting(void);
int sensor_Temp(void);
int sys_format(void);
int volt_distance(float volt);
double map(double v);

// スレッド
int thread_photo(void *ptr);
int thread_speed(void *ptr);
int thread_MOT(void);
int thread_MOT2(void);
int thread_kinsetu(void *ptr);
void LOG_PRINT(char log_txt[256], int log_status );
int SettingRead(void);

#endif