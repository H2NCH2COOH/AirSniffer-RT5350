// data of image
#ifndef __DATALCD_H
#define __DATALCD_H	

#define LCD_W   160
#define LCD_H   128

#define BPP 2 //byte per pixel

//Number
#define NUM_MAX_DIGIT   5 //digit from right to left
#define NUM_WIDTH       26
#define NUM_HEIGHT      50
#define NUM_X_STA       (15+(NUM_MAX_DIGIT-1)*NUM_WIDTH)
#define NUM_Y_STA       20
#define NUM_SIZE        11 //0-9, and backcolor

//Battery
#define BAT_WIDTH       50
#define BAT_HEIGHT      30
#define BAT_SIZE        4


//Unit
#define UNIT_WIDTH      LCD_W
#define UNIT_HEIGHT     20
#define UNIT_X_STA      0
#define UNIT_Y_STA      (NUM_Y_STA+NUM_HEIGHT)

//Net
#define NET_HEIGHT      BAT_HEIGHT
#ifndef NET_WIDTH
    #define NET_WIDTH   NET_HEIGHT
#endif
#define NET_SIZE        2

//Div
#define DIV_SIZE        5

#define DIV_UPPER_WIDTH     LCD_W
#define DIV_UPPER_HEIGHT    (LCD_H-BAT_HEIGHT)
#define DIV_LOWER_WIDTH     (LCD_W-BAT_WIDTH-NET_WIDTH)
#define DIV_LOWER_HEIGHT    BAT_HEIGHT
#define DIV_LOWER_X_STA     0
#define DIV_LOWER_Y_STA     DIV_UPPER_HEIGHT


extern const unsigned char gImage_numchars[NUM_SIZE][BPP*NUM_WIDTH*NUM_HEIGHT];
extern const unsigned char gImage_boot[BPP*LCD_W*LCD_H];
extern const unsigned char gImage_battery[BAT_SIZE][BPP*BAT_WIDTH*BAT_HEIGHT];
extern const unsigned char gImage_unit[BPP*UNIT_WIDTH*UNIT_HEIGHT];
extern const unsigned char gImage_net[NET_SIZE][BPP*NET_HEIGHT*NET_WIDTH];
extern const unsigned char gImage_div_upper[DIV_SIZE][BPP*DIV_UPPER_WIDTH*DIV_UPPER_HEIGHT];
extern const unsigned char gImage_div_lower[DIV_SIZE][BPP*DIV_LOWER_WIDTH*DIV_LOWER_HEIGHT];

#endif  
	 
	 
 	