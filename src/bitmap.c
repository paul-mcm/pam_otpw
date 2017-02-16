#include <sys/stat.h>
#ifdef BSD
#include <sys/syslimits.h>
#else
#include <sys/param.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#define FIELD_MAX 125000

int open_file(char *);
int init_newuid(char *);
int write_byte(int, unsigned char *, int);
int read_byte(int, int, unsigned char *);
int test_bit(unsigned char *, unsigned int);
void set_bit(unsigned char *, unsigned int);

int open_file(char *uid)
{
	int fd, r;
	struct stat sbuff;
	char *base_path = "/var/lib/pam_otpw/";
	char bit_file[PATH_MAX];

#ifdef BSD
	strlcpy(bit_file, base_path, PATH_MAX);
	strlcat(bit_file, uid, PATH_MAX);
#else
	snprintf(bit_file, PATH_MAX, "%s%s", base_path, uid); 
#endif
	if ((r = stat(bit_file, &sbuff)) < 0)
	    if (errno == ENOENT)
		r = init_newuid(bit_file);

	if ((fd = open(bit_file, O_RDWR)) < 0) {
	    printf("open error: %s\n", strerror(errno));
	    return -1;
	}

	return fd;
}

int init_newuid(char *f)
{
	unsigned char bit_field[FIELD_MAX];
        memset(bit_field, 0x00, (FIELD_MAX - 1));

	int r, fd, nwritten = 0;

	if ((fd = open(f, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0) { 
	    printf("open error: %s\n", strerror(errno));
	    return (-1);
        }

	while (nwritten < FIELD_MAX) {
	    if ((r = write(fd, bit_field, FIELD_MAX)) < 0)
		printf("Write error\n");	

	    nwritten += r;
	}

	close(fd);
	return 0;

}

int write_byte(int f, unsigned char *n, int off)
{
	ssize_t r;

	if ((r = pwrite(f, n, 1, off)) <= 0)
	    return -1;

	return 0;
}

int read_byte(int f, int byte_n, unsigned char *b)
{
	if (pread(f, b, 1, byte_n) <= 0) {
	    syslog(LOG_INFO, "Error reading file: %d %s\n", errno, \
	    strerror(errno));
	    return -1;
	}

	return 0;
}

int test_bit(unsigned char *b, unsigned int off)
{
	unsigned char mask = 1 << off;
        return (((b[0] & mask) == mask) ? 1 : 0);
}

void set_bit(unsigned char *b, unsigned int off)
{	
	b[0] |= (1 << off);
}
