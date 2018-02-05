#include "ketugou.h"

extern int btn1, btn2, btn3,sw1,sw2,sw3,sw4,shutdown;
extern int st, t1, t2, mode,kenti,error,teisi,d_teisi,d_end,act;
extern int fd_lcd,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
extern int d_power,g_power,d_state,g_state;
extern int mot_state, mot_state2;

extern int sel_sen;
extern int KOUDEN;
extern int motor1;
extern int motor2;

extern int flg_manpai;

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

/*
*  速度の平均値を測定する
*  前提条件：速度センサを使用しログをとっている
*/

static int sp_flag = 0;          // 連続で同じ条件に入らないようにする
static int speed_count = 0;      // 歯の数を数える変数
static int start = 0, end = 0;   // 時間計測

int ave_speed(int gpio_speed)
{
  /*C言語の場合冒頭で宣言する*/
  int i = 0;
  FILE *fp ;
  char *Speed_FILE[4] = {"dryA.csv", "dryB.csv", "crashA.csv", "crashB.csv"};
  char Date[100];
  speed_ sp[Data_MAX];
  double sum = 0;
  double ave = 0;
  //Vector vec[1024];
  if((fp=fopen(Speed_FILE[gpio_speed % 104],"r"))!=NULL){
      switch (gpio_speed) {
        case SPEED3:
          while(fscanf(fp, "%[^,], %g", Date, sp[i].crashA) != 10){
            sum += sp[i].crashA;
            i++;
          }
          break;
        case SPEED4:
          while(fscanf(fp, "%[^,], %g", Date, sp[i].crashB) != 10){
            sum += sp[i].crashB;
            i++;
          }
          break;
      }
      ave = sum / i + 0.01;
      /*忘れずに閉じる*/
      fclose(fp);
      switch (gpio_speed) {
        case SPEED3:
          time_A = ave;
          break;
        case SPEED4:
          time_B = ave;
          break;
      }
  }else{
    printf("アウトー\n" );
    return 1;
  }
  return 0;
}

// 立ち上がり待ち
void Speed_Rising(void)
{

  double ck_new = 0, ck_old = 0;
  printf("立ち上がりまで待つ\n" );
  start = millis();
  while(1){
    if(speed_rough() == 1){
      end = millis();
      ck_new = (double)(end - start) / 1000;
      if( fabs( ck_new - ck_old ) <= 0.0005 ){
        printf("立ち上がりOK\n");
        break;
      }
    }
  }
}

int read_speed(int gpio_speed )
{
  /* Read Current Switch Status */
  int i;
  int sum = 0;
  //float devi;

  for(i=0; i<10; i++){
        sum += digitalRead( gpio_speed );
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
*   歯車の凹凸検知
*   戻り値：0　→　検知なし or 凸検知 and 歯車の指定枚数分カウントしていない
*          1　→　凹検知 and 歯車の指定枚数分検知
*/
int speed_rough(void)
{
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
              return 1;
          }
          sp_flag = 0;
  }
  return 0;
}

int thread_speed(void *ptr){
  int gpio_speed = sel_sen; // gpioピンの格納
  int gear_;                // 刃の枚数

  int flg_sec = 0;
  // 時間    ////////////
  double ck_sec = 0;
  //////////////////////

  //　ギアの枚数を変更する
  if(gpio_speed == SPEED1){
    gear_ = GEAR_DRY;
    printf("脱水Ａ：");
  }
  if(gpio_speed == SPEED2){
    gear_ = GEAR_DRY;
    printf("脱水B：");
  }
  if(gpio_speed == SPEED3){
    gear_ = GEAR_CRASH;
    printf("減容Ａ：");
    // delay(time_sp*1000); ver.1
    Speed_Rising();
  }
  if(gpio_speed == SPEED4){
    gear_ = GEAR_CRASH;
    printf("減容Ｂ：");
    // delay(time_sp*1000); ver.1
    Speed_Rising();
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

    if(speed_rough() == 1){
      end = millis();
      ck_sec = (double)(end - start) / 1000;
      //printf("end : %d\n", end);
        if(gpio_speed == SPEED1){
          printf("脱水Ａ：");
          sp.dry_secA = ck_sec;
          write_value("dryA");
        }if(gpio_speed == SPEED2){
          printf("脱水B：");
          sp.dry_secB = ck_sec;
          write_value("dryB");
        }if(gpio_speed == SPEED3){
          printf("減容Ａ：");
          sp.crash_secA = ck_sec;
          write_value("crashA");
        }if(gpio_speed == SPEED4){
          printf("減容Ｂ：");
          sp.crash_secB = ck_sec;
          write_value("crashB");
        }
      printf("%.3f sec\n", ck_sec);
      start = millis();
    }else{
      end = millis();
      ck_sec = (double)(end - start) / 1000;
      if(gpio_speed == SPEED1)
      sp.dry_secA   = ck_sec;
      if(gpio_speed == SPEED2)
      sp.dry_secB   = ck_sec;
      if(gpio_speed == SPEED3)
      sp.crash_secA = ck_sec;
      if(gpio_speed == SPEED4)
      sp.crash_secB = ck_sec;
    }

    if(flg_manpai==1) break;
  }
  return 0;
}
