/** \file	mqtt.c
 *  Mar 2022
 *  Maestría en Sistemas Embebidos - Sistemas embebidos distribuidos
 * \brief Contiene las funciones de inicializacion y manejo del protocolo mqtt
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "mqtt_client.h"

/* APID */
#include "config.h"
#include "wifi.h"
#include "io.h"
#include "mqtt.h"
#include "sd.h"
#include "crono.h"

/* TAGs */
static const char *TAG = "MQTT";

/********************************** MQTT **************************************/

/* Definiciones */
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t);
static void mqtt_event_handler(void *, esp_event_base_t , int32_t , void *);
const char* mqtt_broker_default = IP_BROKER_MQTT;
const int mqttPort = PORT_MQTT;
//const char* mqttUser = USER_MQTT;
//const char* mqttPassword = PASSWD_MQTT;
static esp_mqtt_client_handle_t client;
esp_mqtt_client_handle_t MQTT_getClient(void);

// parámetros Wifi
//extern wifi_ap_record_t wifidata;
extern struct micro_t micro;


/*******************************************************************************
MQTT_log(): Subscripción al topic especificado.
*******************************************************************************/
void MQTT_log(const char * mqtt_in_out, const char * topic, const char * mensaje){

        char log_msg[100];
        uint64_t epochs;
        char timestamp[64]="";

        epochs=CRONO_getTime(timestamp, sizeof(timestamp));

        // Para debug interno
        sprintf(log_msg, "<%lld> %s: <%s> <%s>", epochs,mqtt_in_out,topic,mensaje);
        MQTT_publish("test/log",log_msg);

        //Para el cliente/Broker PC
        sprintf(log_msg, "%s: <%s>",topic,mensaje);
        MQTT_publish("test/logEsp",log_msg);

        FILE * f1 = SD_open("log.txt", "a");     // Abro el arcivo log.txt
        SD_printf(f1, "%s\n", log_msg);
        SD_close(f1);
}        

/*******************************************************************************
MQTT_subscribe(): Subscripción al topic especificado.
*******************************************************************************/
void MQTT_subscribe(const char * topic){

  esp_mqtt_client_subscribe(client, topic, 0);

}

/*******************************************************************************
 MQTT_publish(): publica mensaje en el topic especificado.
*******************************************************************************/
void MQTT_publish(const char * topic, const char * mensaje) {

  /* CON PUPLISH *********************************************************
  esp_mqtt_client_publish(client, topic, data, len, qos, retain) */
  esp_mqtt_client_publish(client, topic, mensaje, 0, 0, 0);

  /* CON ENQUEUE ********************************************************
  esp_mqtt_client_enqueue(client, topic, data, len, qos, retain, store);
  //esp_mqtt_client_enqueue(client, topic, mensaje, 0, 0, 0, 1);*/
}


/*******************************************************************************
 MQTT_processTopic(): lee el mensaje MQTT recibido cuando se dispara el evento
 ******************************************************************************/
 void MQTT_processTopic(const char * topic, const char * msg){

   /* Acciones a ejecutar para cada topic recibido */
   if(strcmp("test/inicio", topic)==0 || strcmp("app/inicio", topic)==0) {
        printf("MQTT: <test/inicio> Mensaje recibido: %s\n", msg);
        // Registro en el log
        if(strcmp("test/inicio", topic)==0) MQTT_log(MQTT_IN, "test/inicio", msg);                    
        micro.inicio=atoi(msg);
        if(micro.inicio==0 || micro.inicio>TIME_MAX){
                printf("Se limitó el tiempo inicio: %d\n", TIME_MAX);
                micro.inicio=TIME_MAX;
        }
        if(micro.run_stop==1){
                printf("ERROR- espera que se terminar el muestreo anterior\n");
                }
           else{
                micro.muestra=0;
                micro.run_stop=1;        // Inicio el timer
        }

    }
   if( strcmp("test/stop", topic)==0 || strcmp("app/stop", topic)==0 ) {
        printf("MQTT: <test/stop> Mensaje recibido: %s\n", msg);
        // Registro en el log
        if(strcmp("test/inicio", topic)==0) MQTT_log(MQTT_IN, "test/stop", msg);                    
        micro.run_stop=3;               // Indica que finalizó el muestreo
    }
 }

 
 /*******************************************************************************
 MQTT_suscripciones(): lee el mensaje MQTT recibido cuando se dispara el evento
 ******************************************************************************/
 void MQTT_suscripciones(void){

        MQTT_subscribe("test/inicio");
        MQTT_subscribe("test/stop");
        MQTT_subscribe("app/inicio");
        MQTT_subscribe("app/stop");

 };
 
 /*******************************************************************************
  MQTT_init(): Inicialización de MQTT
  ******************************************************************************/
 void MQTT_init(){

  MQTT_userInit(mqtt_broker_default);
 
 }

 /*******************************************************************************
 MQTT_userIinit(): Inicialización de MQTT definida por el usuario desde un archivo
  ******************************************************************************/
 void MQTT_userInit(const char * mqtt_broker){

   esp_mqtt_client_config_t mqtt_cfg = {
           //.uri = CONFIG_BROKER_URL,
           .host = mqtt_broker,//server,
           //.username = mqttUser,
           //.password = mqttPassword,
           .port = mqttPort,
   };


   ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
   client = esp_mqtt_client_init(&mqtt_cfg); //   Creates mqtt client handle based on the configuration.
   esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
   esp_mqtt_client_start(client); // Starts mqtt client with already created client handle.
   vTaskDelay(50 / portTICK_PERIOD_MS); // waiting 50 ms

   MQTT_suscripciones();

 }

 /* handle cliente MQTT ************************************************/
 esp_mqtt_client_handle_t MQTT_getClient(void)
 {
         return client;
 }


/*****************************************************************************/
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
        //esp_mqtt_client_handle_t client = event->client;
        client = event->client;

        // your_context_t *context = event->context;
        switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
                ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
                break;

        case MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
                break;

        case MQTT_EVENT_SUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                break;

        case MQTT_EVENT_UNSUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
                break;

        case MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                break;

        case MQTT_EVENT_DATA:   // Cuando recibo un mensaje a un topic que estoy subcripto.
                ESP_LOGI(TAG, "MQTT_EVENT_DATA");

                // obtener topic y mensaje recibido
                char rcv_topic[MAX_TOPIC_LENGTH]="";
                char rcv_message[MAX_MSG_LENGTH]="";
                strncpy(rcv_topic, event->topic, event->topic_len);
                strncpy(rcv_message, event->data, event->data_len);
                //ESP_LOGI(TAG, "TOPIC RECEIVED: %s", rcv_topic );
                //ESP_LOGI(TAG, "MESSAGE RECEIVED: %s", rcv_message);
                MQTT_processTopic(rcv_topic, rcv_message);
                break;

        case MQTT_EVENT_ERROR:
                ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
                break;

        default:
                ESP_LOGI(TAG, "Other event id:%d", event->event_id);
                break;
        }
        return ESP_OK;
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
        ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
        mqtt_event_handler_cb(event_data);
}
