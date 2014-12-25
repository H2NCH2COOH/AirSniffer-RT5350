#ifndef _UPLOAD_H_
#define _UPLOAD_H_

#define HOST_ADDR   "115.29.178.169"
#define TARGET_ADDR "/dev/%s"

#define RETRY_TIMES 3

#define TARGET_MAX_LEN	256
#define HEADER_MAX_LEN	256
#define BODY_MAX_LEN	1024

struct data_points
{
	char key[256];
	char value[256];
	
	struct data_points* next;
};

struct data_points* append_data_point(struct data_points* root,const char* key,const char* value);

void free_data_points(struct data_points* root);

int upload(const char* dev_id,struct data_points* data);

#endif /* _UPLOAD_H_ */