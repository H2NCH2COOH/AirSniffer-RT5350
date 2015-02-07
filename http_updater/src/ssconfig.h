#ifndef _SSCONFIG_H_
#define _SSCONFIG_H_

int read_config(const char* file);
int dump_config(const char* file);
void free_config();
void get_config(const char* key,char* value);
void set_config(const char* key,const char* value);
int iterate_configs(void (*func)(const char* key,const char* value));

#endif /* _SSCONFIG_H_ */