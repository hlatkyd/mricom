/*
 * test_unit.c
 *
 * Manual unittest utility for some functions
 */

#include "common.h"

int main(int argc, char **argv){

    // read_meta_times test
    // inlcudes hr2timeval
    if(argc == 2 && strcmp(argv[1],"read_meta_times")==0){
        char file_path[LPATH*2];
        struct times *t;
        struct timeval tv;
        char buf[64] = {0};
        t = malloc(sizeof(struct times));
        memset(t, 0, sizeof(struct times));
        snprintf(file_path, sizeof(file_path), "%s/test/ttlctrl.meta",
                getenv("MRICOMDIR"));

        read_meta_times(t, file_path);
        // convert again to human readable and print
        gethrtime(buf, t->start);
        printf("retrieved start: %s\n", buf);
        gethrtime(buf, t->action);
        printf("retrieved action: %s\n", buf);
        gethrtime(buf, t->stop);
        printf("retrieved stop: %s\n", buf);
        return 0;

    }
    // only hr2timeval
    else if(argc == 2 && strcmp(argv[1], "hr2timeval")==0){

        struct timeval tv;
        char timestr[] = "2020-06-16 20:13:28.974153";
        char buf[64];
        printf("init timestring: %s\n", timestr);
        hr2timeval(&tv, timestr); 

        gethrtime(buf, tv);
        printf("retrieved: %s\n", buf);
        return 0;
    }

}
