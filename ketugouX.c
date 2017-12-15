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

/*
ui.runbtn=0;
ui.stopbtn=0;
ui.sw1=0;
ui.sw2=0;
ui.sw3=0;
ui.sw4=0;
ui.shutdown=0;
int st=0, t1=0, t2=0, mode=1, error=0, d_teisi=0, d_end = 0, act=0;
int fd_lcd=0,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
int d_power,g_power,d_state,g_state;
mot.dstate = MOT_OFF, mot.gstate = MOT_OFF;
*/
/*
int btn1, btn2, btn3,sw1,sw2,sw3,sw4,shutdown;
int st, t1, t2, mode,kenti,error,teisi,d_teisi,d_end,act;
int fd_lcd,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
int d_power,g_power,d_state,g_state;
int mot_state, mot_state2;

double dry_sec;
double crash_sec;

int sel_sen;
int KOUDEN;
int motor1;
int motor2;

int flg_manpai;

double dry_secA;
double dry_secB;
double crash_secA;
double crash_secB;
*/
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

static volatile unsigned long time_prev = 0, time_now;

pthread_t normal;
pthread_t admin;
pthread_t th;
pthread_t th_sp;
pthread_t th_ph;

/*センサ読み込みスレッド*/
int thread_Read(void *ptr){

    while(1){
        kinsetu1=digitalRead(KINSETU1);
        kinsetu2=digitalRead(KINSETU2);
        kinsetu3=digitalRead(KINSETU3);
        delay(100);
    }
    return 0;
}

/*USBでのLOGファイル保存スレッド*/
int USB(void *ptr){

        system("sudo sh /home/pi/usb_log.sh");

    return 0;
}

/*****************************************
*                           外部割り込み                                          *
******************************************/
/*一時停止ボタン*/
void stop(void){
  volatile unsigned long time_prev = 0, time_now;
    if(act==1){
        time_now = millis();
        if(time_now-time_prev > 500){
            st = 1;
            mot_state = MOT_OFF;
            mot_state2 = MOT_OFF;
            printf("一時停止\n");
        }
    time_prev = time_now;
    }
}

/*****************************************
*                         モード処理                                       *
*****************************************/
/*通常モード*/
int thread_normal(void *ptr)
{
	mode=0;
    lcdClear(fd_lcd);
    lcdPosition(fd_lcd,0,1);
    lcdPrintf (fd_lcd, "       \xC3\xDE\xDD\xB9\xDE\xDD  \xD3\xB0\xC0" ) ;
    lcdPosition(fd_lcd,0,2);
    lcdPrintf (fd_lcd, "\xC0\xDE\xAF\xBD\xB2 : " ) ;
    lcdPosition(fd_lcd,0,3);
    lcdPrintf (fd_lcd, "\xB9\xDE\xDD\xD6\xB3 : " ) ;
    lcd();
    int kyori_count1=0;
    int kyori_count2=0;
    int kyori_count3=0;
    int   kyori_state = 0;
    d_end = 0;
    act=0;
    st =0;
    printf("通常モード\n");
    LOG_PRINT("---------通常モード開始---------", LOG_OK);
    digitalWrite(LED1, 1);
    digitalWrite(LED2, 0);
    lcdPosition(fd_lcd,0,0);
    lcdPrintf (fd_lcd, "\xCC\xB8\xDB\xCA\xBE\xAF\xC1\xBC\xCF\xBC\xC0\xB6\x3F       ") ;     //フクロハセッチシマシタカ？
    while(1){
        if(error == 0){
            digitalWrite(RED, 0);
            digitalWrite(YELLOW, 1);
            digitalWrite(GREEN, 0);
        }

        int old_sw1 = sw1;
        int old_sw2 = sw2;
        int old_sw3 = sw3;
        int old_sw4 = sw4;

        btn1=digitalRead(BUTTON1);
        btn2=digitalRead(BUTTON2);
        btn3=digitalRead(BUTTON3);
        sw1=digitalRead(SW1);
        sw2=digitalRead(SW2);
        sw3=digitalRead(SW3);
        sw4=digitalRead(SW4);

        if(btn3 == 1){
                shutdown_btn();
        }

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

/****************************************************************************/
/*                          スタートボタン押した時の動作                              */
/****************************************************************************/
        if(btn2==0 && btn1==1) {
        if(d_power == 1 || g_power == 1){
            st =0;
            kyori_count1 = 0;
            kyori_count2 = 0;
            kyori_count3 = 0;
            if(kinsetu1  == 0) {
					printf("扉が開いている\n");
                    LOG_PRINT("扉が開いている", LOG_NG);
                    error=1;
                    lcd();
                    st=1;
                    delay(100);
            }else if(kinsetu2 == 0) {
					printf("ドッキングされていない\n");
                    LOG_PRINT("ドッキングエラー", LOG_NG);
                    error=2;
                    lcd();
                    st=1;
                    delay(100);
            }else if(kinsetu3  == 0) {
					printf("屑箱が設置されていない\n");
                    LOG_PRINT("屑箱が設置されていない", LOG_NG);
                    error=3;
                    lcd();
                    st=1;
                    delay(100);
            }else{
                 LOG_PRINT("近接センサ部OK", LOG_OK);
                error = 0;
            }

            adc01();
            if(st==0)
            {
                if((dist.ch0)<25 || (dist.ch1)<25){                 //測距：脱水投入口
                    printf("エラー:スポンジの量が多いです（脱水投入口）\n");
                    LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
                    error=4;
                    lcd();
                    st=1;
                    delay(100);
                }else if(dist.ch4<25 || dist.ch5<25){                     //測距：屑箱
                    printf("エラー:スポンジの量が多いです（屑箱）\n");
                    LOG_PRINT("屑箱のスポンジの量が多い",LOG_NG);
                    error=5;
                    lcd();
                    st=1;
                    delay(100);
                }else{
                    LOG_PRINT("屑箱のスポンジの量がちょうどいい", LOG_OK);
                    error = 0;
                }
            }

            if(error==0){
                digitalWrite(RED, 0);
                digitalWrite(YELLOW, 0);
                digitalWrite(GREEN, 1);
            }
            if(error > 0){
                digitalWrite(RED, 1);
                digitalWrite(YELLOW, 0);
                digitalWrite(GREEN, 0);
            }

            if(st==0) {
                printf("スタート\n");
                lcdPosition(fd_lcd,0,0);
                lcdPrintf (fd_lcd, "\xBC\xAE\xD8\xBC\xC3\xB2\xCF\xBD              ") ;      //ショリシテイマス
                error = 0;
                act=1;
                LOG_PRINT("スタート", LOG_OK);
            }

            if(st==0 && g_power== 1 && d_power== 1 && d_state==0 && d_teisi == 0 && d_end ==0)  mot_state = MOT_For;
            if(st==0 && g_power== 1 && d_power== 1 && d_state==1 && d_teisi == 0 && d_end ==0)  mot_state = MOT_Rev;
            if(st==0 && g_power== 1 && g_state==0)  mot_state2 = MOT_For;
            if(st==0 && g_power== 1 && g_state==1)  mot_state2 = MOT_Rev;

			if(mot_state==MOT_For || mot_state==MOT_Rev){
                KOUDEN = PHOTO1;
                pthread_create( &th, NULL, (void*(*)(void*))thread_photo, NULL);    //スレッド[pth]スタート
                sel_sen = SPEED1;
                pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート
                delay(50);
                sel_sen = SPEED2;
                pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート
                delay(50);

			}

			if(mot_state2==MOT_For || mot_state2==MOT_Rev){
                KOUDEN = PHOTO2;
                pthread_create( &th, NULL, (void*(*)(void*))thread_photo, NULL);    //スレッド[pth]スタート
				sel_sen = SPEED3;
                pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート
                delay(50);
                sel_sen = SPEED4  ;
                pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL); //スレッド[speed]スタート
			}

            while(1){
                if(st  == 1)
                {
                    kyori_count1 = 0;
                    kyori_count2 = 0;
                    act=0;
                    mot_state = MOT_OFF;
                    mot_state2 = MOT_OFF;
                    if(error < 1) {
                        lcdPosition(fd_lcd,0,0);
                        lcdPrintf (fd_lcd, "\xCC\xB8\xDB\xCA\xBE\xAF\xC1\xBC\xCF\xBC\xC0\xB6\x3F        ") ;        //フクロハセッチシマシタカ？
                    }
                    if(shutdown == 1) lcdClear(fd_lcd);
                    break;
                }

                if(d_power == 1 && g_power == 0 && d_end == 1){
                    d_end = 0;
                    st=1;
                }

				adc02();
                if(temp.dryA>=MOT_Temp || temp.dryB>=MOT_Temp || temp.crashA>=MOT_Temp || temp.crashB>=MOT_Temp){
                    if(temp.dryA>=MOT_Temp)error = 8;
                    if(temp.dryB>=MOT_Temp)error = 9;
                    if(temp.crashA>=MOT_Temp)error = 10;
                    if(temp.crashB>=MOT_Temp)error = 11;
                    LOG_PRINT("異常な温度を検知", LOG_NG);
                    st = 1;
                    lcd();
                    delay(200);
                }

                if(kinsetu1  == 0 || kinsetu2 == 0 || kinsetu3 == 0)
                {
                    if(kinsetu1 == 0){
                        error=1;
						printf("扉が開いている\n");
                        LOG_PRINT("扉が開いている", LOG_NG);
                    }
                    if(kinsetu2 == 0){
                        error=2;
						printf("ドッキングされていない\n");
                        LOG_PRINT("ドッキングエラー", LOG_NG);
                    }
                    if(kinsetu3 == 0){
                        error=3;
						printf("屑箱が設置されていない\n");
                        LOG_PRINT("屑箱が設置されていない", LOG_NG);
                    }
                    lcd();
                    st = 1;
                    act=0;
                    mot_state = MOT_OFF;
                    mot_state2 = MOT_OFF;
                    break;
                }

                adc01();
                if(dist.ch4<25 || dist.ch5<25){                     //測距：屑箱
                    kyori_count1++;
                    if(kyori_count1 >= 25){
						printf("屑箱が満杯\n");
                        LOG_PRINT("屑箱が満杯",LOG_NG);
                        error=5;
                        lcd();
                        st=1;
                        delay(100);
                    }
                }
                else kyori_count1 = 0;

                if(kyori_state == 0){
                    if(dist.ch2<15 || dist.ch3<15){                 //測距：減容貯蓄部      満杯検知前
                        kyori_count2++;
                        printf("減容貯蓄部満杯カウント  %d\n",kyori_count2);
                        if(kyori_count2 >= 25){
                            LOG_PRINT("減容貯蓄部　満杯検知",LOG_NG);
                            flg_manpai = 1;
                            kyori_count2 = 0;
                            kyori_state = 1;
                            d_teisi = 1;
                            mot_state = MOT_OFF;
                            delay(100);
                        }
                    }
                    else kyori_count2 = 0;
                }
                else if(kyori_state == 1){
                    if(dist.ch2>=15 && dist.ch3>=15){                   //測距：減容貯蓄部      満杯検知後
                        kyori_count3++;
                        printf("減容貯蓄部満杯解除カウント %d\n",kyori_count3);
                        if(kyori_count3>= 50){
                            LOG_PRINT("減容貯蓄部　満杯解除",LOG_OK);
                            flg_manpai = 0;
                            kyori_count3 = 0;
                            kyori_state = 0;
                            d_teisi = 0;
                            if(st==0 && d_power== 1 && d_state==0 && d_teisi == 0 && d_end ==0) mot_state = MOT_For;
                            if(st==0 && d_power== 1 && d_state==1 && d_teisi == 0 && d_end ==0) mot_state = MOT_Rev;
                            delay(100);
                        }
                    }
                }

                if(st==0 && d_power== 1 && d_teisi == 0 && d_end == 0 && dry_sec >= 10 ){                                                   //脱水部　詰まり検知
                    printf("脱水部詰まり検知\n");
                    mot_state = MOT_Clean;
                    printf("%.3f sec\n", dry_sec);
                    while(1){
                        if(st ==1) break;
                        if(dry_sec >= 30){
                            printf("%.3f sec\n", dry_sec);
                            LOG_PRINT("脱水部の詰まり検知",LOG_NG);
                            mot_state = MOT_OFF;
                            error = 6;
                            lcd();
                            st = 1;
                            break;
                        }else if(motor1 == 1){
							motor1 = 0;
                            LOG_PRINT("脱水部の詰まり解消",LOG_OK);
                            if(st==0 && d_power== 1 && d_state==0)  mot_state = MOT_For;
                            if(st==0 && d_power== 1 && d_state==1)  mot_state = MOT_Rev;
                            break;
                        }
                    }
                }

                if(st==0 && g_power== 1 && crash_sec >= 10 ){                                                   //減容部　詰まり検知
                    printf("減容部詰まり検知\n");
                    mot_state2 = MOT_Clean;
                    printf("%.3f sec\n", crash_sec);
                    while(1){
                        if(st ==1) break;

                        if(crash_sec >= 20){
                            printf("%.3f sec\n", crash_sec);
                            LOG_PRINT("減容部の詰まり検知",LOG_NG);
                            mot_state2 = MOT_OFF;
                            error = 7;
                            lcd();
                            st = 1;
                            break;
                        }else if(motor2 == 1){
							motor2 = 0;
                            LOG_PRINT("減容部の詰まり解消",LOG_OK);
                            if(st==0 && g_power== 1 && g_state==0)  mot_state2 = MOT_For;
                            if(st==0 && g_power== 1 && g_state==1)  mot_state2 = MOT_Rev;
                            break;
                        }
                    }
                }

                delay(200);
            }
        }else{
            error = 12;
            lcd();
        }
        }

        if(error > 0){
				ERROR();
				delay(200);
		}

/****************************************************************************/
/*              (一時停止→スタートボタン)を長押しした時の動作                            */
/****************************************************************************/
        if(btn2==1) {
            if(btn1 == 1){
                while(1){
                    btn1=digitalRead(BUTTON1);
                    btn2=digitalRead(BUTTON2);
                    t1++;
                    if(btn1==0 || btn2==0)
                    {
                        t1=0;
                        break;
                    }
                    if(t1 >= 50){
                        t1=0;
                        st = 1;
                        mode=2;                             //スレッド[admin]スタート
                        LOG_PRINT("*****管理者モードに移行******", LOG_OK);
                        break;
                    }
                    delay(100);
                }
            }

        }
        if(mode==2  || shutdown==1) break;
    }
    LOG_PRINT("---------通常モード終了---------", LOG_OK);
    return 0;
}

/*管理モード*/
int  thread_admin(void *ptr)
{
    lcdClear(fd_lcd);
    lcdPosition(fd_lcd,0,1);
    lcdPrintf (fd_lcd, "       \xC3\xDE\xDD\xB9\xDE\xDD  \xD3\xB0\xC0" ) ;
    lcdPosition(fd_lcd,0,2);
    lcdPrintf (fd_lcd, "\xC0\xDE\xAF\xBD\xB2 : " ) ;
    lcdPosition(fd_lcd,0,3);
    lcdPrintf (fd_lcd, "\xB9\xDE\xDD\xD6\xB3 : " ) ;
    lcd();
    st =0;
    act=0;
    mode=0;
    printf("管理者モード\n");
    LOG_PRINT("---------管理者モード開始---------", LOG_OK);
    digitalWrite(LED1, 0);
    digitalWrite(LED2, 1);
    while(1){
lcdPosition(fd_lcd,0,0);
lcdPrintf (fd_lcd, "\xD2\xDD\xC3\xC5\xDD\xBD\xD3\xB0\xC4\xDE \xC1\xAD\xB3\xB2  ") ;         //メンテナンスモード チュウイ

        if(error==0){
            digitalWrite(RED, 0);
            digitalWrite(YELLOW, 1);
            digitalWrite(GREEN, 0);
        }
        if(error > 0){
            digitalWrite(RED, 1);
            digitalWrite(YELLOW, 0);
            digitalWrite(GREEN, 0);
        }

        int old_sw1 = sw1;
        int old_sw2 = sw2;
        int old_sw3 = sw3;
        int old_sw4 = sw4;

        btn1=digitalRead(BUTTON1);
        btn2=digitalRead(BUTTON2);
        btn3=digitalRead(BUTTON3);
        sw1=digitalRead(SW1);
        sw2=digitalRead(SW2);
        sw3=digitalRead(SW3);
        sw4=digitalRead(SW4);

        if(btn3 == 1){
                shutdown_btn();
        }

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
        if(btn2==0 && btn1==1) {                            //スタートボタン押した時の動作
            st =0;
            printf("スタート\n");
            LOG_PRINT("スタート", LOG_OK);
            if(error==0){
                digitalWrite(RED, 0);
                digitalWrite(YELLOW, 0);
                digitalWrite(GREEN, 1);
            }
            if(error > 0){
                digitalWrite(RED, 1);
                digitalWrite(YELLOW, 0);
                digitalWrite(GREEN, 0);
            }
            if(st==0)
            {
                act=1;
                error=0;
            }
            if(st==0 && d_power== 1 && d_state==0)  mot_state = MOT_For;
            if(st==0 && d_power== 1 && d_state==1)  mot_state = MOT_Rev;
            if(st==0 && g_power== 1 && g_state==0)  mot_state2 = MOT_For;
            if(st==0 && g_power== 1 && g_state==1)  mot_state2 = MOT_Rev;

            while(1){
                if(st  == 1) break;
            }
        }

        if(btn2==1) {                                   //(一時停止→スタートボタン)を長押しした時の動作
            if(btn1 == 1){
                while(1){
                    btn1=digitalRead(BUTTON1);
                    btn2=digitalRead(BUTTON2);
                    t2++;
                    if(btn1==0|| btn2==0)
                    {
                        t2=0;
                        break;
                    }
                    if(t2 >= 50){
                        t2=0;
                        st =1;
                        mode=1;                         //スレッド[normal]スタート
                        if(param_init() == -1){
                            printf("param_init Fail\n");
                            exit(1);
                        }
                        IOsetting();
                        LOG_PRINT("*****通常モードに移行*****", LOG_OK);
                        break;
                    }
                    delay(100);
                }
            }
        }

        if(mode==1 || shutdown==1) break;
    }
    LOG_PRINT("---------管理者モード終了---------", LOG_OK);
    return 0;
}

/*****************************************
*                                      関数                                              *
*****************************************/
/*LCD関数*/
int lcd(void){
    if(sw1 == 1){
        lcdPosition(fd_lcd,8,2);
        lcdPrintf (fd_lcd, "ON ") ;
    }
    else {
        lcdPosition(fd_lcd,8,2);
        lcdPrintf (fd_lcd, "OFF") ;
    }

    if(sw3 == 1){
        lcdPosition(fd_lcd,8,3);
        lcdPrintf (fd_lcd, "ON ") ;
    }
    else {
        lcdPosition(fd_lcd,8,3);
        lcdPrintf (fd_lcd, "OFF") ;
    }

    if(sw2 == 0){
        lcdPosition(fd_lcd,15,2);
        lcdPrintf (fd_lcd, "\xBE\xB2   ") ;                 //セイ
    }
    else {
        lcdPosition(fd_lcd,15,2);
        lcdPrintf (fd_lcd, "\xB7\xDE\xAC\xB8") ;        //ギャク
    }

    if(sw4 == 0){
        lcdPosition(fd_lcd,15,3);
        lcdPrintf (fd_lcd, "\xBE\xB2   ") ;                 //セイ
    }
    else {
        lcdPosition(fd_lcd,15,3);
        lcdPrintf (fd_lcd, "\xB7\xDE\xAC\xB8") ;        //ギャク
    }

    if(error > 0){
        lcdPosition(fd_lcd,0,0);
        lcdPrintf (fd_lcd, "\xB4\xD7\xB0No.%d            " ,error) ;
        digitalWrite(BUZZER, 1);
        delay(500);
        digitalWrite(BUZZER, 0);
    }

    return 0;
}

/*シャットダウンボタン*/
void shutdown_btn(void){
    time_now = millis();
    if(time_now-time_prev > 500){
        mot_state = MOT_OFF;
        mot_state2 = MOT_OFF;
        pthread_detach(th);
        pthread_detach(normal);
        pthread_detach(admin);
        st=1;
        shutdown = 1;
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

/*エラー解除ボタン*/
int ERROR(void){
	int buzzer = 0;

	digitalWrite(RED, 1);
	digitalWrite(YELLOW, 0);
	digitalWrite(GREEN, 0);

	while(error > 0){
         btn2=digitalRead(BUTTON2);
         btn3=digitalRead(BUTTON3);

        if(btn3 == 1){
                shutdown_btn();
        }
		if(btn2==1) {
			if(error == 1){
				if(kinsetu1  == 1) error = 0;
				else buzzer = 1;
			}else if(error == 2){
				if(kinsetu2  == 1) error = 0;
				else buzzer = 1;
			}else if(error == 3){
				if(kinsetu3  == 1) error = 0;
				else buzzer = 1;
			}else if(error == 4){
				adc01();
				if(dist.ch0>=25 && dist.ch1>=25) error = 0;
				else buzzer = 1;
			}else if(error == 5){
				adc01();
                if(dist.ch4>=25 && dist.ch5>=25) error = 0;
                else buzzer = 1;
			}else if(error == 6){
				error = 0;
			}else if(error == 7){
				error = 0;
			}else if(error == 8){
				adc02();
                if(temp.dryA<MOT_Temp)error = 0;
				else buzzer = 1;
			}else if(error == 9){
				adc02();
                if(temp.dryB<MOT_Temp)error = 0;
                else buzzer = 1;
			}else if(error == 10){
				adc02();
                if(temp.crashA<MOT_Temp)error = 0;
                else buzzer = 1;
			}else if(error == 11){
				adc02();
                if(temp.crashB<MOT_Temp)error = 0;
                else buzzer = 1;
			}else if(error == 12) error = 0;

			if(buzzer == 1){
				digitalWrite(BUZZER, 1);
				delay(500);
				digitalWrite(BUZZER, 0);
			}
			delay(100);		//チャタ対策
		}
		delay(200);
	}
	lcdPosition(fd_lcd,0,0);
	lcdPrintf (fd_lcd, "\xB4\xD7\xB0\xB6\xB2\xBC\xDE\xAE");
     LOG_PRINT("エラー解除", LOG_OK);
	return 0;
}

/*I/O設定関数*/
void IOsetting(void){

    /**********I/O設定**********/
    pinMode(BUTTON1, INPUT);
    wiringPiISR( BUTTON2, INT_EDGE_RISING, stop );         //一時停止ボタンの外部割り込み設定てきなやつ
    pinMode(BUTTON3, INPUT);
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

/*初期化処理関数*/
void set_init(void){
  btn1=0;
  btn2=0;
  btn3=0;
  sw1=0;
  sw2=0;
  sw3=0;
  sw4=0;
  shutdown=0;
  st=0;
  t1=0;
  t2=0;
  mode=1;
  kenti=0;
  error=0;
  d_teisi=0;
  d_end = 0;
  act=0;
  fd_lcd=0;
  mot_state = MOT_OFF;
  mot_state2=MOT_OFF;
  dry_sec   = 0;
  crash_sec = 0;
  sel_sen = 0;
  KOUDEN=0;
  motor1 = 0;
  motor2 = 0;
  flg_manpai=0;
  sp.dry_secA   = 0;
  sp.dry_secB   = 0;
  sp.crash_secA = 0;
  sp.crash_secB = 0;
}

/*****************************************
*                           メイン処理                                               *
*****************************************/
int main(int argc, char **argv) {

    if(param_init() == -1){
        printf("param_init Fail\n");
        exit(1);
    }

    set_init();

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

    printf("準備完了\n");
    digitalWrite(LIGHT, 1);
    digitalWrite(LED1, 1);
    digitalWrite(LED2, 0);
    lcdPosition(fd_lcd,0,1);
    lcdPrintf (fd_lcd, "       \xC3\xDE\xDD\xB9\xDE\xDD  \xD3\xB0\xC0" ) ;
    lcdPosition(fd_lcd,0,2);
    lcdPrintf (fd_lcd, "\xC0\xDE\xAF\xBD\xB2 : " ) ;
    lcdPosition(fd_lcd,0,3);
    lcdPrintf (fd_lcd, "\xB9\xDE\xDD\xD6\xB3 : " ) ;

    pthread_create(&th,NULL, (void*(*)(void*))thread_MOT, NULL);
    pthread_create(&th,NULL, (void*(*)(void*))thread_MOT2, NULL);
    pthread_create(&th,NULL, (void*(*)(void*))thread_Read, NULL);
    //pthread_create(&th,NULL, (void*(*)(void*))USB, NULL);
    lcd();

    //if(sys_format() != 0) return -1;

    st=0;
    while(1){
        switch(mode){
        case 1:
            pthread_create( &normal, NULL, (void*(*)(void*))thread_normal, NULL);   //スレッド[normal]スタート
            break;

        case 2:
            pthread_create( &admin, NULL, (void*(*)(void*))thread_admin, NULL); //スレッド[admin]スタート
            break;

        }
        delay(1000);
    }

   return 0;
}
