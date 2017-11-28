/*
    速度センサスレッド
    スレッドを開始する前にsel_senという変数にピンの入力を行う
    ex)
    sel_sen = SPEED1;
    pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL);
*/
int thread_speed(void *ptr){

  int speed_count = 0; // 歯の数を数える変数
  int sp_flag = 0;     // 連続で同じ条件に入らないようにする
  int start, end ;     //

  int gpio_speed = sel_sen;
  int gear_;
  int flg_sec = 0;

  double ck_sec = 0;
  dry_sec = 0;
  crash_sec = 0;

  if(gpio_speed == SPEED1 || gpio_speed == SPEED2){
    gear_ = GEAR_DRY;
    flg_sec = 1;
  }else
    gear_ = GEAR_CRASH;

  //struct timeval s, e;
  //gettimeofday( &s, NULL);

  //int i;

  /* Start main routine */
  printf("start\n");
   start = millis();
   //printf("%d\n", start);
  for(;;) {
    if(st  == 1)break;

    if(flg_sec == 1 && d_teisi == 1){
        while(d_teisi){
            if(st == 1) break;
            delay(200);
        }
    }

    read_speed(gpio_speed);
    usleep(100);
    //printf("%d\n",status_speed);

    /*
     *  status_speedについて
     *      1 : 歯車の凸部分の検出
     *      0 : 歯車の凹部分の検出
     * 　凸凹は１セットで検出
     */
    if (status_speed == 1 && sp_flag == 0) {
          //printf("%d\n",status_speed);
          sp_flag = 1;

      }

    /*
     * ギアの歯の数分カウントしたらそこまでの 時間を算出する
     */
    if(status_speed == 0 && sp_flag == 1){
            // printf("%d\n",status_speed);
            speed_count++;
            //printf("count : %d\n\n", speed_count);
            if( (speed_count % gear_ ) == 0 ){
                end = millis();
                ck_sec = (double)(end - start) / 1000;
                //printf("end : %d\n", end);
                printf("%.3f sec\n", ck_sec);
                start = millis();
            }
            sp_flag = 0;
    }
    end = millis();
    ck_sec = (double)(end - start) / 1000;

    if(flg_sec)
        dry_sec = ck_sec;
    else
        crash_sec = ck_sec;
  }
  return 0;
}

//近接センサスレッド
int thread_kinsetu(void *ptr){

    while(1){
        kinsetu1=digitalRead(KINSETU1);
        kinsetu2=digitalRead(KINSETU2);
        kinsetu3=digitalRead(KINSETU3);

        delay(100);
    }
    return 0;
}

//USBでのLOGファイル保存スレッド
int USB(void *ptr){

        system("sudo sh /home/pi/usb_log.sh");

    return 0;
}
