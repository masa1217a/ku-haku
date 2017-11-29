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

/*構造体宣言*/
typedef struct{
    char name[STR_MAX];    // センサなどの名前
    int  value;            // センサなどの値
    //char note[NOTE_];    // 備考
}Vector;

Vector vec[Data_MAX];

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
int FlgKouden;                                         //
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
    int time_count=0;
    int dec_time = 0;                       // 光電経過時間(検知しなくなった時間)
    int wonda = 0;                          // ピン番号格納
    int pht = 0;
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
      if( FlgKouden == 1 ) break;
      pht = digitalRead(wonda);            // 光電読み込み
      delay(50);
    }
    // 停止ボタンで
    FlgKouden = 1;
    // 値の保存 (要改善)
    vec[35].value = FlgKouden;
    if( pht==0 ) printf("光電センサが物体検知\n物体がなくなるまで待つ\n");

    while(st == 0){
        dec_time = 0;

        pht = digitalRead(wonda);

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

                pht=digitalRead( wonda );

                if(dec_time >= 5)
                {
                    if(kouden_num   == 1){
                        dec_time = 0;
                        mot_state = MOT_OFF;
                        d_end = 1;
                        FlgKouden = 0;
                        vec[35].value = FlgKouden;
                        if(write_param() != 0) printf("aaa\n");
                        printf("脱水終了\n");
                        return 0;
                    }else{
                        d_teisi = 0;
                        d_end = 0;
                        dec_time = 0;
                        FlgKouden = 0;
                        vec[35].value = FlgKouden;
                        if(write_param() != 0) printf("aaa\n");
                        teisi=0;
                        mot_state  = MOT_OFF;
                        mot_state2 = MOT_OFF;
                        printf("減容終了\n");
                        return 0;
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

/*****************************************
*                           外部割り込み                                          *
******************************************/
//シャットダウンボタン
void shutdown(void)
{
    time_now = millis();
    if(time_now-time_prev > time_chat){
        mot_state = MOT_OFF;
        mot_state2 = MOT_OFF;
        pthread_detach(th);
        pthread_detach(normal);
        pthread_detach(admin);
        st=1;
        shuttdown = 1;
        mode = 0;

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

        printf("おわり\n");
        //system("shutdown -h now");
        exit(1);
    }
    time_prev = time_now;
}

//一時停止ボタン
void stop(void){
  void *ret;
	if(act==1){
        time_now = millis();
        if(time_now-time_prev > time_chat){
            st = 1;
            teisi=1;
            mot_state = MOT_OFF;
            mot_state2 = MOT_OFF;
            delay(100);
            if (pthread_join(th, &ret) != 0) {
              perror("pthread_create() error");
              exit(3);
            }
            if(write_param() != 0) printf("aaa\n");
            printf("一時停止\n");
        }
    time_prev = time_now;
	}
}

int write_param(void)
{
  /*C言語の場合冒頭で宣言する*/
  FILE *fp ;
  int i;

  /*ファイル(save.csv)に書き込む*/

  if((fp=fopen(Save_FILE,"w"))!=NULL){
      for(i=0;i<size;i++){
          /*カンマで区切ることでCSVファイルとする*/
          fprintf(fp,"%s,%d", vec[i].name, vec[i].value);
      }
      /*忘れずに閉じる*/
      fclose(fp);
  }
  return 0;
}

int SettingRead(void)
{
  /*C言語の場合冒頭で宣言する*/
  FILE *fp ;
  int i = 0, data_count;
  //Vector vec[1024];
  /*ファイル(save.csv)に読み込む*/
  if((fp=fopen(Save_FILE,"r"))!=NULL){
      i=0;
      while(fscanf(fp, "%[^,], %d", vec[i].name, &vec[i].value) != EOF){
        //printf("%d\n", i);
          i++;
      }
      data_count = i;
      /*忘れずに閉じる*/
      fclose(fp);

      // 表示
      for(i=0; i<data_count; i++)
        printf("%s  =  %d  : %d", vec[i].name, vec[i].value, i);

      printf("\nデータの数: %d\n", data_count);
  }
  return 0;
}

int param_init()
{
  //Vector vec[Data_MAX];

  if(SettingRead() != 0) return -1;

  mot1_F          = vec[0].value;
  mot1_R          = vec[1].value;
  mot1_STOP       = vec[2].value;
  mot2_F          = vec[3].value;
  mot2_R          = vec[4].value;
  mot2_STOP       = vec[5].value;
  MOT_Temp        = vec[6].value;
	mot_clean_sec   = vec[7].value;
	mot_format_sec  = vec[8].value;
  LIGHT           = vec[9].value;
  PHOTO1          = vec[10].value;
  PHOTO2          = vec[11].value;
  SPEED1          = vec[12].value;
  SPEED2          = vec[13].value;
  SPEED3          = vec[14].value;
  SPEED4          = vec[15].value;
  GEAR_DRY        = vec[16].value;
  GEAR_CRASH      = vec[17].value;
  time_sp         = vec[18].value;
  KINSETU1        = vec[19].value;
  KINSETU2        = vec[20].value;
  KINSETU3        = vec[21].value;
  GREEN           = vec[22].value;
  RED             = vec[23].value;
  YELLOW          = vec[24].value;
  BUZZER          = vec[25].value;
  BUTTON1         = vec[26].value;
  BUTTON2         = vec[27].value;
  BUTTON3         = vec[28].value;
  LED1            = vec[29].value;
  LED2            = vec[30].value;
  SW1             = vec[31].value;
  SW2             = vec[32].value;
  SW3             = vec[33].value;
  SW4             = vec[34].value;
  FlgKouden      = vec[35].value;
  return 0;
}

void IOsetting(void){

    /**********I/O設定**********/
    pinMode(BUTTON1, INPUT);
    wiringPiISR( BUTTON2, INT_EDGE_RISING, stop );         //一時停止ボタンの外部割り込み設定てきなやつ
    wiringPiISR( BUTTON3, INT_EDGE_RISING, shutdown );         //シャットダウンボタンの外部割り込み設定てきなやつ
    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);
    pinMode(SW3, INPUT);
    pinMode(SW4, INPUT);
    pinMode(PHOTO1, INPUT);
    pinMode(PHOTO2, INPUT);
    pinMode(SPEED1, INPUT);
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
    pinMode(mot2_F,OUTPUT);
    pinMode(mot2_R,OUTPUT);
    /*****************************/

}

int main(void)
{
  if(param_init() == -1){
      printf("param_init Fail\n");
      exit(1);
  }

  if (mcp23017Setup(100,0x20) == -1){         //mcp23017Setup(65以上の任意の数字,MCPのアドレス)
      printf("Setup Fail\n");
      exit(1);
  }

  if (mcp23017Setup(200,0x24) == -1){
      printf("Setup Fail\n");
      exit(1);
  }

  fd_lcd = lcdInit(4,20,8,200,201,209,202,208,203,207,204,206,205);           //LCDの設定
  //lcdInit(行,列,ビット数,RS,E,D0,D1,D2,D3,D4,D5,D6,D7);

  if (wiringPiSetup() == -1){
      printf("Setup Fail\n");
      exit(1);
  }

  IOsetting();

 act = 1;

  KOUDEN =102;
  pthread_create(&th,NULL, (void*(*)(void*))thread_photo, NULL);

  while(1){
    delay(100);
  }
}
