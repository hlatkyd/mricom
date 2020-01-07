/* func.h
 *
 * mricom user functions
 */

/*test functions*/
void test_print(char **args);
void test_fork();
void test_generate_data();
void test_write_data();
void test_write_data_init();
void test_system();
/*util shell functions*/
void addpid(int pid);
void addprocess(int pid, char *name);
void append_to_history(char *cmd);
/*util user funcs*/
void getproc();
void listh();
void listp();
void killp(int procid);
