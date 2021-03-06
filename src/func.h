/* func.h
 *
 * mricom user functions
 */



/* util opaqe functions */
int is_kst_accessible(struct gen_settings *);
int is_ramdisk_accessible(struct gen_settings *);
int is_nicard_accessible(struct gen_settings *);
void listsettings(struct gen_settings *);
void listdevsettings(struct dev_settings *);
void listprocesses(struct processes *p);
void liststudy(struct gen_settings *gs);

/*util user interface funcs*/
void list();
void catdata();

/*main user interface funcs*/
void start();
void stop();
int stop_mribg(int pid);
void reset();
