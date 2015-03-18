#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "wpa_ctrl.h"

#define WPA_CTRL_PATH "/var/run/wpa_supplicant/wlan0"
#define BUFF_SIZE 512
#define CONN_PAGE "sethost"
#define TIME_OUT 30

static struct wpa_ctrl* wpa_ctrl_interface;

static int timeout;

struct wifi_entry
{
	char ssid[64];
	struct wifi_entry* next;
};

static void sigalrm_handler(int signum)
{
	timeout=1;
}

static char* next_line(const char* str)
{
	while(*str!='\n')
	{
		if(*str=='\0')
		{
			return NULL;
		}
		++str;
	}
	return (char*)(str+1);
}

static void send_cmd(const char* cmd,char* res)
{
	size_t len;

	len=BUFF_SIZE;
	wpa_ctrl_request(wpa_ctrl_interface,cmd,strlen(cmd),res,&len,NULL);
	res[len]='\0';
}

static void recv_event(char* res)
{
	size_t len;

	len=BUFF_SIZE;
	wpa_ctrl_recv(wpa_ctrl_interface,res,&len);
	res[len]='\0';
}

static void empty_events()
{
	char str[BUFF_SIZE];
	size_t len;

	while(wpa_ctrl_pending(wpa_ctrl_interface))
	{
		recv_event(str);
	}
}

int main(int argc,char* argv[])
{
	int i;
	char str[BUFF_SIZE];
	char* strp;
	size_t len=sizeof(str);
	const char* cmd;
    int ret=0;

	struct wifi_entry* wifis=NULL;
	struct wifi_entry* temp=NULL;

	wpa_ctrl_interface=wpa_ctrl_open(WPA_CTRL_PATH);
	if(wpa_ctrl_interface==NULL)
	{
		fprintf(stderr,"[AirSniffer][CGI]Can't open wpa_supplicant ctrl interface\n\t%s\nPanic & Exit\n",strerror(errno));
		return -1; //Panic & Exit
	}

	if(wpa_ctrl_attach(wpa_ctrl_interface)<0)
	{
		fprintf(stderr,"[AirSniffer][CGI]Can't attach to wpa_supplicant ctrl interface\n\t%s\nPanic & Exit\n",strerror(errno));
		return -1; //Panic & Exit
	}

	if(argc==1)
	{
		send_cmd("DISCONNECT",str);

		send_cmd("REMOVE_NETWORK all",str);

		empty_events();

		send_cmd("SCAN",str);

		while(1)
		{
			if(wpa_ctrl_pending(wpa_ctrl_interface))
			{
				recv_event(str);
				//printf("Event: %s\n",str);
				if(strstr(str,WPA_EVENT_SCAN_RESULTS)!=NULL)
				{
					//Received scan results
					break;
				}
			}
		}

		send_cmd("SCAN_RESULTS",str);
		//printf("%ld:%s\n",len,str);

		strp=next_line(str);
		while(strp!=NULL&&*strp!='\0')
		{
			for(i=0;i<4;++i)
			{
				while(*strp!='\t')
				{
					++strp;
				}
				//Found tab
				++strp;
			}

			temp=(struct wifi_entry*)malloc(sizeof(struct wifi_entry));
			sscanf(strp,"%s",temp->ssid);
			temp->next=wifis;
			wifis=temp;

			strp=next_line(strp);
		}

		printf(
			"<html>"
				"<head>"
					"<title>Set up WiFi host</title>"
				"</head>"
				"<body>"
					"<h3>Set up WiFi host</h3>"
					"<form name=\"setup\" action=\"/cgi-bin/" CONN_PAGE "\" method=\"get\">"
						"Choose host SSID:"
						"<br />"
		);

		temp=wifis;
		while(temp!=NULL)
		{
			printf(
				"<input type=\"radio\" name=\"ssid\" value=\"%s\" /> %s <br />",
				temp->ssid,
				temp->ssid
			);
			temp=temp->next;
		}

		printf(
						"Password: "
						"<input type=\"text\" name=\"key\" />"
						"<br />"
						"<input type=\"submit\" value=\"Submit\" />"
					"</form>"
				"</body>"
			"</html>"
		);

		while(wifis!=NULL)
		{
			temp=wifis;
			wifis=wifis->next;
			free(temp);
		}
	}
	else if(argc==3)
	{
		send_cmd("ADD_NETWORK",str);
		sscanf(str,"%d",&i);

		sprintf(str,"SET_NETWORK %d ssid \"%s\"",i,argv[1]);
		send_cmd(str,str);

		sprintf(str,"SET_NETWORK %d psk \"%s\"",i,argv[2]);
		send_cmd(str,str);

		sprintf(str,"ENABLE_NETWORK %d",i);
		send_cmd(str,str);

		send_cmd("REASSOCIATE",str);

		timeout=0;
		signal(SIGALRM,sigalrm_handler);
		alarm(TIME_OUT);

		while(!timeout)
		{
			if(wpa_ctrl_pending(wpa_ctrl_interface))
			{
				recv_event(str);
				if(strstr(str,WPA_EVENT_CONNECTED)!=NULL)
				{
					//Connected
					break;
				}
			}
		}

		if(!timeout)
		{
			//printf("Connected to SSID:%s",argv[1]);

			send_cmd("SAVE_CONFIG",str);
            
            ret=0;
		}
		else
		{
			//printf("Failed to connect to SSID:%s<br />Please retry",argv[1]);
            ret=1;
        }
	}

	wpa_ctrl_detach(wpa_ctrl_interface);
	wpa_ctrl_close(wpa_ctrl_interface);
    
    return ret;
}