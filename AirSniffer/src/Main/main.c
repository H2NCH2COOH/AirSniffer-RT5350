#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <sys/time.h>

//#include <ugpio/ugpio.h>

//#include "sensor.h"
//#include "net.h"
//#include "pipecmd.h"

#include "config_key.h"
#include "gpio_pin.h"

#include "lcd.h"

#include "ssconfig.h"
#include "xively.h"

/*---------------------------------------------------------------------------*
 * Configs
 *---------------------------------------------------------------------------*/
#define PING_INTERVAL 120

#define ID_FILE "/etc/config/device.id"
#define CONFIG_FILE "/etc/config/device.conf"

#define AP_DEV "wlan1"
#define HOSTAPD_CONFIG_FILE "/etc/config/hostapd.conf"

#define STA_DEV "wlan0"
#define WPA_CTRL_PATH "/var/run/wpa_supplicant/wlan0"
#define WPA_CONFIG "/etc/config/wpa_supplicant.conf"

#define DHCPD_CONFIG "/etc/config/dhcp.conf"

#define WIFI_SETUP_TIMEOUT 300
#define SIGNAL_FIFO "/tmp/assigfifo"

#define SENSOR_READ_INTERVAL_US 500

#define SENSOR_CALC_INTERVAL 60000

#define GPIO_EVENT_COOLDOWN 1000

#define DATA_AVE_NUMBER 10
#define DATA_SEND_INTERVAL 10
/*---------------------------------------------------------------------------*
 * Globals
 *---------------------------------------------------------------------------*/
static int _panic_=0;

static pid_t pid_of_sensor=0;
static int pipe_to_sensor=0;
static int pipe_from_sensor=0;

static int sensor_pong=0;

static pid_t pid_of_net=0;
static int pipe_to_net=0;
static int pipe_from_net=0;

static int net_pong=0;

static int wifi_setup_flag=0;

static int battery_state=0;
#define BATTERY_STATE_CHARGE    (1<<0)
#define BATTERY_STATE_LOW       (1<<1)
#define BATTERY_STATE_FULL      (1<<2)

static int count=0;
static int sensor_low=0;

static int new_data=0;
static double sensor_data[DATA_AVE_NUMBER];
static int current_data_number=0;
static double* new_data_ptr=sensor_data;
static int data_send_counter=0;

static const char* ID_RAW_DATA="raw_data";
static const char* ID_PM25="PM25";

/*---------------------------------------------------------------------------*
 * Prototypes
 *---------------------------------------------------------------------------*/
//static void set_sigio_handler(int fd,void (*sigaction)(int,siginfo_t*,void*));
//static int start_sensor_process();
//static int start_net_process();
//static void main_sigio_handler(int signal,siginfo_t* info, void* value);
static void main_sigalrm_handler(int signal);
static int wifi_setup();
static double data_convert(double raw);

/*---------------------------------------------------------------------------*
 * set_sigio_handler
 * Set <sigaction> as handler for SIGIO by <fd>
 *---------------------------------------------------------------------------*/
/*static void set_sigio_handler(int fd,void (*action)(int,siginfo_t*,void*))
{
    struct sigaction act;
    int flags;
    
    memset(&act,0,sizeof(act));
    act.sa_sigaction=action;
    act.sa_flags=SA_RESTART|SA_SIGINFO;
    sigaction(SIGIO,&act,NULL);
    fcntl(fd,F_SETOWN,getpid());
    flags=fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL,flags|O_ASYNC);
    fcntl(fd,F_SETSIG,SIGIO);
}*/

/*---------------------------------------------------------------------------*
 * Start sensor process
 *---------------------------------------------------------------------------*/
/*static int start_sensor_process()
{
    int pipe_ends[2];
    int pipe_in,pipe_out;
    pid_t pid;
    
    if(pid_of_sensor>0)
    {
        kill(pid_of_sensor,SIGKILL);
    }
    
    if(pipe_from_sensor>0)
    {
        close(pipe_from_sensor);
    }
    
    if(pipe_to_sensor>0)
    {
        close(pipe_to_sensor);
    }
    
    if(pipe(pipe_ends)==-1)
    {
        perror("[AirSniffer][Main]Error creating pipe");
        return -1;
    }
    pipe_from_sensor=pipe_ends[0];
    pipe_out=pipe_ends[1];
    
    if(pipe(pipe_ends)==-1)
    {
        perror("[AirSniffer][Main]Error creating pipe");
        return -1;
    }
    pipe_to_sensor=pipe_ends[1];
    pipe_in=pipe_ends[0];
    
    pid=fork();
    if(pid==-1)
    {
        perror("[AirSniffer][Main]Error doing fork");
        return -1;
    }
    else if(pid==0)
    {
        //Child
        //Enable async on input pipe
        set_sigio_handler(pipe_in,sensor_sigio_handler);
        
        //Run sensor main function
        sensor_main(pipe_in,pipe_out);
        return -1; //Panic: main function should not exit
    }
    
    //Parent
    pid_of_sensor=pid;
    //Enable async on input pipe
    set_sigio_handler(pipe_from_sensor,main_sigio_handler);
    
    return 0;
}*/

/*---------------------------------------------------------------------------*
 * Start network process
 *---------------------------------------------------------------------------*/
/*static int start_net_process()
{
    int pipe_ends[2];
    int pipe_in,pipe_out;
    pid_t pid;
    
    if(pid_of_net>0)
    {
        kill(pid_of_net,SIGKILL);
    }
    
    if(pipe_from_net>0)
    {
        close(pipe_from_net);
    }
    
    if(pipe_to_net>0)
    {
        close(pipe_to_net);
    }
    
    if(pipe(pipe_ends)==-1)
    {
        perror("[AirSniffer][Main]Error creating pipe");
        return -1;
    }
    pipe_from_net=pipe_ends[0];
    pipe_out=pipe_ends[1];
    
    if(pipe(pipe_ends)==-1)
    {
        perror("[AirSniffer][Main]Error creating pipe");
        return -1;
    }
    pipe_to_net=pipe_ends[1];
    pipe_in=pipe_ends[0];
    
    pid=fork();
    if(pid==-1)
    {
        perror("[AirSniffer][Main]Error doing fork");
        return -1;
    }
    else if(pid==0)
    {
        //Child
        //Enable async on input pipe
        set_sigio_handler(pipe_in,net_sigio_handler);
        
        //Run sensor main function
        net_main(pipe_in,pipe_out);
        return -1; //Panic: main function should not exit
    }
    
    //Parent
    pid_of_net=pid;
    //Enable async on input pipe
    set_sigio_handler(pipe_from_net,main_sigio_handler);
    
    return 0;
}*/

/*---------------------------------------------------------------------------*
 * SIGIO handler
 * Receive comm from children and do things according to CMD byte
 *---------------------------------------------------------------------------*/
/*static void main_sigio_handler(int signal,siginfo_t* info, void* value)
{
    static char buf[128];
    char *sp;
    size_t len;
    int pin,lvl;
    
    len=read(info->si_fd,buf,sizeof(buf));
    
    if(info->si_fd==pipe_from_sensor)
    {
        //From sensor
        switch(buf[0])
        {
            case CMD_PONG:
                sensor_pong=1;
                break;
            case CMD_NEW_DATA:
                if(sscanf(buf+1,"%lf",new_data_ptr)!=1)
                {
                    fprintf(stderr,"[AirSniffer][Main]Can't parse new data. Ignored\n");
                    break;
                }
                printf("[AirSniffer][Main]Received raw data: %f\n",*new_data_ptr);
                new_data=1;
                break;
            case CMD_GET_CONFIG:
                buf[0]=CMD_CONFIG;
                get_config(buf+1,buf+1);
                write(pipe_to_sensor,buf,2+strlen(buf+1));
                break;
            case CMD_SET_CONFIG:
                sp=strchr(buf+1,'=');
                if(sp==NULL)
                {
                    fprintf(stderr,"[AirSniffer][Main]Can't parse set config cmd: %s\n",buf+1);
                    break;
                }
                *sp='\0';
                ++sp;
                set_config(buf+1,sp);
                if(dump_config(CONFIG_FILE)<0)
                {
                    fprintf(stderr,"[AirSniffer][Main]Error dumping config, config lost possible\n");
                }
                break;
            case CMD_GPIO_EVENT:
                if(sscanf(buf+1,"%d,%d",&pin,&lvl)!=2)
                {
                    fprintf(stderr,"[AirSniffer][Main]Can't parse GPIO Event. Ignored\n");
                    break;
                }
                printf("[AirSniffer][Main]Received GPIO event: pin%d->%d\n",pin,lvl);
                switch(pin)
                {
                    case GPIO_PIN_BAT_CHARGE:
                        if(lvl==0)
                        {
                            battery_state|=BATTERY_STATE_CHARGE;
                        }
                        else
                        {
                            battery_state&=~BATTERY_STATE_CHARGE;
                        }
                        break;
                    case GPIO_PIN_BAT_LOW:
                        if(lvl==0)
                        {
                            battery_state|=BATTERY_STATE_LOW;
                        }
                        else
                        {
                            battery_state&=~BATTERY_STATE_LOW;
                        }
                        break;
                    case GPIO_PIN_BACK_BUTTON:
                        if((!wifi_setup_flag)&&(lvl==0))
                        {
                            wifi_setup_flag=1;
                        }
                        break;
                    default:
                        break; //What just happened?
                }
                break;
            default:
                break; //What just happened?
        }
    }
    else if(info->si_fd==pipe_from_net)
    {
        //From net
        switch(buf[0])
        {
            case CMD_PONG:
                net_pong=1;
                break;
            case CMD_DISCONNECTED: //fall-through
            case CMD_CONNECTED:
                if(!wifi_setup_flag)
                {
                    write(pipe_to_sensor,buf,1);
                }
                break;
            case CMD_GET_CONFIG:
                buf[0]=CMD_CONFIG;
                get_config(buf+1,buf+1);
                write(pipe_to_net,buf,2+strlen(buf+1));
                break;
            case CMD_SET_CONFIG:
                sp=strchr(buf+1,'=');
                if(sp==NULL)
                {
                    fprintf(stderr,"[AirSniffer][Main]Can't parse set config cmd: %s\n",buf+1);
                    break;
                }
                *sp='\0';
                ++sp;
                set_config(buf+1,sp);
                if(dump_config(CONFIG_FILE)<0)
                {
                    fprintf(stderr,"[AirSniffer][Main]Error dumping config, config lost possible\n");
                }
                break;
            default:
                break; //What just happened?
        }
    }
    else
    {
        return; //What just happened?
    }
}*/

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
    temp->level=1;
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
            if(temp->level!=i)
            {
                temp->level=i;
                temp->cooldown=GPIO_EVENT_COOLDOWN;
            
                //sprintf(buf,"%d,%d",temp->pin,i);
                printf("[AirSniffer][Main]GPIO event: pin%d->%d",temp->pin,i);
                //send_to_main(CMD_GPIO_EVENT,buf);
                
                switch(temp->pin)
                {
                    case GPIO_PIN_BAT_CHARGE:
                        if(temp->level==0)
                        {
                            battery_state|=BATTERY_STATE_CHARGE;
                        }
                        else
                        {
                            battery_state&=~BATTERY_STATE_CHARGE;
                        }
                        break;
                    case GPIO_PIN_BAT_LOW:
                        if(temp->level==0)
                        {
                            battery_state|=BATTERY_STATE_LOW;
                        }
                        else
                        {
                            battery_state&=~BATTERY_STATE_LOW;
                        }
                        break;
                    case GPIO_PIN_BACK_BUTTON:
                        if((!wifi_setup_flag)&&(temp->level==0))
                        {
                            wifi_setup_flag=1;
                        }
                        break;
                    default:
                        break; //What just happened?
                }
                
                break;
            }
        }
        
        temp=temp->next;
    }
}

/*---------------------------------------------------------------------------*
 * SIGALRM handler
 * Ping children and restart them if died
 *---------------------------------------------------------------------------*/
static void main_sigalrm_handler(int signal)
{
    /*static int state=0;
    char ping[5]={CMD_PING,'a','b','c','\0'};
    
    switch(state)
    {
        case 0: //Ping sensor
            printf("[AirSniffer][Main]Ping sensor\n");
            sensor_pong=0;
            write(pipe_to_sensor,ping,sizeof(ping));
            state=1;
            alarm(1);
            break;
        case 1: //Ping sensor timeout
            if(!sensor_pong&&start_sensor_process()<0)
            {
                //Failed starting child process, panic!
                state=-1;
                _panic_=1;
            }
            else
            {
                state=2;
                alarm(1);
            }
            break;
        case 2: //Ping net
            printf("[AirSniffer][Main]Ping net\n");
            net_pong=0;
            write(pipe_to_net,ping,sizeof(ping));
            state=3;
            alarm(1);
            break;
        case 3: //Ping net timeout
            if(!net_pong&&start_net_process()<0)
            {
                //Failed starting child process, panic!
                state=-1;
                _panic_=1;
            }
            else
            {
                state=0;
                alarm(PING_INTERVAL);
            }
            break;
        case -1: //Panic!
        default:
            _panic_=1;
            state=-1;
            break;
    }*/
    
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
        
        //sprintf(buf,"%f",raw);
        //printf("[AirSniffer][Sensor]Send raw data to main: %s\n",buf);
        //send_to_main(CMD_NEW_DATA,buf);
        
        printf("[AirSniffer][Main]New raw data: %f\n",raw);
        *new_data_ptr=raw;
        new_data=1;
        
        //system("cat /proc/uptime");
    }
    
    poll_gpio();
}

/*---------------------------------------------------------------------------*
 * Start wifi setup
 *---------------------------------------------------------------------------*/
static int wifi_setup()
{
    int fd,ret=0;
    struct pollfd pfd[1];
    char buff[64];
    
    wifi_setup_flag=1;
    
    printf("[AirSniffer][Main]Start wifi setup\n");
    
    //DISPLAY: please wait
    
    /*if(system("ifconfig " AP_DEV " up > /dev/null")!=0) //Auto concat
    {
        fprintf(stderr,"[AirSniffer][Main]Can't start interface " AP_DEV "\n"); //Auto concat
        ret=-1;
        goto exit;
    }*/
    
    system("killall udhcpc");
    system("killall wpa_supplicant");
    
    system("hostapd -B " HOSTAPD_CONFIG_FILE); //Auto concat
    system("ifconfig " STA_DEV " 192.168.1.1"); //Auto concat
    system("udhcpd " DHCPD_CONFIG); //Auto concat
    
    //Assume httpd is working
    
    //DISPLAY: please setup
    
    printf("[AirSniffer][Main]AP ready\n");
    
    //Ceate FIFO for finishing signal
    if(access(SIGNAL_FIFO,F_OK)!=0)
    {
        if(mkfifo(SIGNAL_FIFO,0666)!=0)
        {
            fprintf(stderr,"[AirSniffer][Main]Can't make FIFO when starting wifi setup\n\t%s\n",strerror(errno));
            ret=-1;
            goto exit;
        }
    }
    
    //Open FIFO
    fd=open(SIGNAL_FIFO,O_RDONLY|O_NONBLOCK);
    if(fcntl(fd,F_SETFL,0)<0)
    {
        fprintf(stderr,"[AirSniffer][Main]fcntl() error\n\t%s\n",strerror(errno));
        ret=-1;
        close(fd);
        goto exit;
    }
    
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd,&set);
    
    struct timeval to;
    to.tv_sec=WIFI_SETUP_TIMEOUT;
    to.tv_usec=0;
    
    if(select(fd+1,&set,NULL,NULL,&to)<=0)
    {
        fprintf(stderr,"[AirSniffer][Main]Select time out\n");
        ret=-1;
        close(fd);
        goto exit;
    }
    
    //Read FIFO
    buff[read(fd,buff,sizeof(buff))]='\0';
    
    printf("[AirSniffer][Main]WiFi setup info received: %s\n",buff);
    
    system("killall hostapd");
    system("wpa_supplicant -B -i " STA_DEV " -c " WPA_CONFIG); //Auto concat
    //Call wifi_setup_agent
    if(system(buff)!=0)
    {
        //Setup fail
        fprintf(stderr,"[AirSniffer][Main]wifi_setup_agent error\n");
        
        //DISPLAY: setup fail
        
        ret=-1;
    }
    else
    {
        //Setup success
        printf("[AirSniffer][Main]WiFi setup success\n");
        system("udhcpc -i " STA_DEV " -s /etc/udhcpc.script");
        
        //DISPLAY: setup success
        
        ret=0;
    }
    
    //Close FIFO
    close(fd);
    
    sleep(5);
    
exit:
    system("killall hostapd");
    system("killall udhcpd");
    //system("ifconfig " AP_DEV " down"); //Auto concat
    
    wifi_setup_flag=0;
    
    return ret;
}
/*---------------------------------------------------------------------------*
 * Data convert
 *---------------------------------------------------------------------------*/
static double data_convert(double raw)
{
    double ret;
    ret=raw*400;
    return ret;
}
/*---------------------------------------------------------------------------*
 * Start & Stop timer
 *---------------------------------------------------------------------------*/
static void stop_timer(struct itimerval *old)
{
    struct itimerval itv;
    
    itv.it_interval.tv_sec=0;
    itv.it_interval.tv_usec=0;
    itv.it_value.tv_sec=0;
    itv.it_value.tv_usec=0;
    setitimer(ITIMER_REAL,&itv,old);
}

static void start_timer(struct itimerval *itv)
{
    struct itimerval itv2;
    
    if(itv==NULL)
    {
        itv2.it_interval.tv_sec=0;
        itv2.it_interval.tv_usec=SENSOR_READ_INTERVAL_US;
        itv2.it_value.tv_sec=0;
        itv2.it_value.tv_usec=SENSOR_READ_INTERVAL_US;
        itv=&itv2;
    }
    setitimer(ITIMER_REAL,itv,NULL);
}
/*---------------------------------------------------------------------------*
 * Pause and Display
 *---------------------------------------------------------------------------*/
static void display(int type,int data)
{
    struct itimerval itv;
    
    stop_timer(&itv);
    
    switch(type)
    {
        case 
    }
    
    start_timer(&itv);
}
/*---------------------------------------------------------------------------*
 * GPIO init
 *---------------------------------------------------------------------------*/
static void gpio_init()
{
    //Sensor
    gpio_request(GPIO_PIN_SENSOR,NULL);
    gpio_direction_input(GPIO_PIN_SENSOR);
    gpio_request(GPIO_PIN_SENSOR2,NULL);
    gpio_direction_input(GPIO_PIN_SENSOR2);
    
    //LCD A0
    gpio_request(GPIO_PIN_LED_A0,NULL);
    gpio_direction_output(GPIO_PIN_LED_A0,0);
    
    //Battery
    gpio_request(GPIO_PIN_BAT_CHARGE,NULL);
    gpio_direction_input(GPIO_PIN_BAT_CHARGE);
    gpio_request(GPIO_PIN_BAT_LOW,NULL);
    gpio_direction_input(GPIO_PIN_BAT_LOW);
    gpio_request(GPIO_PIN_BAT_FULL,NULL);
    gpio_direction_input(GPIO_PIN_BAT_FULL);
    
    //Back button
    gpio_request(GPIO_PIN_BACK_BUTTON,NULL);
    gpio_direction_input(GPIO_PIN_BACK_BUTTON);
}
/*---------------------------------------------------------------------------*
 * Main
 *---------------------------------------------------------------------------*/
int main(int argc,char* argv[])
{
    FILE* file;
    char buf[128];
    char id[128];
    char activation_code[128];
    char feed_id[128];
    char api_key[128];
    char c;
    
    struct sigaction act,old;
    
    int flags;
    int fd;
    int ret;
    int i;
    double raw;
    double converted;
    int old_battery_state;
    struct data_points* data;
    
    printf("[AirSniffer][Main]Start\n");
    
    //Don't panic, for now
    _panic_=0;
    
    //Configure GPIOs
    gpio_init();
    
    //modprobe as-spi-gpio
    if(system("modprobe as-spi-gpio")!=0)
    {
        _panic_=1;
        fprintf(stderr,"[AirSniffer][Main]modprobe as-spi-gpio failed\nPanic & Exit\n");
        goto panic;
    }
    
    //modprobe as-spi-gpio-dev
    if(system("modprobe as-spi-gpio-dev")!=0)
    {
        _panic_=1;
        fprintf(stderr,"[AirSniffer][Main]modprobe as-spi-gpio-dev failed\nPanic & Exit\n");
        goto panic;
    }
    
    //init display
    display_init();
    
    display_welcome();
    
    //Set SIGALRM handler
    act.sa_handler=main_sigalrm_handler;
    act.sa_flags=SA_RESTART;
    sigaction(SIGALRM,&act,NULL);
    
    if(access(CONFIG_FILE,F_OK)!=0)
    {
        //device.conf don't exist
        //First boot
        printf("[AirSniffer][Main]First boot\n");
        
        //Read device.id
        if(read_config(ID_FILE)<0)
        {
            fprintf(stderr,"[AirSniffer][Main]Error reading: %s\n\t%s\nPanic & Exit\n",strerror(errno),ID_FILE);
            goto panic;
        }
        
        get_config(CONFIG_KEY_DEVICE_ID,id);
        get_config(CONFIG_KEY_ACTIVATION_CODE,activation_code);
        free_config();
        
        //Write config file for hostapd
        file=fopen(HOSTAPD_CONFIG_FILE,"w");
        fprintf(file,"interface=" STA_DEV "\n"); //Auto concat
        fprintf(file,"driver=nl80211\n");
        fprintf(file,"ssid=AirSniffer_%s\n",id);
        fprintf(file,"channel=0\n");
        fprintf(file,"hw_mode=g\n");
        fprintf(file,"ignore_broadcast_ssid=0\n");
        fprintf(file,"auth_algs=3\n");
        fprintf(file,"wpa=3\n");
        fprintf(file,"wpa_passphrase=12345678\n"); //Maybe should change from device to device
        fprintf(file,"wpa_key_mgmt=WPA-PSK\n");
        fprintf(file,"wpa_pairwise=CCMP\n");
        fprintf(file,"rsn_pairwise=CCMP\n");
        fclose(file);
        
        //Wifi setup
        if(wifi_setup()<0)
        {
            fprintf(stderr,"[AirSniffer][Main]Can't setup wifi host!\nPanic & Exit\n");
            goto panic;
        }
        
        //Register device
        ret=activate_device(activation_code,feed_id,api_key);
        if(ret<0)
        {
            fprintf(stderr,"[AirSniffer][Main]Can't activate device! ret=%d\nPanic & Exit\n",ret);
            goto panic;
        }
        
        printf("[AirSinffer][Main]Device activated\n\tfeed id: %s\n\tapi key: %s\n",feed_id,api_key);
        
        //Write device.conf
        free_config();
        set_config(CONFIG_KEY_DEVICE_ID,id);
        set_config(CONFIG_KEY_FEED_ID,feed_id);
        set_config(CONFIG_KEY_API_KEY,api_key);
        dump_config(CONFIG_FILE);
        free_config();
    }
    
    //Read configs from file
    if(read_config(CONFIG_FILE)<0)
    {
        fprintf(stderr,"[AirSniffer][Main]Error reading config\nPanic & Exit\n");
        goto panic;
    }
    
    //start_sensor_process();
    //start_net_process();
    
    //Start first ping alram
    //alarm(5);
    
    //DISPLAY: please wait
    
    //Start timer
    start_timer(NULL);
    
    old_battery_state=battery_state;
    
    while(!_panic_)
    {
        if(wifi_setup_flag)
        {
            //Stop timer
            stop_timer(NULL);
            
            count=0;
            sensor_low=0;
            
            wifi_setup();
            
            //DISPLAY: please wait
            
            //Restart timer
            start_timer(NULL);
        }
        
        if(old_battery_state!=battery_state)
        {
            if(battery_state==0)
            {
                //DISPLAY: battery clear
            }
            else if(battery_state&BATTERY_STATE_CHARGE)
            {
                //DISPLAY: battery charging
            }
            else if(battery_state&BATTERY_STATE_LOW)
            {
                //DISPLAY: battery low
            }
            
            old_battery_state=battery_state;
        }
        
        if(new_data)
        {
            new_data=0;
            
            if(current_data_number<DATA_AVE_NUMBER)
            {
                ++current_data_number;
            }
            
            ++new_data_ptr;
            if((new_data_ptr-sensor_data)>=DATA_AVE_NUMBER)
            {
                new_data_ptr=sensor_data;
            }
            
            raw=0;
            for(i=0;i<current_data_number;++i)
            {
                raw+=sensor_data[i];
            }
            raw/=current_data_number;
            converted=data_convert(raw);
            
            //DISPLAY: new data
            
            if(++data_send_counter>=DATA_SEND_INTERVAL)
            {
                data_send_counter=0;
                
                //buf[0]=CMD_NEW_DATA;
                //sprintf(buf+1,"%10f,%10f",raw,converted);
                //write(pipe_to_net,buf,2+strlen(buf+1));
                
                sprintf(buf,"%10f",raw);
                data=append_data_point(NULL,ID_RAW_DATA,buf);
                sprintf(buf,"%10d",(int)converted);
                data=append_data_point(data,ID_PM25,buf);
                
                get_config(CONFIG_KEY_FEED_ID,feed_id);
                get_config(CONFIG_KEY_API_KEY,api_key);
                
                //Stop timer
                stop_timer(NULL);
                
                if(update_feed(feed_id,api_key,data)<0)
                {
                    //Update failed
                    //Send disconnected CMD
                    //c=CMD_DISCONNECTED;
                    //write(pipe_to_sensor,&c,1);
                    printf("[AirSniffer][Main]Disconnected\n");
                }
                else
                {
                    printf("[AirSniffer][Main]New data sent\n");
                    //Send connected CMD
                    //c=CMD_CONNECTED;
                    //write(pipe_to_sensor,&c,1);
                    printf("[AirSniffer][Main]Connected\n");
                }
                free_data_points(data);
                
                //Restart timer
                start_timer(NULL);
            }
        }
    }
    
panic:
    //Stop timer
    stop_timer(NULL);
    
    //Free configs
    free_config();
    
    //Kill children
    kill(pid_of_sensor,SIGKILL);
    kill(pid_of_net,SIGKILL);
    
    printf("[AirSniffer][Main]Exit\n");
    
    //DISPLAY: panic screen
    
    return -1; //Panic & Exit
}