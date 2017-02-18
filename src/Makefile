MODULE := pam_otpw.so
BITFILE_DIR := /var/lib/pam_otpw
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
        OS := LINUX

        ifneq ("$(wildcard /etc/debian_version)", "")
        INSTALL_PATH := /lib/x86_64-linux-gnu/security
        endif

        ifneq ("$(wildcard /etc/redhat-release)", "")
        INSTALL_PATH := /usr/lib64/security
        endif
endif

objects = bitmap.o config.o ldap_call.o parse_line.o send_code.o errorlog.o

bitmap.o:
	@gcc -fPIC -D ${OS} -I${includes} -L${libs} -g -c -o bitmap.o bitmap.c
config.o: config.h
	@gcc -fPIC -D ${OS} -I${includes} -L${libs} -g -c -o config.o config.c
parse_line.o: parse_line.h
	@gcc -fPIC -D ${OS} -I${includes} -L${libs} -g -c parse_line.c -o parse_line.o
errorlog.o: errorlog.h
	@gcc -fPIC -D ${OS} -g -c errorlog.c -o errorlog.o
ldap_call.o: ldap_call.h
	@gcc -fPIC -D ${OS} -I${includes} -L${libs} -g -c ldap_call.c -o ldap_call.o
send_code.o:
	@gcc -fPIC -D ${OS} -I${includes} -L${libs} -g -lldap -c -o send_code.o send_code.c
mod: $(objects)
	@gcc -fPIC -DPIC -shared -rdynamic -D ${OS} -I${includes} -L${libs} ${objects} -g -lldap -lbsd -o ${MODULE} pam_otpw.c
	@rm *.o 2> /dev/null || true

install:
	@cp ${MODULE} ${INSTALL_PATH}
	@rm -f $(objects)
	@chown root:root ${INSTALL_PATH}/${MODULE}
	@chmod 755 ${INSTALL_PATH}/${MODULE}
	@mkdir ${BITFILE_DIR} 2> /dev/null || true
	@chown root:root ${BITFILE_DIR}
	@chmod 755 ${BITFILE_DIR}

clean:
	@rm -f ${MODULE}
	@rm -f $(objects)
