               :#include "defines.h"
               :#include "modules/modules.h"
               :
               :void module_init(void) {
               :struct module_list_t *p;
               :char *error;
               :int retry=0;
               :
               :    context();
               :
               :    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "module loading");
               :
               :    global_func[1] = (Function) ioutput;
               :    global_func[2] = (Function) server_write;
               :    global_func[3] = (Function) updatecontext;
               :    global_func[4] = (Function) mycalloc2;
               :    global_func[5] = (Function) mydelete2;
               :    global_func[6] = (Function) getpart2;
               :    global_func[7] = (Function) db_read;
               :    global_func[8] = (Function) db_write;    
               :    global_func[9] = (Function) db_row_clean;
               :    global_func[10] = (Function) server_notice;    
               :    global_func[11] = (Function) server_ctcp;    
               :
               :    if (module_locate(g.module_dir))
               :    	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "problem parsing module directoy");
               :
               :    p = g.module_list;
               :    do {
               :	while (p) {
               :	    if (!p->handle) {
               :		p->handle = dlopen(p->path, RTLD_LAZY);
               :	        if (!p->handle) {
               :	    	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "dlopen(%s): %s", p->name, dlerror());
               :		    p->handle = NULL;
               :		} else {
               :		    p->mod_load = dlsym(p->handle, "mod_load");
               :		    if ((error = dlerror()) != NULL) 
               :			ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "dlsym/load(%s): %s", p->name, error);
               :		    
               :		    p->mod_unload = dlsym(p->handle, "mod_unload");
               :		    if ((error = dlerror()) != NULL) 
               :			ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "dlsym/unload(%s): %s", p->name, error);
               :		
               :		    p->hooks = ((int (*)(char *, Function *, struct global_t *)) p->mod_load) (p->name, global_func, &g);
               :		    if (p->hooks == MODULE_HOOK_RETRY) {
               :			ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "%s: RETRY", p->name);
               :			retry++;
               :		    }
               :		}
               :    	    } else {
               :		if (p->hooks & MODULE_HOOK_RETRY) {
               :		    p->hooks = ((int (*)(char *, Function *, struct global_t *)) p->mod_load) (p->name, global_func, &g);
               :		    retry--; //only support one level of recursion
               :		}
               :	    } 
               :	    p = p->next;
               :	}
               :	p = g.module_list;
               :    } while (retry);
               :
               :    p = g.module_list;
               :
               :    while (p) {
               :	if (p->hooks & MODULE_HOOK_IRCINPUT) {
               :	    p->mod_ircinput = dlsym(p->handle, "mod_ircinput");
               :	    if ((error = dlerror()) != NULL)
               :		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "dlsym/ircinput(%s): %s", p->name, error);
               :	}
               :	if (p->hooks & MODULE_HOOK_PRIVMSG) {
               :	    p->mod_privmsg = dlsym(p->handle, "mod_privmsg");
               :	    if ((error = dlerror()) != NULL)
               :		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "dlsym/privmsg(%s): %s", p->name, error);
               :	}
               :	if (p->hooks & MODULE_HOOK_EVERY20SEC) {
               :	    p->mod_every20sec = dlsym(p->handle, "mod_every20sec");
               :	    if ((error = dlerror()) != NULL)
               :		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "dlsym/every20sec(%s): %s", p->name, error);
               :	}
               :	if (p->hooks & MODULE_HOOK_SERVERQUIT) {
               :	    p->mod_serverquit = dlsym(p->handle, "mod_serverquit");
               :	    if ((error = dlerror()) != NULL)
               :		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "dlsym/serverquit(%s): %s", p->name, error);
               :	}
               :	p = p->next;
               :    }
               :
               :    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module loading finished");
               :    return;
               :}
               :
               :void module_deinit(void) {
               :struct module_list_t *p;
               :int ret;
               :
               :    context();
               :    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_TYPE_INFO, "module unloading");
               :    p = g.module_list;
               :    while (p) {
               :	ret = ((int (*) (char *, struct global_t *)) p->mod_unload) (p->name, &g);
               :	if (ret)
               :	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "module_end: problem unloading %s", p->name);
               :	if (p->handle && dlclose(p->handle))
               :	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "dlclose(%s): %s", p->name, dlerror());
               :	p = p->next;
               :    }
               :
               :    module_list_delete();
               :
               :    return;
               :}
               :
               :int module_locate(const char *d_name) {
               :DIR *handle;
               :struct dirent *handle_next;
               :char *path;
               :	    
               :    context();
               :		
               :    handle = opendir(d_name);
               :    if (!handle) {
               :	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "modules opendir: %s", strerror(errno));
               :        return (T_ERROR);
               :    }
               :				
               :    while ( (handle_next = readdir(handle)) != NULL ) {
               :        if (!(!strcmp(handle_next->d_name, "..")
               :	    || !strcmp(handle_next->d_name, "."))) {
               :	    path = mycalloc(sizeof(char) * (strlen(handle_next->d_name) + strlen(d_name) + 2));
               :	    strncpy(path, d_name, strlen(d_name));
               :	    if (path[strlen(path) - 1] != '/')
               :		strcat(path, "/");
               :	    strcat(path, handle_next->d_name);
               :	    if (!file_isdir(path)) {
               :		module_locate(path);   /* handle_next->d_name */
               :	    } else {
               :		if ((strlen(handle_next->d_name) > 3)
               :	            && (path[strlen(path) - 2] == 's')
               :		    && (path[strlen(path) - 1] == 'o')) {
               :		    module_list_add(handle_next->d_name, path);
               :		}
               :	    }	    
               :	    mydelete(path);
               :	}
               :    }
               :    if (closedir(handle)) {
               :	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "closedir %s", strerror(errno));
               :        return (T_ERROR);
               :    }
               :    return (T_OK);
               :}
               :
               :int file_isdir(char *d_name) {
               :struct stat estado;
               :int res = T_ERROR;
               :char tmpbuff[TEXTLENGTH_STD];
               :
               :    context();
               :		
               :    if (stat(d_name, &estado) >= T_UNDEF) {
               :        if (S_ISDIR(estado.st_mode)) {
               :            if (readlink(d_name, tmpbuff, sizeof(tmpbuff)) == T_ERROR) { // seguro que no es un link
               :                res = T_OK;						 // lo seguimos
               :            }
               :        }
               :    } else {
               :    	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "file_isdir %s", strerror(errno));    
               :    }
               :    return (res);
               :}
               :
               :void module_serverquit(void) {
               :struct module_list_t *p = NULL;
               :    
               :    context();
               :    p = g.module_list;
               :	    
               :    while (p) {
               :        if (p->hooks & MODULE_HOOK_SERVERQUIT) {
               :            if ( ((int (*)(struct global_t *)) 
               :			    p->mod_serverquit) (&g) == T_OK  ) {
               :		break;
               :	    }
               :        }
               :        p = p->next;
               :    }
               :    return;
               :}
               :
               :void module_ircinput(const char *fullline, const char *part2, const char *part3, const char *part4, const char *part5) { /* module_ircinput total:      1  0.0011 */
               :struct module_list_t *p = NULL;
               :    
     1  0.0011 :    context();
               :    p = g.module_list;
               :	    
               :    while (p) {
               :        if (p->hooks & MODULE_HOOK_IRCINPUT) {
               :            if ( ((int (*)(const char *, const char *, const char *, 
               :			    const char *, const char *, struct global_t *)) 
               :			    p->mod_ircinput) (fullline, part2, part3, part4, part5, &g) == T_OK  ) {
               ://		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "irc_input break");
               :		break;
               :	    }
               :        }
               :        p = p->next;
               :    }
               :    return;
               :}
               :
               :void module_privmsg(const char *fullline, const char *nick, const char *hostname, const char *hostmask,
               :		    const char *dest, const char *msg1, const char *msg2, const char *msg3,
               :		    const char *msg4, const char *msg5) {
               :struct module_list_t *p = NULL;
               :    
               :    context();
               :    p = g.module_list;
               :	    
               :    while (p) {
               :        if (p->hooks & MODULE_HOOK_PRIVMSG) {
               :            if ( ((int (*)(const char *, const char *, const char *,
               :			    const char *, const char *, const char *,
               :			    const char *, const char *, const char *,
               :			    const char *, struct global_t *)) 
               :			    p->mod_privmsg) (fullline, nick, hostname, hostmask, 
               :					    dest, msg1, msg2,
               :					    msg3, msg4, msg5, &g) == T_OK  ) {
               ://		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "irc_input break");
               :		break;
               :	    }
               :        }
               :        p = p->next;
               :    }
               :    return;
               :}
               :
               :void module_every20sec(void) {
               :struct module_list_t *p = NULL;
               :    
               :    context();
               :    p = g.module_list;
               :	    
               :    while (p) {
               :        if (p->hooks & MODULE_HOOK_EVERY20SEC) {
               :	    if ( (( int (*) (struct global_t *)) 
               :		p->mod_every20sec) (&g) == T_OK ) {
               ://		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "irc_every20sec break");
               :		break;
               :	    }
               :	}
               :	p = p->next;
               :    }
               :    
               :    return;
               :}
/* 
 * Total samples for file : "src/modules.c"
 * 
 *      1  0.0011
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
