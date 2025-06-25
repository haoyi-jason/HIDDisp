#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "shell.h"
#include "chprintf.h"


#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

typedef struct{
  uint32_t dummy;
  int16_t x;
  int16_t y;
  uint16_t sw;
}_joystick_packet_t;

/* On-Chip ADC Section */
#define ADC_GRP1_NUM_CHANNELS   4 // PA0~3
#define ADC_GRP1_BUF_DEPTH      8
static void adccallback(ADCDriver *adcp)
{
//  while(1);
}

static void adcerror(ADCDriver *adcp, adcerror_t err)
{
//  while(1);
}
static adcsample_t samples[ADC_GRP1_NUM_CHANNELS*ADC_GRP1_BUF_DEPTH];
static const ADCConversionGroup adcgrpcfg = {
  TRUE,
  ADC_GRP1_NUM_CHANNELS,
  adccallback,
  adcerror,
  0,
  ADC_CR2_SWSTART , //| ADC_CR2_EXSEL_0 | ADC_CR2_EXSEL_1 | ADC_CR2_EXSEL_2,
  0,
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_239P5) | ADC_SMPR2_SMP_AN1(ADC_SAMPLE_239P5) | ADC_SMPR2_SMP_AN2(ADC_SAMPLE_239P5) | ADC_SMPR2_SMP_AN3(ADC_SAMPLE_239P5),
  ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
  0,
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2) | ADC_SQR3_SQ4_N(ADC_CHANNEL_IN3)
};


static void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]) 
{

  chprintf(chp,"\r\nWrite Command");
}

static void cmd_read(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  
  chprintf(chp,"\r\nRead Command");
}

static void cmd_line(BaseSequentialStream *chp, int argc, char *argv[]) 
{
  
  chprintf(chp,"\r\nRun Command");
}

static const ShellCommand commands[] = {
  {"write", cmd_write},
  {"read", cmd_read},
  {"line", cmd_line},
  {NULL, NULL}
};

//static const ShellConfig shell_cfg1 = {
//  (BaseSequentialStream *)&SDU1,
//  commands
//};

static SPIConfig spicfg = {
  false,
  NULL,
  GPIOB,
  12,
  SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA
};

static THD_WORKING_AREA(waBlink, 1024);
static THD_FUNCTION(procBlink, arg) 
{
  (void)arg;
  
  while(1){
    palSetPad(GPIOC,2);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOC,2);
    chThdSleepMilliseconds(500);
  }
}

#define MMIO16(addr)    (*(volatile uint16_t*)(addr))
#define UID_ADDR        0x1fffF7e8
#define UUID            ((uint8_t*)UID_ADDR)

#define LINE_L  PAL_LINE(GPIOB,0)
#define LINE_R  PAL_LINE(GPIOB,1)
#define LINE_U  PAL_LINE(GPIOB,2)
#define LINE_D  PAL_LINE(GPIOB,10)
#define LINE_N  PAL_LINE(GPIOB,11)

#define EV_LINE_L       EVENT_MASK(0)
#define EV_LINE_R       EVENT_MASK(1)
#define EV_LINE_U       EVENT_MASK(2)
#define EV_LINE_D       EVENT_MASK(3)
#define EV_LINE_N       EVENT_MASK(4)

static bool clicked = false;


void readChipID(uint8_t *buffer)
{
  for(uint8_t i=0;i<12;i++){
    uint16_t id = MMIO16(UID_ADDR + i*2);
    buffer[i] = UUID[i];
  }
}

static void line_isr(void *arg)
{
  chSysLockFromISR();
  clicked = true;
  chSysUnlockFromISR();
}

int main()
{
  thread_t *shelltp1 = NULL;  
  halInit();
  chSysInit();
  
  uint8_t report[64];
  uint8_t packet[64];
  _joystick_packet_t *joy = (_joystick_packet_t*)packet;
  
//  while(true){
//    chThdSleepMilliseconds(10);
//  }
  
  uint8_t chipId[12];
  uint16_t uid;
  
  readChipID(chipId);
  
  uid = (chipId[1] << 8) | chipId[0];
  
  uint8_t str[8];
  chsnprintf(str,16,"%08d",uid);
  
  for(uint8_t i=0;i<8;i++){
    hid_string3[2 + i*2] = str[i];
    hid_string3[3 + i*2] = 0x0;
  }

//  sduObjectInit(&SDU1);
//  sduStart(&SDU1, &serusbcfg);
  
  palSetLineCallback(LINE_L, line_isr,NULL);
  palSetLineCallback(LINE_R, line_isr,NULL);
  palSetLineCallback(LINE_U, line_isr,NULL);
  palSetLineCallback(LINE_D, line_isr,NULL);
  palSetLineCallback(LINE_N, line_isr,NULL);
  
  palEnableLineEvent(LINE_L, PAL_EVENT_MODE_FALLING_EDGE);
  palEnableLineEvent(LINE_R, PAL_EVENT_MODE_FALLING_EDGE);
  palEnableLineEvent(LINE_U, PAL_EVENT_MODE_FALLING_EDGE);
  palEnableLineEvent(LINE_D, PAL_EVENT_MODE_FALLING_EDGE);
  palEnableLineEvent(LINE_N, PAL_EVENT_MODE_FALLING_EDGE);
  
  hidObjectInit(&UHD1);
  hidStart(&UHD1,&usbhidcfg);
  
  
  usbDisconnectBus(usbhidcfg.usbp);
//  palClearPad(GPIOA,15);
  chThdSleepMilliseconds(1000);
//  palSetPad(GPIOA,15);
//  usbStart(serusbcfg.usbp, &usbcfg);
  usbStart(usbhidcfg.usbp, &usbcfg);
  usbConnectBus(usbhidcfg.usbp);  
  
  palClearPad(GPIOB,9);
  chThdSleepMilliseconds(100);
  palSetPad(GPIOB,9);
  
  oled_disp_init();
//  palClearPad(GPIOB,12);
//  palSetPad(GPIOB,12);
//  palClearPad(GPIOB,12);
//  palSetPad(GPIOB,12);
  //spiTest();
//  serialTest();
  //canTest();
//  chThdCreateStatic(waBlink, sizeof(waBlink), NORMALPRIO, procBlink, NULL);
// shellInit();
  
//  while(1){
//    if (SDU1.config->usbp->state == USB_ACTIVE) {
//      /* Starting shells.*/
//      if (shelltp1 == NULL) {
//        shelltp1 = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
//                                       "shell1", NORMALPRIO + 1,
//                                       shellThread, (void *)&shell_cfg1);
//      }
//
//      /* Waiting for an exit event then freeing terminated shells.*/
//      chEvtWaitAny(EVENT_MASK(0));
//      if (chThdTerminatedX(shelltp1)) {
//        chThdRelease(shelltp1);
//        shelltp1 = NULL;
//      }
//    }
//    else {
//      chThdSleepMilliseconds(200);
//    }
//  }
  for(uint8_t i=0;i<64;i++){
    packet[i] = i +1;
  }
  

  adcStart(&ADCD1,NULL);
  adcStartConversion(&ADCD1,&adcgrpcfg,samples,ADC_GRP1_BUF_DEPTH);
  ADCD1.adc->CR2 |= 0x0;

  while(true){
    //adcStartConversion(&ADCD1,&adcgrpcfg,samples,ADC_GRP1_BUF_DEPTH);
    if(usbhidcfg.usbp->state == USB_ACTIVE){
      size_t n = hidGetReport(0, report,  sizeof(report));
      hidWriteReport(&UHD1,report,n);
      n = hidReadReportt(&UHD1, report, sizeof(report), TIME_MS2I(10));
      if(n > 0){
        if(report[0] == 0xff){
          oled_refersh();
        }
        else if(report[0] == 0xAA){
          n = hidGetReport(0,report,sizeof(report));
          joy = (_joystick_packet_t*)report;
          joy->x = samples[2];
          joy->y = samples[2];
          hidWriteReport(&UHD1,report,sizeof(report));
        }
        else{
          //hidSetReport(0,&report,n);
          report[report[2]+2] = 0x0;
          //oled_write(report[0],&report[2], report[1]);
          oled_writexy(report[0],report[1],&report[3],report[2]);
        }
      }
      if(clicked){
        clicked = false;
        for(uint8_t i=0;i<10;i++){
          hidWriteReport(&UHD1,packet,64);
          chThdSleepMilliseconds(10);
        }
      }
//      if(report[0] == 0x00){
//        report[0] = 0x1;
//        hidWriteReport(&UHD1,report,3);
//      }
//      else{
//        report[0] = 0x0;
//        hidWriteReport(&UHD1,report,3);
//      }
      
    }
      chThdSleepMilliseconds(100);
  }
  return 0;
}