#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

/* DEFINE WHICH PAM INTERFACES WE PROVIDE.
*  MUST BE DEFINED BEFORE INCLUDING PAM HEADERS
*/
#define PAM_SM_AUTH
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>

#define MAX_LOGIN_ATTEMPTS 3

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	int ret, i, pam_err, code;
	int nowarn = 0;
	char n_string[7];

	const char *user = NULL;
	struct pam_conv *conversation;
	struct pam_message mesg;
	struct pam_response *resp;

	mesg.msg_style = PAM_PROMPT_ECHO_OFF;
	mesg.msg = "Enter Passcode: ";
	const struct pam_message *pmesg = &mesg;

        openlog("pam_otpw", LOG_NDELAY, LOG_AUTH);

	if ((ret = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS || \
	    user == NULL) {
	    syslog(LOG_INFO, "%s", "Failure retrieving PAM user");
	    return PAM_AUTH_ERR;
	}

	if ((code = send_code(user)) < 0) {
	    syslog(LOG_INFO, "%s", "Unable to send code to user");
	    return PAM_AUTH_ERR;
	}

	snprintf(n_string, 7, "%06d", code);

	/* Get conversation function for prompting user */
	if ((ret = pam_get_item(pamh, PAM_CONV, (void *) &conversation)) != PAM_SUCCESS ||
	    conversation == NULL) {
	    syslog(LOG_ERR, "%s\n", "Failed to get conversation function");
	    return PAM_AUTH_ERR;
	}

	for (i = 1; i <= MAX_LOGIN_ATTEMPTS; i++) {
	    pam_err = (*conversation->conv)(1, &pmesg, &resp, conversation->appdata_ptr);

	    if (pam_err == PAM_SUCCESS)
	        syslog(LOG_INFO, "%s", "PAM conv() Success");

	    if (resp != NULL)
	        syslog(LOG_INFO, "%s", "PAM conv() returned non-NULL");

	    if (resp != NULL && pam_err == PAM_SUCCESS) {
	        if (strcmp(resp->resp, n_string) == 0) {
		    syslog(LOG_INFO, "OTPW login for %s succeeded", user);
		    return PAM_SUCCESS;
		} else
		    syslog(LOG_INFO, "OTPW login error for %s", user);

	    } else
		syslog(LOG_INFO, "%s", "Error calling conv function");

	    free(resp->resp);
	    free(resp);
	}
	/* FALL THROUGH */
	syslog(LOG_INFO, "PAM OTPW failure for %s", user);
        return PAM_AUTH_ERR;
}

/* PAM entry point for authentication token (password) changes */
int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return(PAM_IGNORE);
}

/* PAM entry point for session creation */
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
        return(PAM_IGNORE);
}

/* PAM entry point for session cleanup */
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
        return(PAM_IGNORE);
}

/* PAM entry point for accounting */
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
        return(PAM_IGNORE);
}

/* PAM entry point for setting user credentials (that is, to actually establish the authenticated user's credentials to the service provider) */
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
        return(PAM_IGNORE);
}
