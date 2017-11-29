
typedef struct{
  double dry_secA   = 0;
  double dry_secB   = 0;
  double crash_secA = 0;
  double crash_secB = 0;
}speed_;

/*
    速度センサスレッド
    スレッドを開始する前にsel_senという変数にピンの入力を行う
    ex)
    sel_sen = SPEED1;
    pthread_create( &th_sp, NULL, (void*(*)(void*))thread_speed, NULL);
*/

double dry_secA   = 0;
double dry_secB   = 0;
double crash_secA = 0;
double crash_secB = 0;

int thread_speed(void *ptr){

  int speed_count = 0;      // 歯の数を数える変数
  int ct_sp = 0;
  int sp_flag = 0;          // 連続で同じ条件に入らないようにする
  int start, end ;          // 時間計測

  int gpio_speed = sel_sen; // gpioピンの格納
  int gear_;                // 刃の枚数

  speed sp;

  // 時間    ////////////
  double ck_sec = 0;
  dry_sec = 0;
  crash_sec = 0;
  //////////////////////

  //　ギアの枚数を変更する
  switch ( gpio_speed ) {
    case SPEED1:
      gear_ = GEAR_DRY;
      printf("脱水Ａ：");
    case SPEED2:
      gear_ = GEAR_DRY;
      printf("脱水B：");
    case SPEED3:
      gear_ = GEAR_CRASH;
      printf("減容Ａ：");
    case SPEED4:
      gear_ = GEAR_CRASH;
      printf("減容Ｂ：");
  }

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
     *  凸凹は１セットで検出
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
                switch ( gpio_speed ) {
                  case SPEED1:
                    printf("脱水Ａ：");
                    sp.dry_secA = ck_sec;
                  case SPEED2:
                    printf("脱水B：");
                    sp.dry_secB = ck_sec;
                  case SPEED3:
                    printf("減容Ａ：");
                    sp.crash_secA = ck_sec;
                  case SPEED4:
                    printf("減容Ｂ：");
                    sp.crash_secB = ck_sec;
                }
                ct_sp++;
                printf("%.3f sec\n", ck_sec);
                start = millis();
            }
            sp_flag = 0;
    }
    end = millis();
    ck_sec = (double)(end - start) / 1000;

    switch ( gpio_speed ) {
      case SPEED1:
        dry_secA   = ck_sec;
      case SPEED2:
        dry_secB   = ck_sec;
      case SPEED3:
        crash_secA = ck_sec;
      case SPEED4:
        crash_secB = ck_sec;
    }
  }
  return 0;
}
