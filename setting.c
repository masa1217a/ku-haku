/* get_param.c */
#include "ketugou.h"

void write_param(void)
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
  }else{
    printf("%sが開けません", Save_FILE);
    exit(1);
  }
}

void write_value(char *sensor_name)
{
  /*C言語の場合冒頭で宣言する*/
  FILE *fp ;
  //int value;
  char name[STR_MAX];
  char date_time[STR_MAX];

// 時間
  time_t timer;
  struct tm *date;

  /* 時間取得 */
  timer = time(NULL);
  date = localtime(&timer);
  strftime(date_time, sizeof(date_time), "[%Y/%m/%d %H:%M:%S] ", date);

  /*ファイルに書き込む*/
  strcpy(name, sensor_name);
  strcat(name, ".csv");
  if((fp=fopen(name,"a"))!=NULL){
      /*カンマで区切ることでCSVファイルとする*/
      if(strcmp(sensor_name, "Distance")==0)
        fprintf(fp, "%s, %d, %d, %d, %d, %d, %d\n", date_time, dist.ch0, dist.ch1, dist.ch2, dist.ch3, dist.ch4, dist.ch5);
      else if(strcmp(sensor_name, "Temprature")==0)
        fprintf(fp, "%s, %g , %g , %g , %g\n", date_time, temp.dryA , temp.dryB, temp.crashA , temp.crashB);
      else if(strcmp(sensor_name, "dryA") == 0)
        fprintf(fp, "%s, %g\n", date_time,  sp.dry_secA);
      else if(strcmp(sensor_name, "dryB") == 0)
		fprintf(fp, "%s, %g\n", date_time, sp.dry_secB);
      else if(strcmp(sensor_name, "crashA") == 0)
		fprintf(fp, "%s, %g\n", date_time, sp.crash_secA);
      else if(strcmp(sensor_name, "crashB") == 0)
		fprintf(fp, "%s, %g\n", date_time, sp.crash_secB);
      /*忘れずに閉じる*/
      fclose(fp);
  }else{
    printf("%sが開けません", name);
    exit(1);
  }
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
  FlgKouden       = vec[35].value;
  time_A          = vec[36].value;
  time_B          = vec[37].value;
  return 0;
}
