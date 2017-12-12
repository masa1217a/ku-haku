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

// 温度センサ
int sensor_Temp(void)
{
  adc02();
  printf("モーター停止中\n");
  if(temp.dryA>=MOT_Temp){
    printf("エラー:異常な温度を検知 : 脱水部A\n");
    LOG_PRINT("異常な温度を検知 : 脱水部A", LOG_NG);
    error=8;
    lcd();
  }
  if( temp.dryB>=MOT_Temp ){
    printf("エラー:異常な温度を検知 : 脱水部B\n");
    LOG_PRINT("異常な温度を検知 : 脱水部B", LOG_NG);
    error=9;
    lcd();
  }
  if( temp.crashA>=MOT_Temp ){
    printf("エラー:異常な温度を検知 : 減容部A\n");
    LOG_PRINT("異常な温度を検知 : 減容部A", LOG_NG);
    error=10;
    lcd();
  }
  if( temp.crashB>=MOT_Temp){
    printf("エラー:異常な温度を検知 : 減容部B\n");
    LOG_PRINT("異常な温度を検知 : 減容部B", LOG_NG);
    error=11;
    lcd();
  }
  if( error == 8 || error == 9){
    delay(200);
    return -1;
  }else if( error == 10 || error == 11){
    delay(200);
    return -2;
  }else{
    LOG_PRINT("動作停止中：温度 OK", LOG_OK);
    return 0;
  }
}

/*****************************************
*               初期化処理                *
*****************************************/
int sys_format(void){
    st = 0;
    teisi = 0;
    int i;
    int fstart,fend;
    // 初期化のフラグ
    int flg_for[10] = {0,0,0,0,0,0,0,0,0,0};

    digitalWrite(RED,    0);
    digitalWrite(YELLOW, 1);
    digitalWrite(GREEN,  0);
    printf("---------初期モード開始---------\n\n");
    LOG_PRINT("---------初期モード開始---------", LOG_OK);
    digitalWrite(LED1,1);
    digitalWrite(LED2,0);

    lcdPosition(fd_lcd,0,0);
    lcdPrintf (fd_lcd, "\xBD\xB2\xAF\xC1\xA6\xBD\xCD\xDE\xC3\x4F\x46\x46\xC6\xBC\xC3\xB8\xC0\xDE\xBB\xB2") ;        //スイッチヲスベテOFFニシテクダサイ
    while(1){
        if(shutdown ==1) break;
        int old_sw1 = sw1;
        int old_sw2 = sw2;
        int old_sw3 = sw3;
        int old_sw4 = sw4;

        sw1=digitalRead(SW1);
        sw2=digitalRead(SW2);
        sw3=digitalRead(SW3);
        sw4=digitalRead(SW4);

        if(sw1 == 0)    d_power= 0;             //脱水　電源
        else   d_power = 1;
        if(sw2 == 0)    d_state = 0;                //脱水　正/逆
        else   d_state = 1;
        if(sw3 == 0)    g_power = 0;                //減容　電源
        else   g_power = 1;
        if(sw4 == 0)    g_state = 0;                //減容　正/逆
        else   g_state = 1;

        if(sw1 != old_sw1) lcd();
        if(sw2 != old_sw2) lcd();
        if(sw3 != old_sw3) lcd();
        if(sw4 != old_sw4) lcd();

        if(sw1==0 && sw2==0 && sw3==0 && sw4==0)    break;
    }

    // 袋の設置確認
    lcdPosition(fd_lcd,0,0);
    lcdPrintf (fd_lcd, "\xCC\xB8\xDB\xCA\xBE\xAF\xC1\xBC\xCF\xBC\xC0\xB6\x3F       ") ;     //フクロハセッチシマシタカ？
    while(digitalRead(BUTTON1) == 0 && kinsetu3 == 0){ //袋を設置したらスタートボタンを押す(ボタンが押されるまで待つ）
      if(digitalRead(BUTTON1) == 1 && kinsetu3 == 0){
        error = 4;
        lcd();
        printf("屑箱を設置してください\n");
        LOG_PRINT("屑箱なし", LOG_NG);
      }
    }
    LOG_PRINT("袋設置", LOG_OK);

    lcdPosition(fd_lcd,0,0);
    lcdPrintf (fd_lcd, "\xBE\xAF\xC4\xB1\xAF\xCC\xDF\xC1\xAD\xB3          ") ;      //セットアップチュウ
    sleep(1);
    while(!flg_for[9]){
        while(1){

            /* 4.   脱水部投入扉が閉じているか */
            if(kinsetu1 == 0 ){
                printf("扉を閉めてください\n");
                LOG_PRINT("扉が開いている", LOG_NG);
                error=1;      //エラーNo.
                lcd();
                flg_for[0] = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("扉 OK", LOG_OK);
             flg_for[0] = 1;
            }
            //printf("flg_4 = %d\n",  flg_4);

            /* 1. 脱水部と減容部のドッキングがされているか  */
            if(kinsetu2 == 0 ){
                printf("脱水部と減容部のドッキングがされていません\n");
                LOG_PRINT("ドッキングエラー", LOG_NG);
                error=2;        // エラーNO.
                lcd();
                flg_for[1] = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("ドッキング OK", LOG_OK);
             flg_for[1] = 1;
            }
            //printf("\rflg_1 = %d\n",  flg_1);

            /* 2.   屑箱が設置されているか                */
            if(kinsetu3 == 0 ){
                printf("屑箱を設置してください\n");
                LOG_PRINT("屑箱なし", LOG_NG);
                error=3;
                lcd();
                flg_for[2] = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("屑箱設置中", LOG_OK);
             flg_for[3] = 1;
            }
            //printf("flg_2 = %d\n",  flg_2);

            /* 5.   脱水部にスポンジが残されていないか       */
            adc01();
            if(st==0&&teisi==0 && dist.ch0<25 && dist.ch1<25)
            {
                printf("エラー:投入口のスポンジの量が多いです\n");
                LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
                error=4;
                lcd();
                flg_for[3] = 0;
                delay(200);
                break;
            }else{
                LOG_PRINT("投入口のスポンジの量 OK", LOG_OK);
                flg_for[3] = 1;
            }
            //printf("flg_5 = %d\n",  flg_5);

            /* 6.   減容部にスポンジが残されていないか       */
            adc01();
            if(st==0&&teisi==0 && dist.ch2<25 && dist.ch3<25)
            {
                printf("エラー:スポンジの量が多いです\n");
                LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
                error=5;
                lcd();
                flg_for[4] = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("投入口のスポンジの量がちょうどいい", LOG_OK);
             flg_for[4] = 4;
            }
            //printf("flg_6 = %d\n",  flg_6);

            /* 3.   屑箱内にスポンジが残っていないか       */
            adc01();
            if(st==0&&teisi==0 && dist.ch4<25 && dist.ch5<25 )
            {
                printf("エラー:屑箱内のスポンジの量が多いです\n");
                LOG_PRINT("屑箱内のスポンジの量が多い",LOG_NG);
                error=6;
                lcd();
                flg_for[5] = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("投入口のスポンジの量 OK", LOG_OK);
             flg_for[5] = 1;
            }
            //printf("flg_3 = %d\n",  flg_3);

            /* 7.   モータの温度が安定動作できる範囲であるか  */
            if(sensor_Temp() == 0) flg_for[6] = 1;
            else flg_for[6] = 0;

            //printf("flg_7 = %d\n",  flg_7);

            // モーターの動作停止中の光電センサ
            /*
            KOUDEN = PHOTO1;
            pthread_create( &th, NULL, (void*(*)(void*))thread_photo, NULL);    //スレッド[pth]スタート
            delay(50);
            KOUDEN = PHOTO2;
            pthread_create( &th, NULL, (void*(*)(void*))thread_photo, NULL);    //スレッド[pth]スタート
            fstart = millis();
            while(1){
              fend = millis();
              if( ((fend - fstart) / 1000) == 5)
            }
            */

            /* 8.   脱水部と減容部の詰まり確認　 */
            sel_sen = SPEED1;
            pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート
            delay(50);
            sel_sen = SPEED2;
            pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート
            delay(50);
            sel_sen = SPEED3;
            pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート
            delay(50);
            sel_sen = SPEED4  ;
            pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート

            // 動作停止中の検知するかどうか
            /*
            mot_state = MOT_OFF;
            mot_state2 = MOT_OFF;
            while(1){
              if( crash_sec >= 2 ){
                if( dry_sec >= 7 ){
                  lcdPosition(fd_lcd,0,0);
                  lcdPrintf (fd_lcd, "\xBF\xB8\xC4\xDE\xBE\xDD\xBB\xA0\xBE\xB2\xBC\xDE\xAE\xB3      ") ;      //ソクドセンサ　セイジョウ
                }
              }
            }
            */
            mot_state = MOT_Format;
            mot_state2 = MOT_Format;
            printf("モーター動作中\n");
            while(1){
                if(shutdown ==1) break;

                /*
                *   温度センサが異常になるか
                *   速度センサが歯車の停を検知した時
                */
                if( sensor_Temp() == -1 ){
                  mot_state = MOT_OFF;
                }
                else if( sp.dry_secA >= 10 || sp.dry_secB >= 10 ){
                    mot_state = MOT_Clean;
                    //mot_state2 = MOT_Clean;
                    //printf("%.3f sec\n", dry_sec);
                }

                if( sensor_Temp() == -2 ){
                  mot_state2 = MOT_OFF;
                }
                else if( sp.crash_secA >= 2 || sp.crash_secB >= 2 ){
                  mot_state2 = MOT_Clean;
                }

                if(motor1 == 1) mot_state = MOT_OFF;
                if(motor2 == 1) mot_state2 = MOT_OFF;

                delay(100);

                // クリーンモードをクリアした時
                if(motor1 == 1 && motor2 == 1){
                    flg_for[7] = 1;
                    break;
                }
                delay(500);
            }

            if( flg_for[7] = 1 )   LOG_PRINT("詰まりなし", LOG_OK);
            else {
                error = 6;
                lcd();
                printf("エラー:詰まりを検知\n");
                LOG_PRINT("詰まりを検知", LOG_NG);
            }
            //printf("flg_8 = %d\n\n",  flg_8);

            /*9.スポンジ残ってないか確認*/
            // モーター動作中に温度、速度、光電、近接が機能しているか
            KOUDEN = PHOTO1;
            pthread_create( &th, NULL, (void*(*)(void*))thread_photo, NULL);    //スレッド[pth]スタート
            delay(50);
            KOUDEN = PHOTO2;
            pthread_create( &th, NULL, (void*(*)(void*))thread_photo, NULL);    //スレッド[pth]スタート
            mot_state  = MOT_For_check;
            mot_state2 = MOT_For_check;
            while(1){
                if(motor1 == 1) mot_state = MOT_OFF;
                if(motor2 == 1) mot_state2 = MOT_OFF;

                delay(100);
                if (motor1 == 1 && motor2 == 1){
                    LOG_PRINT("スポンジ無し", LOG_OK);
                    flg_for[8] = 1;
                    break;
                }
                delay(100);
            }
            //printf("flg_9 = %d\n\n",  flg_9);
            pthread_detach(th_ph);

            pthread_detach(th_sp);

            // 終了条件
            for(i=0; i<9; i++){
              if(flg_for[i] == 0) break;
            }

            if(i == 9){
                st=1;
                flg_for[i] = 1;
                error=0;
                delay(200);
                break;
            }
        }
        while(error != 0 ){
            if(shutdown ==1) break;
            digitalWrite(RED,    1);
            digitalWrite(YELLOW, 0);
            digitalWrite(GREEN,  0);
            //モーターＯＦＦ
            //エラー処理
            adc01();
                 if( kinsetu1 == 1 ) error = 0;
            else if( kinsetu2 == 1 ) error = 0;
            else if( kinsetu3 == 1 ) error = 0;
            else if( dist.ch4>=25 && dist.ch5>=25 )
                error = 0;
            else if( dist.ch0>=25 && dist.ch1>=25 )
                error = 0;
            else if( dist.ch2>=25 && dist.ch3>=25 )
                error = 0;
            else if( temp.dryA>=MOT_Temp || temp.dryB>=MOT_Temp ||
                                 temp.crashA>=MOT_Temp || temp.crashB>=MOT_Temp)
                error = 0;

            delay(200);
        }
        digitalWrite(RED,    0);
        digitalWrite(YELLOW, 1);
        digitalWrite(GREEN,  0);

    }
    printf("---------初期モード終了---------\n");
    LOG_PRINT("---------初期モード終了---------", LOG_OK);
    return 0;
}
