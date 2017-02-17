#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "config.h"
#include "ldap_call.h"

#define MAXRAND 1000000

int send_code(const char *);
int load_config(struct configuration *);
int sendmail(const char *, const char *);

/*
 * Loads config from pam_ldap.conf, does LDAP query 
 * and sends email to smsGateway
 */
int send_code(const char *user)
{
	int fd, r;
	int byte_offset;
	unsigned int bit_offset;
	unsigned char bits = 0x00;
	char n_string[7];
	struct uid_info uinfo;
        struct configuration config;
        struct configuration *cfg_ptr = &config;
	char *default_config = "/etc/pam_ldap.conf";
	char hostname[MAXHOSTNAMELEN];

	if (gethostname(hostname, MAXHOSTNAMELEN) == -1) {
	    log_ret("Failed to determine hostname");
	    strcpy(hostname, "unkown host");
	}
#ifdef BSD
	uint32_t n_code;
#else
	long int n_code;
#endif
	cfg_ptr->config_file = default_config;

	if (load_config(cfg_ptr) != 0) {
	    log_msg("Error in configuration");
	    free_config(cfg_ptr);
	}

	uinfo.uid = strndup(user, LOGIN_NAME_MAX);

	if (do_set_ldap_opts(cfg_ptr) != 0) {
	    log_msg("Failed to set LDAP options");
	    goto fail;
	}

	if (ldap_query2(cfg_ptr, &uinfo) != 0) {
	    free_config(cfg_ptr);
	    goto fail;
	}

	free_config(cfg_ptr);

	if (uinfo.smsgateway == NULL || uinfo.email == NULL) {
	    log_msg("Insufficient data for %s", user);
	    goto fail;
	}

	if ((fd = open_file(uinfo.uid)) < 0) {
	    log_msg("Error opening bit file for user %s: %d %s\n", \
	    uinfo.uid, errno);
	    goto fail;
	}

	if (flock(fd, LOCK_EX) < 0) {
	    log_ret("flock(2) failed: ", errno);
	    goto fail;
	}

	while (1) {
#ifdef BSD
	    n_code = arc4random_uniform(MAXRAND);
#else
	    /* XXX NEEDS BETTER SEEDING */
	    srandom(time(NULL));
	    n_code = random() % MAXRAND;
#endif
	    byte_offset = n_code / 8;
	    bit_offset = n_code % 8;

	    if (read_byte(fd, byte_offset, &bits) < 0) {
		log_msg("Error reading bitfile for user %s", uinfo.uid);
		goto fail;
	    }

	    if (test_bit(&bits, bit_offset) == 0) {
		set_bit(&bits, bit_offset);
	 	if (write_byte(fd, &bits, byte_offset) < 0) {
		    log_msg("Error updating bitfile for user %s", uinfo.uid);
		    goto fail;
		}
		break;
	    } else
		continue;
	}

	if (flock(fd, LOCK_UN) < 0) {
	    log_ret("flock unlock failed: ", errno);
	    goto fail;
	}

	close(fd);

	if (n_code >= 0) {
	    r = snprintf(n_string, 7, "%06d", n_code);
	    if ((r = sendmail(uinfo.smsgateway, n_string)) < 0) {
		log_msg(LOG_INFO, "%s\n", "Email error");
		goto fail;
	    }
	}

	return n_code;

	fail:
	close(fd);
	free_uidinfo(&uinfo);
	return -1;
}

int load_config(struct configuration *c)
{
	set_cfg_defaults(c);

        if (build_config(c) < 0)
	    return -1;

        /* SET LDAP INIT FUNCTION */
        if (c->use_tls == TLS_TRUE)
	    c->init_f = do_ldap_tls_init;
        else
	    c->init_f = do_ldap_init;

        if (validate_config(c) < 0) {
	    log_msg("Invalid config file");
	    return -1;
	}

        return 0;
}

int sendmail(const char *to, const char *code)
{
	int retval = -1;
	char msg_str[32] = "Pass Code -- ";
	char *msg = strncat(msg_str, code, 32);

	FILE *mailpipe = popen("/usr/sbin/sendmail -t -f \
	    gateway@iacpublishinglabs.com", "w");

	if (mailpipe != NULL) {
	    fprintf(mailpipe, "To: %s\n", to);
	    fprintf(mailpipe, "From: PAMOTPW\n");
	    fwrite(msg, 1, strlen(msg), mailpipe);
	    fwrite("\n.\n", 1, 2, mailpipe);
	    pclose(mailpipe);
	    retval = 0;
     	} else {
	    perror("Failed to invoke sendmail");
	}
	return retval;
}
