/* func.h
 *
 * mricom user functions
 */

/*test functions*/
void test_print(char **args);
void test_fork();
void test_rand_membuf(double time);
void test_daq_simulate();
void test_write_data();
void test_write_data_init();
void test_system();

/* daq functions */
void daq_timer_start();
void daq_timer_stop();
double daq_timer_elapsed();

void daq_init_kstfile();
void daq_append_membuf();
void daq_update_kstfile();
void daq_update_window();
void daq_launch_kst();
void daq_start_acq();

/*util shell functions*/
void addpid(int pid);
void process_add(int pid, char *name);
void process_remove(int pid);

/* util opaqe functions */
int is_kst_accessible();
int is_ramdisk_accessible();
int is_nicard_accessible();
int fprintf_header(FILE *);
//TODO remove zombie procids, etc
void procmonitor();

/*util user interface funcs*/
void listsettings();
void listdevsettings();
void catdata();
void listp();
void killp(int procid);

/*main user interface funcs*/
void start();
void stop();
void reset();
