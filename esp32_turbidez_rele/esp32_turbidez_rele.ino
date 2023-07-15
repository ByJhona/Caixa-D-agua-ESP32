#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <string.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#define WIFI_NOME "ESP_32_PROJECT"
#define WIFI_SENHA "123456789"
#define TEMPO_ESPERA_MAXIMO_WIFI 20000
#define TEMPO_DELAY_TASK 3000

#define API_KEY "AIzaSyARa9LjX-_oE7nGRihgHg893UEw2YNAyGw"
#define DATABASE_URL "https://caixa-d-agua-esp32-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define FIREBASE_EMAIL "esp32@iot.com"
#define FIREBASE_SENHA "esp32iot"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
AsyncWebServer server(80);

IPAddress local_IP(192, 168, 4, 100);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);




#define TURB 35

// GLOBAIS
int nivel_turbidez = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TURB, INPUT);

  xTaskCreatePinnedToCore(conectar_wifi, "conectar_wifi", 2048, NULL, 5, NULL, 1);

  xTaskCreatePinnedToCore(criar_server, "criar servidor", 2048, NULL, 5, NULL, 0);

  xTaskCreatePinnedToCore(receber_nivel_turbidez, "receber turbidez", 10000, NULL, 1, NULL, 1);
}

void loop() {
  // Nada aqui, volte mais tarde!!
}

void receber_nivel_turbidez(void* params) {
  while (1) {
    int media_turb = 0;
    for (int i = 0; i < 100; i++) {
      media_turb += analogRead(TURB);
    }
    media_turb /= 100;

    if (media_turb >= 400 && media_turb <= 2100) {
      nivel_turbidez = map(media_turb, 400, 2100, 100, 0);
    } else if (media_turb < 400) {
      nivel_turbidez = map(400, 400, 2100, 100, 0);
    } else if (media_turb > 2100) {
      nivel_turbidez = map(2100, 400, 2100, 100, 0);
    }
    vTaskDelay(500);

    for (int i = 0; i < 100; i++) {
      nivel_turbidez = i;
      vTaskDelay(3000);
    }
  }
}


void conectar_wifi(void* params) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    WiFi.disconnect();
    Serial.print("Conectando-se ao WiFi");


    if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
    WiFi.begin(WIFI_NOME, WIFI_SENHA);


    unsigned long TEMPO_ESPERA_ATUAL_WIFI = millis();
    // Essa função while pode ficar em loop, tentar limitar com um tempo limite
    while (WiFi.status() != WL_CONNECTED && (millis() - TEMPO_ESPERA_ATUAL_WIFI < TEMPO_ESPERA_MAXIMO_WIFI)) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      Serial.print('.');
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Não foi possivel conectar!!");
      continue;
    }
    Serial.println("");
    Serial.println("Conectado!!");
    Serial.print("Connected com IP: ");
    Serial.println(WiFi.localIP());
  }
}

void criar_server(void* params) {

  while (1) {
    server.on("/nivel_turbidez", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send_P(200, "text/plain", String(nivel_turbidez).c_str());
    });
    server.begin();
    vTaskDelay(50000);
    vTaskDelete(NULL);
  }
}
