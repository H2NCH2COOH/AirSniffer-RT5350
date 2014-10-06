#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
//#include <ugpio/ugpio.h>
#include "lcd.h"
#include "datalcd.h"

#include "gpio_pin.h"

#define DEVPATH "/dev/spidev25.0"

RegParas InitDataPos[]={
{0,3},{3,3},{6,6},{12,1},{13,3},{16,1},{17,2}, //
{19,2},{21,2},{23,1},{24,1},{25,1},{26,1}
};
const u8 InitData[]={
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




u16 BACK_COLOR=WHITE;    //背景色
u16 POINT_COLOR=BLACK;   //画笔色	 
 
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
void LCD_WR_DATA_FOR_REG_INIT(RegParaPos pos)
{
	
	uwrite(InitData+InitDataPos[pos].offset,InitDataPos[pos].len);
}

void LCD_WR_REG(u8 da)	 
{
	
    gpio_set_value(GPIO_PIN_LED_A0,0);
	uwrite(&da,1);
	gpio_set_value(GPIO_PIN_LED_A0,1);
}

void LCD_WR16(u16 da)
{
	char tmp[2], *buf;
	buf=(char* )&da;
	tmp[1]=*buf;
	tmp[0]=*(buf+1);
	uwrite(tmp, 2);
}


void Address_set(u8 xsta,u8 ysta,u8 xend,u8 yend)
{  
	static struct NodeAddress {
		u8 starthigh;
		u8 startlow;
		u8 endhigh;
		u8 endlow
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


void LCD_Init(void)
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
void LCD_Clear(u16 Color)
{
	LCD_Fill(0,0,LCD_W-1,LCD_H-1,Color);
}


//画点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(u16 x,u16 y)
{
	Address_set(x,y,x,y);//设置光标位置 
	LCD_WR16(POINT_COLOR); 	    
} 	 
//画一个大点
//POINT_COLOR:此点的颜色
void LCD_DrawPoint_big(u16 x,u16 y)
{
	LCD_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);
} 
static void LCD_BlockWrite(const u8 *buffer, size_t nbyte)
{
	const u8 *ptr=buffer+nbyte-900;
	for(;buffer<ptr;buffer+=900)
	{
		uwrite(buffer,900);
	}
	uwrite(buffer,ptr-buffer+900);
}
static void LCD_Set_Color_For_Fill(u8 *buffer, size_t nbyte, u16 color)
{
	if(color==0xffff)
	{
		memset(buffer,0xff,nbyte);
		return;
	}
	u8 *ptr1=(u8 *)&color, *ptr2=buffer+nbyte;
	u8 b=*(ptr1++), a=*ptr1;
	for(;buffer<ptr2;)
	{
		*(buffer++)=a;
		*(buffer++)=b;
	}
}
//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(u8 xsta,u8 ysta,u8 xend,u8 yend,u16 color)
{          
	u8 *buffer; 
	size_t nbyte=BPP*(xend-xsta+1)*(yend-ysta+1);
	Address_set(xsta,ysta,xend,yend);
	
	buffer = (u8 *) malloc (nbyte);
	if (buffer==NULL) {
        fprintf(stderr,"[LCD]Failed malloc buffer of size %ld\n\tFatal Error Exit\n",nbyte);
        exit(1);
    }
	LCD_Set_Color_For_Fill(buffer,nbyte,color);
	LCD_BlockWrite(buffer,nbyte);
	free(buffer);
}
void LCD_ShowImageRaw(u8 xsta, u8 ysta, u8 xlen, u8 ylen, const u8 *image)
{
	// check validation, to be fulfilled
	// flow control
	// write
	// for security, don't write too much one time
	// might need return errno
	u8 xend=xsta+xlen-1, yend=ysta+ylen-1;
	if(xend>LCD_W-1 || yend>LCD_H-1 || image==NULL) {
        fprintf(stderr,"[LCD]LCD_ShowImageRaw(): Illegal arg(s), return\n");
        return;
    }
	size_t nbyte=BPP*xlen*ylen;
	Address_set(xsta,ysta,xend,yend);
	LCD_BlockWrite(image,nbyte);
}

void swap(u16 *x, u16 *y)
{
	*x^=*y;
	*y^=*x;
	*x^=*y;
}

//画线
//x0,y0:起点坐标
//x1,y1:终点坐标  
void LCD_DrawLine(u16 x0, u16 y0, u16 x1, u16 y1)
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
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void Draw_Circle(u16 x0,u16 y0,u8 r)
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





void show_unit() {
	LCD_ShowImageRaw(UXSTA,UYSTA,UNITWIDTH,UNITHEIGHT,gImage_unit);
}

void display_welcome() {
	LCD_ShowImageRaw(0,0,LCD_W,LCD_H,gImage_boot);
}

void display_battery(char state) {
	const u8 *ptr;
    int i;
    switch(state) {
        case 'f':
            i=0;
            break;
        case 'l':
            i=1;
            break;
        case 'c':
            i=2;
            break;
        case 'n':
            i=3;
            break;
    }
    
    ptr=gImage_battery[i];
    LCD_ShowImageRaw(LCD_W-CHARGEWIDTH,LCD_H-CHARGEHEIGHT,CHARGEWIDTH,CHARGEHEIGHT,ptr);
}

static void shownumRaw(int num,int bit) {
	if(num<0 || num>10 || bit<0 || bit>=NMAXBIT) {
        fprintf(stderr,"[LCD]shownumRaw(): Illegal arg(s), return\n");
        return;
    }
	LCD_ShowImageRaw(NXSTA-bit*NUMWIDTH, NYSTA, NUMWIDTH, NUMHEIGHT, gImage_numchars[num]);
}

void display_nums(int num) {
	int bit, token=0, nlist[3];
    
    if(num>=100000)
    {
        num=99999;
    }
    else if(num<0)
    {
        num=0;
    }
    
	for(bit=0;bit<NMAXBIT;bit++) {
		nlist[bit]=num%10;
		num/=10;
	}
	for(bit=NMAXBIT-1;bit>=0;bit--) {
        if(nlist[bit]||token||bit==0) {
            token=1;
            shownumRaw(nlist[bit],bit);
        }
        else {
            shownumRaw(10,bit);
        }
	}
}

void display_net_conn(int wifion) {
		LCD_ShowImageRaw(LCD_W/2,LCD_H-NETHEIGHT,NETHEIGHT,NETHEIGHT,(wifion) ? gImage_net30[0] : gImage_net30[1]);
}


static void display_screenraw(int k, int modeupdown)
{
	const u8 *ptr1, *ptr2;
	if(modeupdown) {
		ptr1=gImage_div_upper[k];
		ptr2=gImage_div_lower[k];
	}
	else {
		//ptr1=gImage_div_left[k];
		//ptr2=gImage_div_right[k];
	}
	
	LCD_ShowImageRaw(0,0,DIVWIDTH_1,DIVHEIGHT_1,ptr1);
	LCD_ShowImageRaw(DIV2XSTA,DIV2YSTA,DIVWIDTH_2,DIVHEIGHT_2,ptr2);
}
void display_screen(int k)
{
	display_screenraw(k, MODEDIVUPDOWN);
}


void display_init()
{
	LCD_Init();
}

void display_data(int num)
{
	display_nums(num);
}
