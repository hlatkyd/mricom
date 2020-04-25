#include "blockstim.h"

/* This is a digital triggering solution without addititive timing error
 * and microsec resolution.
 *
 *
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
    comedi_insn *insn[3];
    char conf[] = "blockstim.conf";   // config file, parse for defaults
    char logfile[] = "blockstim.log";
    char parent_name[32];

    unsigned int subdev = 2;            // digital io subdevice on 6035E: 
    unsigned int chan = 0;              // digital output channel
    int usec_ttl1;                      // time while TTL is on in microsec
    int usec_ttl0;                      // time while TTL is off
    int duration;                       // full duration in seconds
    int n;                              // number of cycles

    struct timeval tv;                  // for gettimeofday
    struct timespec nano;               // for nanosleep
    nano.tv_sec = 0;                    //
    nano.tv_nsec = 1;                   //
    int usec_temp;
    //TODO check parent id, if mricom write local pid
    getppname(parent_name);

    // wrinting out timing to file
    //TODO make deader?
    FILE *fp;
    fp = fopen(logfile,"w+");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }
    //

    int i;
    int ret;
    int usec, usec_start, usec_target;

    duration = 200;
    usec_ttl1 = 30; //30
    usec_ttl0 = 9970; //9970
    n = (int) ((duration * 1e6) / (usec_ttl1+usec_ttl0));
    printf("n: %d\n",n);


    dev = comedi_open("/dev/comedi0");
    if(dev == NULL){
        comedi_perror("comedi_open");
    }
    // check if subdev is correct
    ret = comedi_get_subdevice_type(dev, subdev);
    if(ret != COMEDI_SUBD_DIO){
        printf("wrong subdev");
        exit(1);
    }
    comedi_dio_config(dev, subdev, chan, COMEDI_OUTPUT);

    usec_start = 0;
    gettimeofday(&tv,NULL);
    usec_target = usec_start + usec_ttl1;
    printf("starting...\n");
    for(i=0; i < n; i++){

        // TODO remove this, only for testing
        if(TESTLOG == 1){
            
            usec_temp = getusecdelay(tv);
            testlog(fp,i,usec_temp);
            printf("%d, %d\n",i,usec_temp);
        }
        do {
            usec = getusecdelay(tv);
            nanosleep(&nano, NULL);
        } while(usec < usec_target);
        usec_target += usec_ttl1;
        comedi_dio_write(dev, subdev, chan, 1);
        do{
            usec = getusecdelay(tv);
            nanosleep(&nano, NULL);
        } while(usec < usec_target);
        usec_target += usec_ttl0;
        comedi_dio_write(dev, subdev, chan, 0);

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
