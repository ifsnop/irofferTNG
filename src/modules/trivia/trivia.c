#define _MOD_
#include "../modules.h"
#define MAX_CHANNELS 20

struct trivia_t {
    char *pregunta;
    char *nick_bot;
    char *canal;
    int tema_pregunta;
    int segs;
};

struct trivia_t trivia[MAX_CHANNELS];

Function *f = NULL;

int mod_load(char *name, Function * f2, struct global_t * g2) {
int i;

    f = f2;

    context();
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s loading", name);
    
    for (i=0; i<MAX_CHANNELS; i++) {
        trivia[i].pregunta = NULL;
	trivia[i].nick_bot = NULL;
	trivia[i].canal = NULL;
	trivia[i].tema_pregunta = -1;
	trivia[i].segs = 0;
    }    
    return (MODULE_HOOK_PRIVMSG | MODULE_HOOK_EVERY1SEC | MODULE_HOOK_DUMPSTATUS | MODULE_HOOK_SERVERQUIT);
}

int mod_unload(char *name, struct global_t * g2) {
int i;

    context();

    for (i=0; i<MAX_CHANNELS; i++) {
        if (trivia[i].tema_pregunta != -1) {
	    mydelete(trivia[i].pregunta);
	    mydelete(trivia[i].nick_bot);
	    mydelete(trivia[i].canal);
	    trivia[i].tema_pregunta=-1;
	    trivia[i].segs=0;
	}
    }    
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s unloading", name);
    return (T_OK);
}

int mod_privmsg(char *fullline, char *nick, char *hostname,
                char *hostmask, char *dest, char *msg1,
                char *msg2, char *msg3, char *msg4,
                char *msg5, struct global_t * g2) {
struct db_row_t row;
char *ptr_tema;
char *tema;
char *tmpbuff,*tmp;
unsigned int i,ii;
int indice_trivia = 0;

    context();

    tmpbuff = mycalloc(strlen(fullline)*2 + 1);

    for (i=0,ii=0; i<strlen(fullline); i++,ii++) {
	if ( fullline[i] == '\'' ) {
	    tmpbuff[ii] = '\\'; ii++;
	    tmpbuff[ii] = '\'';
	} else {
	    tmpbuff[ii] = tolower(fullline[i]);
	}
    }

    if (strstr(tmpbuff, "wit trivia") && strstr(tmpbuff, "pregunta") && strstr(tmpbuff, "tema")) {
	while ((trivia[indice_trivia].tema_pregunta != -1) && (indice_trivia < MAX_CHANNELS)) indice_trivia++;
	if (indice_trivia == MAX_CHANNELS) {
	    ioutput( OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "trivia.so: out of channel slots. increment MAX_CHANNELS");
	    mydelete(tmpbuff);
	    return T_OK;
	}
	if ( (ptr_tema = strstr(tmpbuff, "tema: ")) != 0 ) {
	    ptr_tema += 6; i = 0;
	    tema = mycalloc(TEXTLENGTH_STD);
	    while ( (ptr_tema[i] != '\0') && (ptr_tema[i] != '.')) i++;
	    ptr_tema[i] = '\0';
	    strncpy(tema, ptr_tema, TEXTLENGTH_STD);
	    row = db_read("SELECT id FROM rb.trivial_temas WHERE tema='%s'", tema);
	    if (row.num_fields >= 1) {
		trivia[indice_trivia].tema_pregunta = atoi(row.r[0]);
    		db_row_clean(row);
	    } else {
		db_row_clean(row);
		if (db_write("INSERT INTO rb.trivial_temas (tema) VALUES ('%s')", tema))
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "trivia.so: dupe theme (%s)", tema);
		row = db_read("SELECT id FROM rb.trivial_temas WHERE tema='%s'", tema);
		if (row.num_fields >= 1) {
		    trivia[indice_trivia].tema_pregunta = atoi(row.r[0]);
		}
		db_row_clean(row);
	    }
	    if (trivia[indice_trivia].tema_pregunta != -1) {
		trivia[indice_trivia].nick_bot = mycalloc(strlen(nick) + 1);
		strcpy(trivia[indice_trivia].nick_bot, nick);
		trivia[indice_trivia].canal = mycalloc(strlen(dest) + 1);
		strcpy(trivia[indice_trivia].canal, dest);
		trivia[indice_trivia].segs = 0;
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "trivia.so (1): %d/%d bot(%s) canal(%s) tema(%d)", indice_trivia, MAX_CHANNELS, trivia[indice_trivia].nick_bot, trivia[indice_trivia].canal, trivia[indice_trivia].tema_pregunta);
	    }
	    mydelete(tmpbuff);
	    mydelete(tema);
	    return (T_OK);
	}
    }

    for (indice_trivia = 0; indice_trivia < MAX_CHANNELS; indice_trivia++) {
	if ( (trivia[indice_trivia].tema_pregunta != -1)
	    && (!strcasecmp(trivia[indice_trivia].nick_bot, nick))
	    && (!trivia[indice_trivia].pregunta)
	    && (!strcasecmp(trivia[indice_trivia].canal,dest)) )
	    break;    
    }
    if (indice_trivia < MAX_CHANNELS) {
	int k = 0, j = 0, length = 0;
	//esto es una pregunta!
	trivia[indice_trivia].pregunta = mycalloc(TEXTLENGTH_MAX);
	strncpy(trivia[indice_trivia].pregunta, tmpbuff, TEXTLENGTH_MAX);
	trivia[indice_trivia].pregunta = stripnonprintable(trivia[indice_trivia].pregunta);
	length = strlen(trivia[indice_trivia].pregunta);
	k++; // get rid off initial ":"
	while ( (trivia[indice_trivia].pregunta[k] != ':') && (k<length) ) k++;
	k+=2; //strip also the color code
	while ( k < length ) {
	    trivia[indice_trivia].pregunta[j] = trivia[indice_trivia].pregunta[k];
	    k++; j++;
	}
	trivia[indice_trivia].pregunta[j] = '\0';

	for (i=0; i<strlen(trivia[indice_trivia].pregunta); i++) 
	    if (trivia[indice_trivia].pregunta[i] == '\'')
		trivia[indice_trivia].pregunta[i] = '"';

	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "trivia.so (2): %d/%d bot(%s) canal(%s) pregunta(%s) tema(%d)", indice_trivia, MAX_CHANNELS, trivia[indice_trivia].nick_bot, trivia[indice_trivia].canal, trivia[indice_trivia].pregunta, trivia[indice_trivia].tema_pregunta);
        mydelete(tmpbuff);
	return (T_OK);
    } 

    tmp = strstr(tmpbuff, "la respuesta era ");
    for (indice_trivia = 0; indice_trivia < MAX_CHANNELS; indice_trivia++) {
	if ( (trivia[indice_trivia].tema_pregunta != -1)
	    && (!strcasecmp(trivia[indice_trivia].nick_bot, nick))
	    && (trivia[indice_trivia].pregunta)
	    && (!strcasecmp(trivia[indice_trivia].canal,dest)) 
	    && (tmp) )
	    break;    
    }
    if ( indice_trivia < MAX_CHANNELS ) {
	int j = 0;
	unsigned long id = 0, h = 0;
	char *respuesta, *respuesta_ptr, *id_ptr;

	respuesta = mycalloc(TEXTLENGTH_MAX);
	strncpy(respuesta, tmpbuff, TEXTLENGTH_STD - 1);

	respuesta_ptr = strstr(respuesta, "la respuesta era ");
	respuesta_ptr = stripnonprintable(respuesta_ptr);
	respuesta_ptr += strlen("la respuesta era ");

	while ( (respuesta_ptr[h] != '.') && (h < strlen(respuesta_ptr)) ) h++;
    	respuesta_ptr[h] = '\0';
	id_ptr = respuesta_ptr + h + 1;

	id_ptr = strstr(id_ptr, "#");
	id_ptr++; //comienzo del id de la pregunta
	while ( id_ptr[j] != ')' ) j++;
	id_ptr[j] = '\0';
	id = atol(id_ptr);

	if (db_write("INSERT INTO rb.trivial_preguntas (id,pregunta,respuesta,tema_id) VALUES ('%d', '%s', '%s','%d')", 
	    id, trivia[indice_trivia].pregunta, respuesta_ptr, trivia[indice_trivia].tema_pregunta)) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "trivia.so error adding %d/%d bot(%s) canal(%s) id(%d) pregunta(%s) respuesta(%s) tema(%d)", 
	    indice_trivia, MAX_CHANNELS, trivia[indice_trivia].nick_bot, trivia[indice_trivia].canal, id, trivia[indice_trivia].pregunta, respuesta_ptr, trivia[indice_trivia].tema_pregunta);
	    if (db_write("UPDATE rb.trivial_preguntas SET pregunta='%s', respuesta='%s', tema_id='%d' WHERE id='%d'", 
	    trivia[indice_trivia].pregunta, respuesta_ptr, trivia[indice_trivia].tema_pregunta, id)) {
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "trivia.so error double adding %d/%d bot(%s) canal(%s) id(%d) pregunta(%s) respuesta(%s) tema(%d)", 
		indice_trivia, MAX_CHANNELS, trivia[indice_trivia].nick_bot, trivia[indice_trivia].canal, id, trivia[indice_trivia].pregunta, respuesta_ptr, trivia[indice_trivia].tema_pregunta);
	    }
	}	    

	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "trivia.so (3): %d/%d bot(%s) canal(%s) id(%d) pregunta(%s) respuesta(%s) tema(%d)", indice_trivia, MAX_CHANNELS, trivia[indice_trivia].nick_bot, trivia[indice_trivia].canal, id, trivia[indice_trivia].pregunta, respuesta_ptr, trivia[indice_trivia].tema_pregunta);

	mydelete(trivia[indice_trivia].nick_bot);
	mydelete(trivia[indice_trivia].pregunta);
	mydelete(trivia[indice_trivia].canal);
	trivia[indice_trivia].tema_pregunta = -1;
	trivia[indice_trivia].segs = 0;
	mydelete(respuesta);
	mydelete(tmpbuff);
	return (T_OK);
    }

    mydelete(tmpbuff);
    return (T_IGNORE);
}

int mod_serverquit(struct global_t * g2) {
int i;

    context();

    for (i=0; i<MAX_CHANNELS; i++) {
	trivia[i].segs = 0;
	trivia[i].tema_pregunta = -1;
	mydelete(trivia[i].pregunta);
	mydelete(trivia[i].nick_bot);
	mydelete(trivia[i].canal);
    }
    return (T_IGNORE);
}

int mod_every1sec(struct global_t * g2) {
int i;

    context();
    for (i=0; i<MAX_CHANNELS; i++) {
	if (trivia[i].tema_pregunta != -1) {
	    trivia[i].segs++;
	    if (trivia[i].segs >= 120) {
		trivia[i].segs = 0;
		mydelete(trivia[i].pregunta);
		mydelete(trivia[i].nick_bot);
		mydelete(trivia[i].canal);
		trivia[i].tema_pregunta = -1;
    		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "trivia.so: reseting variable %d/%d", i, MAX_CHANNELS);
	    }
	}
    }
    return (T_IGNORE);
}

int mod_dumpstatus(struct global_t * g2) {
int i;
    context();
    
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "trivia.so: dumping status start");
    for (i=0; i<MAX_CHANNELS; i++) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "trivia.so: %d/%d segs(%d) canal(%s) nick_bot(%s) tema(%d) pregunta(%s)",
	    i, MAX_CHANNELS, trivia[i].segs,
	    trivia[i].canal ? trivia[i].canal : "undefined",
	    trivia[i].nick_bot ? trivia[i].nick_bot : "undefined",
	    trivia[i].tema_pregunta,
	    trivia[i].pregunta ? trivia[i].pregunta : "undefined"
	);
    }
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "trivia.so: dumping status end");
    return (T_IGNORE);
}
