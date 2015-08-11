#include <stdio.h>

void rot(int &x, int &y)
{
    int tmp = y;
    y = x;
    x = 50 - tmp;
}

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
    fprintf(outputFile, "-1\n");
    fclose(inputFile);
    fclose(outputFile);
    return 0;
}
