#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "img.h"

/*---------------------------------------------------------------------------*/
//Image define
extern struct image image_welcome={"/imgs/welcome/welcome.img",LCD_W,LCD_H,NULL};
struct image image_num_blank={"/imgs/numbers/blank.img",NUM_WIDTH,NUM_HEIGHT,NULL};
struct image image_num[10]=
{
    {"/imgs/numbers/0.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/1.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/2.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/3.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/4.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/5.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/6.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/7.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/8.img",NUM_WIDTH,NUM_HEIGHT,NULL},
    {"/imgs/numbers/9.img",NUM_WIDTH,NUM_HEIGHT,NULL}
};

struct image image_net_connnected={"/imgs/net/connected.img",NET_WIDTH,NET_HEIGHT,NULL};
struct image image_net_disconnnected={"/imgs/net/disconnected.img",NET_WIDTH,NET_HEIGHT,NULL};

struct image image_bat_full={"/imgs/battery/full.img",BAT_WIDTH,BAT_HEIGHT,NULL};
struct image image_bat_half={"/imgs/battery/half.img",BAT_WIDTH,BAT_HEIGHT,NULL};
struct image image_bat_low={"/imgs/battery/low.img",BAT_WIDTH,BAT_HEIGHT,NULL};
struct image image_bat_charge={"/imgs/battery/charge.img",BAT_WIDTH,BAT_HEIGHT,NULL};
struct image image_bat_blank={"/imgs/battery/blank.img",BAT_WIDTH,BAT_HEIGHT,NULL};

struct image image_unit={"/imgs/unit/unit.img",UNIT_WIDTH,UNIT_HEIGHT,NULL};

struct image image_temp_bg={"/imgs/temp/bg.img",TEMP_WIDTH,TEMP_HEIGHT,NULL};
struct image image_temp_num_blank={"/imgs/temp/blank.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL};
struct image image_temp_num[10]=
{
    {"/imgs/temp/0.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/1.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/2.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/3.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/4.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/5.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/6.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/7.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/8.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL},
    {"/imgs/temp/9.img",TEMP_NUM_WIDTH,TEMP_NUM_HEIGHT,NULL}
};

struct image image_please_wait={"/imgs/upper/please_wait.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_data_bg={"/imgs/upper/data_bg.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_please_setup={"/imgs/upper/please_setup.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_setup_fail={"/imgs/upper/setup_fail.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_setup_success={"/imgs/upper/setup_success.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};
/*---------------------------------------------------------------------------*/

int load_image(struct image* img)
{
    FILE* f;
    unsigned char* buff;
    size_t s;
    
    s=BPP*(img->width)*(img->height);
    f=fopen(img->file_name,"rb");
    if(f==NULL)
    {
        fprintf(stderr,"[AirSniffer][img]Can't open image file: %s\n",img->file_name);
        img->img=NULL;
        return -1;
    }
    
    buff=(unsigned char*)malloc(s);
    fread(buff,1,s,f);
    img->img=buff;
    fclose(f);
    
    return 0;
}

void free_image(struct image* img)
{
    if(img->img)
    {
        free(img->img);
        img->img=NULL;
    }
}
