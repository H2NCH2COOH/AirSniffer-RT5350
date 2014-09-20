#ifndef _SSHTTP_H_
#define _SSHTTP_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#define SSHTTP_BUFF_SIZE 10240

#define GET		0
#define PUT		1
#define POST	2
#define DELETE	3

#define TIMEOUT 5

int sshttp_request
(
    const char* host,
    const char* target,
    int method,
    const char* header[],
    int header_num,
    const char* body,
    size_t body_len,
    char* responce
);

#endif /* _SSHTTP_H_ */