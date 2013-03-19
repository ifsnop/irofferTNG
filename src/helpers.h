
void timeval_update(struct timeval *a, const struct timeval b);
void timeval_reset(struct timeval *a);
char *removenonprintable(char *str1);
char *stripnonprintable(char *str1);
char *getpart2(const char *line, int howmany, const char *src_function, const char *src_file, int src_line);
void module_list_add(const char *name, const char *path);
void module_list_delete(void);
int timer_wait(int queue_number, int howlong);
void timer_wait_reset(void);
void timer_wait_update(int secs);

