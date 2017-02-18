#include <sys/stat.h>

#include <fcntl.h>
#include "config.h"

int build_config(struct configuration *c)
{
	FILE *fptr;

        if ((fptr = fopen(c->config_file, "ro")) == NULL) {
	    log_ret("Failed to open config file", errno);
	    return -1;
	}
	
        if (read_config(c, fptr) < 0) {
	    log_msg("Error setting up configuration");
	    fclose(fptr);
	    return -1;
	} else {
	    fclose(fptr);
	    return 0;
	}
}

int read_config(struct configuration *cfg, FILE *fp)
{
	int n_bytes;
        char *line, *l;
        long offset;

        for (;;) {
	    n_bytes = (line_byte_cnt(fp));

	    if (n_bytes == 1) {
		fseek(fp, 1, SEEK_CUR);
		continue;
	    } else if (n_bytes == 0)
		break;

	    if ((line = malloc((size_t)(n_bytes + 1))) == NULL) {
		log_ret("malloc error while reading config file", errno);
		return -1;
	    }

	    if ((l = fgets(line, (n_bytes + 1), fp)) == NULL) {
		if (feof(fp)) {
		    free(line);
		    break;
	    	} else {
		    log_msg("error reading config file", errno);
		    return -1;
		}
	    }

	    if (check_line(l) != 0) {
		free(l);
		continue;
	    }

	    if (parse_line(l, cfg) < 0) {
		free(line);
		return -1;
	    }
        }
	
	return 0;
}

int 
parse_line(char *l, struct configuration *cfg)
{
	char 		*k, *v;		/* key -> value */
	const char 	*errstr;

        while(isblank((int)l[0]) != 0) /* XXX HACK */
            l++;

	k = strsep(&l, " ");
	v = rm_space(l);
	rm_end_space(k);

	if (strcasecmp("BIND_TIMELIMIT", k) == 0) {
#ifdef BSD
	    cfg->net_timeout = strtonum(v, 0, 600, &errstr);
	    if (errstr != NULL) {
		log_msg("Config error: Bad value for network_timeout");
		return(-1);
	    }
#else
	    cfg->net_timeout = atoi(v);
#endif
	}

	if (strcasecmp("TIMELIMIT", k) == 0) {
#ifdef BSD
	    cfg->srch_timelimit = strtonum(v, 0, 600, &errstr);
		if (errstr != NULL) {
		    log_msg("Config error: Bad value for search_timeout");
		    return(-1);
		}
#else
	    cfg->srch_timelimit = atoi(v);
#endif
	}

	if (strcasecmp("TLS_CACERTFILE", k) == 0) {
#ifdef BSD
            strlcpy(cfg->ca_certpath, v, sizeof(cfg->ca_certpath));
#elif LINUX
            strncpy(cfg->ca_certpath, v, sizeof(cfg->ca_certpath) - 1);
            cfg->ca_certpath[strlen(cfg->ca_certpath)] = '\0';
#endif
	} else if (strcasecmp("URI", k) == 0)
	    cfg->uri_string = strndup(v, strlen(v));
		
	else if (strcasecmp("BASE", k) == 0)
	    cfg->ldap_search_base = strndup(v, strlen(v));

	else if (strcasecmp("SCOPE", k) == 0) {
	    if (strcasecmp("BASE", v) == 0)
		cfg->scope = LDAP_SCOPE_BASE;
	    else if (strcasecmp("ONE", v) == 0)
		cfg->scope = LDAP_SCOPE_ONELEVEL;
	    else if (strcasecmp("SUB", v) == 0)
		cfg->scope = LDAP_SCOPE_SUBTREE;
	    else {
		log_msg("Config error: Bad scope option\n");
	 	return(-1);
	    }
	 } else if (strcasecmp("SSL", k) == 0) {
	    if (strcasecmp("start_tls", v) == 0)
		cfg->use_tls = TLS_TRUE;
	    else if (strcasecmp("on", v) == 0)
		cfg->use_tls = TLS_TRUE;
	    else if (strcasecmp("off", v) == 0)
		cfg->use_tls = TLS_FALSE;
	    else {
		log_msg("Config error: Bad tls options\n");
		return(-1);
	    }
	 }

	return 0;
}

void test_config(struct configuration *c)
{
        printf("use_tls %d\n\n", c->use_tls);
        printf("ca_certpath %s\n\n", c->ca_certpath);
        printf("ldap_search_base: %s\n\n", c->ldap_search_base);
        printf("idle_timeout: %d\n\n", c->idle_timeout);
        printf("uri_string: %s\n\n", c->uri_string);
}

void set_cfg_defaults(struct configuration *c)
{
	c->net_timeout = 10;
	c->srch_timelimit = 10;
	c->idle_timeout = 10;
	c->sync_timeout = 10;
	c->use_tls = TLS_TRUE;
	c->scope = LDAP_SCOPE_SUB;
}	

int validate_config(struct configuration *c)
{
	struct stat sb; 
        int fd;

        if (c->use_tls == TLS_TRUE) {
            if (strlen(c->ca_certpath) == 0) {
                log_msg("Configuration Error: CA_CERTPATH not defined");
		return -1;
	    }

            if (stat(c->ca_certpath, &sb) == -1) {
                log_ret("Configuration Error: Failed to stat CA certfile %s", \
                    c->ca_certpath, errno);
		return -1;
	    }

            if ((fd = open(c->ca_certpath, O_RDONLY)) == -1) {
                log_ret("Can't read CA cert file %s", c->ca_certpath, errno);
		return -1;
            } else
                close(fd);
        }

	if (c->ldap_search_base == NULL) {
            log_msg("Configuration Error: No value for LDAP_SEARCHBASE");
            return -1;
        }

        if (c->uri_string == NULL) {
            log_msg("Configuration Error: no LDAP servers given");
            return -1;
        }
        return 0;
}

void free_config(struct configuration *c)
{
        free(c->ldap_search_base);
        free(c->uri_string);
};

void free_uidinfo(struct uid_info *u)
{
	free(u->uid);
	free(u->smsgateaddr);
	free(u->email);
};
