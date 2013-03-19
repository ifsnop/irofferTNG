               :#include "defines.h"
               :
               :void update_timeval(struct timeval a, struct timeval b) {
               :
               :    a.tv_sec = b.tv_sec;
               :    a.tv_usec = b.tv_usec;
               :
               :return;
               :}
               :
               :char *removenonprintable(char *str1) {
               :unsigned int i;
               :
               :    if (!str1)
               :        return NULL;
               :    for (i = 0; i < strlen(str1); i++) {
               :        if (str1[i] == 0x01 || /* ctcp */
               :	    str1[i] == 0x02 || /* bold */
               :	    str1[i] == 0x03 || /* color */
               :	    str1[i] == 0x09 || /* tab */
               :	    str1[i] == 0x0A || /* return */
               :	    str1[i] == 0x0D || /* return */
               :	    str1[i] == 0x0F || /* end formatting */
               :	    str1[i] == 0x16 || /* inverse */
               :	    str1[i] == 0x1F)  /* underline */ {
               :	
               ://	 ((((unsigned char)str1[i]) >= 0xA1)  && (((unsigned char)str1[i]) <= 0xFF))  ||
               :/* ifsnop */	    
               :	    str1[i] = '.';
               :	}
               :    }
               :    return str1;
               :}
               :
               :char *stripnonprintable(char *str1) {
               :unsigned int i;
               :unsigned int j=0;
               :
               :
               :    if (!str1)
               :        return NULL;
               :    for (i = 0; i < strlen(str1); i++) {
               :        if (! (str1[i] == 0x01 || /* ctcp */
               :	    str1[i] == 0x02 || /* bold */
               :	    str1[i] == 0x03 || /* color */
               :	    str1[i] == 0x09 || /* tab */
               :	    str1[i] == 0x0A || /* return */
               :	    str1[i] == 0x0D || /* return */
               :	    str1[i] == 0x0F || /* end formatting */
               :	    str1[i] == 0x16 || /* inverse */
               :	    str1[i] == 0x1F) ) /* underline */ {
               :	    str1[j] = str1[i];
               :	    j++;
               :	}
               :    }
               :    bzero(str1 + j, TEXTLENGTH_MAX - j);
               :    return str1;
               :}
               :
               :char *getpart2(const char *line, int howmany, const char *src_function, const char *src_file, int src_line) {
               :char *part;
               :int i,j,k;
               :
               :    context();
               :
               :    i=0; j=0;
               :    for (k=1; k<howmany; k++) {
               :        while (line[i] != ' ')
               :            if (line[i] == '\0')
               :                return NULL;
               :            else
               :                i++;
               :        i++;
               :    }
               :
               :    if (line[i] == '\0')
               :        return NULL;
               :
               :    part = mycalloc2(TEXTLENGTH_MAX, src_function, src_file, src_line);
               :    while ((line[i] != ' ') && (line[i] != '\0')) {
               :        part[j] = line[i];
               :        i++; j++;
               :    }
               :    part[j]='\0';
               :
               :    return part;
               :}
               :
               :void module_list_add(const char *name, const char *path) {
               :struct module_list_t *p = NULL;
               :
               :    context();
               :
               :    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "add module: [%s]", path);
               :
               :    p = g.module_list;
               :    while (p) {
               :        if (!strcmp(p->name, name))
               :            break; /* found, p will be non-null */
               :        p = p->next;
               :    }
               :
               :    if (p == NULL) { /* not found or empty, add at beginning */
               :	p = g.module_list;
               :	g.module_list = (struct module_list_t *) mycalloc(sizeof(struct module_list_t));
               :	g.module_list->name = (char *) mycalloc(strlen(name) + 1);
               :	g.module_list->path = (char *) mycalloc(strlen(path) + 1);
               :    	strcpy(g.module_list->name, name);
               :        strcpy(g.module_list->path, path);
               :        g.module_list->next = p;
               :    }
               :    return;
               :}
               :
     1  0.0011 :void module_list_delete() { /* module_list_delete total:      1  0.0011 */
               :    struct module_list_t *p = NULL;
               :
               :    context();
               :
               :    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "deleting module list");
               :
               :    while (g.module_list) {
               :	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "deleting module [%s]", g.module_list->name);
               :	mydelete(g.module_list->name);
               :	mydelete(g.module_list->path);
               :	p = g.module_list;
               :	g.module_list = g.module_list->next;
               :	mydelete(p);
               :    }
               :
               :    return;
               :};
               :
   549  0.6293 :int timer_wait(int queue_number, int howlong) { /* timer_wait total:   4065  4.6594 */
               :
  1202  1.3778 :    context();
               :
   176  0.2017 :    if (g.timer_wait_queue[queue_number] == T_ERROR) {
               :	g.timer_wait_queue[queue_number] = howlong;
               :	return T_ERROR;	
               :    } else {
   464  0.5318 :	if (g.timer_wait_queue[queue_number] == T_OK) {
               :	    g.timer_wait_queue[queue_number] = T_ERROR;
               :	    return T_OK;
               :	}
               :    }
   505  0.5788 :    return g.timer_wait_queue[queue_number];
  1169  1.3399 :}
               :
               :void timer_wait_reset(void) {
               :int i;
               :
               :    context();
               :
               :    for (i=0; i<TIMER_QUEUE_SIZE; i++)
               :	g.timer_wait_queue[i] = T_ERROR;
               :
               :    return;
               :}
               :
     1  0.0011 :void timer_wait_update(int secs) { /* timer_wait_update total:      1  0.0011 */
               :int i;
               :
               :    context();
               :
               :    for (i=0; i<TIMER_QUEUE_SIZE; i++)
               :	if (g.timer_wait_queue[i] > 0)
               :	    g.timer_wait_queue[i] -= secs;
               :
               :    return;
               :}
/* 
 * Total samples for file : "src/helpers.c"
 * 
 *   4067  4.6617
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
