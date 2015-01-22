// interface lies just above color definition
#ifndef __LCD_H
#define __LCD_H		

#include <stdint.h>
#include "img.h"

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
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430

#define DARKBLUE      	 0X01CF
#define LIGHTBLUE      	 0X7D7C
#define GRAYBLUE       	 0X5458
 
#define LIGHTGREEN     	 0X841F
#define LGRAY 			 0XC618

#define LGRAYBLUE        0XA651
#define LBBLUE           0X2B12
 
void LCD_Clear(uint16_t Color);
void LCD_DrawPoint(uint16_t x,uint16_t y);//画点
void LCD_DrawPoint_big(uint16_t x,uint16_t y);//画一个大点
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y); //读点
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawLine1(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);	   
void LCD_Fill(uint8_t xsta,uint8_t ysta,uint8_t xend,uint8_t yend,uint16_t color);

void LCD_ShowImageRaw(uint8_t xsta, uint8_t ysta, uint8_t xlen, uint8_t ylen, const uint8_t *image);

/*---------------------------------------------------------------------------*/
void display_init();

void display_welcome();
void display_battery(char state);
void display_data(int num);
void display_unit(int unit);
void display_net(int conn);
void display_upper(struct image* img);
void display_temp(int temp);
void display_temp_bg();
void display_temp_blank();
void display_spinner(int frame);
void display_int(int num,int max_digit,int max,struct image* digits,struct image* blank,unsigned int x,unsigned int y);
/*---------------------------------------------------------------------------*/

#endif



