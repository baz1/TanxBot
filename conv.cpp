#include <stdio.h>

void rot(int &x, int &y)
{
    int tmp = y;
    y = x;
    x = 50 - tmp;
}

void sqrot(int &x1, int &y1, int &x2, int &y2)
{
    int tmp = y1;
    y1 = x1;
    x1 = 50 - y2;
    y2 = x2;
    x2 = 50 - tmp;
}

#define TIMES(n) for (int i = n; i-- > 0;)

int main(void)
{
    FILE *inputFile = fopen("walls.txt", "r");
    FILE *outputFile = fopen("map.txt", "w");
    int x1, y1, x2, y2;
    while (true)
    {
        fscanf(inputFile, "%d", &x1);
        if (x1 < 0)
            break;
        fscanf(inputFile, "%d%d%d", &y1, &x2, &y2);
        fprintf(outputFile, "%d %d %d %d\n", x1 - 1, y1 - 1, x2 - 1, y2 - 1);
        rot(x1, y1);
        rot(x2, y2);
        fprintf(outputFile, "%d %d %d %d\n", x1 - 1, y1 - 1, x2 - 1, y2 - 1);
        rot(x1, y1);
        rot(x2, y2);
        fprintf(outputFile, "%d %d %d %d\n", x1 - 1, y1 - 1, x2 - 1, y2 - 1);
        rot(x1, y1);
        rot(x2, y2);
        fprintf(outputFile, "%d %d %d %d\n", x1 - 1, y1 - 1, x2 - 1, y2 - 1);
    }
    fprintf(outputFile, "-1\n\n\n");
    fclose(inputFile);
    inputFile = fopen("paths.txt", "r");
    for (int nrot = 0; nrot < 4; ++nrot)
    {
        fseek(inputFile, 0, SEEK_SET);
        while (true)
        {
            fscanf(inputFile, "%d", &x1);
            if (x1 < 0)
                break;
            fscanf(inputFile, "%d%d%d", &y1, &x2, &y2);
            TIMES(nrot) sqrot(x1, y1, x2, y2);
            fprintf(outputFile, "%d %d %d %d\n", x1 - 1, y1 - 1, x2 - 1, y2 - 1);
            while (true)
            {
                fscanf(inputFile, "%d", &x1);
                if (x1 < 0)
                    break;
                fscanf(inputFile, "%d%d%d", &y1, &x2, &y2);
                TIMES(nrot) sqrot(x1, y1, x2, y2);
                fprintf(outputFile, "%d %d %d %d ", x1 - 1, y1 - 1, x2 - 1, y2 - 1);
                fscanf(inputFile, "%d%d", &x1, &y1);
                TIMES(nrot) rot(x1, y1);
                fprintf(outputFile, "%d %d\n", x1 - 1, y1 - 1);
            }
            fscanf(inputFile, "%d%d", &x1, &y1);
            TIMES(nrot) rot(x1, y1);
            fprintf(outputFile, "-1 %d %d\n\n", x1 - 1, y1 - 1);
        }
    }
    fprintf(outputFile, "-1\n");
    fclose(inputFile);
    fclose(outputFile);
    return 0;
}
