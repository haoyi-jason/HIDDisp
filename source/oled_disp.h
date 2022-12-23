#ifndef _OLED_DISP_
#define _OLED_DISP_

void oled_disp_init();
void oled_write(uint8_t row, uint8_t *msg, uint8_t len);
void olde_clear();
void oled_writexy(uint8_t x, uint8_t y, uint8_t *msg, uint8_t len);
void oled_refersh();
#endif