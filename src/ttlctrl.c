/*
 * ttlctrl.c
 *
 * Mricom subprogram to control timing between stimulus and sequence start.
 * ttlctrl waits for digital input directly from console. If the input is
 * correct ttlctrl sends TTL signal to both console (sequence waiting on 
 * xgate for example) and the blockstim subprogram.  
 *
 * Input is a single integer between 0-7, which is challenged against the
 * 3 bit input coming from the console.
 * 
 */


#include "ttlctrl.h"

#define MPIDLOG 0 // save instance in mpid.log
#define LOGGING 1 // save TTl actions with timestamps to a logfile
#define TESTING 0

int main(int argc, char **argv){

    struct gen_settings *gs;
    struct dev_settings *ds;
    struct mpid *mp;

    comedi_t *dev;
    char mricomdir[LPATH];
    int subdev, chan;
    int inchan[N_USER_BITS];
    int i, ret;
    int waitbits; // console start confir signal int between 0-2**N_USER_BITS
    int ttl_w = 30; // width of TTL signal

    // check env
    if(getenv("MRICOMDIR") == NULL){
        fprintf(stderr, "ttlctrl: no MRICOMDIR in environment\n");
        exit(1);
    }
    if(argc == 2){
        waitbits = atoi(argv[1]);
        if(waitbits < 0 || waitbits > (int)pow(2,N_USER_BITS)){
            fprintf(stderr,"ttlctrl: incorrect input\n");
            exit(1);
        }
    } else {
        // allow wrong aruments only for testing purposes
        if(TESTING != 1){
            fprintf(stderr,"ttlctrl: incorrect input\n");
            exit(1);
        }
    }

    strcpy(mricomdir, getenv("MRICOMDIR"));

    gs = malloc(sizeof(struct gen_settings));
    ds = malloc(sizeof(struct dev_settings));

    parse_gen_settings(gs);
    parse_dev_settings(ds);

    subdev = ds->ttlctrl_subdev;
    chan = ds->ttlctrl_out_chan; 

    for(i=0;i<N_USER_BITS;i++){
        inchan[i] = ds->ttlctrl_usr_chan[i];
    }

    // channel setup
    dev = comedi_open(ds->devpath);
    if(dev == NULL){
        comedi_perror("comedi_open");
        exit(1);
    }

    comedi_dio_config(dev, subdev, chan, COMEDI_OUTPUT);
    comedi_dio_write(dev, subdev, chan , 0);

    // in testing case only ask for user getchar
    if(TESTING == 1){
        printf("TESTING=1; press ENTER\n");
        getchar();
        comedi_dio_write(dev, subdev, chan , 1);
        usleep(ttl_w);
        comedi_dio_write(dev, subdev, chan , 0);
    } else {
        // wait for confirmation bits
        for(i=0;i<N_USER_BITS;i++){
            comedi_dio_config(dev, subdev, inchan[i],COMEDI_INPUT);
        }
        ret = wait_user_bits(dev, subdev, inchan, waitbits);
        if(ret == 0){
            comedi_dio_write(dev, subdev, chan , 1);
            usleep(ttl_w);
            comedi_dio_write(dev, subdev, chan , 0);

        } else if(ret == -1){
            fprintf(stderr, "ttlctrl: console timeout error\n");
        } else {
            //TODO
            fprintf(stderr, "unknown err\n");
            exit(1);
        }
    }
    
    // wait input from console
    
    // trig

    free(gs);
    free(ds);
    return 0;
}

/*
 * Function: wait_user_bits
 * ------------------------
 *  Check input from console, 
 */
#define TIMEOUT_SEC 30
int wait_user_bits(comedi_t *dev, int subdev, int chan[N_USER_BITS], int num){

    int i;
    int bit[N_USER_BITS];
    int result;
    struct timeval tv1, tv2;
    gettimeofday(&tv1,NULL);
    while(1){
        result = 0;
        gettimeofday(&tv2,NULL);
        for(i=0;i<N_USER_BITS;i++){
            comedi_dio_read(dev, subdev, chan[i], &bit[i]);
            result += (int)pow(2,i) * bit[i];
        }
        if(result == num){
            break;
        }
        if((tv2.tv_sec - tv1.tv_sec) > TIMEOUT_SEC){
            return -1;
        }
        usleep(5);

    }
    return 0;
}
