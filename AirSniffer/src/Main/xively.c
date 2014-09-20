#include "xively.h"
#include "sshttp.h"

static char target[TARGET_MAX_LEN];
static char header[HEADER_MAX_LEN];
static char body[BODY_MAX_LEN];

struct data_points* append_data_point(struct data_points* root,const char* id,const char* value)
{
	struct data_points* temp;
	
	temp=(struct data_points*)malloc(sizeof(struct data_points));
	strcpy(temp->id,id);
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

int update_feed(const char* feed_id,const char* api_key,struct data_points* data)
{
	const char* temp;
	struct data_points* d;
	size_t s;
	int i,ret=0;
    const char* hs[1];
	
	/*-----build target-----*/
	s=0;
	temp="/v2/feeds/";
	strcpy(target,temp);
	s+=strlen(temp);
	
	temp=feed_id;
	strcpy(target+s,temp);
	s+=strlen(temp);
	
	temp=".json";
	strcpy(target+s,temp);
	s+=strlen(temp);
	
	target[s]='\0';
	
	/*-----build header-----*/
	s=0;
	temp="X-ApiKey:";
	strcpy(header,temp);
	s+=strlen(temp);
	
	temp=api_key;
	strcpy(header+s,temp);
	s+=strlen(temp);
	header[s]='\0';
	
	/*-----build body-----*/
	s=0;
	temp="{\"datastreams\":[";
	strcpy(body,temp);
	s+=strlen(temp);
	
	for(d=data;d!=NULL;d=d->next)
	{
		temp="{\"id\":\"";
		strcpy(body+s,temp);
		s+=strlen(temp);
		
		strcpy(body+s,d->id);
		s+=strlen(d->id);
		
		temp="\",\"current_value\":\"";
		strcpy(body+s,temp);
		s+=strlen(temp);
		
		strcpy(body+s,d->value);
		s+=strlen(d->value);
		
		temp="\"}";
		strcpy(body+s,temp);
		s+=strlen(temp);
		
		if(d->next!=NULL)
		{
			body[s++]=',';
		}
	}
	
	temp="]}";
	strcpy(body+s,temp);
	s+=strlen(temp);
	
	body[s]='\0';
	
	for(i=0;i<RETRY_TIMES;++i)
	{
		//printf("Update feed request:\n%s\n%s\n%s\n",target,(&header)[0],body);
        hs[0]=header;
        ret=sshttp_request("api.xively.com",target,PUT,hs,1,body,s,NULL);
		if(ret==200)
		{
			return 0;
		}
		//possible delay
	}
	
	return -1;
}

int activate_device(const char* activation_code,char* feed_id,char* api_key)
{
	const char* temp;
	char* bp;
    char* bp2;
	size_t s;
	int i,ret=0;
    long l;
	
	/*-----build target-----*/
	s=0;
	temp="/v2/devices/";
	strcpy(target,temp);
	s+=strlen(temp);
	
	temp=activation_code;
	strcpy(target+s,temp);
	s+=strlen(temp);
	
	temp="/activate";
	strcpy(target+s,temp);
	s+=strlen(temp);
	
	target[s]='\0';
	
	for(i=0;i<RETRY_TIMES;++i)
	{
		ret=sshttp_request("api.xively.com",target,GET,NULL,0,NULL,0,body);
		if(ret==200)
		{
		   break;
		}
		//possible delay
	}
	
	if(ret!=200)
	{
		return -1;
	}
	
	/*-----api key-----*/
	temp="\"apikey\"";
	bp=strstr(body,temp);
	if(bp==NULL)
	{
		return -2;
	}
	bp+=strlen(temp);
	
	bp=strchr(bp,'\"');
	if(bp==NULL)
	{
		return -3;
	}
	++bp;
	
	bp2=strchr(bp,'\"');
	if(bp2==NULL)
	{
		return -4;
	}
	
	s=bp2-bp;
	
	strncpy(api_key,bp,s);
	api_key[s]='\0';
	
	/*-----feed id-----*/
	temp="\"feed_id\":";
	bp=strstr(body,temp);
	if(bp==NULL)
	{
		return -2;
	}
	bp+=strlen(temp);
	
	sscanf(bp,"%ld",&l);
    sprintf(feed_id,"%ld",l);
    
    return 0;
}
