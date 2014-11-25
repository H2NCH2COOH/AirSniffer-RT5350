#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
//#include <ugpio/ugpio.h>
#include "lcd.h"
#include "img.h"

#include "gpio_pin.h"

#define DEVPATH "/dev/spidev25.0"

RegParas InitDataPos[]={
{0,3},{3,3},{6,6},{12,1},{13,3},{16,1},{17,2}, //
{19,2},{21,2},{23,1},{24,1},{25,1},{26,1}
};
const uint8_t InitData[]={
0x05,0x3c,0x3c, //0xb1
0x05,0x3c,0x3c, //0xb2
0x05,0x3c,0x3c,0x05,0x3c,0x3c, //0xb3
0x03, //0xb4
0x28,0x08,0x04, //0xc0
0xc0, //0xc1
0x0d,0x00, //0xc2
0x8d,0x2a, //0xc3
0x8d,0xee, //0xc4
0x1a, //0xc5
0xa0, //0x36 Memory Access Control
0x05, //0x3a
0x01, //0x35 TE ON Mode
};

uint16_t BACK_COLOR=WHITE;    //背景色
uint16_t POINT_COLOR=BLACK;   //画笔色
 
static void uwrite(const void * buf, size_t nbyte) {
	if(buf==NULL || nbyte==0)
		return;
	int fd;
	if((fd=open(DEVPATH,O_WRONLY))<0) {
		fprintf(stderr,"[LCD]Can't open dev file: " DEVPATH "\n\tFatal Error Exit\n");
		exit(1);
	}
	write(fd,buf,nbyte);
	close(fd);
}
static void LCD_WR_DATA_FOR_REG_INIT(RegParaPos pos)
{
	
	uwrite(InitData+InitDataPos[pos].offset,InitDataPos[pos].len);
}

static void LCD_WR_REG(uint8_t da)	 
{
	
    gpio_set_value(GPIO_PIN_LED_A0,0);
	uwrite(&da,1);
	gpio_set_value(GPIO_PIN_LED_A0,1);
}

static void LCD_WR16(uint16_t da)
{
	char tmp[2], *buf;
	buf=(char* )&da;
	tmp[1]=*buf;
	tmp[0]=*(buf+1);
	uwrite(tmp, 2);
}


static void Address_set(uint8_t xsta,uint8_t ysta,uint8_t xend,uint8_t yend)
{  
	static struct NodeAddress {
		uint8_t starthigh;
		uint8_t startlow;
		uint8_t endhigh;
		uint8_t endlow
	} address={0,0,0,0};
	address.startlow=xsta;
	address.endlow=xend;
	LCD_WR_REG(0x2a);
	uwrite(&address,4);

	address.startlow=ysta;
	address.endlow=yend;
	LCD_WR_REG(0x2b);
	uwrite(&address,4);
	
	LCD_WR_REG(0x2C);

}


static void LCD_Init(void)
{

	gpio_request(GPIO_PIN_LED_A0,NULL);
	gpio_direction_output(GPIO_PIN_LED_A0,1);
    
    gpio_request(GPIO_PIN_LED_RESET,NULL);
	gpio_direction_output(GPIO_PIN_LED_RESET,1);
    
	gpio_set_value(GPIO_PIN_LED_RESET,0);
    usleep(1000);
    gpio_set_value(GPIO_PIN_LED_RESET,1);
    usleep(1000);
    
   	LCD_WR_REG(0x11); //Sleep out
	usleep(2000); //Delay
	//------------------------------------ST7735S Frame Rate-----------------------------------------//
	LCD_WR_REG(0xB1);
	LCD_WR_DATA_FOR_REG_INIT(POS_B1);
	LCD_WR_REG(0xB2);
	LCD_WR_DATA_FOR_REG_INIT(POS_B2);
	LCD_WR_REG(0xB3);
	LCD_WR_DATA_FOR_REG_INIT(POS_B3);
	//------------------------------------End ST7735S Frame Rate-----------------------------------------//
	LCD_WR_REG(0xB4); //Dot inversion
	LCD_WR_DATA_FOR_REG_INIT(POS_B4);
	LCD_WR_REG(0xC0); //?
	LCD_WR_DATA_FOR_REG_INIT(POS_C0);
	LCD_WR_REG(0xC1);
	LCD_WR_DATA_FOR_REG_INIT(POS_C1);
	LCD_WR_REG(0xC2);
	LCD_WR_DATA_FOR_REG_INIT(POS_C2);
	LCD_WR_REG(0xC3);
	LCD_WR_DATA_FOR_REG_INIT(POS_C3);
	LCD_WR_REG(0xC4);
	LCD_WR_DATA_FOR_REG_INIT(POS_C4);
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
	LCD_WR_REG(0xC5); //VCOM
	LCD_WR_DATA_FOR_REG_INIT(POS_C5);
	LCD_WR_REG(0x36); //MX, MY, MV, RGB mode
	LCD_WR_DATA_FOR_REG_INIT(POS_36);
	
	
	LCD_WR_REG(0x3A); //65k mode
	LCD_WR_DATA_FOR_REG_INIT(POS_3A);
	
	LCD_WR_REG(0x35); //TE
	LCD_WR_DATA_FOR_REG_INIT(POS_35);

	LCD_WR_REG(0x29); //Display on
	LCD_WR_REG(0x2C);



}
//清屏函数
//Color:要清屏的填充色
void LCD_Clear(uint16_t Color)
{
	LCD_Fill(0,0,LCD_W-1,LCD_H-1,Color);
}

//画点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	Address_set(x,y,x,y);//设置光标位置 
	LCD_WR16(POINT_COLOR); 	    
}
	 
//画一个大点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint_big(uint16_t x,uint16_t y)
{
	LCD_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);
}

static void LCD_BlockWrite(const uint8_t *buffer, size_t nbyte)
{
	const uint8_t *ptr=buffer+nbyte-900;
	for(;buffer<ptr;buffer+=900)
	{
		uwrite(buffer,900);
	}
	uwrite(buffer,ptr-buffer+900);
}

static void LCD_Set_Color_For_Fill(uint8_t *buffer, size_t nbyte, uint16_t color)
{
	if(color==0xffff)
	{
		memset(buffer,0xff,nbyte);
		return;
	}
	uint8_t *ptr1=(uint8_t *)&color, *ptr2=buffer+nbyte;
	uint8_t b=*(ptr1++), a=*ptr1;
	for(;buffer<ptr2;)
	{
		*(buffer++)=a;
		*(buffer++)=b;
	}
}

//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(uint8_t xsta,uint8_t ysta,uint8_t xend,uint8_t yend,uint16_t color)
{          
	uint8_t *buffer; 
	size_t nbyte=BPP*(xend-xsta+1)*(yend-ysta+1);
	Address_set(xsta,ysta,xend,yend);
	
	buffer = (uint8_t *) malloc (nbyte);
	if (buffer==NULL) {
        fprintf(stderr,"[LCD]Failed malloc buffer of size %ld\n\tFatal Error Exit\n",nbyte);
        exit(1);
    }
	LCD_Set_Color_For_Fill(buffer,nbyte,color);
	LCD_BlockWrite(buffer,nbyte);
	free(buffer);
}

void LCD_ShowImageRaw(uint8_t xsta, uint8_t ysta, uint8_t xlen, uint8_t ylen, const uint8_t *image)
{
	// check validation, to be fulfilled
	// flow control
	// write
	// for security, don't write too much one time
	// might need return errno
	uint8_t xend=xsta+xlen-1, yend=ysta+ylen-1;
	if(xend>LCD_W-1 || yend>LCD_H-1 || image==NULL) {
        fprintf(stderr,"[AirSniffer][LCD]LCD_ShowImageRaw(): Illegal arg(s)\n");
        return;
    }
	size_t nbyte=BPP*xlen*ylen;
	Address_set(xsta,ysta,xend,yend);
	LCD_BlockWrite(image,nbyte);
}

static void swap(uint16_t *x, uint16_t *y)
{
	*x^=*y;
	*y^=*x;
	*x^=*y;
}

//画线
//x0,y0:起点坐标
//x1,y1:终点坐标  
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	int dx= abs(x1-x0), sx=(x0<x1) ? 1 : -1;
	int dy=-abs(y1-y0), sy=(y0<y1) ? 1 : -1;
	int err=dx+dy, e2;
	
	for(;;)
	{
		LCD_DrawPoint(x0,y0);
		e2=2*err;
		if(e2>=dy)
		{
			if(x0==x1) break;
			err+=dy; x0+=sx;
		}
		if(e2<=dx)
		{
			if(y0==y1) break;
			err+=dx; y0+=sy;
		}
	}
}

//画矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a);             //3           
		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0-a,y0+b);             //1       
		LCD_DrawPoint(x0-b,y0-a);             //7           
		LCD_DrawPoint(x0-a,y0-b);             //2             
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-b,y0+a);             
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b);
	}
} 

/*---------------------------------------------------------------------------*/
static void display_img(struct image* img,int x,int y)
{
    load_image(img);
    if(img->img)
    {
        LCD_ShowImageRaw(x,y,img->width,img->height,img->img);
        free_image(img);
    }
}

static void display_int
(
    int num,
    int max_digit,
    int max,
    struct image* digits,
    struct image* blank,
    int x,
    int y
)
{
	int i;
    int d;
    int flag=0;
    int w;
    
    w=digits[0].width;
    
    if(num>max)
    {
        num=max;
    }
    else if(num<0) //TODO: add possible minus display
    {
        num=0;
    }
    
	for(i=0;i<max_digit;++i)
    {
		if(flag)
        {
            display_img(blank,x,y);
            x-=w;
        }
        else
        {
            d=num%10;
            num/=10;
            
            display_img(digits+d,x,y);
            x-=w;
            
            if(num==0)
            {
                flag=1;
            }
        }
	}
}

void display_init()
{
    LCD_Init();
}

void display_welcome()
{
    display_img(&image_welcome,0,0);
}

void display_battery(char state)
{
    struct image* img;
    
    switch(state)
    {
        case 'f':
            img=&image_bat_full;
            break;
        case 'l':
            img=&image_bat_low;
            break;
        case 'c':
            img=&image_bat_charge;
            break;
        case 'b':
            img=&image_bat_blank;
            break;
    }
    
    display_img(img,BAT_X_STA,BAT_Y_STA);
}

void display_data(int data)
{
    display_int
    (
        data,
        NUM_MAX_DIGIT,
        NUM_MAX,
        image_num,
        &image_num_blank,
        NUM_X_STA,
        NUM_Y_STA
    );
}

void display_unit()
{
    display_img(&image_unit,UNIT_X_STA,UNIT_Y_STA);
}

void display_net(int conn)
{
    struct image* img;
    switch(conn)
    {
        case 1:
            img=&image_net_connnected;
            break;
        case 0:
            img=&image_net_disconnnected;
            break;
    }
}

void display_upper(struct image* img)
{
    display_img(img,UPPER_X_STA,UPPER_Y_STA);
}

void display_temp(int temp)
{
    display_int
    (
        temp,
        TEMP_MAX_DIGIT,
        TEMP_MAX,
        image_temp_num,
        &image_temp_num_blank,
        TEMP_NUM_X_STA,
        TEMP_NUM_Y_STA
    );
}

void display_temp_bg()
{
    display_img(&image_temp_bg,TEMP_X_STA,TEMP_Y_STA);
}
