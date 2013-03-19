#include "defines.h"

void timeval_reset(struct timeval *a) {
    
    a->tv_sec = 0;
    a->tv_usec = 0;
    return;
}

void timeval_update(struct timeval *a, const struct timeval b) {

    a->tv_sec = b.tv_sec;
    a->tv_usec = b.tv_usec;
    return;
}

char *removenonprintable(char *str1) {
unsigned int i;

    if (!str1)
        return NULL;
    for (i = 0; i < strlen(str1); i++) {
        if (str1[i] == 0x01 || /* ctcp */
	    str1[i] == 0x02 || /* bold */
	    str1[i] == 0x03 || /* color */
	    str1[i] == 0x09 || /* tab */
	    str1[i] == 0x0A || /* return */
	    str1[i] == 0x0D || /* return */
	    str1[i] == 0x0F || /* end formatting */
	    str1[i] == 0x16 || /* inverse */
	    str1[i] == 0x1F)  /* underline */ {
	
//	 ((((unsigned char)str1[i]) >= 0xA1)  && (((unsigned char)str1[i]) <= 0xFF))  ||
/* ifsnop */	    
	    str1[i] = '.';
	}
    }
    return str1;
}

char *stripnonprintable(char *str1) { //string has to be TEXTLENGTH_MAX
unsigned int i;
unsigned int j=0;


    if (!str1)
        return NULL;
    for (i = 0; i < strlen(str1); i++) {
        if (! (str1[i] == 0x01 || /* ctcp */
	    str1[i] == 0x02 || /* bold */
	    str1[i] == 0x03 || /* color */
	    str1[i] == 0x09 || /* tab */
	    str1[i] == 0x0A || /* return */
	    str1[i] == 0x0D || /* return */
	    str1[i] == 0x0F || /* end formatting */
	    str1[i] == 0x16 || /* inverse */
	    str1[i] == 0x1F) ) /* underline */ {
	    str1[j] = str1[i];
	    j++;
	}
    }
//    bzero(str1 + j, TEXTLENGTH_MAX - j);
    str1[j] = '\0';
    return str1;
}

char *getpart2(const char *line, int howmany, const char *src_function, const char *src_file, int src_line) {
char *part;
int i,j,k;

    context();

    i=0; j=0;
    for (k=1; k<howmany; k++) {
        while (line[i] != ' ')
            if (line[i] == '\0')
                return NULL;
            else
                i++;
        i++;
    }

    if (line[i] == '\0')
        return NULL;

    part = mycalloc2(TEXTLENGTH_MAX, src_function, src_file, src_line);
    while ((line[i] != ' ') && (line[i] != '\0')) {
        part[j] = line[i];
        i++; j++;
    }
    part[j]='\0';

    return part;
}

void module_list_add(const char *name, const char *path) {
struct module_list_t *p = NULL;

    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "add module: [%s]", path);

    p = g.module_list;
    while (p) {
        if (!strcmp(p->name, name))
            break; /* found, p will be non-null */
        p = p->next;
    }

    if (p == NULL) { /* not found or empty, add at beginning */
	p = g.module_list;
	g.module_list = (struct module_list_t *) mycalloc(sizeof(struct module_list_t));
	g.module_list->name = (char *) mycalloc(strlen(name) + 1);
	g.module_list->path = (char *) mycalloc(strlen(path) + 1);
    	strcpy(g.module_list->name, name);
        strcpy(g.module_list->path, path);
        g.module_list->next = p;
    }
    return;
}

void module_list_delete(void) {
    struct module_list_t *p = NULL;

    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "deleting module list");

    while (g.module_list) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "deleting module [%s]", g.module_list->name);
	mydelete(g.module_list->name);
	mydelete(g.module_list->path);
	p = g.module_list;
	g.module_list = g.module_list->next;
	mydelete(p);
    }

    return;
};

int timer_wait(int queue_number, int howlong) {

    context();

    if (g.timer_wait_queue[queue_number] <= T_ERROR) {
	g.timer_wait_queue[queue_number] = howlong;
	return T_ERROR;	
    } else {
	if (g.timer_wait_queue[queue_number] == T_OK) {
	    g.timer_wait_queue[queue_number] = T_ERROR;
	    return T_OK;
	}
    }
    return g.timer_wait_queue[queue_number];
}

void timer_wait_reset(void) {
int i;

    context();

    for (i=0; i<TIMER_QUEUE_SIZE; i++)
	g.timer_wait_queue[i] = T_ERROR;

    return;
}

void timer_wait_update(int secs) {
int i;

    context();

    for (i=0; i<TIMER_QUEUE_SIZE; i++)
	if (g.timer_wait_queue[i] > 0)
	    g.timer_wait_queue[i] -= secs;

    return;
}
