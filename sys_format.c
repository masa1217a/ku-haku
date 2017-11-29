/*****************************************
*               初期化処理                *
*****************************************/
int sys_format(void){
    st = 0;
    teisi = 0;
    int fstart,fend;
    // 初期化のフラグ
    int flg_1   = 0;
    int flg_2   = 0;
    int flg_3   = 0;
    int flg_4   = 0;
    int flg_5   = 0;
    int flg_6   = 0;
    int flg_7   = 0;
    int flg_8   = 0;
    int flg_9   = 0;
    int flg_end = 0;

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
        if(shuttdown ==1) break;
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
    while(!flg_end){
        while(1){

            /* 4.   脱水部投入扉が閉じているか */
            if(kinsetu1 == 0 ){
                printf("扉を閉めてください\n");
                LOG_PRINT("扉が開いている", LOG_NG);
                error=1;      //エラーNo.
                lcd();
                flg_1 = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("扉 OK", LOG_OK);
             flg_1 = 1;
            }
            //printf("flg_4 = %d\n",  flg_4);

            /* 1. 脱水部と減容部のドッキングがされているか  */
            if(kinsetu2 == 0 ){
                printf("脱水部と減容部のドッキングがされていません\n");
                LOG_PRINT("ドッキングエラー", LOG_NG);
                error=2;        // エラーNO.
                lcd();
                flg_2 = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("ドッキング OK", LOG_OK);
             flg_2 = 1;
            }
            //printf("\rflg_1 = %d\n",  flg_1);

            /* 2.   屑箱が設置されているか                */
            if(kinsetu3 == 0 ){
                printf("屑箱を設置してください\n");
                LOG_PRINT("屑箱なし", LOG_NG);
                error=3;
                lcd();
                flg_3 = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("屑箱設置中", LOG_OK);
             flg_3 = 1;
            }
            //printf("flg_2 = %d\n",  flg_2);

            /* 5.   脱水部にスポンジが残されていないか       */
            adc01();
            if(st==0&&teisi==0 && distance_adc01_ch0<25 && distance_adc01_ch1<25)
            {
                printf("エラー:投入口のスポンジの量が多いです\n");
                LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
                error=4;
                lcd();
                flg_4 = 0;
                delay(200);
                break;
            }else{
                LOG_PRINT("投入口のスポンジの量 OK", LOG_OK);
                flg_4 = 1;
            }
            //printf("flg_5 = %d\n",  flg_5);

            /* 6.   減容部にスポンジが残されていないか       */
            adc01();
            if(st==0&&teisi==0 && distance_adc01_ch2<25 && distance_adc01_ch3<25)
            {
                printf("エラー:スポンジの量が多いです\n");
                LOG_PRINT("投入口のスポンジの量が多い",LOG_NG);
                error=5;
                lcd();
                flg_5= 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("投入口のスポンジの量がちょうどいい", LOG_OK);
             flg_5 = 1;
            }
            //printf("flg_6 = %d\n",  flg_6);

            /* 3.   屑箱内にスポンジが残っていないか       */
            adc01();
            if(st==0&&teisi==0 && distance_adc01_ch4<25 && distance_adc01_ch5<25 )
            {
                printf("エラー:屑箱内のスポンジの量が多いです\n");
                LOG_PRINT("屑箱内のスポンジの量が多い",LOG_NG);
                error=6;
                lcd();
                flg_6 = 0;
                delay(200);
                break;
            }else{
             LOG_PRINT("投入口のスポンジの量 OK", LOG_OK);
             flg_6 = 1;
            }
            //printf("flg_3 = %d\n",  flg_3);

            /* 7.   モータの温度が安定動作できる範囲であるか  */
            adc02();
            printf("モーター停止中\n");
            if(temp_adc02_ch0>=MOT_Temp){
              printf("エラー:異常な温度を検知 : 脱水部A\n");
              LOG_PRINT("異常な温度を検知 : 脱水部A", LOG_NG);
              error=8;
              lcd();
            }
            if( temp_adc02_ch1>=MOT_Temp ){
              printf("エラー:異常な温度を検知 : 脱水部B\n");
              LOG_PRINT("異常な温度を検知 : 脱水部B", LOG_NG);
              error=9;
              lcd();
            }
            if( temp_adc02_ch2>=MOT_Temp ){
              printf("エラー:異常な温度を検知 : 減容部A\n");
              LOG_PRINT("異常な温度を検知 : 減容部A", LOG_NG);
              error=10;
              lcd();
            }
            if() temp_adc02_ch3>=MOT_Temp){
              printf("エラー:異常な温度を検知 : 減容部B\n");
              LOG_PRINT("異常な温度を検知 : 減容部B", LOG_NG);
              error=11;
              lcd();
            }
            if( error >= 8 && error <= 11){
              flg_7 = 0;
              delay(200);
              break;
            }else{
              LOG_PRINT("動作停止中：温度 OK", LOG_OK);
              flg_7 = 1;
            }
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

            while(1){
                if(shuttdown ==1) break;
                if( dry_sec >= 10 ){
                    mot_state = MOT_Clean;
                    mot_state2 = MOT_Clean;
                    //printf("%.3f sec\n", dry_sec);
                    while(1){
                        if(shuttdown ==1) break;
                        if(dry_sec >= 12){
                            flg_8 = 0;
                            break;
                        }else if(mot_state == MOT_OFF && mot_state2==MOT_OFF){
                            flg_8 = 1;
                            break;
                        }
                    }
                }else if(mot_state == MOT_OFF && mot_state2==MOT_OFF){
                    flg_8 =1;
                    break;
                }
            }

            if(  flg_8 == 1 )   LOG_PRINT("詰まりなし", LOG_OK);
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
                if (mot_state == MOT_OFF && mot_state2 == MOT_OFF){
                    LOG_PRINT("スポンジ無し", LOG_OK);
                    flg_9 = 1;
                    break;
                }
                delay(100);
            }
            //printf("flg_9 = %d\n\n",  flg_9);
            pthread_detach(th_ph);

            pthread_detach(th_sp);

            // 終了条件
            if(flg_1 == 1 && flg_2 == 1 && flg_3 == 1 && flg_4 == 1 &&
               flg_5 == 1 && flg_6 == 1 && flg_7 == 1 && flg_8 == 1 && flg_9 == 1 ){
                 st=1;
                flg_end = 1;
                error=0;
                delay(200);
                break;
            }
        }
        while(error != 0 ){
            if(shuttdown ==1) break;
            digitalWrite(RED,    1);
            digitalWrite(YELLOW, 0);
            digitalWrite(GREEN,  0);
            //モーターＯＦＦ
            //エラー処理
            adc01();
                 if( kinsetu1 == 1 ) error = 0;
            else if( kinsetu2 == 1 ) error = 0;
            else if( kinsetu3 == 1 ) error = 0;
            else if( distance_adc01_ch4>=25 && distance_adc01_ch5>=25 )
                error = 0;
            else if( distance_adc01_ch0>=25 && distance_adc01_ch1>=25 )
                error = 0;
            else if( distance_adc01_ch2>=25 && distance_adc01_ch3>=25 )
                error = 0;
            else if( temp_adc02_ch0>=MOT_Temp || temp_adc02_ch1>=MOT_Temp ||
                                 temp_adc02_ch2>=MOT_Temp || temp_adc02_ch3>=MOT_Temp)
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
