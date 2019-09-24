#include <stdio.h>

float fahr_to_celsius(float fahr)
{
    return (5.0 / 9.0) * (fahr - 32.0);
    ;
}

int test_pointeur(int *p)
{
    return 0;
}

int main()
{
    int debut, fin, pas;

    debut = 0;
    fin = 300;
    pas = 20;
    printf("%p\n", &debut);
    printf("%lu\n", sizeof(fin));
    printf("Saisir le pas (défaut = 20)\n");
    scanf("%d", &pas);
    for (float far = debut; far < fin; far += pas)
    {
        printf("Fahr: %3.0f Celsius: %5.1f\n", far, fahr_to_celsius(far));
    }
    printf("vos mères");
    return 0;
}
//nb gcc uno.c && ./a.out pour exéc