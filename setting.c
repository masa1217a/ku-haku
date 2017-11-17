/* get_param.c */
#include <stdio.h>
#include <string.h>

#define STR_MAX 256
#define CONFIG_FILE "settings.txt"

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

int read_param(char *param_name)
{
    int i = 0, j = 0, m = 0;
    int output_param;
    char str[STR_MAX], param[STR_MAX], pmi[STR_MAX];
    FILE *fin;

    if ((fin = fopen(CONFIG_FILE, "r")) == NULL) {
        printf("fin error:[%s]\n", CONFIG_FILE);
        return -1; /* system error */
    }

    for(;;) {
        if (fgets(str, STR_MAX, fin) == NULL) {
            /* EOF */
            fclose(fin);
            return -3; /* not found keyword */
        }
        if (!strncmp(str, param_name, strlen(param_name))) {
            while (str[i++] != ':') {
               ;
            }
            while ( str[i] != ' '){
              pmi[m++] = str[i++];
            }
            pmi[m] = '\0';
            printf("%s : ", pmi);
            while (str[i++] != '=') {
                ;
            }
            while (str[i] != ' ') {
                param[j++] = str[i++];
            }
            param[j] = '\0';
            printf("param : %s\n", param);
            fclose(fin);
            output_param = atoi(param);
            return output_param;
        }
    }
    fclose(fin);
    return -1; /* not reachable */
}

char search_param(char *param_name, int parami)
{
    int i = 0, j = 0;
    int output_param;
    char ssi[3];
    char str[STR_MAX], param[STR_MAX];
    FILE *fin, *fout;

    sprintf(ssi, "%d", parami);

    if ((fin = fopen(CONFIG_FILE, "r+")) == NULL) {
        printf("fin error:[%s]\n", CONFIG_FILE);
        return -1; /* system error */
    }

    for(;;) {
        if (fgets(str, STR_MAX, fin) == NULL) {
            /* EOF */
            printf("探し物はありませんでした");
            fclose(fin);
            return -3; /* not found keyword */
        }
        if (!strncmp(str, param_name, strlen(param_name))) {
            while (str[i++] != '=') {
                ;
            }
            while(str[i] != ' '){
              str[i] = ssi[0];
              i++;
            }
            printf("%s\n", str);
            *param = *str;
            fclose(fin);
            return *param;
        }
    }
      fclose(fout);
    return -1; /* not reachable */
}

char write_param(char *param_name, char *pmname, int parami)
{
    int i = 0, j = 0;
    int output_param;
    char ssi[3];
    char str[STR_MAX], param[STR_MAX];
    FILE *fin, *fout;

    *str = search_param(param_name, parami);

    if ((fout = fopen(CONFIG_FILE, "r+")) == NULL) {
        printf("fin error:[%s]\n", CONFIG_FILE);
        return -1; /* system error */
    }

    for(;;) {
        if (fgets(param, STR_MAX, fout) == NULL) {
            /* EOF */
            printf("探し物はありませんでした");
            fclose(fout);
            return -3; /* not found keyword */
        }
        if (!strncmp(param, param_name, strlen(param_name))) {
            while (param[i++] != '=') {
                ;
            }

            fputs(str, fout);
            fclose(fout);
            return 0;
        }
      }
      fclose(fout);
    return -1; /* not reachable */
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
  if(param_init()!=0) printf("11111\n");
  if(write_param("photo_conf", "PHOTO2", 1) != 0) printf("00000\n");

  return 0;
}
