
#include "main.h"
#include "config.h"

#define Delay_500MS 500
#define Delay_100MS 100

const char *TAG = "MAIN";
volatile uint16_t data;     // lectura del ADC en el instante actual
volatile uint32_t data_max; // lectura máxima del ADC registrada


struct micro_t micro;
struct data_t dato[N_max];


/*******************************************************************************
 Funciones Locales
******************************************************************************/
/*************************************************************
 void name_long_to_short(char *, char *, uint8_t)
 Esta función recibe el nombre largo y la cantidad máxima
 de caracteres que debe acotar el nombre largo.
 También recbe el puntero del nombre corto, y en él devuelve
 el resultado
*************************************************************/
void name_long_to_short(char * corto, char * largo, uint8_t max_caracteres){

  uint8_t longitud;

  longitud=strlen(largo);
  if(longitud>max_caracteres){
      strncpy(corto, largo +longitud-max_caracteres, longitud);
      *corto='A';
      }
  else{
      strncpy(corto,largo,longitud);
      }
}

/*****************************************************************************
 umbral (void* arg): solamente envía MQTT's cuando se está leyendo
*******************************************************************************/
void umbral(void){
  // Definición de umbrales
  int umbral0 = (int)(SENSOR_REP + (SENSOR_MAX-SENSOR_REP)/6.0);
  int umbral1 = (int)(SENSOR_REP + (SENSOR_MAX-SENSOR_REP)/3.0);
  int umbral2 = (int)(SENSOR_REP + (SENSOR_MAX-SENSOR_REP)/4.0*3);

  if(micro.run_stop==2){
    if(umbral0<fabs(data_max) && fabs(data_max)<umbral1){
      printf("ATENCIÓN \t%i\n", data_max);
      MQTT_publish("test/alarmas","0");
      MQTT_log(MQTT_OUT, "test/alarmas", "0");                    // Registro en el log
      }
    else if(umbral1<fabs(data_max) && fabs(data_max)<umbral2){
          printf("PRECAUCIÓN \t%i\n", data_max);
          MQTT_publish("test/alarmas","1");
          MQTT_log(MQTT_OUT, "test/alarmas", "1");                // Registro en el log
        }
      else if(fabs(data_max)>umbral2){
          printf("ALERTA! \t%i\n", data_max);
          MQTT_publish("test/alarmas","2");
          MQTT_log(MQTT_OUT, "test/alarmas", "2");                // Registro en el log
          }
        else{
          printf("REPOSO .... \n");
          MQTT_publish("test/alarmas","4");
          MQTT_log(MQTT_OUT, "test/alarmas", "4");                // Registro en el log
          }
    data_max=0;
  }
}

/*******************************************************************************
 Programa principal
******************************************************************************/
void app_main(void)
{
  // Declaración de variables
  char config_ssid[32];
  char config_pass[64];
  char config_broker_mqtt[32];
  char config_delay[16];
  char config_msg[128];
  int  data_delay;
  char file_name[32];
  char file_name_corto[12];
  uint8_t loops_umbral;
  int n;
 
  micro.t_muestreo=PERIODO_MUESTREO;
  micro.epoch_init=EPOCH_INICIAL;


  // Inicializaciones

  // Use POSIX and C standard library functions to work with files.
  SD_mount();
  IO_adcInit();
  CRONO_timerInit();

  

  FILE * f = SD_open("config.txt", "r");        // Archivo para lectura

  SD_gets(f, config_ssid, sizeof(config_ssid));
  SD_gets(f, config_pass, sizeof(config_pass));
  SD_gets(f, config_broker_mqtt, sizeof(config_broker_mqtt));
  SD_gets(f, config_delay, sizeof(config_delay));
  SD_gets(f, config_msg, sizeof(config_msg));

  // Verificación de lo leido
  ESP_LOGI(TAG,"ssid --> %s", config_ssid);
  ESP_LOGI(TAG,"pass --> %s", config_pass);
  ESP_LOGI(TAG,"broker_mqtt --> %s", config_broker_mqtt);
  ESP_LOGI(TAG,"delay --> %s", config_delay);
  ESP_LOGI(TAG,"msg --> %s", config_msg);

  data_delay=atoi(config_delay);
  ESP_LOGI(TAG,"delay --> %d ms", data_delay);

  SD_close(f);
  
  // Inicializo red y MQTT
  WIFI_userInit(config_ssid, config_pass);
  MQTT_userInit(config_broker_mqtt);

  // Inicializo SNPT
  CRONO_sntpInit();

  FILE * f1 = SD_open("log.txt", "w");     // Creo el arcivo log.txt
  SD_close(f1);
  
  loops_umbral=0;
  while(1){
    CRONO_delayMs(Delay_100MS);

    switch(micro.run_stop){
      case 1:
        CRONO_timerStart((uint64_t) micro.t_muestreo);            // Lanzo el contador por default es 100Hz
        micro.run_stop=2;
        break;
      case 2:                                                     // En el estado 2 se está muestreando
        break;
      case 3:
        CRONO_timerStop();                                        // Para el muestreo
        micro.run_stop=4;

        // Ceo el nombre del archivo
        sprintf(file_name, "A%lld.csv", micro.epoch_init);
        //**************************************************************************************************************
        printf("EL nombre del archivo a crear=%s\n",file_name);         // es del tipo A1651826659407.csv
        name_long_to_short(file_name_corto, file_name,12);              // no puedo crear archivos mayor a 12 caracteres
        printf("EL nombre del archivo creado=%s\n",file_name_corto);    // es del tipo A6659407.csv
        //**************************************************************************************************************
        
        FILE * f2 = SD_open(file_name_corto, "w");                  // Archivo para escritura

        //Ciclo de lectura-Encabezado
        SD_printf(f2, "milisegundos,datos\n");
        for(n=0;n<micro.muestra;n++){
          SD_printf(f2, "%lli,%i\n", (dato+n)->epochs, (dato+n)->valor_adc);
        }
        printf("Se escribió el arcivo=%s\n",file_name_corto);

        SD_close(f2);
        micro.epoch_init=EPOCH_INICIAL;
        break; 
    }

    // Miestras mide se chequean los umbrales cada 500mseg, en otra condición espera
    loops_umbral++;
    if (loops_umbral>LOOPS_UMBRAL){
      umbral();
      loops_umbral=0;
    } 

  }

  // All done, unmount partition and disable SDMMC or SPI peripheral
  SD_unmount();

}
