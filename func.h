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
void daq_start_acq()
/*util shell functions*/
void addpid(int pid);
void process_add(int pid, char *name);
void process_remove(int pid);
void append_to_history(char *cmd);

/*util user interface funcs*/
void getproc();
void listh();
void listp();
void killp(int procid);

/*main user interface funcs*/
void start();
void stpo();
void reset();
