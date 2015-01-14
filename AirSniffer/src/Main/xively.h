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

struct xively_data_points
{
	char id[256];
	char value[256];
	
	struct xively_data_points* next;
};

struct xively_data_points* xively_append_data_point(struct xively_data_points* root,const char* id,const char* value);

void xively_free_data_points(struct xively_data_points* root);

int xively_update_feed(const char* feed_id,const char* api_key,struct xively_data_points* data);

int xively_activate_device(const char* activation_code,char* feed_id,char* api_key);

#endif /* _XIVELY_H_ */