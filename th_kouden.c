
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
