#include "ch.h"
#include "hal.h"
#include "ssd1362.h"
#include "oled_disp.h"

#include "ssd1306_font.c"

static const I2CConfig i2ccfg = {
  OPMODE_I2C,
  100000,
  STD_DUTY_CYCLE,
};

static const SSD1362Config config = {
  &I2CD1,
  &i2ccfg,
  SSD1362_SAD_0X78,
};

static SSD1362Driver ssd1362;

void oled_disp_init()
{
  ssd1362ObjectInit(&ssd1362);
  
  palClearPad(GPIOB,9);
  chThdSleepMilliseconds(10);
  palSetPad(GPIOB,9);
  
  ssd1362Start(&ssd1362,&config);
  
  ssd1362FillScreen(&ssd1362,0x0);
  
//  char msg[32];
//  
//  sprintf(msg,"Test of Grididea.com.tw\0");
//  ssd1362GotoXy(&ssd1362,0,0);
//  ssd1362Puts(&ssd1362,msg,&ssd1306_font_11x18,0xc);
//
//  sprintf(msg,"Test of 112233\0");
//  ssd1362GotoXy(&ssd1362,0,20);
//  ssd1362Puts(&ssd1362,msg,&ssd1306_font_11x18,0xc);
//
//  sprintf(msg,"Test of 445566\0");
//  ssd1362GotoXy(&ssd1362,0,40);
//  ssd1362Puts(&ssd1362,msg,&ssd1306_font_11x18,0xc);


  ssd1362UpdateScreen(&ssd1362);
}


void oled_write(uint8_t row, uint8_t *msg, uint8_t len)
{
  uint8_t szWrite = (len > 23)?23:len; // max 23 characters
  if(row < 3){
    ssd1362GotoXy(&ssd1362,0,row*20);
    ssd1362Puts(&ssd1362,msg,&ssd1306_font_11x18,0xc);
  }
  ssd1362UpdateScreen(&ssd1362);
}

void olde_clear()
{
  ssd1362FillScreen(&ssd1362,0x0);
  ssd1362UpdateScreen(&ssd1362);
}
