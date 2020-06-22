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
        printf("init tstr: %s\n", timestr);
        hr2timeval(&tv, timestr); 

        gethrtime(buf, tv);
        printf("retrieved: %s\n", buf);
        return 0;
    }
    // extract_analogdaq
    else if(argc == 2 && strcmp(argv[1],"extract_analogdaq")==0){

        struct gen_settings *gs;
        gs = malloc(sizeof(struct gen_settings));
        memset(gs, 0, sizeof(struct gen_settings));
        parse_gen_settings(gs);

        long int n_lines;
        char id[] = "s_2020060104";
        char seq[] = "epip_whisker_0";
        char studydir[LPATH*2] = {0};
        char seqdir[LPATH*2] = {0};
        char datadir[LPATH*2] = {0};
        // names for analog data slicing
        char adaqtsvrel[] = "analogdaq.tsv";
        char adaqmetarel[] = "analogdaq.meta";
        char ttlctrlmrel[] = "ttlctrl.meta";
        char phystsvrel[] = "phys.tsv";
        // for slicing analog acquisition data
        char ttlctrlm[LPATH*2] = {0};
        char adaqtsv[LPATH*2] = {0};
        char adaqmeta[LPATH*2] = {0};
        char phystsv[LPATH*2] = {0};

        snprintf(studydir, sizeof(studydir),"%s/%s",gs->studies_dir, id);
        snprintf(seqdir, sizeof(seqdir),"%s/%s/%s",gs->studies_dir, id, seq);
        snprintf(adaqtsv, sizeof(adaqtsv),"%s/%s",studydir, adaqtsvrel);
        snprintf(adaqmeta, sizeof(adaqmeta),"%s/%s",studydir, adaqmetarel);
        snprintf(ttlctrlm, sizeof(ttlctrlm),"%s/%s",seqdir, ttlctrlmrel);
        snprintf(phystsv, sizeof(phystsv),"%s/%s",seqdir, phystsvrel);
        // ACTION!
        extract_analogdaq(adaqtsv, adaqmeta, ttlctrlm, phystsv);
        n_lines = count_lines(adaqtsv);
        printf("n_lines = %ld\n",n_lines);
    }
    // getsecdiff
    else if(argc == 2 && strcmp(argv[1],"getsecdiff")){

        double diff;
        struct timeval tv1, tv2;
        printf("test with delays sleep and usleep\n");
        gettimeofday(&tv1, NULL);
        sleep(1);
        usleep(156702);
        gettimeofday(&tv2, NULL);
        diff = getsecdiff(tv1, tv2);
        printf("diff = %lf\n",diff);

        //
        printf("test with timestrings and hr2timeval\n");
        char timestr1[] = "2020-06-16 20:13:28.974153";
        char timestr2[] = "2020-06-16 20:15:45.923633";

        hr2timeval(&tv1, timestr1);
        hr2timeval(&tv2, timestr2);
        printf("times:\n%s\n%s\n",timestr1, timestr2);
        diff = getsecdiff(tv1, tv2);
        printf("calc diff = %lf\n",diff);

        printf("converting back to human radable\n");
        char buf[64] = {0};
        gethrtime(buf, tv1);
        printf("%s\n", buf);
        gethrtime(buf, tv2);
        printf("%s\n", buf);
    }

}
