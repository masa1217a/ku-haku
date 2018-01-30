#include "ketugou.h"

extern int btn1, btn2, btn3,sw1,sw2,sw3,sw4,shutdown;
extern int st, t1, t2, mode,kenti,error,teisi,d_teisi,d_end,act;
extern int fd_lcd,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
extern int d_power,g_power,d_state,g_state;
extern int mot_state, mot_state2;

extern double dry_sec;
extern double crash_sec;

extern int sel_sen;
extern int KOUDEN;
extern int motor1;
extern int motor2;

extern int flg_manpai;

/*****************************************
*				スレッド処理			     *
*****************************************/
/************************/
/*     光電スレッド
*     60秒検知しなかったら終了
*     検知したら脱水モータを止める
*     一定時間経過後脱水モーターをスタート
*     これを1サイクルとする　メイン処理
*     終了時は脱水とモーターを止める
*/

static int time_count = 0;
static int pht;

/*
    光電の無反応時間を検知
*/
void photo_sec(void)
{
  time_count++;
  if((time_count % 10) == 0){
    printf("検知：無　　" );
    printf("経過時間：%d\n", (time_count % 10));
  }
  if((time_count % 10) == 60){
    printf("スポンジがなくなりました。\n");
    printf("処理を終了します。\n");
    st = 1;
  }
  delay(100);
}

/*
    スポンジ検知
*/
void read_photo(int photo)
{
  pht = digitalRead(photo); // 光電の状態を見る

  if(pht == 0)
    printf("光電　反応：有\n");
  else
    //printf("光電　反応：無\n");
}

/*
  　検知後の待ち処理
*/
void wait_photo(void)
{
  int i;
  for(i=0;i<100;i++){
    delay(100);
  }
}

/*
    モーターの状態
*/
void photo_mot(photo_state)
{
  switch (photo_state) {
    case 0:   //　光電検知後、脱水モータを停止
      mot_state = MOT_OFF;
      break;
    case 1;  //　終了時の処理
      mot_state = MOT_OFF;
      mot_state2 = MOT_Clean;
      break;
    case 2:
      mot_state = MOT_For;
  }
}

int thread_photo(void *ptr){
    while(1)
    {
      photo_sec();
      if(st == 1) break;
      read_photo();
      if(pht == 0)
        photo_mot(0);
      wait_photo();
      photo_mot(2);
    }
    return 0;
}
