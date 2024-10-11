#include "revert_string.h"
#include <stdlib.h>
#include <string.h>

void RevertString(char *str)
{
    if (str == NULL)
        return;

    int i, j;
    char temp;

    for (i = 0, j = strlen(str) - 1; i < j; i++, j--)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

