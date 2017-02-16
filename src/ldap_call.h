#ifndef LDAP_CALL_H
#define LDAP_CALL_H

#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>


#include <ldap.h>
#include "config.h"

#define TLS_OPT LDAP_OPT_X_TLS_HARD

void arg_fail(const char *);
void opt_fail(int);
int do_set_ldap_opts(struct configuration *);
int do_ldap_init(LDAP **, char *);
int start_tls(LDAP *);
int do_ldap_tls_init(LDAP **, char *);
int ldap_query2(struct configuration *, struct uid_info *);

#endif
