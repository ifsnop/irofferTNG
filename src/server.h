
int server_connect(void);
void server_send(void);
void server_read(void);
void server_write(char *msg, int send_type);
void server_parseline(char *line);
void server_notice(const char *nick, const char *format, ...);
void server_ctcp(const char *nick, const char *format, ...);
void privmsg_parse(char *line);
