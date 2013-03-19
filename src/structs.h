struct db_row_t {
    int num_fields;    
    char **r;
};

struct module_list_t {
    char *name;
    char *path;
    struct module_list_t *next;
    void *handle;
    int hooks;
    int (*mod_load);
    int (*mod_unload);
    void (*mod_everyloop);
    void (*mod_every1sec);
    void (*mod_every20sec);
    void (*mod_query);
    void (*mod_privmsg);
    void (*mod_ircinput);
    void (*mod_serverquit);
    void (*mod_dumpstatus);
};

struct meminfo_t {
    void *ptr;
    const char *src_func;
    const char *src_file;
    int src_line;
    struct timeval alloctime;
    short size;
};

struct context_t {
    const char *file;
    const char *func;
    int line;
    struct timeval tv;
};

struct send_queue_t {
    char *line;
    struct send_queue_t *next;
};

struct global_t {
//internal
    int selfstatus;
    fd_set readset;
    fd_set writeset;
    int serverstatus;
    int serversocket;
    int servertimereconnect;
    struct timeval startuptime;
    struct timeval currenttime;
    struct timeval lastservercontact;
    struct module_list_t *module_list;
    int mysql_connected;
    MYSQL * mysql_dbh;
    pthread_mutex_t mutex_db;

    unsigned long ourip;
    char ourip_text[16];

    struct send_queue_t *send_queue_ptr;
    int send_queue_count;

    int linecut;
    char *server_input_line;
    
    int timer_wait_queue[TIMER_QUEUE_SIZE];
    
//by config item
    int usenatip;
    char *irc_host;
    char *irc_port;
    char **irc_channel;
    char *irc_channel_joined;
    unsigned int irc_channel_count;
    char *conf_file; //[TEXTLENGTH_STD];
    char *log_file; //[TEXTLENGTH_STD];
    long log_level;
    char *pid_file; //[TEXTLENGTH_STD];
#if DEBUG_ENABLED
    char *core_file; //[TEXTLENGTH_STD];
#endif
    char *user_login; //[TEXTLENGTH_STD];
    char *user_realname; //[TEXTLENGTH_STD];
    char *user_nick; //[TEXTLENGTH_STD];
    char *user_modes; //[TEXTLENGTH_STD];
    bool user_retry_nick;
    char *module_dir; //[TEXTLENGTH_STD];
    char *mysql_host; //[TEXTLENGTH_STD];
    char *mysql_user; //[TEXTLENGTH_STD];
    char *mysql_pass; //[TEXTLENGTH_STD];
    char *mysql_db; //[TEXTLENGTH_STD];
    char *mysql_table_irc_users; //[TEXTLENGTH_STD];
    char *pserve_version; //[TEXTLEGNTH_STD];
    char *pserve_trigger; //[TEXTLEGNTH_STD];
    char *pserve_speed; //[TEXTLEGNTH_STD];

//debug profiler
    struct context_t context_log[DEBUG_MAX_CONTEXTS];
    int context_cur_ptr;
    struct meminfo_t *meminfo;
    int meminfo_count;
    int meminfo_depth;
};
		
#ifdef _MAIN_
struct global_t g;
#else
extern struct global_t g;
#endif
