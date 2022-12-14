#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "shell.h"
#include "chprintf.h"


#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

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

int main()
{
  thread_t *shelltp1 = NULL;
  halInit();
  chSysInit();
  
//  sduObjectInit(&SDU1);
//  sduStart(&SDU1, &serusbcfg);
  
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
  while(true){
    if(usbhidcfg.usbp->state == USB_ACTIVE){
      uint8_t report[64];
      //size_t n;
      size_t n = hidGetReport(0, report,  sizeof(report));
      hidWriteReport(&UHD1,report,n);
      n = hidReadReportt(&UHD1, report, sizeof(report), TIME_MS2I(10));
      if(n > 0){
        if(report[0] == 0xff){
          oled_refersh();
        }
        else{
          //hidSetReport(0,&report,n);
          report[report[2]+2] = 0x0;
          //oled_write(report[0],&report[2], report[1]);
          oled_writexy(report[0],report[1],&report[3],report[2]);
        }
      }
      chThdSleepMilliseconds(50);
    }
  }
  return 0;
}