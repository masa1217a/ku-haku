#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

/*
 * スレッドパラメータ格納用
 */
typedef struct {
  char printVal;
  int interval;
} MY_THREAD_ARG;

/*
 * スレッドイニシャル関数
 */
void *myThread(void *arg)
{
  MY_THREAD_ARG *my_thread_arg =(MY_THREAD_ARG*)arg;
  int i = 0;

  // スレッドaのみ2秒sleep
  if (my_thread_arg->printVal == 'a') {
    sleep(2);
  }

  // 自スレッド識別情報の取得
  pthread_t self_thread = pthread_self();

  // デタッチ
  int status = pthread_detach(self_thread);
  if (status != 0) {
    fprintf(stderr, "failed to detatch\n");
  }

  for(i = 0; i < 5; i++) {
    fprintf(stderr,"%c", my_thread_arg->printVal);
    sleep(my_thread_arg->interval);
  }

  return arg;
}

int main(int argc,char *argv[])
{
  int status;
  void *thread_return;

  // スレッドa用のパラメータ
  pthread_t thread_a;
  MY_THREAD_ARG thread_a_arg;
  thread_a_arg.printVal = 'a';
  thread_a_arg.interval = 1;

  // スレッドb用のパラメータ
  pthread_t thread_b;
  MY_THREAD_ARG thread_b_arg;
  thread_b_arg.printVal = 'b';
  thread_b_arg.interval = 2;

  // スレッドaを生成
  status=pthread_create(&thread_a, NULL, myThread, &thread_a_arg);
  if(status!=0){
    fprintf(stderr, "\n");
    fprintf(stderr,"failed to create thread_a");
    fprintf(stderr, "\n");
    exit(1);
  }

  // スレッドbを生成
  status=pthread_create(&thread_b, NULL, myThread, &thread_b_arg);
  if(status!=0){
    fprintf(stderr, "\n");
    fprintf(stderr,"failed to create thread_a");
    fprintf(stderr, "\n");
    exit(1);
  }

  // スレッドaが終了するのを待つ.
  status = pthread_join(thread_a, &thread_return);
  if (status != 0) {
    fprintf(stderr, "\n");
    fprintf(stderr,"failed to join thread_a");
    fprintf(stderr, "\n");
    exit(1);
  } else {
    // スレッドのイニシャル関数の返り値をチェック.
    assert(&thread_a_arg == thread_return);
  }

  // スレッドbはデタッチしているので,joinに失敗する.
  status = pthread_join(thread_b, &thread_return);
  if (status != 0) {
    fprintf(stderr, "\nexpected result\n");
  } else {
    fprintf(stderr, "\n");
    fprintf(stderr,"successed to join thread_a");
    fprintf(stderr, "\n");
    exit(1);
  }

  fprintf(stderr, "\n");

  return 0;
}
