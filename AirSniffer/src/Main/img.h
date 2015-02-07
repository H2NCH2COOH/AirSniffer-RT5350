#ifndef _IMG_H_
#define _IMG_H_	

#define LCD_W   160
#define LCD_H   128

#define BPP 2 //byte per pixel

//UPPER
#define UPPER_WIDTH         LCD_W
#define UPPER_HEIGHT        98
#define UPPER_X_STA         0
#define UPPER_Y_STA         0    

//Number
#define NUM_MAX_DIGIT       5 //digit from right to left
#define NUM_MAX             99999
#define NUM_MIN             0
#define NUM_WIDTH           30
#define NUM_HEIGHT          36
#define NUM_X_STA           (0+(NUM_MAX_DIGIT-1)*NUM_WIDTH)
#define NUM_Y_STA           20

//Unit
#define UNIT_WIDTH          LCD_W
#define UNIT_HEIGHT         20
#define UNIT_X_STA          0
#define UNIT_Y_STA          70

//Battery
#define BAT_WIDTH           20
#define BAT_HEIGHT          (LCD_H-UPPER_HEIGHT)
#define BAT_X_STA           0
#define BAT_Y_STA           UPPER_HEIGHT

//Temperature
#define TEMP_WIDTH          80
#define TEMP_HEIGHT         (LCD_H-UPPER_HEIGHT)
#define TEMP_X_STA          (BAT_X_STA+BAT_WIDTH)
#define TEMP_Y_STA          UPPER_HEIGHT
#define TEMP_MAX            999
#define TEMP_MIN            -99
#define TEMP_MAX_DIGIT      3
#define TEMP_NUM_WIDTH      20
#define TEMP_NUM_HEIGHT     24
#define TEMP_NUM_X_STA      (TEMP_X_STA+1+(TEMP_MAX_DIGIT-1)*TEMP_NUM_WIDTH)
#define TEMP_NUM_Y_STA      (TEMP_Y_STA+2)

//Net
#define NET_HEIGHT          (LCD_H-UPPER_HEIGHT)
#define NET_WIDTH           NET_HEIGHT
#define NET_X_STA           (TEMP_X_STA+TEMP_WIDTH)
#define NET_Y_STA           UPPER_HEIGHT

//Spinner
#define SPINNER_HEIGHT      (LCD_H-UPPER_HEIGHT)
#define SPINNER_WIDTH       SPINNER_HEIGHT
#define SPINNER_FRAME_COUNT 6
#define SPINNER_X_STA       (NET_X_STA+NET_WIDTH)
#define SPINNER_Y_STA       UPPER_HEIGHT

//ID
#define ID_WIDTH            20
#define ID_HEIGHT           24
#define ID_X_STA            20
#define ID_Y_STA            26
#define ID_Y_STEP           (ID_HEIGHT+10)
#define ID_CHAR_PER_LINE    6
#define ID_NUMBER_OF_LINE   2

//IP
#define IP_WIDTH            12
#define IP_HEIGHT           15
#define IP_X_STA            10
#define IP_Y_STA            26

struct image
{
    const char* file_name;
    unsigned int width;
    unsigned int height;
    unsigned char* img;
};

struct pixel
{
    char byte[BPP];
};

extern struct image image_welcome;

extern struct image image_num_blank;
extern struct image image_num[10];
extern struct image image_num_minus;

extern struct image image_net_connnected;
extern struct image image_net_disconnnected;
extern struct image image_net_blank;

extern struct image image_bat_full;
extern struct image image_bat_half;
extern struct image image_bat_low;
extern struct image image_bat_charge;
extern struct image image_bat_blank;

extern struct image image_unit_pcs;
extern struct image image_unit_ug;

extern struct image image_temp_bg;
extern struct image image_temp_blank;

extern struct image image_please_wait;
extern struct image image_upper_blank;
extern struct image image_please_setup;
extern struct image image_setup_fail;
extern struct image image_setup_success;
extern struct image image_id_title;
extern struct image image_ip_title;

extern struct image image_spinner[SPINNER_FRAME_COUNT];
extern struct image image_spinner_blank;

int load_image(struct image* img);
void free_image(struct image* img);

int get_pixel(struct image* img,unsigned int x,unsigned int y,struct pixel* p);
int set_pixel(struct image* img,unsigned int x,unsigned int y,struct pixel* p);

int resize_image(struct image* src,struct image* dst);

#endif /* _IMG_H_ */
