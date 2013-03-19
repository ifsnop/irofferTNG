#include "defines.h"

void ioutput(int dest, const char *format, ...) {

char tempstr[TEXTLENGTH_MAX];
char datestr[TEXTLENGTH_STD];
char tempstr2[TEXTLENGTH_MAX];
va_list args;
int fd;
char type;


    if (!format) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "empty msg!!!!!!");
	return;   
    }

    va_start(args, format);
    vsnprintf(tempstr, TEXTLENGTH_MAX - 2, format, args);
    va_end(args);    

    strftime(datestr, TEXTLENGTH_STD - 1, "%Y-%m-%d %H:%M:%S", localtime(&g.currenttime.tv_sec));

    type = 'U';

    if (dest & OUTPUT_TYPE_DEBUG) type = '#';
    if (dest & OUTPUT_TYPE_DEBUG_EXTRA) type = 'e';
    if (dest & OUTPUT_TYPE_INFO) type = 'i';
    if (dest & OUTPUT_TYPE_WARN) type = '!';
    if (dest & OUTPUT_TYPE_SQL) type = '$';
    if (dest & OUTPUT_TYPE_CRITICAL) type = '!';

    if (!(g.log_level & dest)) return; 

    bzero(tempstr2, TEXTLENGTH_MAX);
    sprintf(tempstr2,"%c %s: %s\n", type, datestr, tempstr);

    if (dest & OUTPUT_DEST_NO)
	printf("%s", tempstr2);
    
    if (dest & OUTPUT_DEST_LOG) {
	if (g.log_file != T_UNDEF) {
	    fd = open(g.log_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	    if (fd < 0) {
		ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "open log file: %s", strerror(errno));
		exit(EXIT_FAILURE);
	    }
	    if (lseek(fd, 0, SEEK_END) < 0)
		ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "couldn't lseek on log file: %s", strerror(errno));
	    if (write(fd, tempstr2, strlen(tempstr2)) != (ssize_t) strlen(tempstr2))
	    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "couldn't write on log file: %s", strerror(errno));
	    if (close(fd) < 0)
		ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "couldn't close log file: %s", strerror(errno));
	}
    }

#if DEBUG_ENABLED
    if (dest & OUTPUT_DEST_CORE) {
	if (g.core_file != T_UNDEF) {
	    fd = open(g.core_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	    if (fd < 0) {
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "open core file: %s", strerror(errno));
		return;
	    }
	    if (lseek(fd, 0, SEEK_END) < 0)
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "couldn't lseek on core file: %s", strerror(errno));
	    if (write(fd, tempstr2, strlen(tempstr2)) != (ssize_t) strlen(tempstr2))
	    	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "couldn't write on core file: %s", strerror(errno));
	    if (close(fd) < 0)
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "couldn't close core file: %s", strerror(errno));
	}
    }
#endif

    if (dest & OUTPUT_TYPE_CRITICAL)
	g.selfstatus = SELF_STATUS_EXIT;    

    return;
}
