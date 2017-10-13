
#include <stdio.h>
#include <stdlib.h>    /* exit(  )で必要 */

void main(void);

void main(void)
{
    FILE *fp;
    int c;

        /* ファイルを開くのに失敗したら */
        /* プログラムを終了して、シェルに戻る */
    if ((fp = fopen( "settings.txt", "r")) == NULL) {
        fprintf(stderr, "Can't Open File\n");
        exit(2);
    }

                        /* ファイルの終わりに達するまで */
    while ((c = fgetc(fp)) != EOF)    /* 一文字読み込み */
        fputc(c, stdout);             /* 画面に表示 */

    fclose(fp);                       /* ファイルを閉じる */
}
