#ifndef _XIVELY_H_
#define _XIVELY_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define RETRY_TIMES 3

#define TARGET_MAX_LEN	256
#define HEADER_MAX_LEN	256
#define BODY_MAX_LEN	1024

struct data_points
{
	char id[256];
	char value[256];
	
	struct data_points* next;
};

struct data_points* append_data_point(struct data_points* root,const char* id,const char* value);

void free_data_points(struct data_points* root);

int update_feed(const char* feed_id,const char* api_key,struct data_points* data);

int activate_device(const char* activation_code,char* feed_id,char* api_key);

#endif /* _XIVELY_H_ */