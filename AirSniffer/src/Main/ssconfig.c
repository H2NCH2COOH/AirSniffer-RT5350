#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ssconfig.h"

struct config_item
{
    char key[128];
    char value[256];
    struct config_item* next;
};

static struct config_item* root=NULL;

void free_config()
{
    struct config_item* t;
    while(root!=NULL)
    {
        t=root;
        root=root->next;
        free(t);
    }
}

int dump_config(const char* file)
{
    FILE* f;
    struct config_item* item=root;
    
    f=fopen(file,"w");
    if(f==NULL)
    {
        fprintf(stderr,"[ssconfig]Can't open config file: %s\n\t%s\n",file,strerror(errno));
        return -1;
    }
    
    while(item!=NULL)
    {
        fputs(item->key,f);
        fputc('=',f);
        fputs(item->value,f);
        fputc('\n',f);
        item=item->next;
    }
    
    fclose(f);
    
    return 0;
}

int read_config(const char* file)
{
    char buf[512];
    char* sp;
    FILE* f;
    struct config_item* new_item;
    
    free_config();
    
    f=fopen(file,"r");
    if(f==NULL)
    {
        fprintf(stderr,"[ssconfig]Can't open config file: %s\n\t%s\n",file,strerror(errno));
        return -1;
    }
    
    while(!feof(f))
    {
        if(fgets(buf,sizeof(buf),f)==NULL)
        {
            break;
        }
        if(buf[strlen(buf)-1]=='\n')
        {
            buf[strlen(buf)-1]='\0'; //Remove the '\n'
        }
        
        if(strlen(buf)==0||buf[0]=='#')
        {
            continue;
        }
        
        sp=strchr(buf,'=');
        if(sp==NULL)
        {
            fprintf(stderr,"[ssconfig]Can't parse config file line: \"%s\"\nLine skipped\n",buf);
            continue;
        }
        *sp='\0';
        ++sp;
        
        new_item=(struct config_item*)malloc(sizeof(struct config_item));
        new_item->next=root;
        root=new_item;
        strcpy(root->key,buf);
        strcpy(root->value,sp);
    }
    
    fclose(f);
    
    return 0;
}

void get_config(const char* key,char* value)
{
    struct config_item* item=root;
    
    while(item!=NULL)
    {
        if(strcmp(item->key,key)==0)
        {
            strcpy(value,item->value);
            return;
        }
        item=item->next;
    }
    
    strcpy(value,"");
}

void set_config(const char* key,const char* value)
{
    struct config_item* item=root;
    
    while(item!=NULL)
    {
        if(strcmp(item->key,key)==0)
        {
            strcpy(item->value,value);
            break;
        }
        item=item->next;
    }
    
    if(item==NULL)
    {
        item=(struct config_item*)malloc(sizeof(struct config_item));
        item->next=root;
        root=item;
        strcpy(root->key,key);
        strcpy(root->value,value);
    }
}
