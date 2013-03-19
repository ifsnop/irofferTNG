
void signal_alarm(int sig);
void signal_deinit(void);
void signal_init(void);
void signal_crash(int sig);
void signal_shutdown(int sig);
void signal_user1(int sig);
#if DEBUG_ENABLED
void signal_user2(int sig);
#endif
