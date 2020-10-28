#include "OLED_Driver.h"
#include "ASCII_Font.h"
#include "main.h"
#include "gpio.h"
#include "stdlib.h"


uint8_t color_byte[2],color_Damp_byte[2];

uint16_t wheel;
uint8_t OLED_GRAM[2*96*128];
uint16_t color_now,color_half,color_min;

#ifdef __cplusplus
extern "C"  {
#endif


OLED_Driver::OLED_Driver(void) {
}
  
u16 OLED_Driver::ConvertColor(u8 red,u8 green,u8 blue)
{
	return red<<11|green<<6|blue;
}


void OLED_Driver::Set_Wheel(u16 WheelPos)	{
	
	uint8_t red,green,blue;
	wheel = WheelPos;
	if(WheelPos < 32) 
	{
		red = 0;
		green = WheelPos;
		blue = (31 - WheelPos);
	}
	else if(WheelPos < (32*2)) 
	{          
		red = WheelPos-32;
		green = 32*2-1 - WheelPos;
		blue = 0;
	}
	else
	{
		red = (32*3)-1 - WheelPos;
		green = 0;
		blue = WheelPos-(32*2);
	}
	color_Damp_byte[0] = red<<3|(green>>2);
	color_Damp_byte[1] = green<<6|blue;
}
void OLED_Driver::Calc_Color(void){
				
	uint8_t red,green,blue;
	uint8_t red1,green1,blue1;
	red = color_byte[0]>>3;
	green = ((color_byte[0]&0x7)<<3)|color_byte[1]>>5;
	blue = color_byte[1]&0x1f;
	
	red1 = color_Damp_byte[0]>>3;
	green1 = ((color_Damp_byte[0]&0x7)<<3)|color_Damp_byte[1]>>5;
	blue1 = color_Damp_byte[1]&0x1f;
	
	if(red>red1)
	{
		red--;
	}
	else if(red<red1)
	{
		red++;
	}
	if(green>green1+1)
	{
		green-=2;
	}
	else if(green+1<green1)
	{
		green+=2;
	}
	if(blue>blue1)
	{
		blue--;
	}
	else if(blue<blue1)
	{
		blue++;
	}
	color_now = red<<11|green<<5|blue;
	color_byte[0] = color_now>>8;
	color_byte[1] = color_now&0xff;
	red >>=1;
	green >>=1;
	blue >>=1;
	color_half = red<<11|green<<5|blue;
	red >>=1;
	green >>=1;
	blue >>=1;
	color_min = red<<11|green<<5|blue;
	
}


uint16_t OLED_Driver::RandomColor(void){
	
	uint8_t red,green,blue;
	red = rand();
	__ASM("NOP");
	green = rand();
	__ASM("NOP");
	blue = rand();
	return (red<<11|green<<5|blue);
}

void OLED_Driver::Set_DampColor(uint16_t color)  {
  
  color_Damp_byte[0] = (uint8_t)(color >> 8);
  color_Damp_byte[1] = (uint8_t)(color & 0x00ff);
}




void OLED_Driver::Set_Color(uint16_t color)  {
  
  color_byte[0] = (uint8_t)(color >> 8);
  color_byte[1] = (uint8_t)(color & 0x00ff);
}


//void OLED_Driver::Set_FillColor(uint16_t color)  {
//  
//  color_fill_byte[0] = (uint8_t)(color >> 8);
//  color_fill_byte[1] = (uint8_t)(color & 0x00ff);
//}
//  

void OLED_Driver::Write_Command(uint8_t cmd)  {
  
  OLED_CS(GPIO_PIN_RESET);
  
#if  INTERFACE_4WIRE_SPI
  
  OLED_DC(GPIO_PIN_RESET);
  
  while(HAL_SPI_Transmit(&hspi1,&cmd,0x01,0x10) != HAL_OK);
  
  OLED_DC(GPIO_PIN_SET);
  
#elif INTERFACE_3WIRE_SPI
  
  uint8_t i;
	uint16_t hwData = 0;
	
  hwData = (uint16_t)cmd & ~0x0100;

	for(i = 0; i < 9; i ++) {
		OLED_SCK(GPIO_PIN_RESET);
    if(hwData & 0x0100) {
      OLED_DIN(GPIO_PIN_SET);
		}
    else  {
      OLED_DIN(GPIO_PIN_RESET);
		}
    OLED_SCK(GPIO_PIN_SET);
		hwData <<= 1;
	}

  
#endif
  
  OLED_CS(GPIO_PIN_SET);
}


void OLED_Driver::Write_Data(uint8_t dat) {
  
  OLED_CS(GPIO_PIN_RESET);
  
#if  INTERFACE_4WIRE_SPI
  
  OLED_DC(GPIO_PIN_SET);
  
  while(HAL_SPI_Transmit(&hspi1,&dat,0x01,0x10) != HAL_OK);
  
  OLED_DC(GPIO_PIN_RESET);
  
#elif INTERFACE_3WIRE_SPI
  
  uint8_t i;
	uint16_t hwData = 0;
	
  hwData = (uint16_t)dat | 0x0100;
	
	for(i = 0; i < 9; i ++) {
    OLED_SCK(GPIO_PIN_RESET);
		if(hwData & 0x0100) {
      OLED_DIN(GPIO_PIN_SET);
		}
    else  {
      OLED_DIN(GPIO_PIN_RESET);
		}
    OLED_SCK(GPIO_PIN_SET);
		hwData <<= 1;
	}
  
#endif
  
  OLED_CS(GPIO_PIN_SET);
  
}

void OLED_Driver::Write_Data(uint8_t* dat_p, long length) {
  
  OLED_CS(GPIO_PIN_RESET);
  
#if INTERFACE_4WIRE_SPI
  
  OLED_DC(GPIO_PIN_SET);
  
  while(HAL_SPI_Transmit(&hspi1,dat_p,length,0x10) != HAL_OK);
  
  OLED_DC(GPIO_PIN_RESET);
  
#elif INTERFACE_3WIRE_SPI
  
  uint8_t i,j;
	uint16_t hwData = 0;
	

  for(i = 0; i < length; i++) {
    
    hwData = (uint16_t)dat_p[i] | 0x0100;
    
    for(j = 0; j < 9; j ++) {
      OLED_SCK(GPIO_PIN_RESET);
      if(hwData & 0x0100) {
        OLED_DIN(GPIO_PIN_SET);
      } else {
        OLED_DIN(GPIO_PIN_RESET);
      }
      OLED_SCK(GPIO_PIN_SET);
      hwData <<= 1;
    }
  }
#endif
  
  OLED_CS(GPIO_PIN_SET);
  
}


void OLED_Driver::RAM_Address(void)  {
  
  Write_Command(0x15);
  Write_Data(0x0);
  Write_Data(0x7f);

  Write_Command(0x75);
  Write_Data(0x20);
  Write_Data(0x7f);
}


void OLED_Driver::Refrash_Screen(void)  {
  
  RAM_Address();
  Write_Command(0x5C);
//  for(i=0;i<;i++)  {
      Write_Data(OLED_GRAM,128*96*2);//RAM data clear
//  }
}
  

void OLED_Driver::Clear_Screen(void)  {
  
  int i,j;
  for(i=0;i<96;i++)  {
    for(j=0;j<128;j++)  {
      OLED_GRAM[2*j+i*128*2] = 0;
      OLED_GRAM[2*j+1+i*128*2] = 0;
    }
  }
}
  
void OLED_Driver::Set_Coordinate(uint16_t x, uint16_t y)  {

  if ((x >= SSD1351_WIDTH) || (y >= SSD1351_HEIGHT)) 
    return;
  //Set x and y coordinate
  Write_Command(SSD1351_CMD_SETCOLUMN);
  Write_Data(x);
  Write_Data(SSD1351_WIDTH-1);
  Write_Command(SSD1351_CMD_SETROW);
  Write_Data(y);
  Write_Data(SSD1351_HEIGHT-1);
  Write_Command(SSD1351_CMD_WRITERAM);
}
  
void OLED_Driver::Set_Address(uint8_t column, uint8_t row)  {
  
  Write_Command(SSD1351_CMD_SETCOLUMN);  
  Write_Data(column);	//X start 
  Write_Data(column);	//X end 
  Write_Command(SSD1351_CMD_SETROW); 
  Write_Data(row);	//Y start 
  Write_Data(row+7);	//Y end 
  Write_Command(SSD1351_CMD_WRITERAM); 
}
  
  
void  OLED_Driver::Invert(bool v) {
  
  if (v)
    Write_Command(SSD1351_CMD_INVERTDISPLAY);
  else
    Write_Command(SSD1351_CMD_NORMALDISPLAY);
}

void OLED_Driver::Draw_Pixel(long x, long y)
{
  // Bounds check.
  if ((x >= SSD1351_WIDTH) || (y >= SSD1351_HEIGHT)) return;
  if ((x < 0) || (y < 0)) return;

	OLED_GRAM[2*x+y*128*2] = color_byte[0];
	OLED_GRAM[2*x+1+y*128*2] = color_byte[1];
  
}

void OLED_Driver::Draw_Pixel(long x, long y,uint16_t color)
{
  // Bounds check.
  if ((x >= SSD1351_WIDTH) || (y >= SSD1351_HEIGHT)) return;
  if ((x < 0) || (y < 0)) return;

	OLED_GRAM[2*x+y*128*2] = (uint8_t)(color >> 8);
	OLED_GRAM[2*x+1+y*128*2] = (uint8_t)(color & 0x00ff);
  
}
  
  
void OLED_Driver::Device_Init(void) {

  OLED_CS(GPIO_PIN_RESET);

  OLED_RST(GPIO_PIN_RESET);
  HAL_Delay(40);
  OLED_RST(GPIO_PIN_SET);
  HAL_Delay(20);
    
  Write_Command(0xfd);	// command lock
  Write_Data(0x12);
  Write_Command(0xfd);	// command lock
  Write_Data(0xB1);

  Write_Command(0xae);	// display off
  Write_Command(0xa4); 	// Normal Display mode

  Write_Command(0x15);	//set column address
  Write_Data(0x00);     //column address start 00
  Write_Data(0x7f);     //column address end 95
  Write_Command(0x75);	//set row address
  Write_Data(0x20);     //row address start 00
  Write_Data(0x7f);     //row address end 63	

  Write_Command(0xB3);
  Write_Data(0xF1);

  Write_Command(0xCA);	
  Write_Data(0x7F);

  Write_Command(0xa0);  //set re-map & data format
  Write_Data(0x74);     //Horizontal address increment

  Write_Command(0xa1);  //set display start line
  Write_Data(0x00);     //start 00 line

  Write_Command(0xa2);  //set display offset
  Write_Data(0x00);

  Write_Command(0xAB);	
  Write_Command(0x01);	

  Write_Command(0xB4);	
  Write_Data(0xA0);	  
  Write_Data(0xB5);  
  Write_Data(0x55);    

  Write_Command(0xC1);	
  Write_Data(0xC8);	
  Write_Data(0x80);
  Write_Data(0xC0);

  Write_Command(0xC7);	
  Write_Data(0x0F);

  Write_Command(0xB1);	
  Write_Data(0x32);

  Write_Command(0xB2);	
  Write_Data(0xA4);
  Write_Data(0x00);
  Write_Data(0x00);

  Write_Command(0xBB);	
  Write_Data(0x17);

  Write_Command(0xB6);
  Write_Data(0x01);

  Write_Command(0xBE);
  Write_Data(0x05);

  Write_Command(0xA6);

  Clear_Screen();
	Refrash_Screen();
  Write_Command(0xaf);	 //display on
}

  
// Draw a horizontal line ignoring any screen rotation.
void OLED_Driver::Draw_FastHLine(int16_t x, int16_t y, int16_t length) {
  // Bounds check
		int16_t x0=x;
    do
    {
        Draw_Pixel(x, y);   // 逐点显示，描出垂直线
        x++;
    }
    while(x0+length>=x);
}
// Draw a horizontal line ignoring any screen rotation.
void OLED_Driver::Draw_FastHLine(int16_t x, int16_t y, int16_t length,uint16_t color) {
  // Bounds check
		int16_t x0=x;
    do
    {
        Draw_Pixel(x, y,color);   // 逐点显示，描出垂直线
        x++;
    }
    while(x0+length>=x);
}
  
  // Draw a vertical line ignoring any screen rotation.
void OLED_Driver::Draw_FastVLine(int16_t x, int16_t y, int16_t length)  {
  // Bounds check
		int16_t y0=y;
    do
    {
        Draw_Pixel(x, y);   // 逐点显示，描出垂直线
        y++;
    }
    while(y0+length>=y);
}
  // Draw a vertical line ignoring any screen rotation.
void OLED_Driver::Draw_FastVLine(int16_t x, int16_t y, int16_t length,uint16_t color)  {
  // Bounds check
		int16_t y0=y;
    do
    {
        Draw_Pixel(x, y,color);   // 逐点显示，描出垂直线
        y++;
    }
    while(y0+length>=y);
}
  
void OLED_Driver::Display_hbmp(int x,int y,int w,int h,const u8 *ch,uint16_t color)
{
	u16 i,j;
	u16 red,green,blue;
	u16 red1,green1,blue1;
	u16 Factor;
//	color = 0xFFFF;
	red = color>>11;
	green = (color&0x7E0)>>5;
	blue = color&0x1F;
	for(j=0;j<h;j++)
		for(i=0;i<(w+1)/2;i++)
		{
			Factor = (ch[j*((w+1)/2)+i]&0xF0)>>4;
			if(red>=15)
				red1 = red-0xF+Factor;
			else
				red1 = Factor;
			if(green>=30)
				green1=green-30+Factor*2;
			else
				green1 = Factor*2;
			if(blue>=15)
				blue1=blue-15+Factor;
			else
				blue1=Factor;
			
			if(Factor)
				Draw_Pixel(x+i*2+0,y+j,red1<<11|(green1<<5)|(blue1));
			
			Factor = (ch[j*((w+1)/2)+i]&0xF);
			if(red>=15)
				red1 = red-0xF+Factor;
			else
				red1 = Factor;
			if(green>=30)
				green1=green-30+Factor*2;
			else
				green1 = Factor*2;
			if(blue>=15)
				blue1=blue-15+Factor;
			else
				blue1=Factor;
			
			if(Factor)
				Draw_Pixel(x+i*2+1,y+j,red1<<11|(green1<<5)|(blue1));
		}
}	


#ifdef __cplusplus
}
#endif


