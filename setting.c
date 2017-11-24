/* get_param.c */
#include <stdio.h>
#include <stdlib.h>
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
int photo_conf;                                         // 正常運転以外で停止した場合１で保存される(非常停止、停止)
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

char n[STR_MAX] = "aa";
int s=1;
char m[2]= "gg";

int read_param(char *param_name)
{
    int i = 0, j = 0;
    int output_param;
    char str[STR_MAX], param[STR_MAX];
    FILE *fin;

    if ((fin = fopen(Setting_FILE, "r")) == NULL) {
        printf("fin error:[%s]\n", Setting_FILE);
        return -1; /* system error */
    }

    for(;;) {
        if (fgets(str, STR_MAX, fin) == NULL) {
            /* EOF */
            fclose(fin);
            return -3; /* not found keyword */
        }
        if (!strncmp(str, param_name, strlen(param_name))) {
            while (str[i++] != '=') {
                ;
            }
            while (str[i] != ' ') {
                param[j++] = str[i++];
            }
            param[j] = '\0';
            printf("%14s : %3s\n", param_name , param);
            fclose(fin);
            output_param = atoi(param);
            return output_param;
        }
    }
    fclose(fin);
    return -1; /* not reachable */
}

int write_param(void)
{
  /*C言語の場合冒頭で宣言する*/
  FILE *fp ;
  int i;
  int value;
  char name[STR_MAX];

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
  photo_conf      = vec[35].value;
  return 0;
}

int main()
{
  //Vector vec[Data_MAX];
  //if(param_init()!=0) printf("11111\n");
  //if(write_param() != 0) printf("00000\n");
  if(param_init() != 0) printf("000a0\n");
  vec[35].value = 1;
  write_param();
  //SettingRead();

  return 0;
}
