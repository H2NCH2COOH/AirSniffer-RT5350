// data of image
#ifndef __DATALCD_H
#define __DATALCD_H	


#define LCD_W 160
#define LCD_H 128
#define BPP 2 // byte per pixel

#define NMAXBIT 2   // begin at bit 0, right to left
#define NUMWIDTH 32
#define NUMHEIGHT 50
#define NXSTA (15+NMAXBIT*NUMWIDTH)
#define NYSTA 20
#define NUMOFNUM 11 //0-9, and backcolor

// battery charge coin, 
#define CHARGEWIDTH 50
#define CHARGEHEIGHT 30


// unit coin, times new rome bold italic 10, 3- size 7
#define UNITWIDTH 48  // can be extend to 48
#define UNITHEIGHT 28  // can be reduce to 20 around
#define UXSTA (NXSTA+NUMWIDTH)
#define UYSTA (NYSTA+(NUMHEIGHT-UNITHEIGHT)/2)

// net conn coin
#define NETHEIGHT 30
#ifndef NETWIDTH
#define NETWIDTH NETHEIGHT
#endif

// config for display_screen
#define MODEDIVUPDOWN 1
#define DIVSIZE 1

#define DIVUPPERWIDTH LCD_W
#define DIVUPPERHEIGHT (LCD_H-CHARGEHEIGHT)
#define DIVLOWERWIDTH (LCD_W-CHARGEWIDTH-NETWIDTH)
#define DIVLOWERHEIGHT CHARGEHEIGHT

#define DIVLEFTWIDTH (LCD_W-CHARGEWIDTH-NETWIDTH)
#define DIVLEFTHEIGHT LCD_H
#define DIVRIGHTWIDTH (CHARGEWIDTH+NETWIDTH)
#define DIVRIGHTHEIGHT (LCD_H-CHARGEHEIGHT)

#if MODEDIVUPDOWN==1
	#define DIVWIDTH_1 DIVUPPERWIDTH
	#define DIVHEIGHT_1 DIVUPPERHEIGHT
	#define DIVWIDTH_2 DIVLOWERWIDTH
	#define DIVHEIGHT_2 DIVLOWERHEIGHT
	#define DIV2XSTA 0
	#define DIV2YSTA DIVUPPERHEIGHT
#else
	#define DIVWIDTH_1 DIVLEFTWIDTH
	#define DIVHEIGHT_1 DIVLEFTHEIGHT
	#define DIVWIDTH_2 DIVRIGHTWIDTH
	#define DIVHEIGHT_2 DIVRIGHTHEIGHT
	#define DIV2XSTA DIVLEFTWIDTH
	#define DIV2YSTA 0
#endif

extern const unsigned char gImage_numchars[NUMOFNUM][BPP*NUMWIDTH*NUMHEIGHT];
extern const unsigned char gImage_boot[40960];
extern const unsigned char gImage_battery[2][BPP*CHARGEWIDTH*CHARGEHEIGHT];
extern const unsigned char gImage_unit[BPP*UNITWIDTH*UNITHEIGHT];
extern const unsigned char gImage_net30[2][1800];
extern const unsigned char gImage_net30_on_2[2280];
extern const unsigned char gImage_div_upper[DIVSIZE][BPP*DIVUPPERWIDTH*DIVUPPERHEIGHT];
extern const unsigned char gImage_div_lower[DIVSIZE][BPP*DIVLOWERWIDTH*DIVLOWERHEIGHT];
extern const unsigned char gImage_div_left[DIVSIZE][BPP*DIVLEFTWIDTH*DIVLEFTHEIGHT];
extern const unsigned char gImage_div_right[DIVSIZE][BPP*DIVRIGHTWIDTH*DIVRIGHTHEIGHT];

#endif  
	 
	 
 	