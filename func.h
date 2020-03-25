/* func.h
 *
 * mricom user functions
 */

/*test functions*/
void test_print(char **args);
void test_fork();
void test_randfill_buffer();
void test_generate_loop();
void test_write_data();
void test_write_data_init();
void test_system();

/* daq functions */
void daq_init_kstfile();
void daq_save_buffer();
void daq_update_window();
void daq_start_kst();
void daq_start_acq();
/*util shell functions*/
void addpid(int pid);
void process_add(int pid, char *name);
void process_remove(int pid);
/* util opaqe functions */
void parse_settings_file();
int is_kst_accessible();
int is_ramdisk_accessible();
int is_nicard_accessible();

/*util user interface funcs*/
void listp();
void killp(int procid);

/*main user interface funcs*/
void start();
void stpo();
void reset();
