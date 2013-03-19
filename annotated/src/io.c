               :#include "defines.h"
               :
               :void ioutput(int dest, const char *format, ...) { /* ioutput total:      3  0.0034 */
               :
               :char tempstr[TEXTLENGTH_MAX];
               :char datestr[TEXTLENGTH_STD];
               :char tempstr2[TEXTLENGTH_MAX];
               :va_list args;
               :int fd;
               :char type;
               :
               :    va_start(args, format);
               :    vsnprintf(tempstr, TEXTLENGTH_MAX - 2, format, args);
               :    va_end(args);    
               :
               :    strftime(datestr, TEXTLENGTH_STD - 1, "%Y-%m-%d %H:%M:%S", localtime(&g.currenttime.tv_sec));
               :
               :    type = 'U';
               :/*
               :    if (dest & OUTPUT_TYPE_DEBUG_EXTRA)
               :	return; //too much
               :*/
               :    if (dest & OUTPUT_TYPE_DEBUG) {
               :	type = '#';
               :	if (!DEBUG_ENABLED) goto next;	
               :    }
               :    if (dest & OUTPUT_TYPE_DEBUG_EXTRA)
               :	type = '$';
               :    if (dest & OUTPUT_TYPE_INFO)
               :	type = ' ';
               :    if (dest & OUTPUT_TYPE_WARN)
               :	type = '+';
               :    if (dest & OUTPUT_TYPE_SQL) {
               :	type = '·';
               :	if (!DEBUG_ENABLED) goto next;	
               :    }
               :    if (dest & OUTPUT_TYPE_CRITICAL)
               :	type = '!';
               :
               :    bzero(tempstr2, TEXTLENGTH_MAX);
               :    sprintf(tempstr2,"%c %s: %s\n", type, datestr, tempstr);
               :
               :    if (dest & OUTPUT_DEST_NO)
               :	printf("%s", tempstr2);
               :    
               :    if (dest & OUTPUT_DEST_LOG) {
               :	if (g.logfile != T_UNDEF) {
     1  0.0011 :	    fd = open(g.logfile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
               :	    if (fd < 0) {
               :		ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "open log file: %s", strerror(errno));
               :		exit(EXIT_FAILURE);
               :	    }
               :	    if (lseek(fd, 0, SEEK_END) < 0)
               :		ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "couldn't lseek on file: %s", strerror(errno));
     1  0.0011 :	    if (write(fd, tempstr2, strlen(tempstr2)) != (ssize_t) strlen(tempstr2))
               :	    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "couldn't write on file: %s", strerror(errno));
     1  0.0011 :	    if (close(fd) < 0)
               :		ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "couldn't close file: %s", strerror(errno));
               :	}
               :    }
               :
               :next:
               :    if (dest & OUTPUT_TYPE_CRITICAL)
               :	g.selfstatus = SELF_STATUS_EXIT;    
               :
               :    return;
               :}
/* 
 * Total samples for file : "src/io.c"
 * 
 *      3  0.0034
 */


/* 
 * Command line: opannotate --source --output-dir=annotated/ ./rb 
 * 
 * Interpretation of command line:
 * Output annotated source file with samples
 * Output all files
 * 
 * CPU: Athlon, speed 1333 MHz (estimated)
 * Counted CPU_CLK_UNHALTED events (Cycles outside of halt state) with a unit mask of 0x00 (No unit mask) count 100000
 */
