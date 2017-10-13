/* get_param.c */
#include <stdio.h>
#include <string.h>

#define STR_MAX 256
#define CONFIG_FILE "settings.txt"

char read_param(char *param_name)
{
    int i = 0, j = 0;
    char str[STR_MAX], param[STR_MAX];
    FILE *fin;

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
        if (!strncmp(str, param_name, strlen(param_name))) {
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

int main()
{
  if(read_param("LOG_FILE") != 0) return -1;

  return 0;
}
