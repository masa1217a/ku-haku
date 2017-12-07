
#include "ketugou.h"

static char LOG_FILE[100] =  "/home/pi/LOG/log.txt";        /* ログディレクトリ(通常)  */
static FILE *log_file;        /* 通常ログ */


/*************************************************
        ログ
        log_txt : ログに記載する内容
        log_status : 現状(エラーか正常か)
            0 : 正常(LOG_OK)
            1 : 異常(LOG_NG)
*************************************************/
void LOG_PRINT(char log_txt[256], int log_status )
{
        time_t timer;
        struct tm *date;
        char log_str[logN];
        char log_readline[logN] = {'\0'};
        char log_cp[2];
       // int log_mon;

        /* 時間取得 */
        timer = time(NULL);
        date = localtime(&timer);
        strftime(log_str, sizeof(log_str), "[%Y/%m/%d %H:%M:%S] ", date);

        //printf("%2d %2d\n", date->tm_mon + 1, date->tm_sec);
        /* ファイルのオープン */
            if ((log_file = fopen(LOG_FILE, "r")) == NULL) {
                fprintf(stderr, "%sのオープンに失敗しました.\n", LOG_FILE);
                exit(EXIT_FAILURE);
            }

            /* ファイルの終端まで文字を読み取り表示する */
            fgets(log_readline, logN, log_file);
                //printf("%s\n", log_readline);

            /* ファイルのクローズ */
            fclose(log_file);

        // ファイルの1行目の日付(月)を抽出
        strncpy(log_cp, log_readline+6, 2);
        // 数字に変換
       // log_mon = atof(log_cp);
        //printf("%s \n", log_cp);
        //printf("%d\n", log_mon);
        //printf("%d\n", date->tm_mon+1);

        /*
        // 毎月データを削除
        if((date->tm_mon+1) != log_mon){
                //strcat(log_file, "_%2d", date->tm_mon );
                if(remove(LOG_FILE) == 0){
                        printf("log.txtファイルを削除しました\n", LOG_FILE );
                }else{
                        printf("ファイル削除に失敗しました\n");
                }
        }*/

        // 月ごとにファイルを作る
        sprintf(LOG_FILE,"/home/pi/LOG/log_%4d_%d.txt", date->tm_year+1900, date->tm_mon+1);

        // ファイルオープン
        if ((log_file = fopen(LOG_FILE, "a")) == NULL) {
                printf("***file open error!!***\n");
                printf("%s\n", LOG_FILE);
                exit(EXIT_FAILURE);        /* エラーの場合は通常、異常終了する */
        }

        if(log_status == LOG_OK ){
                                strcat(log_str, "[complete]");
        }else{
                                strcat(log_str,  "[  error ]");
        }

        /* 文字列結合 */
        strcat(log_str,log_txt );   // 場所
        strcat(log_str,"\n");

        fputs(log_str, log_file);
        fclose(log_file);

        return;

}
