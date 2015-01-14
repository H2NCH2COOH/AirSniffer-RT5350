#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "img.h"

/*---------------------------------------------------------------------------*/
//Image define
struct image image_welcome={"/imgs/welcome/welcome.img",LCD_W,LCD_H,NULL};
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
    {"/imgs/numbers/9.img",NUM_WIDTH,NUM_HEIGHT,NULL},
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
struct image image_temp_blank={"/imgs/temp/blank.img",TEMP_WIDTH,TEMP_HEIGHT,NULL};

struct image image_please_wait={"/imgs/upper/please_wait.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_data_bg={"/imgs/upper/data_bg.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_please_setup={"/imgs/upper/please_setup.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_setup_fail={"/imgs/upper/setup_fail.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_setup_success={"/imgs/upper/setup_success.img",UPPER_WIDTH,UPPER_HEIGHT,NULL};

struct image image_spinner[SPINNER_FRAME_COUNT]=
{
    {"/imgs/spinner/0.img",SPINNER_WIDTH,SPINNER_HEIGHT,NULL},
    {"/imgs/spinner/1.img",SPINNER_WIDTH,SPINNER_HEIGHT,NULL},
    {"/imgs/spinner/2.img",SPINNER_WIDTH,SPINNER_HEIGHT,NULL},
    {"/imgs/spinner/3.img",SPINNER_WIDTH,SPINNER_HEIGHT,NULL},
    {"/imgs/spinner/4.img",SPINNER_WIDTH,SPINNER_HEIGHT,NULL},
    {"/imgs/spinner/5.img",SPINNER_WIDTH,SPINNER_HEIGHT,NULL},
};
struct image image_spinner_blank={"/imgs/spinner/blank.img",SPINNER_WIDTH,SPINNER_HEIGHT,NULL};
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
    if(img&&img->img)
    {
        free(img->img);
        img->img=NULL;
    }
}

int get_pixel(struct image* img,unsigned int x,unsigned int y,struct pixel* p)
{
    size_t i,s;
    
    if(img->img==NULL)
    {
        fprintf(stderr,"[AirSniffer][img]get_pixel(): Image is NULL\n");
        return -1;
    }
    
    if(x>=img->width||y>=img->height)
    {
        fprintf(stderr,"[AirSniffer][img]get_pixel(): Index out of image\n");
        return -1;
    }
    
    s=BPP*(x+img->width*y);
    for(i=0;i<BPP;++i)
    {
        p->byte[i]=img->img[s+i];
    }
    
    return 0;
}

int set_pixel(struct image* img,unsigned int x,unsigned int y,struct pixel* p)
{
    size_t i,s;
    
    if(img->img==NULL)
    {
        fprintf(stderr,"[AirSniffer][img]set_pixel(): Image is NULL\n");
        return -1;
    }
    
    if(x>=img->width||y>=img->height)
    {
        fprintf(stderr,"[AirSniffer][img]set_pixel(): Index out of image\n");
        return -1;
    }
    
    s=BPP*(x+img->width*y);
    for(i=0;i<BPP;++i)
    {
        img->img[s+i]=p->byte[i];
    }
    
    return 0;
}

static unsigned int round_div(unsigned int dividend, unsigned int divisor)
{
    return (dividend+(divisor/2))/divisor;
}

int resize_image(struct image* src,struct image* dst)
{
    unsigned int x,y;
    unsigned int sx,sy;
    size_t s;
    struct pixel p;
    
    if(src->img==NULL)
    {
        fprintf(stderr,"[AirSniffer][img]resize_image(): Source image is NULL\n");
        return -1;
    }
    
    free_image(dst);
    s=BPP*(dst->width)*(dst->height);
    dst->img=(unsigned char*)malloc(s);
    
    for(y=0;y<dst->height;++y)
    {
        sy=round_div(y*src->height,dst->height);
        for(x=0;x<dst->width;++x)
        {
            sx=round_div(x*src->width,dst->width);
            get_pixel(src,sx,sy,&p);
            set_pixel(dst,x,y,&p);
        }
    }
    
    return 0;
}
