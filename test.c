#include <stdio.h>

int main(void)
{
    char buf[256], name[256];
    int hour, min, hour2, min2;
    FILE *fp;

    fp = fopen("settings.ini", "r");
    if (!fp) return 1;
    if (!fgets(buf, sizeof buf, fp)) return 1;
    if (sscanf(buf, "%d,%d,%d,%d,%s", &hour, &min, &hour2, &min2, name) != 5)
        return 1;
    printf("%02d:%02d %02d:%02d [%s]\n", hour, min, hour2, min2, name);
    fclose(fp);
 /*
    fp = fopen("Data.txt", "r");
    if (!fp) return 1;
    if (!fgets(buf, sizeof buf, fp)) return 1;
    if (sscanf(buf, "%d%d%s", &hour, &min, name) != 3) return 1;
    printf("%d %d [%s]\n", hour, min, name);
    fclose(fp);
 */
    return 0;
}
