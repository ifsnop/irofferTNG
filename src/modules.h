
void module_init(void);
void module_deinit(void);
int module_locate(const char *d_name);
void module_ircinput(const char *fullline, const char *part2, const char *part3, const char *part4, const char *part5);
void module_privmsg(const char *fullline, const char *nick, const char *hostname, const char *hostmask,
                    const char *dest, const char *msg1, const char *msg2, const char *msg3,
                    const char *msg4, const char *msg5);
void module_every1sec(void);
void module_every20sec(void);
void module_serverquit(void);
void module_dumpstatus(void);
int file_isdir(char *d_name);
