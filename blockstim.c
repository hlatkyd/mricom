#include "blockstim.h"

#define BLOCKSTIM_TESTING 0
#define VERBOSE_BLOCKSTIM 0
#define LOG_TTL_LEADING 1   // log the rising edge of TTL
#define LOG_SEC 1           // write log file in sec format

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
     *      1 - ?? TODO
     */

    comedi_t *dev;
    char devpath[] = "/dev/comedi0";
    //TODO use absolute paths here
    char conf[] = "blockstim.conf";     // config file, parse for defaults
    char datafile[] = "blockstim.tsv";   // tab separated data file
    char metafile[] = "blockstim.meta"; // metadata for tsv file
    FILE *fp;
    char parent_name[32];
    int is_mricom_child = 0; // set to 1 later if true
    struct header *h;
    char cmdl[64]; // command line call to process in /proc/pid/cmdline

    char design[16];
    struct blockstim_settings *bs;
    struct times *t;

    int usec_ttl1;                      // time while TTL is on in microsec
    int usec_ttl0;                      // time while TTL is off
    int duration;                       // full duration in seconds

    struct timeval start_tv, stop_tv;   // calc elapsed time
    double diff;
    struct timeval tv;                  // for TTL timing
    struct timespec nano;               // for wait with nanosleep
    nano.tv_sec = 0;                    //
    nano.tv_nsec = 1;                   //
    int usec_temp;                      // for logging
    int usec, usec_start, usec_target;  // measure, set target times

    int i,j, n;
    int ret;
    int TEST = 0;                       // indicate test stim, no saving
    
    h = malloc(sizeof(struct header));
    t = malloc(sizeof(struct times));
    gettimeofday(&start_tv,NULL);
    h->timestamp = start_tv;
    t->start = start_tv;
    strcpy(h->proc, argv[0]);
    // check if parent is mricom
    getppname(parent_name);
    if(strcmp(parent_name, "mricom") == 0){
        is_mricom_child = 1;
        //TODO local pid control here
    }
    bs = malloc(sizeof(struct blockstim_settings));

    if (argc == 3 && strcmp(argv[1], "design") == 0){
        strcpy(design,argv[2]);
        parse_bstim_conf(bs, conf, design);
        if(strncmp(argv[2], "test", 4) == 0){
            TEST = 1; // set to not save csv and log
        }
    } else {
        // default to design 1
        parse_bstim_conf(bs, conf, "default");
    }

    usec_ttl1 = bs->ttl_usecw;
    usec_ttl0 = (int)(1.0 / bs->ttl_freq * 1000000) - usec_ttl1;
    n = (int) ((bs->on_time * 1e6) / (usec_ttl1+usec_ttl0));

    // only for testing
    if(BLOCKSTIM_TESTING > 0){
        printf("subdev %d\n",bs->subdev);
        printf("chan %d\n",bs->chan);
        printf("start_delay %lf\n",bs->start_delay);
        printf("on_time %lf\n",bs->on_time);
        printf("off_time %lf\n",bs->off_time);
        printf("ttl_usecw %d\n",bs->ttl_usecw);
        printf("ttl_freq %lf\n",bs->ttl_freq);
        printf("n_blocks %d\n",bs->n_blocks);
        printf("usec ttl0 : %d\n",usec_ttl0);
        printf("n in an ON block: %d\n",n);
        //printf("press any key\n");
        //getchar();
    }
    // open data file
    fp = fopen(datafile,"w");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }
    // prepare header
    fprintf_common_header(fp,h, argv);
    append_bs_chdata(fp, bs);
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
    t->action = tv;
    diff = getsecdiff(start_tv, tv);
    //printf("setup time: %lf\n",diff);
    usec_target = usec_start + (int)(bs->start_delay * 1000000.0);
    //printf("starting...\n");
    for(j=0; j < bs->n_blocks; j++){
        // this is an ON block
        for(i=0; i < n; i++){
            // wait for usec target reach and then set TTL ON
            do {
                usec = getusecdelay(tv);
                nanosleep(&nano, NULL);
            } while(usec < usec_target);
            usec_target += usec_ttl1;
            comedi_dio_write(dev, bs->subdev, bs->chan, 1);
            // wait for usec target and then set TTL OFF
            do{
                usec = getusecdelay(tv);
                nanosleep(&nano, NULL);
            } while(usec < usec_target);
            usec_target += usec_ttl0;
            comedi_dio_write(dev, bs->subdev, bs->chan, 0);

            // log TTL time values
            usec_temp = getusecdelay(tv);
            append_bs_data(fp,i,j,usec_temp, usec_ttl1);
        }
        // OFF block, set usec target
        usec_target += (int)(bs->off_time * 1000000.0);

    }
    gettimeofday(&stop_tv, NULL);
    t->stop = stop_tv;
    diff = getsecdiff(start_tv, stop_tv);
    // close tsv data file
    fclose(fp);
    // write metadata file
    fp = fopen(metafile,"w");
    if(fp == NULL){
        printf("blockstim: cannot open %s\n",metafile);
        exit(1);
    }
    // print same header for metadata file as well
    fprintf_common_header(fp, h, argv);
    fprintf_bstim_meta(fp, h, bs, t);
    fclose(fp);
    comedi_close(dev);
}
// append column names and units in 2 rows
void append_bs_chdata(FILE *fp, struct blockstim_settings *bs){

    char cols[3][8] = {"N_TTL","TIME","N_BLOCK"};
    char units[3][8] = {"n","usec","n"};
    int i;
    fprintf(fp,"%s,%s,%s\n",cols[0],cols[1],cols[2]);
    fprintf(fp,"%s,%s,%s\n",units[0],units[1],units[2]);

}
// append TTL timing data
void append_bs_data(FILE *fp, int n, int b, int time, int usec_ttl1){

    if(LOG_TTL_LEADING == 1){
        time -= usec_ttl1;
    }
    fprintf(fp,"%d,%d,%d\n",n, time, b);

}
void fprintf_bstim_meta(FILE *fp, 
                        struct header *h,
                        struct blockstim_settings *bs, 
                        struct times *t){

    char *buf;
    buf = malloc(sizeof(char)*64);
    fprintf(fp, "\n%% BLOCKSTIM SETTINGS\n");
    fprintf(fp, "device=%s\n",bs->device);
    fprintf(fp, "subdev=%d\n",bs->subdev);
    fprintf(fp, "chan=%d\n",bs->chan);
    fprintf(fp, "start_delay=%lf\n",bs->start_delay);
    fprintf(fp, "on_time=%lf\n",bs->on_time);
    fprintf(fp, "off_time=%lf\n",bs->off_time);
    fprintf(fp, "ttl_usecw=%d\n",bs->ttl_usecw);
    fprintf(fp, "ttl_freq=%lf\n",bs->ttl_freq);
    fprintf(fp, "n_blocks=%d\n",bs->n_blocks);
    fprintf(fp, "\n%% TIMING\n");
    gethrtime(buf, t->start);
    fprintf(fp, "start=%s\n",buf);
    gethrtime(buf, t->action);
    fprintf(fp, "action=%s\n",buf);
    gethrtime(buf, t->stop);
    fprintf(fp, "stop=%s\n",buf);
}

/* Function: parse_bstim_conf
 * -------------------------------
 * Fill struct blockstim_settings from config file
 *
 * Args:
 */

int parse_bstim_conf(struct blockstim_settings *bs, char *conffile, char *n){

    #define N_PARS 8            // number of params set in conf file

    FILE *fp;
    char line[128];
    char buf[128];
    char *token;
    double temp;
    int len;
    int i = 0; int j = 0; int count = 0;
    int start = 0;

    fp = fopen(conffile, "r");
    if(fp == NULL){
        printf("\nparser_bstim_conf: could not open file %s\n",conffile);
        exit(1);
    }
    while(fgets(line, 128, fp)){
        // ignore whitespace and comments
        if(line[0] == '\n' || line[0] == '\t'
           || line[0] == '#' || line[0] == ' '){
            continue;
        }
        count++;
        //remove whitespace
        remove_spaces(line);
        //remove newline
        len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = '\0';
        /* general settings */

        token = strtok(line,"=");
        // find first line referring to design 'n'
        if(strcmp(token,"DESIGN") == 0){
            token = strtok(NULL,"=");
            if(strcmp(token,n) == 0){
                // found the line, save number
                start = count;
                continue;
            } else {
                continue;
            }
        } else if (start != 0 && count > start && count < start + N_PARS + 1){
            // read params here
            if (strcmp(token,"SUBDEV") == 0){
                token = strtok(NULL,"=");
                bs->subdev = atoi(token);
                continue;
            }
            if (strcmp(token,"CHAN") == 0){
                token = strtok(NULL,"=");
                bs->chan = atoi(token);
                continue;
            }
            if (strcmp(token,"START_DELAY") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->start_delay = temp;
                continue;
            }
            if (strcmp(token,"ON_TIME") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->on_time = temp;
                continue;
            }
            if (strcmp(token,"OFF_TIME") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf",&temp);
                bs->off_time = temp;
                continue;
            }
            if (strcmp(token,"TTL_USECW") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->ttl_usecw = temp;
                continue;
            }
            if (strcmp(token,"TTL_FREQ") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->ttl_freq = temp;
                continue;
            }
            if (strcmp(token,"N_BLOCKS") == 0){
                token = strtok(NULL,"=");
                bs->n_blocks = atoi(token);
                continue;
            }
        } else {
            continue;
        }
    }
    return 0;
}

