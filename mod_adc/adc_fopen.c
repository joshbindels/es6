#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char** argv)
{
    pid_t f = fork();

    printf("Running<%d>\n", f);

    char msg0[128];
    char msg1[128];
    char msg2[128];
    FILE* fp0 = fopen("/dev/adc0", "r");
    FILE* fp1 = fopen("/dev/adc1", "r");
    FILE* fp2 = fopen("/dev/adc2", "r");

    int i = 0;
    for(i=0; i<100; i++)
    {
        fread(msg0, 1, 128, fp0);
        usleep(1);
        fread(msg1, 1, 128, fp1);
        usleep(1);
        fread(msg2, 1, 128, fp2);
        usleep(1);
        printf("PID:%d VALS: %s, %s, %s\n", f,msg0, msg1, msg2);
    }

    fclose(fp0);
    fclose(fp1);
    fclose(fp2);
    printf("Done<%d>\n", f);


    return 0;
}
