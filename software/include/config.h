/** \file config.h
 *  Mar 2022
 *  Maestría en SIstemas Embebidos - Sistemas embebidos distribuidos
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIG_BLINK_GPIO 2

/* configuración WIFI */
#define ESP_WIFI_SSID "ssid"     // CONFIG_ESP_WIFI_SSID  // "SSID wifi"
#define ESP_WIFI_PASS "pass"     // CONFIG_ESP_WIFI_PASS  // "pass wifi"

/*  Configuracion MQTT  */
#define PORT_MQTT 1883            //CONFIG_PORT_MQTT      // default
#define IP_BROKER_MQTT "boker"    //CONFIG_IP_BROKER_MQTT // Broker MQTT

/*  configuración IO */
#define BLINK_GPIO CONFIG_BLINK_GPIO  // port 2 para NodeMcu-23S

/* Configuración SD  */

/* Configuración CRONO  */
#define SENSOR_REP 1909
#define SENSOR_MAX 4095

/* Configuración varias */
#define LOOPS_UMBRAL  5          // Veces de 100mseg en testear los umbrales

#define TIME_MAX 60              // Tiempo máximo de muestreo en segundos 
#define FREC_MUESTREO 100
#define PERIODO_MUESTREO  (1000/FREC_MUESTREO)  // Período de muestreo en ms
#define EPOCH_INICIAL 11111111   // Es e archivo A11111111.csv por default 

/* Estructuras generales */
struct micro_t{
  uint8_t inicio;           // Se guarda el tiempo de mustra de 1 a 60 seg
  uint8_t run_stop;         // 0-Stop no se estan tomando muestras     
                            // 1-Run  se estan tomando muestras 
                            // 2-Paro el muestreo
                            // 3- Abro los el archivo y guardo las muestras
  uint16_t t_muestreo;      // período de muestreo se guarda el número de muestra
  uint16_t muestra;         // es el número de muestra
  uint64_t epoch_init;      // se guarda el tiempo cuando comienza a tomar la muestra
  uint64_t epoch_finish;    // se guarda el tiempo final de muestra

};

#define N_max 6200      // Maxima cantidad de datos a guardar
struct data_t{           //array de datos
  int64_t epochs;       
  int16_t valor_adc;     
};

#endif
