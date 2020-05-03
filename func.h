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

/*util user interface funcs*/
void list();
void catdata();
void listp();

/*main user interface funcs*/
void start();
void stop();
void reset();
