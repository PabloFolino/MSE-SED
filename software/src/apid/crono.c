/** \file	crono.c
 *  Feb 2022
 *  Maestría en Sistemas Embebidos - Sistemas emebebidos distribuidos
 * \brief Contiene las funciones para manejo de tiempos y reloj del sistema */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include <math.h>
/*  APID  */
#include "crono.h"
#include "io.h"
#include "config.h"
#include "mqtt.h"

/* TAGs */
static const char *TAG1 = "CRONO/TIMER";
static const char *TAG2 = "CRONO/SNTP";

/********************************** TIMER *************************************/

esp_timer_handle_t periodic_timer;
extern volatile uint16_t data;
extern volatile uint32_t data_max=0;
extern struct micro_t micro;
extern struct data_t dato[N_max];



/*****************************************************************************
 CRONO_timerCallback(void* arg): callback para la interrpción de timer.
*******************************************************************************/
static void CRONO_timerCallback(void* arg)
{
  int64_t epoch_actual;
  char timestamp[64]="";
  char file_name[32];

  switch (micro.run_stop){
    case 2:
      if(micro.muestra==0){
        micro.epoch_init=CRONO_getTime(timestamp, sizeof(timestamp));
        micro.epoch_finish=micro.epoch_init+micro.inicio*1000;
        printf("micro.epoch_init: %lli\n", micro.epoch_init);
        printf("micro.epoch_finish: %lli\n", micro.epoch_finish);

        sprintf(file_name, "A%lld.csv", micro.epoch_init);    // Registro en el log
        MQTT_log("Nuevo evento-->", "Archivo:", file_name);                    
      }
  
      (dato+micro.muestra)->valor_adc=IO_readAdc();
      epoch_actual=CRONO_getTime(timestamp, sizeof(timestamp));
      (dato+micro.muestra)->epochs=epoch_actual-micro.epoch_init;

	    // Se usa para el chequeo de umbrales 
      if (fabs(data_max) < fabs((dato+micro.muestra)->valor_adc)) data_max = (dato+micro.muestra)->valor_adc;

      micro.muestra++;
      if(epoch_actual>=micro.epoch_finish){
        micro.run_stop=3;               // Indica que finalizó el muestreo
      } 
      break;
    default:
      break;
  }

}

/*****************************************************************************
 CRONO_timerInit(): inicialización del temporizador de alta presición.
*******************************************************************************/
void CRONO_timerInit(){

 /* Create two timers:
 * 1. a periodic timer which will run every 0.5s, and print a message
 * 2. a one-shot timer which will fire after 5s, and re-start periodic
 *    timer with period of 1s. */

  const esp_timer_create_args_t periodic_timer_args = {
          .callback = &CRONO_timerCallback,
          /* name is optional, but may help identify the timer when debugging */
          .name = "periodic"
  };

  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  /* The timer has been created but is not running yet */

}

/*****************************************************************************
 CRONO_timerStart(): arranca el temporizador periodico con un periodo de interrupción
"interval" en milisegundos.
IMPORTANTE: no volver a ejecutar esta función si el timer ya está corriendo.
Para reiniciar el timer, primero debe detenerse.
*******************************************************************************/
void CRONO_timerStart(uint64_t interval_ms){

  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, interval_ms * 1000));
  ESP_LOGI(TAG1, "Started timers, time since boot: %lld us", esp_timer_get_time());

}

/*****************************************************************************
CRONO_timerStop(): detiene el temporizador periodico.
*******************************************************************************/
void CRONO_timerStop(void){

  esp_timer_stop(periodic_timer);

}


/**************************** DELAYs & SLEEPs *********************************/

/******************************************************************************
CRONO_delayMs(): introduce un delay de "delay_ms" milisegundos
******************************************************************************/
void CRONO_delayMs(int time_ms){

  vTaskDelay(time_ms / portTICK_PERIOD_MS);

}


/******************************************************************************
CRONO_sleepMs(): pone a dormir al dispositivo durante "time_ms" milisegundos
*******************************************************************************/
void CRONO_sleepMs(uint64_t time_ms){

  esp_sleep_enable_timer_wakeup(time_ms * 1000);
  esp_light_sleep_start();

}

/********************************* SNTP ***************************************/

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep. */

RTC_DATA_ATTR static int boot_count = 0;

/*******************************************************************************
CRONO_getTime(): Lectura de tiempo actual. Recibe un puntero para devolver por
referencia una cadena "timebuff" que contendrá la información de tiempo en formato
texto, y un entero "timebuff_size" que indica el largo máximo reservado en el
buffer. Además, devuelve un entero con la marca de tiempo en formato EPOCH.
*******************************************************************************/
int64_t CRONO_getTime(char * timebuff, int timebuff_size)
{

  time_t now;
  struct tm timeinfo; // tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst
  struct timeval tv;
  char strftime_buf[timebuff_size];

  // update 'now' variable with current time
  time(&now);

  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  /* strftime(): TIMESTAMP FORMATs: https://www.cplusplus.com/reference/ctime/strftime/ */

  strcpy(timebuff, strftime_buf);
  //ESP_LOGI(TAG2, "Timestamp in text format: %s", timebuff );

  gettimeofday(&tv, NULL);
  //ESP_LOGI(TAG2, "Timestamp in UNIX Epoch format: %lld sec ", tv.tv_sec * (int64_t)1e6 );
  //ESP_LOGI(TAG2, "Timestamp in UNIX Epoch format: %lld usec ", tv.tv_sec * (int64_t)1e6 + tv.tv_usec);

  /*  Return UNIX Epoch in seconds (started in 1970)  */
  //return tv.tv_sec * (int64_t)1e6;   // Seconds
  return tv.tv_sec * (int64_t)1e3 + (int64_t)(tv.tv_usec/1e3); // micro-Seconds
  //return tv.tv_sec * (int64_t)1e6 + tv.tv_usec; // micro-Seconds

}


/*******************************************************************************
 CRONO_sntpInit(): Sincronización NTP y configuración de tiempo del sistema
*******************************************************************************/
void CRONO_sntpInit(void)
{

  ++boot_count;
  ESP_LOGI(TAG2, "Boot count: %d", boot_count);

  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 10;
  char strftime_buf[64];

  // initialize sntp
  ESP_LOGI(TAG2, "Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  #ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
  #endif
  sntp_init();

  // wait for time to be set
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
      ESP_LOGI(TAG2, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
  }

  // update 'now' variable with current time
  time(&now);

  // Set timezone to Buenos Aires
  setenv("TZ", "WART4WARST,J1/0,J365/25", 1);
  tzset();
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG2, "The current date/time in Buenos Aires is: %s", strftime_buf);


  if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
    struct timeval outdelta;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
      adjtime(NULL, &outdelta);
      ESP_LOGI(TAG2, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
      (long)outdelta.tv_sec,
      outdelta.tv_usec/1000,
      outdelta.tv_usec%1000);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }

}
