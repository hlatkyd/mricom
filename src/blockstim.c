#include "blockstim.h"

#define BLOCKSTIM_TESTING 1
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
    struct blockstim_settings *bs;
    struct dev_settings *dvs;
    struct gen_settings *gs;
    struct mpid *mp;
    char devpath[16];
    int chan;
    int subdev;
    char mricomdir[128];    // root dir, should be in environment variables
    char conf[128] = {0};// config file containing designs and defaults
    char datafile[128] = {0};// tab separated data tsv
    char metafile[128] = {0};//metadata for tsv
    char dataf_name[] = "blockstim.tsv";
    char metaf_name[] = "blockstim.meta";
    FILE *fp;
    FILE *fp_meta;
    char parent_name[32];
    int is_mricom_child = 0; // set to 1 later if true
    struct header *h;
    char cmdl[64]; // command line call to process in /proc/pid/cmdline

    struct times *t;
    struct timeval start_tv, action_tv, stop_tv;   // calc elapsed time
    struct timeval tv;                  // for TTL timing

    int usec_ttl1;                      // time while TTL is on in microsec
    int usec_ttl0;                      // time while TTL is off

    struct timespec nano;               // for wait with nanosleep
    nano.tv_sec = 0;                    //
    nano.tv_nsec = 1;                   //
    int usec_temp;                      // for logging
    int usec, usec_start, usec_target;  // measure, set target times

    int i,j, n;
    int ret;
    int TEST = 0;                       // indicate test stim, no saving
    
    int procadd = 0;                      // 0, or 1 if proc added to local pid

    signal(SIGINT, sighandler);

    mp = malloc(sizeof(struct mpid));
    gs = malloc(sizeof(struct gen_settings));
    parse_gen_settings(gs);
    getppname(parent_name);
    fill_mpid(mp);
    ret = processctrl_add(gs->mpid_file, mp, "START");
    if(strcmp(parent_name, "mricom") == 0 || BLOCKSTIM_TESTING == 1){
        is_mricom_child = 1;
        //TODO local pid control here
        /*
        ret = processctrl_add(gs->mpid_file, mp, "START");
        if(ret == 0){
            procadd = 1;
        */
    }
    h = malloc(sizeof(struct header));
    t = malloc(sizeof(struct times));
    gettimeofday(&start_tv,NULL);
    h->timestamp = start_tv;
    t->start = start_tv;

    bs = malloc(sizeof(struct blockstim_settings));
    dvs = malloc(sizeof(struct dev_settings));
    // parse settings file
    parse_dev_settings(dvs);
    // set file paths in workdir
    strcpy(mricomdir,getenv("MRICOMDIR"));
    snprintf(conf, sizeof(conf),"%s/%sblockstim.conf",mricomdir,CONF_DIR);
    snprintf(datafile, sizeof(datafile),"%s/%sblockstim.tsv",mricomdir,DATA_DIR);
    snprintf(metafile, sizeof(metafile),"%s/%sblockstim.meta",mricomdir,DATA_DIR);

    strcpy(devpath,dvs->devpath);
    subdev = dvs->stim_trig_subdev;
    chan = dvs->stim_trig_chan;

    strcpy(h->proc, argv[0]);
    // check if parent is mricom

    if (argc == 3 && strcmp(argv[1], "design") == 0){
        //strcpy(design,argv[2]);
        parse_bstim_conf(bs, conf, argv[2]);
        if(strncmp(argv[2], "test", 4) == 0){
            TEST = 1; // set to not save csv and log
        }
    } else {
        // default to design 1
        parse_bstim_conf(bs, conf, "default");
    }

    if(VERBOSE_BLOCKSTIM == 1){
        printf_bs(bs);
    }
    usec_ttl1 = bs->ttl_usecw;
    usec_ttl0 = (int)(1.0 / bs->ttl_freq * 1000000) - usec_ttl1;
    n = (int) ((bs->on_time * 1e6) / (usec_ttl1+usec_ttl0));

    // open data file
    fp = fopen(datafile,"w");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }
    fp_meta = fopen(metafile,"w");
    if(fp_meta == NULL){
        printf("blockstim: cannot open %s\n",metafile);
        exit(1);
    }
    // prepare header
    fprintf_common_header(fp,h, argc, argv);
    append_bs_chdata(fp, bs);
    // write metadata file
    // print same header for metadata file as well
    fprintf_common_header(fp_meta, h, argc, argv);
    // device setup
    dev = comedi_open(devpath);
    if(dev == NULL){
        comedi_perror("comedi_open");
        exit(1);
    }
    // check if subdev is correct
    ret = comedi_get_subdevice_type(dev, subdev);
    if(ret != COMEDI_SUBD_DIO){
        printf("wrong subdev\n");
        exit(1);
    }
    comedi_dio_config(dev, subdev, chan, COMEDI_OUTPUT);

    usec_start = 0;
    gettimeofday(&action_tv,NULL);
    gettimeofday(&tv,NULL);
    t->action = action_tv;
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
            comedi_dio_write(dev, subdev, chan, 1);
            // wait for usec target and then set TTL OFF
            do{
                usec = getusecdelay(tv);
                nanosleep(&nano, NULL);
            } while(usec < usec_target);
            usec_target += usec_ttl0;
            comedi_dio_write(dev, subdev, chan, 0);

            // log TTL time values
            usec_temp = getusecdelay(tv);
            append_bs_data(fp,i,j,usec_temp, usec_ttl1);
        }
        // OFF block, set usec target
        usec_target += (int)(bs->off_time * 1000000.0);

    }
    gettimeofday(&stop_tv, NULL);
    t->stop = stop_tv;
    // close tsv data file
    fclose(fp);
    fprintf_bstim_meta(fp_meta, h, bs, t);
    fclose(fp_meta);
    comedi_close(dev);

    ret = processctrl_add(gs->mpid_file, mp, "STOP");
    if(ret != 0){
        printf("blockstim: processctrl_remove error");
    }
    return 0;
}
// append column names and units in 2 rows
void append_bs_chdata(FILE *fp, struct blockstim_settings *bs){

    char cols[3][8] = {"N_TTL","TIME","N_BLOCK"};
    char units[3][8] = {"n","usec","n"};
    int i;
    char *d = DELIMITER;
    fprintf(fp,"%s%s%s%s%s\n",cols[0],d,cols[1],d,cols[2]);
    fprintf(fp,"%s%s%s%s%s\n",units[0],d,units[1],d,units[2]);

}
// append TTL timing data
void append_bs_data(FILE *fp, int n, int b, int time, int usec_ttl1){

    if(LOG_TTL_LEADING == 1){
        time -= usec_ttl1;
    }
    char *d = DELIMITER;
    fprintf(fp,"%d%s%d%s%d\n",n, d, time, d, b);

}
void fprintf_bstim_meta(FILE *fp, 
                        struct header *h,
                        struct blockstim_settings *bs, 
                        struct times *t){

    char *buf;
    buf = malloc(sizeof(char)*64);
    fprintf(fp, "\n%% BLOCKSTIM SETTINGS\n");
    fprintf(fp, "device=%s\n",bs->device);
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
    free(buf);
}

void printf_bs(struct blockstim_settings *bs){

    printf("\n%% BLOCKSTIM SETTINGS\n");
    printf("device=%s\n",bs->device);
    printf("start_delay=%lf\n",bs->start_delay);
    printf("on_time=%lf\n",bs->on_time);
    printf("off_time=%lf\n",bs->off_time);
    printf("ttl_usecw=%d\n",bs->ttl_usecw);
    printf("ttl_freq=%lf\n",bs->ttl_freq);
    printf("n_blocks=%d\n",bs->n_blocks);
}
/* Function: parse_bstim_conf
 * -------------------------------
 * Fill struct blockstim_settings from config file
 *
 * Args:
 */

int parse_bstim_conf(struct blockstim_settings *bs, char *conffile, char *n){

    #define N_PARS 6            // number of params set in conf file

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
