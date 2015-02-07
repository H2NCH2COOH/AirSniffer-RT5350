#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ssconfig.h"

#define CONFIG_FILE "/etc/config/device.conf"

struct config
{
    char key[128];
    char value[256];
    struct config* next;
};

static char* config_file=CONFIG_FILE;
static struct config* new_cfg=NULL;

void copy(const char* key,const char* value)
{
    struct config* new_item;
    
    new_item=(struct config*)malloc(sizeof(struct config));
    new_item->next=new_cfg;
    new_cfg=new_item;
    strcpy(new_cfg->key,key);
    strcpy(new_cfg->value,value);
}

int main(int argc,char* argv[])
{
    int ret;
    struct config* temp;
    
    if(argc<2)
    {
        fprintf(stderr,"[cfg_updater]Need new config file\n");
        return -1;
    }
    
    if(argc==3)
    {
        config_file=argv[2];
    }
    
    ret=read_config(argv[1]);
    if(ret<0)
    {
        fprintf(stderr,"[cfg_updater]Can't open new config file\n");
        return -2;
    }
    
    ret=iterate_configs(copy);
    if(ret<=0)
    {
        printf("[cfg_updater]No new config item\n");
        return 0;
    }
    
    read_config(config_file);
    
    while(new_cfg!=NULL)
    {
        temp=new_cfg;
        new_cfg=new_cfg->next;
        set_config(temp->key,temp->value);
        free(temp);
    }
    
    ret=dump_config(config_file);
    if(ret<0)
    {
        fprintf(stderr,"[cfg_updater]Can't write modified config\n");
        return -1;
    }
    
    return 0;
}