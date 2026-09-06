#ifndef __POSIX_FALLOCATE_H__
#define __POSIX_FALLOCATE_H__

#include <sys/stat.h>

int posix_fallocate(int fd, off_t offset, off_t len);

#endif // __POSIX_FALLOCATE_H__