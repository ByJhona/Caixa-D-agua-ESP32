#define TURB 35
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define WIFI_NOME "OI"
#define WIFI_SENHA "12345678j"

void setup() {
  Serial.begin(115200);
  pinMode(TURB, INPUT);
    xTaskCreate(receber_turbidez, "firebase_turbidez", 10000, NULL, 1, NULL);

  WiFi.disconnect();
    Serial.print("Conectando-se ao WiFi");
    WiFi.begin(WIFI_NOME, WIFI_SENHA);
    while (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      Serial.print('.');
    }
}

void loop() {
  
}

void receber_turbidez( void* params) {
  while(1){
  int media_turb = 0;

  for (int i = 0; i < 100; i++) {
    media_turb += analogRead(TURB);
  }
  media_turb /= 100;
  Serial.println(media_turb);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  //Tensao de 3.3V
  /*if (media_turb > 400 && media_turb < 2000) {
    Serial.println(map(media_turb, 400, 2000, 100, 1));
    Serial.println((media_turb));
  }*/
  //Tensao de 5V
  /*if (media_turb > 900 && media_turb < 3500) {
    Serial.println(map(media_turb, 900, 3500, 100, 1));
    Serial.println((media_turb));
  }*/
}/*
void firebase_turbidez(void* params) {
  while (1) {
    int val_turbidez = receber_turbidez();
    if (val_turbidez > 400 && val_turbidez < 2000) {
      int val_conv = map(val_turbidez, 400, 2000, 100, 1);
      Firebase.RTDB.setInt(&fbdo, ("/nivel_turbidez/"), val_conv);
      Serial.println((val_turbidez));

       vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}*/
