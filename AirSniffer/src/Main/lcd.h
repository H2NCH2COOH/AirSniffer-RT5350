// interface lies just above color definition
#ifndef __LCD_H
#define __LCD_H		



#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long

//IO连接  
#define LCD_AO   10 	    //数据/命令切换		1 data, 0 command		
#define LCD_SDA  9 	    //数据										
#define LCD_SCK  8     //时钟	



typedef struct RegParameters{
	unsigned int offset;
	unsigned int len;
}RegParas;

typedef enum 
{
POS_B1=0,
POS_B2,
POS_B3,
POS_B4,
POS_C0,
POS_C1,
POS_C2,
POS_C3,
POS_C4,
POS_C5,
POS_36,
POS_3A,
POS_35,
}RegParaPos;


extern  u16 BACK_COLOR, POINT_COLOR;   //背景色，画笔色

void LCD_Init(void); 
void LCD_Clear(u16 Color);
void Address_set(u8 xsta,u8 ysta,u8 xend,u8 yend);
void LCD_WRITE8(u8 da);
void LCD_WR16(u16 da);
void LCD_WR_REG(u8 da);

void LCD_DrawPoint(u16 x,u16 y);//画点
void LCD_DrawPoint_big(u16 x,u16 y);//画一个大点
u16  LCD_ReadPoint(u16 x,u16 y); //读点
void Draw_Circle(u16 x0,u16 y0,u8 r);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawLine1(u16 x1, u16 y1, u16 x2, u16 y2);

void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);	   
void LCD_Fill(u8 xsta,u8 ysta,u8 xend,u8 yend,u16 color);


void LCD_ShowImageRaw(u8 xsta, u8 ysta, u8 xlen, u8 ylen, u8 *image);

void display_battery(char state);
void display_nums(int num);
void showunit();
void display_net_conn(int wifion);
void displayscreenraw(int k, int modeupdown);
void display_welcome();
void displayscreen(int k);
void display_init();
void diaplay_data(int num);
void display_data_init(int num);

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)


					  		 
#endif  
	 
	 




