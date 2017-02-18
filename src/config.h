#ifndef CONFIG_H
#define CONFIG_H

#include <errno.h>
#include <stdlib.h>

#ifdef BSD
#include <sys/syslimits.h>
#else
#include <sys/param.h>
#endif

#include <ldap.h>
#include "parse_line.h"

struct uid_info {
	char *uid;
	char *smsgateaddr;
	char *email;
};


struct configuration {
	char		*config_file;
        char		*ldap_search_base;
	int		scope;
	int 		(*init_f)(LDAP **, char *);
        char 		*uri_string;
        struct uri_list *uris;
        unsigned int	idle_timeout;				/* HOW LONG TO KEEP IDLE CONNECTION OPEN */

	/* LDAP OPT SETTINGS */
        int 		use_tls;				/* YES/NO */
        char 		ca_certpath[PATH_MAX + 1];				
	int		net_timeout;				/* connect(2) TIMEOUT */
	int		srch_timelimit;				/* LDAP SEARCH TIMEOUT */
	int		sync_timeout;				/* SYNCHRONOUS CALL TIMEOUT */

};

enum tls {
	TLS_FALSE,
        TLS_TRUE,
};

int read_config(struct configuration *, FILE *);
int parse_line(char *, struct configuration *);
void test_config(struct configuration *);
void set_cfg_defaults(struct configuration *);
int validate_config(struct configuration *);
void free_uidinfo(struct uid_info *);

#endif
