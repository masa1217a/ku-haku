#include "ketugou.h"

extern int btn1, btn2, btn3,sw1,sw2,sw3,sw4,shutdown;
extern int st, t1, t2, mode,kenti,error,teisi,d_teisi,d_end,act;
extern int fd_lcd,kinsetu1,kinsetu2,kinsetu3,kinsetu4,kinsetu5,status_speed;
extern int d_power,g_power,d_state,g_state;
extern int mot_state, mot_state2;

extern int motor1;
extern int motor2;

speed_ sp;

/*****************************************
*                           スレッド処理                                          *
*****************************************/
//脱水モータースレッド
int thread_MOT(void){

    int m_start, m_end;
    double m_sec;
    // フラグ
    int flg_c=0, flg_f=0,flg_fc=0;

    digitalWrite(mot1_F,0);
    digitalWrite(mot1_R,0);

    for(;;){
        switch(mot_state){
            case MOT_OFF: //モータがOFFの場合
                digitalWrite(mot1_R,0);//mot_gyakutenが1だったら逆転動作する
                                          //                  0だったら動作を停止する
                digitalWrite(mot1_F,0);   ///mot_gyakutenが1だったら正転動作する
                break;

            case MOT_For://モーターが正転の場合
                digitalWrite(mot1_R,1);
                usleep(50000);
                digitalWrite(mot1_F,1);
                break;

            case MOT_Rev://モータが逆転の場合
                digitalWrite(mot1_F,1);
                usleep(50000);
                digitalWrite(mot1_R,0);
                break;

            case MOT_Clean:
                if(flg_c == 0){
                    m_start = millis();
                    flg_c=1;
                    printf("つまり処理　開始\n");
                }
                m_end = millis();
                m_sec = (double)(m_end - m_start) / 1000;
                if(m_sec <= 5){
                    digitalWrite(mot1_F,0);
                    digitalWrite(mot1_R,0);
                 }else if(m_sec > 5 && m_sec <= 15){
                    digitalWrite(mot1_F,1);
                    digitalWrite(mot1_R,0);
                }else if(m_sec > 15&& m_sec <= 20){
                    digitalWrite(mot1_F,0);
                    digitalWrite(mot1_R,0);
                 }else if(m_sec > 20 && m_sec <=30){
                    digitalWrite(mot1_F,1);
                    digitalWrite(mot1_R,1);
                }else{
					motor1 = 1;
                    printf("つまり処理 正常終了\n");
                    flg_c = 0;
                    delay(100);
                }

            break;

            case MOT_Format:

                if(flg_f == 0){
                    m_start = millis();
                    flg_f=1;
                    printf("初期モーター駆動\n");
                }
                m_end = millis();
                m_sec = (double)(m_end - m_start) / 1000;
                if(m_sec <= 1){
                    digitalWrite(mot1_F,0);
                    digitalWrite(mot1_R,0);
                }else if(m_sec > 1 && m_sec <= 5){
                    digitalWrite(mot1_F,1);
                    digitalWrite(mot1_R,0);
                }else if(m_sec > 5 && m_sec <= 6){
                    digitalWrite(mot1_F,0);
                    digitalWrite(mot1_R,0);
                }else if(m_sec > 6 && m_sec <= 11){
                    digitalWrite(mot1_F,1);
                    digitalWrite(mot1_R,1);
                 }else if(m_sec > 11 && m_sec <= 12){
                    digitalWrite(mot1_F,0);
                    digitalWrite(mot1_R,0);
                }else if(m_sec > 12 && m_sec <= 17 ){
                    digitalWrite(mot1_F,1);
                    digitalWrite(mot1_R,0);
                }else{
					          motor1 = 1;
                    flg_f = 0;
                    printf("モーター駆動　終了\n");
                }
            break;

            case MOT_For_check:
                if(flg_fc == 0){
                    m_start = millis();
                    flg_fc=1;
                }
                m_end = millis();
                m_sec = (double)(m_end - m_start) / 1000;

                if(m_sec < 10){
                    digitalWrite(mot2_F,1);
                    digitalWrite(mot2_R,1);
                }else{
					motor1 = 1;
                    flg_fc = 0;
                    printf("スポンジ残り検知　終了\n");
                }
            break;

			case MOT_change_For:
                digitalWrite(mot1_R,0);
                digitalWrite(mot1_F,0);
                delay(3000);
                digitalWrite(mot1_R,1);
                digitalWrite(mot1_F,1);
			break;

			case MOT_change_Rev:
                digitalWrite(mot1_R,0);
                digitalWrite(mot1_F,0);
                delay(3000);
                digitalWrite(mot1_R,0);
                digitalWrite(mot1_F,1);
			break;
			
            default:
                printf("デフォルトです\n");
                break;
        }
        delay(100);
    }
}

//減容モータースレッド
int thread_MOT2(void){

    int m_start, m_end;
    double m_sec;
    // フラグ
    int flg_c=0, flg_f=0,flg_fc;

    digitalWrite(mot2_F,0);
    digitalWrite(mot2_R,0);

    for(;;){
        switch(mot_state2){
            case MOT_OFF: //モータがOFFの場合
                digitalWrite(mot2_R,0);//mot_gyakutenが1だったら逆転動作する
                                          //                  0だったら動作を停止する
                digitalWrite(mot2_F,0);   ///mot_gyakutenが1だったら正転動作する
                break;

            case MOT_For://モーターが正転の場合
                digitalWrite(mot2_R,0);
                usleep(50000);
                digitalWrite(mot2_F,1);
                break;

            case MOT_Rev://モータが逆転の場合
                digitalWrite(mot2_F,0);
                usleep(50000);
                digitalWrite(mot2_R,1);
                break;

            case MOT_Clean:
                if(flg_c == 0){
                    m_start = millis();
                    flg_c=1;
                    printf("クリーン処理　開始\n");
                }
                m_end = millis();
                m_sec = (double)(m_end - m_start) / 1000;
                if(m_sec <= mot_clean_sec){
                    digitalWrite(mot2_F,0);
                    digitalWrite(mot2_R,1);
                }else if(m_sec > mot_clean_sec && m_sec <= mot_clean_sec * 2){
                    digitalWrite(mot2_F,1);
                    digitalWrite(mot2_R,0);
                }else if(m_sec > mot_clean_sec * 2 && m_sec <= mot_clean_sec * 3){
                    digitalWrite(mot2_F,0);
                    digitalWrite(mot2_R,1);
                }else if(m_sec > mot_clean_sec * 3 && m_sec <= 4){
                    digitalWrite(mot2_F,1);
                    digitalWrite(mot2_R,0);
                }else{
                    printf("クリーン処理 正常終了\n");
					motor2 = 1;
                    flg_c = 0;
                }

            break;

            case MOT_Format:
                if(flg_f == 0){
                    m_start = millis();
                    flg_f=1;
                    printf("初期モーター駆動\n");
                }
                m_end = millis();
                m_sec = (double)(m_end - m_start) / 1000;

                if(m_sec <= mot_format_sec){
                    digitalWrite(mot2_F,1);
                    digitalWrite(mot2_R,0);
                }else if(m_sec > mot_format_sec && m_sec <= mot_format_sec * 2){
                    digitalWrite(mot2_F,0);
                    digitalWrite(mot2_R,1);
                }else if(m_sec > mot_format_sec * 2 && m_sec <= mot_format_sec * 3){
                    digitalWrite(mot2_F,1);
                    digitalWrite(mot2_R,0);
                }else{
					motor2 = 1;
                    flg_f = 0;
                    printf("モーター駆動　正常終了\n");
                }
            break;

            case MOT_For_check:
                if(flg_fc == 0){
                    m_start = millis();
                    flg_fc=1;
                }
                m_end = millis();
                m_sec = (double)(m_end - m_start) / 1000;

                if(m_sec < 10){
                    digitalWrite(mot2_F,1);
                    digitalWrite(mot2_R,0);
                }else{
					motor2 = 1;
                    flg_fc = 0;
                    printf("スポンジ残り検知　正常終了\n");
                }
            break;

            default:
                printf("デフォルトです\n");
                break;
        }
        delay(100);
    }
}
