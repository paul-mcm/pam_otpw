#include	<errno.h>
#include	<stdarg.h>
#include	<syslog.h>
#include	<string.h>
#include	<stdio.h>
#include 	<stdlib.h>

#define MAXLINE 1024

static void	log_doit(int, int, const char *, va_list ap);
void		log_open(const char *i, int, int);
void		log_ret(const char *, ...);
void		log_syserr(const char *, ...);
void		log_msg(const char *, ...);
void		log_die(const char *, ...);
void            log_ldap_quit(char *, int);
void            log_ldap_msg(char *, int);
