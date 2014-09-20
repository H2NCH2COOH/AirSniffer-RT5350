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

//#include <ugpio/ugpio.h>

#include "sensor.h"
#include "pipecmd.h"
#include "gpio_pin.h"

/*---------------------------------------------------------------------------*
 * Configs
 *---------------------------------------------------------------------------*/
/* 
 * FIXME:
 *      Due to unknown reason, it seems only one of every two SIGALRM can be 
 *      captured. So the timer interval is halved
 */
#define SENSOR_READ_INTERVAL_US 250

#define SENSOR_CALC_INTERVAL 60000

#define GPIO_EVENT_COOLDOWN 1000

/*---------------------------------------------------------------------------*
 * Globals
 *---------------------------------------------------------------------------*/
static int pipe_from_main;
static int pipe_to_main;

static int count=0;
static int sensor_low=0;

static char* requesting_config=NULL;

/*---------------------------------------------------------------------------*
 * Send comm to main through pipe
 * !!!<data> must be terminated by '\0'!!!
 * !!!<data> should be short!!!
 *---------------------------------------------------------------------------*/
static void send_to_main(char cmd,const char* data)
{
    static char buf[128];
    buf[0]=cmd;
    strcpy(buf+1,data);
    write(pipe_to_main,buf,2+strlen(buf+1));
}

/*---------------------------------------------------------------------------*
 * GPIO Event
 *---------------------------------------------------------------------------*/
struct gpio_reg
{
    int pin;
    int level;
    int cooldown;
    struct gpio_reg* next;
};
struct gpio_reg* gpio_to_poll=NULL;

static void add_gpio(int p)
{
    struct gpio_reg* temp;
    
    temp=(struct gpio_reg*)malloc(sizeof(struct gpio_reg));
    temp->pin=p;
    temp->level=-1;
    temp->next=gpio_to_poll;
    gpio_to_poll=temp;
}

static void poll_gpio()
{
    static char buf[128];
    
    struct gpio_reg* temp;
    int i;
    
    temp=gpio_to_poll;
    
    while(temp!=NULL)
    {
        if(temp->cooldown>0)
        {
            --(temp->cooldown);
        }
        else
        {
            i=gpio_get_value(temp->pin);
            if(temp->level==-1)
            {
                temp->level=i;
            }
            else if(temp->level!=i)
            {
                temp->level=i;
                temp->cooldown=GPIO_EVENT_COOLDOWN;
            
                sprintf(buf,"%d,%d",temp->pin,i);
                printf("[AirSniffer][Sensor]Send GPIO event to main: pin%d->%d",temp->pin,i);
                send_to_main(CMD_GPIO_EVENT,buf);
                
                break;
            }
        }
        
        temp=temp->next;
    }
}

/*---------------------------------------------------------------------------*
 * SIGIO handler
 * Receive comm from main and do things according to CMD byte
 *---------------------------------------------------------------------------*/
void sensor_sigio_handler(int signal,siginfo_t* info, void* value)
{
    static char buf[128];
    size_t len;
    
    len=read(info->si_fd,buf,sizeof(buf));
    
    if(info->si_fd==pipe_from_main)
    {
        switch(buf[0])
        {
            case CMD_PING:
                printf("[AirSniffer][Sensor]Pinged by main\n");
                send_to_main(CMD_PONG,"");
                break;
            case CMD_DISCONNECTED:
                //TODO: disconnected
                //TEST
                printf("[AirSniffer][Sensor]Disconnected\n");
                //TEST
                break;
            case CMD_CONNECTED:
                //TODO: connected
                //TEST
                printf("[AirSniffer][Sensor]Connected\n");
                //TEST
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
 * SIGALRM handler
 *---------------------------------------------------------------------------*/
static void sensor_sigalrm_handler(int signal)
{
    static char buf[128];
    
    double raw;
    
    if(gpio_get_value(GPIO_PIN_SENSOR)==0)
    {
        ++sensor_low;
    }
    
    ++count;
    if(count>=SENSOR_CALC_INTERVAL)
    {
        count=0;
        
        raw=(double)sensor_low/SENSOR_CALC_INTERVAL;
        sensor_low=0;
        
        sprintf(buf,"%f",raw);
        system("cat /proc/uptime");
        //printf("[AirSniffer][Sensor]Send raw data to main: %s\n",buf);
        //send_to_main(CMD_NEW_DATA,buf);
    }
    
    poll_gpio();
}

/*---------------------------------------------------------------------------*
 * Main
 *---------------------------------------------------------------------------*/
void sensor_main(int pipe_in,int pipe_out)
{
    struct itimerval itv;
    struct sigaction act;
    
    printf("[AirSniffer][Sensor]Start\n");
    
    pipe_from_main=pipe_in;
    pipe_to_main=pipe_out;
    
    add_gpio(GPIO_PIN_BAT_CHARGE);
    add_gpio(GPIO_PIN_BAT_LOW);
    add_gpio(GPIO_PIN_BACK_BUTTON);
    
    act.sa_handler=sensor_sigalrm_handler;
    act.sa_flags=SA_RESTART;
    sigaction(SIGALRM,&act,NULL);
    
    //Start timer
    itv.it_interval.tv_sec=0;
    itv.it_interval.tv_usec=SENSOR_READ_INTERVAL_US;
    itv.it_value.tv_sec=0;
    itv.it_value.tv_usec=SENSOR_READ_INTERVAL_US;
    setitimer(ITIMER_REAL,&itv,NULL);
    
    while(1)
    {
        ;
    }
}