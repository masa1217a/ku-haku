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
        printf("光電：脱水部\n");
    }else{
        wonda = PHOTO2;                       // 減容部
        kouden_num = 0;
        printf("光電：減容部\n");
    }

    pht = digitalRead(wonda);
    // printf("kouden No: %d\n",kouden_num);

    if(pht == 1)printf("物体検知まで待つ\n");
    while(pht == 1) //pht:1=受光　　pht:0=物体検知
    {
      //if( FlgKouden == 1 ) break;      // 停止ボタンで動作を止める
      if(kenti == 1) break;
      if(st == 1)break;
      pht = digitalRead(wonda);            // 光電読み込み
      delay(50);
    }
    // 停止ボタンで
    //FlgKouden = 1;
    // 値の保存 (要改善)
    vec[35].value = FlgKouden;
    if( pht==0 ) printf("光電センサが物体検知\n物体がなくなるまで待つ\n");
	kenti = 1;
    while(st == 0){
        dec_time = 0;

        pht = digitalRead(wonda);

 //       if(kouden_num == 1 && d_end == 1)break;
/*
        if(kouden_num == 1 && d_teisi == 1){
                while(d_teisi){
                        if(st == 1) break;
                        delay(100);
                }
        }
        */
        while(mot_state != MOT_For){					//脱水モーター逆転中は正転になるまで待つ	(MOT_For意外だと動作しなくなってしまうのでMOT_For以外の時に終了検知する場所あるなら修正)
			if(st == 1) break;
			delay(100);
        }
                
        if(kouden_num == 1 && mot_state == MOT_Clean){
                while(MOT_Clean){
                        if(st == 1) break;
                        delay(100);
                }
        }

        if(pht==1)
        {
            printf("カウント開始%d\n", d_end);
            time_count=0;
            while(pht == 1){
                time_count++;
                if(kouden_num == 1 && d_teisi == 1){			//貯蓄部の満杯信号が来たら停止信号来るまで待つ
                    while(d_teisi){
                        if(st == 1) break;
                        delay(100);
					}
                }

                while(mot_state != MOT_For){					//脱水モーター逆転中は正転になるまで待つ	(MOT_For意外だと動作しなくなってしまうのでMOT_For以外の時に終了検知する場所あるなら修正)
					if(st == 1) break;
					delay(100);
                }

                pht=digitalRead( wonda );

                if(dec_time >= 10)		//終了秒数　10だと10秒
                {
					/*
                    if(kouden_num   == 1){
                        dec_time = 0;
                        mot_state = MOT_OFF;
                        d_end = 1;
                        FlgKouden = 0;
                        vec[35].value = FlgKouden;
                        write_param();
                        printf("脱水終了\n");
                        return 0;
                    }else{
                        d_teisi = 0;
                        d_end = 0;
                        dec_time = 0;
                        FlgKouden = 0;
                        vec[35].value = FlgKouden;
                        write_param();
                        teisi=0;
                        mot_state  = MOT_OFF;
                        mot_state2 = MOT_OFF;
                        printf("減容終了\n");
                        return 0;
                    }
                    */
                        d_teisi = 0;
                        dec_time = 0;
                        //FlgKouden = 0;
                        //vec[35].value = FlgKouden;
                        //write_param();
                        teisi=0;
                        kenti = 0;
                        mot_state  = MOT_OFF;
                        mot_state2 = MOT_OFF;
                        st = 1;
						teisi = 0;
						d_end = 0;
                        lcdPosition(fd_lcd,0,0);
                       lcdPrintf (fd_lcd, "\xCC\xB8\xDB\xCA\xBE\xAF\xC1\xBC\xCF\xBC\xC0\xB6\x3F        ") ;        //フクロハセッチシマシタカ？ 
                        printf("終了\n");
                        return 0;
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
                //if(kouden_num   == 1 && d_end == 1 ) break;

                delay(50);
            }
        }
        delay(50);
    }
    return 0;
}
