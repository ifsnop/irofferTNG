
/*
 * Copyright (c) Abraham vd Merwe <abz@blio.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "scan.h"

/* initialize scanner */
int scan_open (scan_t *s,const char *filename)
{
   if ((s->fd = open (filename,O_RDONLY)) < 0)
	 {
		return (-1);
	 }

   s->c = 0;
   s->line = 1;

   return (0);
}

/* clean up */
void scan_close (scan_t *s)
{
   close (s->fd);
}

#ifdef DEBUG
#define PRINT_DEBUG(s,c) print_debug(s,c)
static void print_debug (scan_t *sc,char c)
{
   char s[10];
   s[0] = c;
   s[1] = '\0';
   if (c == ' ')
	 strcpy (s,"<SPC>");
   else if (c == '\t')
	 strcpy (s,"<TAB>");
   else if (c == '\n')
	 strcpy (s,"<NL>");
   else if (!c)
	 strcpy (s,"<EOF>");
}
#else	/* #ifdef DEBUG */
#define PRINT_DEBUG(s,c)
#endif	/* #ifdef DEBUG */

#define READ0()																						\
	{																								\
	   ssize_t result;																				\
	   if (s->c)																					\
		 c = s->c, s->c = 0;																		\
	   else if ((result = read (s->fd,&c,1)) <= 0)													\
		 {																							\
			return (result);																		\
		 }																							\
	   else PRINT_DEBUG (s,c);																		\
	}

#define READ1(x)																					\
	{																								\
	   ssize_t result;																				\
	   if (s->c)																					\
		 c = s->c, s->c = 0;																		\
	   else if ((result = read (s->fd,&c,1)) <= 0)													\
		 {																							\
			if (x) free (x);																	\
			return (-1);																			\
		 }																							\
	   else PRINT_DEBUG (s,c);																		\
	}

/* initial guess for the size of a string */
#define SIZE	64

static int scan_keyword (scan_t *s)
{
   char c;
   int n = 0,len = SIZE;

   if ((s->token.keyword = (char *) malloc (len * sizeof (char))) == NULL)
	 goto out_of_memory;

   for (;;)
	 {
		READ1(s->token.keyword);

		if (isalnum (c) || c == '_')
		  {
			 if (n >= len)
			   {
				  char *tmp;
				  len += SIZE;
				  if ((tmp = (char *) realloc (s->token.keyword,len * sizeof (char))) == NULL)
					goto out_of_memory;
				  s->token.keyword = tmp;
			   }
			 s->token.keyword[n++] = c;
		  }
		else if (c == '=' || c == ' ' || c == '\n' || c == '\t' || c == ',' || c == '}')
		  {
			 char *tmp;
			 s->token.keyword[n++] = '\0';
			 if ((tmp = (char *) realloc (s->token.keyword,n * sizeof (char))) == NULL)
			   goto out_of_memory;
			 s->token.keyword = tmp;

			 s->c = c;
			 if (!strcmp (s->token.keyword,"true"))
			   {
				  free (s->token.keyword);
				  s->token.boolean = 1;
				  return (TOK_BOOLEAN);
			   }
			 else if (!strcmp (s->token.keyword,"false"))
			   {
				  free (s->token.keyword);
				  s->token.boolean = 0;
				  return (TOK_BOOLEAN);
			   }
			 else return (TOK_KEYWORD);
		  }
		else
		  {
			 free (s->token.keyword);
			 return (-1);
		  }
	 }

out_of_memory:
   if (s->token.keyword != NULL) free (s->token.keyword);
   return (-1);
}

static int scan_integer (scan_t *s)
{
   char c,buf[256];
   int n = 0;

   for (;;)
	 {
		READ1(NULL);

		if (isdigit (c) ||
			(!n && c == '-') ||
			(n == 1 && buf[0] == '0' && c == 'x') ||
			(n == 2 && buf[0] == '-' && buf[1] == '0' && c == 'x'))
		  {
			 if (n >= 255)
			   {
				  return (-1);
			   }
			 buf[n++] = c;
		  }
		else if (c == ' ' || c == '\n' || c == '\t' || c == ',' || c == '}')
		  {
			 char *endptr;
			 buf[n++] = '\0';
			 s->token.integer = strtol (buf,&endptr,0);
			 if (*endptr != '\0')
			   {
				  return (-1);
			   }
			 if ((s->token.integer == LONG_MIN || s->token.integer == LONG_MAX) && errno == ERANGE)
			   {
				  return (-1);
			   }
			 s->c = c;
			 return (TOK_INTEGER);
		  }
		else
		  {
			 return (-1);
		  }
	 }
}

static int scan_string (scan_t *s)
{
   char c;
   int n = 0,len = SIZE,finished = 0;

   if ((s->token.string = (char *) malloc (len * sizeof (char))) == NULL)
	 goto out_of_memory;

   /* we know the '"' is cached, so we can do this */
   s->c = 0;

   for (;;)
	 {
		READ1(s->token.string);

		if (!finished)
		  {
			 if (n >= len)
			   {
				  char *tmp;
				  len += SIZE;
				  if ((tmp = (char *) realloc (s->token.string,len * sizeof (char))) == NULL)
					goto out_of_memory;
				  s->token.string = tmp;
			   }
			 if (c != '"' || s->token.string[n - 1] == '\\')
			   s->token.string[n++] = c;
			 else
			   finished = 1;
		  }
		else if (finished && (c == ' ' || c == '\n' || c == '\t' || c == ',' || c == '}'))
		  {
			 char *tmp;
			 s->token.string[n++] = '\0';
			 if ((tmp = (char *) realloc (s->token.string,n * sizeof (char))) != NULL)
			   s->token.string = tmp;
			 s->c = c;
			 return (TOK_STRING);
		  }
		else
		  {
			 free (s->token.string);
			 return (-1);
		  }
	 }

out_of_memory:
   if (s->token.string != NULL) free (s->token.string);
   return (-1);
}

/* scan for next token */
int scan (scan_t *s)
{
   char c;
   int comment = 0;

   for (;;)
	 {
		READ0();

		if (!comment || c == '\n')
		  switch (c)
			{
			   /* ignore spaces */
			 case ' ':
			 case '\t':
			   break;

			   /* on NL, increment line counter */
			 case '\n':
			   comment = 0;
			   s->line++;
			   break;

			   /* single-character tokens are returned as-is */
			 case '=':
			 case '{':
			 case ',':
			 case '}':
			   return (c);

			   /* on COMMENT, ignore everything until we reach NL */
			 case '#':
			   comment = 1;
			   break;

			   /* multi-character tokens are handled seperately */
			 default:
			   s->c = c;
			   c = tolower (c);
			   if (isalpha (c) || c == '_')
				 return (scan_keyword (s));
			   else if (isdigit (c) || c == '-')
				 return (scan_integer (s));
			   else if (c == '"')
				 return (scan_string (s));
			   else goto parse_error;
			}
	 }

parse_error:
   return (-1);
}

