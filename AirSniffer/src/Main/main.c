#define _GNU_SOURCE

#include "main.h"

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
#include "img.h"

#include "ssconfig.h"

#include "xively.h"
#include "upload.h"

/*---------------------------------------------------------------------------*
 * Globals
 *---------------------------------------------------------------------------*/
static int _panic_=0;

#ifdef USE_WATCHDOG
    static int watchdog;
    #define feed_dog() write(watchdog,"f",1)
#endif

static char buff[128];

static int battery_state=0;

static int count=0;
static int sensor_low=0;

static int new_data=0;
static double sensor_data[DATA_AVE_NUMBER];
static int current_data_number=0;
static double* new_data_ptr=sensor_data;
static int data_send_counter=0;

static const char* KEY_RAW_DATA="pm25raw";
static const char* KEY_TEMP="temp";

static int spinner_count=0;
static int spinner_frame=0;

static int debug;

static int* timers[TIMERS_SIZE]={0};

static int fb_pressing=0;
static int fb_timer=0;

static int displaying_unit=0;

static int state=STATE_PM25;

static double calibration_m=1;
static double calibration_a=0;

/*---------------------------------------------------------------------------*
 * Prototypes
 *---------------------------------------------------------------------------*/
static void main_sigalrm_handler(int signal);
static int wifi_setup();
static int data_convert(double raw,int unit_type);
static void stop_timer(struct itimerval *old);
static void start_timer(struct itimerval *itv);
static void display(int type,int data);
static void add_gpio(int p,int interval,int cooldown);
static void poll_gpio();
static void gpio_event_handler(int pin,int level);
static int add_timer(int* timer);
static void remove_timer(int* timer);
/*---------------------------------------------------------------------------*
 * GPIO event functions
 *---------------------------------------------------------------------------*/
struct gpio_reg
{
    int pin;
    int level;
    int interval;
    int cooldown;
    int counter;
    struct gpio_reg* next;
};
static struct gpio_reg* gpio_to_poll=NULL;

static void add_gpio(int p,int interval,int cooldown)
{
    struct gpio_reg* temp;
    
    temp=gpio_to_poll;
    while(temp!=NULL)
    {
        if(temp->pin==p)
        {
            temp->interval=interval;
            temp->cooldown=cooldown;
            return;
        }
        temp=temp->next;
    }
    
    temp=(struct gpio_reg*)malloc(sizeof(struct gpio_reg));
    temp->pin=p;
    temp->level=-1;
    temp->interval=interval;
    temp->cooldown=cooldown;
    temp->counter=0;
    temp->next=gpio_to_poll;
    gpio_to_poll=temp;
}

static void poll_gpio()
{
    struct gpio_reg* temp;
    int i;
    
    temp=gpio_to_poll;
    
    while(temp!=NULL)
    {
        if(temp->counter>0)
        {
            --(temp->counter);
        }
        else
        {
            i=gpio_get_value(temp->pin);
            temp->counter=temp->interval;
            
            if(temp->level!=i)
            {
                temp->level=i;
                if(temp->cooldown>0)
                {
                    temp->counter=temp->cooldown;
                }
                gpio_event_handler(temp->pin,temp->level);
            }
        }
        
        temp=temp->next;
    }
}
/*---------------------------------------------------------------------------*
 * GPIO poll event handler
 * Called for every gpio event during gpio poll
 *---------------------------------------------------------------------------*/
static void gpio_event_handler(int pin,int level)
{
    printf("[AirSniffer][Main]GPIO event: pin%d->%d\n",pin,level);
    
    switch(pin)
    {
        case GPIO_PIN_BAT_FULL:
            if(level==0)
            {
                battery_state|=BATTERY_STATE_FULL;
            }
            else
            {
                battery_state&=~BATTERY_STATE_FULL;
            }
            break;
#if PIN_VER==4
        case GPIO_PIN_BAT_CHARGE:
            if(level==0)
            {
                battery_state|=BATTERY_STATE_CHARGE;
            }
            else
            {
                battery_state&=~BATTERY_STATE_CHARGE;
            }
            break;
#endif
        case GPIO_PIN_BAT_LOW:
            if(level==1)
            {
                battery_state|=BATTERY_STATE_LOW;
            }
            else
            {
                battery_state&=~BATTERY_STATE_LOW;
            }
            break;
        case GPIO_PIN_BACK_BUTTON:
            if
            (
                (state==STATE_PM25)&&
                (level==0)
            )
            {
                state=STATE_WIFI_SETUP;
            }
            break;
        case GPIO_PIN_FRONT_BUTTON:
            if
            (
                (state!=STATE_WIFI_SETUP)&&
                (fb_pressing||fb_timer==0)
            )
            {
                if(fb_pressing==0&&level==0)
                {
                    //Pressed
                    fb_pressing=1;
                    fb_timer=FB_TIMER_LONG;
                }
                else if(fb_pressing==1&&level==1)
                {
                    //Released
                    fb_pressing=0;
                    //printf("[Debug]fb_timer=%d",fb_timer);
                    if(fb_timer>0)
                    {
                        //Short press
                        printf("[AirSniffer][Main]Front button short press\n");
                        fb_timer=0;
                        switch(state)
                        {
                            case STATE_PM25:
                                displaying_unit=
                                    (displaying_unit==UNIT_TYPE_PCS)?
                                    UNIT_TYPE_UG:UNIT_TYPE_PCS;
                                break;
                            case STATE_ID:
                                state=STATE_IP;
                                break;
                            case STATE_IP:
                                state=STATE_ID;
                                break;
                        }
                    }
                    else
                    {
                        //Long press
                        printf("[AirSniffer][Main]Front button long press\n");
                        switch(state)
                        {
                            case STATE_PM25:
                                state=STATE_ID;
                                break;
                            case STATE_ID:
                            case STATE_IP:
                                state=STATE_PM25;
                                break;
                        }
                    }
                    
                    fb_timer=FB_TIMER_CD;
                }
            }
            break;
        default:
            break; //What just happened?
    }
}
/*---------------------------------------------------------------------------*
 * Add/remove timer to timers
 *---------------------------------------------------------------------------*/
static int add_timer(int* timer)
{
    int i;
    for(i=0;i<TIMERS_SIZE;++i)
    {
        if(timers[i]==NULL||timers[i]==timer)
        {
            timers[i]=timer;
            return 0;
        }
    }
    return -1;
}
static void remove_timer(int* timer)
{
    int i;
    for(i=0;i<TIMERS_SIZE;++i)
    {
        if(timers[i]==timer)
        {
            timers[i]=NULL;
            return;
        }
    }
}
/*---------------------------------------------------------------------------*
 * SIGALRM handler
 * Ping children and restart them if died
 *---------------------------------------------------------------------------*/
static void main_sigalrm_handler(int signal)
{
    double raw;
    int i;
    
    if(state==STATE_PM25)
    {   
        //Sensor
        ++count;
        if(gpio_get_value(GPIO_PIN_SENSOR)==0)
        {
            ++sensor_low;
        }
        if(count>=SENSOR_CALC_INTERVAL)
        {
            raw=(double)sensor_low/count;
            
            count=0;
            sensor_low=0;
            
            printf("[AirSniffer][Main]New raw data: %f\n",raw);
            *new_data_ptr=raw;
            new_data=1;
        }
        
        //Spinner
        ++spinner_count;
        if(spinner_count>=SPINNER_FRAME_LEN)
        {
            spinner_count=0;
            spinner_frame=(spinner_frame+1)%SPINNER_FRAME_COUNT;
        }
    }
    
    //Timer
    for(i=0;i<TIMERS_SIZE;++i)
    {
        if(timers[i]&&(*(timers[i])>0))
        {
            --*(timers[i]);
        }
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
    FILE* file;
    
    printf("[AirSniffer][Main]Start wifi setup\n");
    
    display(DISPLAY_TYPE_PLEASE_WAIT,0);
    
    //Write config file for hostapd
    get_config(CONFIG_KEY_DEVICE_ID,buff);
    buff[8]='\0';
    file=fopen(HOSTAPD_CONFIG_FILE,"w");
    fprintf(file,"interface=" STA_DEV "\n"); //Auto concat
    fprintf(file,"driver=nl80211\n");
    fprintf(file,"ssid=AirS_%s\n",buff);
    fprintf(file,"channel=0\n");
    fprintf(file,"hw_mode=g\n");
    fprintf(file,"ignore_broadcast_ssid=0\n");
    fprintf(file,"auth_algs=3\n");
    fprintf(file,"wpa=3\n");
    fprintf(file,"wpa_passphrase=12345678\n"); //Maybe should change from time to time
    fprintf(file,"wpa_key_mgmt=WPA-PSK\n");
    fprintf(file,"wpa_pairwise=CCMP\n");
    fprintf(file,"rsn_pairwise=CCMP\n");
    fclose(file);
    
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
    sleep(1);
    system("udhcpd " DHCPD_CONFIG); //Auto concat
    
    while(system("pgrep udhcpd")!=0)
    {
        fprintf(stderr,"[AirSniffer][Main][WiFi Setup]dhcpd didn't start, restart\n");
        system("udhcpd " DHCPD_CONFIG); //Auto concat
        sleep(1);
    }
    
    //Assume httpd is working
    
    display(DISPLAY_TYPE_PLEASE_SETUP,0);
    
    printf("[AirSniffer][Main]AP ready\n");
    
#ifdef USE_WATCHDOG
    write(watchdog,"V",1);
    close(watchdog);
    watchdog=0;
#endif
    
    
    //Ceate FIFO for finishing signal
    if(access(SIGNAL_FIFO,F_OK)!=0)
    {
        if(mkfifo(SIGNAL_FIFO,0666)!=0)
        {
            fprintf(stderr,"[AirSniffer][Main]Can't make FIFO when starting wifi setup\n\t%s\n",strerror(errno));
            ret=-4;
            goto exit;
        }
    }
    
    //Open FIFO
    fd=open(SIGNAL_FIFO,O_RDONLY|O_NONBLOCK);
    if(fcntl(fd,F_SETFL,0)<0)
    {
        fprintf(stderr,"[AirSniffer][Main]fcntl() error\n\t%s\n",strerror(errno));
        ret=-3;
        goto exit_close_fd;
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
        ret=-2;
        goto exit_close_fd;
    }
    
    //Read FIFO
    buff[read(fd,buff,sizeof(buff))]='\0';
    
    printf("[AirSniffer][Main]WiFi setup info received: %s\n",buff);
    
    system("killall hostapd");
    system("killall udhcpd");
    system("wpa_supplicant -B -i " STA_DEV " -c " WPA_CONFIG); //Auto concat
    //Call wifi_setup_agent
    if(system(buff)!=0)
    {
        //Setup fail
        fprintf(stderr,"[AirSniffer][Main]wifi_setup_agent error\n");
        
        display(DISPLAY_TYPE_SETUP_FAIL,0);
        
        ret=-1;
    }
    else
    {
        //Setup success
        printf("[AirSniffer][Main]WiFi setup success\n");
        system("udhcpc -i " STA_DEV " -s /etc/udhcpc.script");
        
        display(DISPLAY_TYPE_SETUP_SUCC,0);
        
        ret=0;
    }
    
    //Close FIFO
    close(fd);
    
    sleep(5);
    goto succ;

exit_close_fd:
    close(fd);
exit:
    system("killall hostapd");
    system("killall udhcpd");
    system("wpa_supplicant -B -i " STA_DEV " -c " WPA_CONFIG); //Auto concat

    //system("ifconfig " AP_DEV " down"); //Auto concat
succ:
#ifdef USE_WATCHDOG
    watchdog=open(WATCHDOG_DEV,O_WRONLY);
    if(watchdog<=0)
    {
        _panic_=1;
        fprintf(stderr,"[AirSniffer][Main]Can't open watchdog\nPanic & Exit\n");
        ret=-1;
    }
    feed_dog();
#endif
    
    return ret;
}
/*---------------------------------------------------------------------------*
 * Data convert
 *---------------------------------------------------------------------------*/
static int data_convert(double raw,int unit_type)
{
    double ret;
    
    switch(unit_type)
    {
        case UNIT_TYPE_PCS:
            ret=raw*60000;
            break;
        case UNIT_TYPE_UG:
            ret=raw*3000;
            break;
        default:
            ret=0;
            break;
    }
    
    return (int)ret;
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
    static int current_is_data=0;
    static int current_has_temp=0;
    
    struct itimerval itv;
    
    stop_timer(&itv);
    
    switch(type)
    {
        case DISPLAY_TYPE_PLEASE_WAIT:
            current_is_data=0;
            display_upper(&image_please_wait);
            break;
        case DISPLAY_TYPE_UNIT:
            display_unit(displaying_unit);
            break;
        case DISPLAY_TYPE_DATA:
            if(!current_is_data)
            {
                display_upper(&image_upper_blank);
                display_unit(displaying_unit);
                current_is_data=1;
            }
            display_data(data);
            break;
        case DISPLAY_TYPE_PLEASE_SETUP:
            current_is_data=0;
            display_upper(&image_please_setup);
            break;
        case DISPLAY_TYPE_SETUP_SUCC:
            current_is_data=0;
            display_upper(&image_setup_success);
            break;
        case DISPLAY_TYPE_SETUP_FAIL:
            current_is_data=0;
            display_upper(&image_setup_fail);
            break;
        case DISPLAY_TYPE_BATTERY:
            display_battery((char)data);
            break;
        case DISPLAY_TYPE_WIFI_CONN:
            display_net(data);
            break;
        case DISPLAY_TYPE_TEMP:
            if(data!=TEMP_NO_DEVICE)
            {
                if(current_has_temp==0)
                {
                    display_temp_bg();
                    current_has_temp=1;
                }
                display_temp(data);
            }
            else
            {
                if(current_has_temp)
                {
                    display_temp_blank();
                    current_has_temp=0;
                }
            }
            break;
        case DISPLAY_TYPE_SPINNER:
            display_spinner(data);
            break;
        case DISPLAY_TYPE_ID:
            display_upper(&image_upper_blank);
            display_upper(&image_id_title);
            get_config(CONFIG_KEY_DEVICE_ID,buff);
            display_id(buff);
            break;
        case DISPLAY_TYPE_IP:
            display_upper(&image_upper_blank);
            display_upper(&image_ip_title);
            read_ip(buff);
            display_ip(buff);
            break; 
        default:
            break;
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
    
    //Battery
#if PIN_VER==4
    gpio_request(GPIO_PIN_BAT_CHARGE,NULL);
    gpio_direction_input(GPIO_PIN_BAT_CHARGE);
    add_gpio(GPIO_PIN_BAT_CHARGE,2000,0);
#endif
    gpio_request(GPIO_PIN_BAT_LOW,NULL);
    gpio_direction_input(GPIO_PIN_BAT_LOW);
    add_gpio(GPIO_PIN_BAT_LOW,2000,0);
    gpio_request(GPIO_PIN_BAT_FULL,NULL);
    gpio_direction_input(GPIO_PIN_BAT_FULL);
    add_gpio(GPIO_PIN_BAT_FULL,2000,0);
    
    //Back button
    gpio_request(GPIO_PIN_BACK_BUTTON,NULL);
    gpio_direction_input(GPIO_PIN_BACK_BUTTON);
    add_gpio(GPIO_PIN_BACK_BUTTON,10,1000);

    //Front button
    gpio_request(GPIO_PIN_FRONT_BUTTON,NULL);
    gpio_direction_input(GPIO_PIN_FRONT_BUTTON);
    add_gpio(GPIO_PIN_FRONT_BUTTON,10,0);
}
/*---------------------------------------------------------------------------*
 * SIGINT handler
 *---------------------------------------------------------------------------*/
void sigint_handler()
{
    struct itimerval itv;
    
    stop_timer(&itv);
    
    fprintf(stderr,"\nSIGINT!\ndebug=%d\n",debug);
    
    exit(1);
}
/*---------------------------------------------------------------------------*
 * Read temperature
 *---------------------------------------------------------------------------*/
int read_temp()
{
    FILE* f;
    int t,r,s=1;
    
    //return rand()%40;
    
    f=popen("read_temp.sh","r");
    if(f==NULL)
    {
        fprintf(stderr,"[AirSniffer][Main]Error read temperature\n");
        return TEMP_NO_DEVICE;
    }
    fgets(buff,sizeof(buff),f);
    pclose(f);
    
    if(strcmp(buff,"No device")==0)
    {
        //No device
        printf("[AirSniffer][Main]Temperature: No device\n");
        return TEMP_NO_DEVICE;
    }
    
    t=atoi(buff);
    if(t<0)
    {
        t=-t;
        s=-1;
    }
    t/=100;
    r=t%10;
    t/=10;
    if(r>=5)
    {
        ++t;
    }
    t*=s;
    printf("[AirSniffer][Main]Read temperature: %d\n",t);
    return t;
}
/*---------------------------------------------------------------------------*
 * Read ip addr
 *---------------------------------------------------------------------------*/
void read_ip(char* buf)
{
    FILE* f;
    
    buf[0]='\0';
    f=popen("ifconfig " STA_DEV " | awk '/inet addr/{print substr($2,6)}'","r");
    if(f==NULL)
    {
        fprintf(stderr,"[AirSniffer][Main]Error read temperature\n");
        return TEMP_NO_DEVICE;
    }
    fgets(buf,128,f);
    pclose(f);
    
    if(buf[strlen(buf)-1]=='\n')
    {
        buf[strlen(buf)-1]='\0';
    }
    
    printf("[AirSniffer][Main]Read ip: %s\n",buf);
}
/*---------------------------------------------------------------------------*
 * Main
 *---------------------------------------------------------------------------*/
int main(int argc,char* argv[])
{
    FILE* file;
    char feed_id[128];
    char api_key[128];
    char c;
    
    struct sigaction act,old;
    
    int flag;
    int fd;
    int ret;
    int i,t;
    double raw;
    double current_display_data=-1;
    int current_displaying_unit;
    int current_state;
    int converted;
    int last_battery_state;
    struct data_points* data;
    struct xively_data_points* xively_data;
    struct itimerval itv;
    int last_spinner_frame;
    
    printf("[AirSniffer][Main]Start\n");
    
#ifdef USE_WATCHDOG
    watchdog=open(WATCHDOG_DEV,O_WRONLY);
    if(watchdog<=0)
    {
        _panic_=1;
        fprintf(stderr,"[AirSniffer][Main]Can't open watchdog\nPanic & Exit\n");
        goto panic;
    }
    feed_dog();
#endif
    
    //Don't panic, for now
    _panic_=0;
    
    //Set SIGINT handler
    signal(SIGINT,sigint_handler);
    
    //Configure GPIOs
    gpio_init();
    
    //Set SIGALRM handler
    act.sa_handler=main_sigalrm_handler;
    act.sa_flags=SA_RESTART;
    sigaction(SIGALRM,&act,NULL);
    
    //Read configs from file
    if(read_config(CONFIG_FILE)<0)
    {
        fprintf(stderr,"[AirSniffer][Main]Error reading config\nPanic & Exit\n");
        goto panic;
    }
    
    //Add default configs to old version config file
    flag=0;
    
    get_config(CONFIG_KEY_UPLOAD,buff);
    if(buff[0]=='\0')
    {
        set_config(CONFIG_KEY_UPLOAD,"xively");
        flag=1;
    }
    get_config(CONFIG_KEY_TELNET,buff);
    if(buff[0]=='\0')
    {
        set_config(CONFIG_KEY_TELNET,"n");
        flag=1;
    }
    get_config(CONFIG_KEY_UNIT,buff);
    if(buff[0]=='\0')
    {
        set_config(CONFIG_KEY_UNIT,"pcs");
        flag=1;
    }
    get_config(CONFIG_KEY_CALIBRATION_M,buff);
    if(buff[0]=='\0')
    {
        set_config(CONFIG_KEY_CALIBRATION_M,"1");
        flag=1;
    }
    get_config(CONFIG_KEY_CALIBRATION_A,buff);
    if(buff[0]=='\0')
    {
        set_config(CONFIG_KEY_CALIBRATION_A,"0");
        flag=1;
    }
    
    //Save config to file
    if(flag)
    {
        dump_config(CONFIG_FILE);
    }
    
    //Read display unit
    get_config(CONFIG_KEY_UNIT,buff);
    if(strcmp(buff,"pcs")==0)
    {
        displaying_unit=UNIT_TYPE_PCS;
    }
    else if(strcmp(buff,"ug")==0)
    {
        displaying_unit=UNIT_TYPE_UG;
    }
    
    //Read calibration
    get_config(CONFIG_KEY_CALIBRATION_M,buff);
    calibration_m=atof(buff);
    if(calibration_m<=0)
    {
        calibration_m=1;
    }
    get_config(CONFIG_KEY_CALIBRATION_A,buff);
    calibration_a=atof(buff);
    
    display(DISPLAY_TYPE_PLEASE_WAIT,0);
    
#if PIN_VER==5
    t=read_temp();
    display(DISPLAY_TYPE_TEMP,t);
#endif

    last_battery_state=-1;
    last_spinner_frame=-1;
    current_displaying_unit=displaying_unit;
    current_state=state;
    
    add_timer(&fb_timer);
    
    //Start timer
    start_timer(NULL);
    
    while(!_panic_)
    {
#ifdef USE_WATCHDOG
        feed_dog();
#endif
        debug%=100;
        
        //Switch state
        if(current_state!=state)
        {
            stop_timer(NULL);
            
            printf("[AirSniffer][Main]State change %d->%d\n",current_state,state);
            
            switch(state)
            {
                case STATE_PM25:
                    display(DISPLAY_TYPE_PLEASE_WAIT,0);
                    break;
                case STATE_WIFI_SETUP:
                    //Clear data spinner battery wifi
                    count=0;
                    sensor_low=0;
                    current_display_data=-1;
                    
                    display(DISPLAY_TYPE_SPINNER,-1);
                    spinner_count=0;
                    spinner_frame=0;
                    last_spinner_frame=-1;
                    
                    display(DISPLAY_TYPE_BATTERY,'b');
                    last_battery_state=-1;
                    
                    display(DISPLAY_TYPE_WIFI_CONN,-1);
                    
                    wifi_setup();
                    
                    display(DISPLAY_TYPE_PLEASE_WAIT,0);
                    
                    state=STATE_PM25;
                    break;
                case STATE_ID:
                    if(current_state==STATE_PM25)
                    {
                        //Clear data and spinner
                        count=0;
                        sensor_low=0;
                        current_display_data=-1;
                        
                        display(DISPLAY_TYPE_SPINNER,-1);
                        spinner_count=0;
                        spinner_frame=0;
                        last_spinner_frame=-1;
                        
                        display(DISPLAY_TYPE_WIFI_CONN,-1);
                    }
                    
                    display(DISPLAY_TYPE_ID,0);
                    break;
                case STATE_IP:
                    display(DISPLAY_TYPE_IP,0);
                    break;
            }
            
            current_state=state;
            start_timer(NULL);
        }
        
        if(fb_pressing&&!fb_timer)
        {
            //Timer expires
            gpio_event_handler(GPIO_PIN_FRONT_BUTTON,1);
        }
        
        //Spinner
        if(state==STATE_PM25&&last_spinner_frame!=spinner_frame)
        {
            last_spinner_frame=spinner_frame;
            display(DISPLAY_TYPE_SPINNER,spinner_frame);
        }
        
        //Battery State Task
        if(last_battery_state!=battery_state)
        {
            debug+=100;
            
            if(battery_state==0)
            {
                display(DISPLAY_TYPE_BATTERY,'h');
            }
            else if(battery_state&BATTERY_STATE_FULL)
            {
                display(DISPLAY_TYPE_BATTERY,'f');
            }
            else if(battery_state&BATTERY_STATE_CHARGE)
            {
                display(DISPLAY_TYPE_BATTERY,'c');
            }
            else if(battery_state&BATTERY_STATE_LOW)
            {
                display(DISPLAY_TYPE_BATTERY,'l');
            }
            
            last_battery_state=battery_state;
        }
        
        //New Data
        if(state==STATE_PM25&&new_data)
        {
            debug+=200;
            
            new_data=0;
            
#ifdef USE_WATCHDOG
            feed_dog();
#endif
            
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
            
            //Apply calibration
            raw*=calibration_m;
            raw+=calibration_a;
            
            converted=data_convert(raw,current_displaying_unit);
            current_display_data=raw;
            
#if PIN_VER==5
            //Read temperature
            stop_timer(NULL);
            t=read_temp();
            display(DISPLAY_TYPE_TEMP,t);
            start_timer(NULL);
#endif
            
            display(DISPLAY_TYPE_DATA,converted);
            
            if(++data_send_counter>=DATA_SEND_INTERVAL)
            {
                data_send_counter=0;
                
                printf("[AirSniffer][Main]Sending new data\n");
                
                //Stop timer
                stop_timer(NULL);
                
                debug+=800;
                
                get_config(CONFIG_KEY_UPLOAD,buff);
                if(strcmp(buff,"none")!=0)
                {
                    if(strcmp(buff,"aliyun")==0)
                    {
                        sprintf(buff,"%10f",raw);
                        data=append_data_point(NULL,KEY_RAW_DATA,buff);
                        if(t!=TEMP_NO_DEVICE)
                        {
                            sprintf(buff,"%10d",t);
                            data=append_data_point(data,KEY_TEMP,buff);
                        }
                        get_config(CONFIG_KEY_DEVICE_ID,buff);
                        ret=upload(buff,data);
                        free_data_points(data);
                    }
                    else
                    {
                        sprintf(buff,"%10f",raw);
                        xively_data=xively_append_data_point(NULL,KEY_RAW_DATA,buff);
                        if(t!=TEMP_NO_DEVICE)
                        {
                            sprintf(buff,"%10d",t);
                            xively_data=xively_append_data_point(xively_data,KEY_TEMP,buff);
                        }
                        get_config(CONFIG_KEY_FEED_ID,feed_id);
                        get_config(CONFIG_KEY_API_KEY,api_key);
                        ret=xively_update_feed(feed_id,api_key,xively_data);
                        xively_free_data_points(xively_data);
                    }

                    if(ret!=0)
                    {
                        //Upload failed
                        printf("[AirSniffer][Main]Disconnected\n");
                        display(DISPLAY_TYPE_WIFI_CONN,0);
                    }
                    else
                    {
                        //Upload success
                        printf("[AirSniffer][Main]New data sent\n");
                        printf("[AirSniffer][Main]Connected\n");
                        display(DISPLAY_TYPE_WIFI_CONN,1);
                    }
                }
                
                //Restart timer
                start_timer(NULL);
            }
        }
        
        //Change unit
        if
        (
            state==STATE_PM25&&
            displaying_unit!=current_displaying_unit
        )
        {
            if(current_display_data>0)
            {
                current_displaying_unit=displaying_unit;
                stop_timer(NULL);
                display(DISPLAY_TYPE_UNIT,0);
                converted=data_convert(current_display_data,displaying_unit);
                display(DISPLAY_TYPE_DATA,converted);
                sprintf(buff,(displaying_unit==UNIT_TYPE_PCS)? "pcs":"ug");
                set_config(CONFIG_KEY_UNIT,buff);
                dump_config(CONFIG_FILE);
                start_timer(NULL);
            }
            else
            {
                displaying_unit=current_displaying_unit;
            }
        }
        
        debug+=400;
        
        //Make sure timer is running
        while(1)
        {
            getitimer(ITIMER_REAL,&itv);
            
            if
            (
                itv.it_interval.tv_sec==0&&
                itv.it_interval.tv_usec==0&&
                itv.it_value.tv_sec==0&&
                itv.it_value.tv_usec==0
            )
            {
                fprintf(stderr,"[AirSniffer][Main]timer==0, try to reset timer\n");
                start_timer(NULL);
            }
            else
            {
                break;
            }
        }
        
        pause();
    }
    
panic:
    //Stop timer
    stop_timer(NULL);
    
    //Free configs
    free_config();
    
    printf("[AirSniffer][Main]Exit\n");
    
    return -1; //Panic & Exit
}