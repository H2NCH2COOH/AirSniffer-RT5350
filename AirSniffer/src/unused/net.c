#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>

#include "net.h"
#include "pipecmd.h"
#include "config_key.h"
#include "xively.h"

/*---------------------------------------------------------------------------*
 * Configs
 *---------------------------------------------------------------------------*/
//#define WPA_CTRL_PATH "/var/run/wpa_supplicant"

/*---------------------------------------------------------------------------*
 * Globals
 *---------------------------------------------------------------------------*/
static int pipe_from_main;
static int pipe_to_main;

static const char* ID_RAW_DATA="raw_data";
static const char* ID_PM25="PM25";

static char FEED_ID[64];
static char API_KEY[64];

static char* requesting_config=NULL;

/*---------------------------------------------------------------------------*
 * Get config from main
 * !!!BLOCK!!!
 *---------------------------------------------------------------------------*/
static void get_config(const char* key,char* buf)
{
    char buff[128];
    
    requesting_config=buf;
    buff[0]=CMD_GET_CONFIG;
    strcpy(buff+1,key);
    write(pipe_to_main,buff,2+strlen(buff+1));
    
    while(requesting_config!=NULL)
    {
        pause();
    }
}

/*---------------------------------------------------------------------------*
 * SIGIO handler
 * Receive comm from main and do things according to CMD byte
 *---------------------------------------------------------------------------*/
void net_sigio_handler(int signal,siginfo_t* info, void* value)
{
    static char buf[128];
    char c;
    size_t len;
    
    struct data_points* data;
    char* sp;
    
    len=read(info->si_fd,buf,sizeof(buf));
    
    if(info->si_fd==pipe_from_main)
    {
        switch(buf[0])
        {
            case CMD_PING:
                printf("[AirSniffer][Net]Pinged by main\n");
                c=CMD_PONG;
                write(pipe_to_main,&c,1);
                break;
            case CMD_NEW_DATA:
                sp=strchr(buf+1,',');
                if(sp==NULL)
                {
                    fprintf(stderr,"[AirSniffer][Net]Can't parsse new data: %s\n",buf+1);
                    break;
                }
                *sp='\0';
                ++sp;
                
                printf("[AirSniffer][Net]New data received: raw=%s,pm25=%s\n",buf+1,sp);
                
                data=append_data_point(NULL,ID_RAW_DATA,buf+1);
                data=append_data_point(data,ID_PM25,sp);
                
                if(update_feed(FEED_ID,API_KEY,data)<0)
                {
                    //Update failed
                    //Send disconnected CMD
                    c=CMD_DISCONNECTED;
                    write(pipe_to_main,&c,1);
                }
                free_data_points(data);
                printf("[AirSniffer][Net]New data sent\n");
                
                //Send connected CMD
                c=CMD_CONNECTED;
                write(pipe_to_main,&c,1);
                break;
            case CMD_CONFIG:
                strcpy(requesting_config,buf+1);
                requesting_config=NULL;
                break;
            default:
                break; //What just happened?
        }
    }
    else
    {
        return; //What just happened?
    }
}

/*---------------------------------------------------------------------------*
 * Main
 *---------------------------------------------------------------------------*/
void net_main(int pipe_in,int pipe_out)
{
    char c;
    
    //struct wpa_ctrl* wpa_ctrl_interface;
    //char buf[1024];
    size_t len;
    
    printf("[AirSniffer][Net]Start\n");
    
    pipe_from_main=pipe_in;
    pipe_to_main=pipe_out;
    
    /*wpa_ctrl_interface=wpa_ctrl_open(WPA_CTRL_PATH);
    if(wpa_ctrl_interface==NULL)
    {
        fprintf(stderr,"[AirSniffer][Net]Can't open wpa_supplicant ctrl interface\nPanic & Exit\n");
        return; //Panic & Exit
    }
    
    if(wpa_ctrl_attach(wpa_ctrl_interface)<0)
    {
        fprintf(stderr,"[AirSniffer][Net]Can't attach to wpa_supplicant ctrl interface\nPanic & Exit\n");
        return; //Panic & Exit
    }*/
    
    get_config(CONFIG_KEY_FEED_ID,FEED_ID);
    get_config(CONFIG_KEY_API_KEY,API_KEY);
    
    printf("[AirSniffer][Net]Read config: feed id=%s\n",FEED_ID);
    printf("[AirSniffer][Net]Read config: api key=%s\n",API_KEY);
    
    while(1)
    {
        pause();
    }
    
    /*while(1)
    {
        //Receive wpa event
        if(wpa_ctrl_recv(wpa_ctrl_interface,buf,&len)==0)
        {
            //wpa event
            if(strstr(buf,WPA_EVENT_DISCONNECTED)!=NULL)
            {
                //Disconnected
                c=CMD_DISCONNECTED;
                write(pipe_to_main,&c,1);
            }
            else if(strstr(buf,WPA_EVENT_CONNECTED)!=NULL)
            {
                //Connected
                c=CMD_CONNECTED;
                write(pipe_to_main,&c,1);
            }
            else
            {
                //Ignore other events
            }
        }
    }*/
}