/* get_param.c */
#include <stdio.h>
#include <string.h>

#define STR_MAX 256
#define CONFIG_FILE "settings.txt"

void usage(void)
{
    printf("usage: ./get_param [parameter_name]\n");
}

char main(int argc, char *argv[])
{
    int i = 0, j = 0;
    char str[STR_MAX], param[STR_MAX];
    FILE *fin;

    if (argc < 2) {
        usage();
        return -2; /* operation miss */
    }

    if ((fin = fopen(CONFIG_FILE, "r")) == NULL) {
        printf("fin error:[%s]\n", CONFIG_FILE);
        return -1; /* system error */
    }

    for(;;) {
        if (fgets(str, STR_MAX, fin) == NULL) {
            /* EOF */
            fclose(fin);
            return -3; /* not found keyword */
        }
        if (!strncmp(str, argv[1], strlen(argv[1]))) {
            while (str[i++] != '=') {
                ;
            }
            while (str[i] != ' ') {
                param[j++] = str[i++];
            }
            param[j] = '\0';
            printf("param : %s\n", param);
            fclose(fin);
            return 0;
        }
    }
    fclose(fin);
    return -1; /* not reachable */
}
