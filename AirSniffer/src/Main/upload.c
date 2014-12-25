#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "upload.h"
#include "sshttp.h"

static char target[TARGET_MAX_LEN];
static char header[HEADER_MAX_LEN];
static char body[BODY_MAX_LEN];

struct data_points* append_data_point(struct data_points* root,const char* key,const char* value)
{
	struct data_points* temp;
	
	temp=(struct data_points*)malloc(sizeof(struct data_points));
	strcpy(temp->key,key);
	strcpy(temp->value,value);
	temp->next=root;
	
	return temp;
}
void free_data_points(struct data_points* root)
{
	struct data_points* t;
	while(root!=NULL)
	{
		t=root;
		root=root->next;
		free(t);
	}
}

int upload(const char* dev_id,struct data_points* data)
{
	const char* temp;
	struct data_points* d;
	size_t s;
	int i,ret=0;
    const char* hs[1];
	
	/*-----build target-----*/
    sprintf(target,TARGET_ADDR,dev_id);
	
	/*-----build header-----*/
	s=0;
	temp="Content-Type: application/x-www-form-urlencoded";
	strcpy(header,temp);
	s+=strlen(temp);
	header[s]='\0';
	
	/*-----build body-----*/
	s=0;
	temp="data=%7B";/* data={ */
	strcpy(body,temp);
	s+=strlen(temp);
	
	for(d=data;d!=NULL;d=d->next)
	{
		temp="%22";/* " */
		strcpy(body+s,temp);
		s+=strlen(temp);
		
		strcpy(body+s,d->key);
		s+=strlen(d->key);
		
		temp="%22%3a";/* ": */
		strcpy(body+s,temp);
		s+=strlen(temp);
		
		strcpy(body+s,d->value);
		s+=strlen(d->value);
		
		if(d->next!=NULL)
		{
			body[s++]=',';
		}
	}
	
	temp="%7D";/* } */
	strcpy(body+s,temp);
	s+=strlen(temp);
	
	body[s]='\0';
    
	for(i=0;i<RETRY_TIMES;++i)
	{
		//printf("Update feed request:\n%s\n%s\n%s\n",target,(&header)[0],body);
        hs[0]=header;
        ret=sshttp_request(HOST_ADDR,target,POST,hs,1,body,s,NULL);
		if(ret==200)
		{
			return 0;
		}
		//possible delay
	}
	
	return ret;
}