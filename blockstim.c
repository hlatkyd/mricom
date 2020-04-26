#include "blockstim.h"
#define BLOCKSTIM_DEBUG 1

/* This is a digital triggering solution without addititive timing error
 * and microsec resolution.
 *
 * Usage:
 * Fill blockstim.conf file with appropriate parameters for a desired block
 * design. then call "./blockstim design [x]" from command line
 */


int main(int argc, char *argv[]){
    /* block:
     *          ttl_width
     *          _________       ________          
     *          |       |       |       |        
     *          |       |       |       |
     *          |       |       |       |
     * _________|       |_______|       |________
     *
     *          [   1 / frq     ]                    
     *
     *
     * run: start_delay,(block) x n_cycles, end_delay
     *
     * ARGS:
     *      1 - ??
     */



    comedi_t *dev;
    char devpath[] = "/dev/comedi0";
    char conf[] = "blockstim.conf";   // config file, parse for defaults
    char logfile[] = "blockstim.log";
    char parent_name[32];
    int is_mricom_child = 0; // set to 1 later if true

    char design[16];
    struct blockstim_settings *bs;

    int usec_ttl1;                      // time while TTL is on in microsec
    int usec_ttl0;                      // time while TTL is off
    int duration;                       // full duration in seconds

    struct timeval tv;                  // for gettimeofday
    struct timespec nano;               // for nanosleep
    nano.tv_sec = 0;                    //
    nano.tv_nsec = 1;                   //
    int usec_temp;                      // for logging
    int usec, usec_start, usec_target;  // measure, set target times

    int i, n;
    int ret;
    
    // check if parent is mricom
    getppname(parent_name);
    if(strcmp(parent_name, "mricom") == 0){
        is_mricom_child = 1;
        //TODO local pid control here
    }
    bs = malloc(sizeof(struct blockstim_settings));

    if (argc == 3 && strcmp(argv[1], "design") == 0){
        strcpy(design,argv[2]);
        parse_blockstim_conf(bs, conf, design);
    } else {
        // default to design 1
        parse_blockstim_conf(bs, conf, "default");
    }



    usec_ttl1 = bs->ttl_usecw;
    usec_ttl0 = (int)(1.0 / bs->ttl_freq * 1000000) - usec_ttl1;
    n = (int) ((bs->on_time * 1e6) / (usec_ttl1+usec_ttl0));
    printf("n: %d\n",n);

    if(BLOCKSTIM_DEBUG > 0){
        printf("subdev %d\n",bs->subdev);
        printf("chan %d\n",bs->chan);
        printf("on_time %lf\n",bs->on_time);
        printf("off_time %lf\n",bs->off_time);
        printf("ttl_usecw %d\n",bs->ttl_usecw);
        printf("ttl_freq %lf\n",bs->ttl_freq);
        printf("n_blocks %d\n",bs->n_blocks);
        printf("press any key\n");
        printf("usec ttl0 : %d\n",usec_ttl0);
        getchar();
    }
    // wrinting out timing to file
    //TODO make header?
    FILE *fp;
    fp = fopen(logfile,"w+");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }
    // device setup
    dev = comedi_open(devpath);
    if(dev == NULL){
        comedi_perror("comedi_open");
    }
    // check if subdev is correct
    ret = comedi_get_subdevice_type(dev, bs->subdev);
    if(ret != COMEDI_SUBD_DIO){
        printf("wrong subdev");
        exit(1);
    }
    comedi_dio_config(dev, bs->subdev, bs->chan, COMEDI_OUTPUT);

    usec_start = 0;
    gettimeofday(&tv,NULL);
    usec_target = usec_start + usec_ttl1;
    printf("starting...\n");
    // this is an ON block
    for(i=0; i < n; i++){

        // TODO remove this, only for testing
        if(BLOCKSTIM_DEBUG > 0){
            
            usec_temp = getusecdelay(tv);
            testlog(fp,i,usec_temp);
            printf("%d, %d\n",i,usec_temp);
        }
        do {
            usec = getusecdelay(tv);
            nanosleep(&nano, NULL);
        } while(usec < usec_target);
        usec_target += usec_ttl1;
        comedi_dio_write(dev, bs->subdev, bs->chan, 1);
        do{
            usec = getusecdelay(tv);
            nanosleep(&nano, NULL);
        } while(usec < usec_target);
        usec_target += usec_ttl0;
        comedi_dio_write(dev, bs->subdev, bs->chan, 0);

    }
    comedi_close(dev);
}

int getusecdelay(struct timeval tv1){

    struct timeval tv2;
    int time;
    int mic;
    double mega = 1000000;
    gettimeofday(&tv2,NULL);
    time = (tv2.tv_sec - tv1.tv_sec) * mega;
    mic = (tv2.tv_usec - tv1.tv_usec);
    mic += time;
    return mic;
}

int testlog(FILE *fp, int n, int time){

    fprintf(fp,"%d,%d\n",n,time);

}
