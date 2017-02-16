#include "ldap_call.h"

int do_ldap_init(LDAP **l, char *u)
{
	int r;

        if ((r = ldap_initialize(l, u)) != LDAP_SUCCESS) {
            log_ldap_msg("Failed to init ldap handle:", r);
            return(-1);
        }
}

int do_ldap_tls_init(LDAP **l, char *u)
{
	if (do_ldap_init(l, u) < 0) 
	    return(-1);

	if (start_tls(*l) < 0)
	    return(-1);

	return(0);
}

int start_tls(LDAP *l)
{
	int r; 
	if ((r = ldap_start_tls_s(l, NULL, NULL ) != LDAP_SUCCESS)) {
	    log_ldap_msg("TLS initializatin failed", r);
	    return(-1);
	}
        return(0);
}

/* THESE SHOULD BE FATAL ERRORS */
int do_set_ldap_opts(struct configuration *c)
{
	const struct timeval n_to = { c->net_timeout, 0 };
        const struct timeval sync_to = { c->sync_timeout, 0 };
	int v = LDAP_VERSION3;
	int tls = TLS_OPT;
	int r;

	if ((r = ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &v)) \
	!= LDAP_OPT_SUCCESS) {
	    log_ldap_quit("error setting protocl version.", r);
	    return(-1);
        }

        if ((r = ldap_set_option(NULL, LDAP_OPT_DEFBASE, c->ldap_search_base)) \
                != LDAP_OPT_SUCCESS) {
            log_ldap_quit("error setting DEFBASE.", r);
            return(-1);
        }

        if ((r = ldap_set_option(NULL, LDAP_OPT_TIMELIMIT, &c->srch_timelimit)) \
                != LDAP_OPT_SUCCESS) {
            log_ldap_quit("error setting TIMELIMIT", r);
            return(-1);
        }

	if ((r = ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &n_to)) \
                != LDAP_OPT_SUCCESS) {
            log_ldap_quit("error setting NETWORK_TIMEOUT.", r);
            return(-1);
        }

        if ((r = ldap_set_option(NULL, LDAP_OPT_TIMEOUT, &sync_to)) \
	    != LDAP_OPT_SUCCESS) {
            log_ldap_quit("error setting TIMEOUT.", r);
            return(-1);
        }

        /* Set TLS Option */
	if (c->use_tls == TLS_TRUE) {
	    if ((r = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE, c->ca_certpath)) \
		!= LDAP_OPT_SUCCESS) {
		log_ldap_quit("error setting CACERTFILE location.", r);
		return(-1);
	    }

	    if ((r = ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &tls)) \
		!= LDAP_OPT_SUCCESS) {
		log_ldap_quit("error setting REQUIRE_CERT.", r);
        	return(-1);
	    }
	}
	return 0;
}

int ldap_query2(struct configuration *c, struct uid_info *u)
{
	LDAP				*ldap;
	LDAPMessage 			*msg, *entry;
	struct berval 			**vals;
	char				*phone;
	char				*from_addr;
	int				r, i;
	char				fltr_str[LOGIN_NAME_MAX + 4] = "uid=";
	char				*attrs[] = { "smsGateway", "mail", '\0' };

#ifdef BSD
	strlcat(fltr_str, u->uid, sizeof(fltr_str));
#elif LINUX
	strncat(fltr_str, u->uid, sizeof(fltr_str) - 1 - strlen(fltr_str));
#endif

	if (c->init_f(&ldap, c->uri_string) < 0)
	    log_die(LOG_INFO, "%s", "init ldap handle failed");

	if ((r = ldap_search_ext_s(ldap, c->ldap_search_base, c->scope, fltr_str, \
	    attrs, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &msg)) < 0) {
	    syslog(LOG_INFO, "ldap search error: %d - %s", r, ldap_err2string(r));
	    exit(-1);
        }

        if ( ldap_count_entries(ldap, msg) == 1 ) {
	    entry = ldap_first_entry(ldap, msg);

	    if ((vals = ldap_get_values_len(ldap, entry, attrs[0])) != NULL) 
		u->smsgateway = strndup(vals[0]->bv_val, 32);

	    if ((vals = ldap_get_values_len(ldap, entry, attrs[1])) != NULL)
		u->email = strndup(vals[0]->bv_val, 256);

	    ldap_value_free_len(vals);

	} else {
	    syslog(LOG_INFO, "No LDAP entry found for %s\n", u->uid);
	}

        ldap_msgfree(msg);
        ldap_destroy(ldap);

	return 0;

}
