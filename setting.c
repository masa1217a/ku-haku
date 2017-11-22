/* get_param.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_MAX 256
#define NOTE_ 500
#define Setting_FILE "settings.txt"
#define Save_FILE "save.csv"
#define size  1

/*構造体宣言*/
typedef struct{
    char name[STR_MAX];    // センサなどの名前
    int  value;            // センサなどの値
    //char note[NOTE_];    // 備考
}Vector;

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
  Vector *ary = (Vector*)malloc(sizeof(Vector)*size);

  for(i=0;i<size;i++){
      /*
        ここに変化した値を入れる
      */
      Vector vec;
      strcpy(vec.name, n);
      vec.value = s;
      ary[i] = vec;
  }
  /*ファイル(save.csv)に書き込む*/
  if((fp=fopen(Save_FILE,"w"))!=NULL){
      for(i=0;i<size;i++){
          /*カンマで区切ることでCSVファイルとする*/
          fprintf(fp,"%s, %d\n",ary[i].name, &ary[i].value);
      }
      /*忘れずに閉じる*/
      fclose(fp);
  }
  return 0;
}

int read_(void)
{
  /*C言語の場合冒頭で宣言する*/
  FILE *fp ;
  int i = 0, data_count;
  Vector vec[1024];
  /*ファイル(save.csv)に読み込む*/
  if((fp=fopen(Save_FILE,"r"))!=NULL){
      i=0;
      while(fscanf(fp, "%[^,], %d", vec[i].name, &vec[i].value) != EOF){
          i++;
      }
      data_count = i;
      /*忘れずに閉じる*/
      fclose(fp);

      // 表示
      for(i=0; i<data_count-1; i++)
        printf("%s  =  %d\n",vec[i].name, vec[i].value);

      printf("データの数: %d\n", data_count);
  }
  return 0;
}

int param_init()
{

  if((mot1_F     = read_param("mot1_F")) < 0)     return -1;
  if((mot1_R     = read_param("mot1_R")) < 0)     return -1;
  if((mot1_STOP  = read_param("mot1_STOP")) < 0)  return -1;
  if((mot2_F     = read_param("mot2_F")) < 0)     return -1;
  if((mot2_R     = read_param("mot2_R")) < 0)     return -1;
  if((mot2_STOP  = read_param("mot2_STOP")) < 0)  return -1;
  if((LIGHT      = read_param("LIGHT")) < 0)      return -1;
  if((PHOTO1      = read_param("PHOTO1")) < 0)      return -1;
  if((PHOTO2     = read_param("PHOTO2")) < 0)     return -1;
  if((SPEED1     = read_param("SPEED1")) < 0)     return -1;
  if((SPEED2     = read_param("SPEED2")) < 0)     return -1;
  if((SPEED3     = read_param("SPEED3")) < 0)     return -1;
  if((SPEED4     = read_param("SPEED4")) < 0)     return -1;
  if((GEAR_DRY   = read_param("GEAR_DRY")) < 0)   return -1;
  if((GEAR_CRASH = read_param("GEAR_CRASH")) < 0) return -1;
  if((time_sp    = read_param("time_sp")) < 0)    return -1;
  if((KINSETU1   = read_param("KINSETU1")) < 0)   return -1;
  if((KINSETU2   = read_param("KINSETU2")) < 0)   return -1;
  if((KINSETU3   = read_param("KINSETU3")) < 0)   return -1;
  if((GREEN      = read_param("GREEN")) < 0)      return -1;
  if((RED        = read_param("RED")) < 0)        return -1;
  if((YELLOW     = read_param("YELLOW")) < 0)     return -1;
  if((BUZZER     = read_param("BUZZER")) < 0)     return -1;
  if((BUTTON1    = read_param("BUTTON1")) < 0)    return -1;
  if((BUTTON2    = read_param("BUTTON2")) < 0)    return -1;
  if((BUTTON3    = read_param("BUTTON3")) < 0)    return -1;
  if((LED1       = read_param("LED1")) < 0)       return -1;
  if((LED2       = read_param("LED2")) < 0)       return -1;
  if((SW1        = read_param("SW1")) < 0)        return -1;
  if((SW2        = read_param("SW2")) < 0)        return -1;
  if((SW3        = read_param("SW3")) < 0)        return -1;
  if((SW4        = read_param("SW4")) < 0)        return -1;

  return 0;
}

int main()
{
  //if(param_init()!=0) printf("11111\n");
  //if(write_param() != 0) printf("00000\n");
  if(read_() != 0) printf("000a0\n");

  return 0;
}
