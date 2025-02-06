
#include <clib/clib.h>

#include <stdio.h>

void test1() {
    char *buf = "hello world this is a test string for split";
    char **result = NULL;
    int length = 0;
    split_str(buf, ' ', &result, &length);
    const char *expected[] = {"hello", "world", "this", "is", "a", "test", "string", "for", "split"};
    for (int i = 0; i < length; i++) {
        ASSERT(strcmp(result[i], expected[i]) == 0);
    }
    free_split_str(result, length);
}

void test2() {
    char str[] = "hello,world,this,is,a,test";
    char delimiter = ',';
    char **result = NULL;
    int length = 0;

    split_str(str, delimiter, &result, &length);

    const char *expected[] = {"hello", "world", "this", "is", "a", "test"};
    for (int i = 0; i < length; i++) {
        ASSERT(strcmp(result[i], expected[i]) == 0);
    }

    // Free the allocated memory
    free_split_str(result, length);
}

int main(int argc, char **argv) {
    
    test1();
    test2();
    return 0;
}
