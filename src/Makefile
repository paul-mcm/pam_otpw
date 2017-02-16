OS := $(shell uname)

ifeq ($(OS), OpenBSD)
        libs = /usr/local/lib
        includes = /usr/local/include
        OS_TYPE = BSD
endif
ifeq ($(OS), Linux)
        libs = /usr/lib64               # NEEDED ONLY FOR RHEL
        includes = /usr/include
        OS_TYPE = LINUX
endif

objects = bitmap.o config.o ldap_call.o parse_line.o send_code.o errorlog.o

bitmap.o:
	@gcc -fPIC -D ${OS_TYPE} -I${includes} -L${libs} -g -c -o bitmap.o bitmap.c
config.o: config.h
	@gcc -fPIC -D ${OS_TYPE} -I${includes} -L${libs} -g -c -o config.o config.c
parse_line.o: parse_line.h
	@gcc -fPIC -D ${OS_TYPE} -I${includes} -L${libs} -g -c parse_line.c -o parse_line.o
errorlog.o: errorlog.h
	@gcc -fPIC -D ${OS_TYPE} -g -c errorlog.c -o errorlog.o
ldap_call.o: ldap_call.h
	@gcc -fPIC -D ${OS_TYPE} -I${includes} -L${libs} -g -c ldap_call.c -o ldap_call.o
send_code.o:
	@gcc -fPIC -D ${OS_TYPE} -I${includes} -L${libs} -g -lldap -c -o send_code.o send_code.c
mod: $(objects)
	@gcc -fPIC -DPIC -shared -rdynamic -D ${OS_TYPE} -I${includes} -L${libs} ${objects} -g -lldap -o pam_otpw.so pam_otpw.c
	@rm *.o 2> /dev/null || true

install:
	@cp pam_otpw.so /usr/lib64/security/
	@rm -f $(objects)
	@chown root:root /usr/lib64/security/pam_otpw.so
	@chmod 755 /usr/lib64/security/pam_otpw.so
	@mkdir /var/lib/pam_otpw 2> /dev/null || true
	@chown root:root /var/lib/pam_otpw
	@chmod 755 /var/lib/pam_otpw

clean:
	@rm -f pam_otpw.so
	@rm -f $(objects)
