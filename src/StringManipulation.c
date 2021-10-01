#include "StringManipulation.h"
#include <stdlib.h> // malloc
#include <string.h>


char *AddPrefix(const char *prefix, const char *str) {
    int str_len = strlen(str);
    int prefix_len = strlen(prefix);
    int result_len = str_len + prefix_len;
    char *result = (char *) malloc(result_len * sizeof(char));
    strncpy(result, prefix, prefix_len);
    strncpy(result + prefix_len, str, str_len);
    return result;
}

char *AddPostfix(const char *str, const char *postfix) {
    return AddPrefix(str, postfix);
}
