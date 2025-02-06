
#include <clib/clib.h>

#include <stdio.h>
int main(int argc, char **argv) {
    
    char *path = argv[1];
    if (!path_exist(path)) {
        ERR("path %s not exist", path);
        return 0;
    }
    del_dir(path);
    return 0;
}
