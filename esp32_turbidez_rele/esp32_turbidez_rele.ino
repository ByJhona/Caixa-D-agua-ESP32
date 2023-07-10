#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <string.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define WIFI_NOME "OI"
#define WIFI_SENHA "12345678j"
#define AP_NOME "ESP_32_PROJECT"
#define AP_SENHA "123456789"
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




#define TURB 35

// GLOBAIS
int nivel_turbidez = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TURB, INPUT);
  WiFi.mode(WIFI_MODE_APSTA);

  xTaskCreatePinnedToCore(conectar_wifi, "conectar_wifi", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(criar_ap, "conectar AP", 2048, NULL, 5, NULL, 0);

  xTaskCreatePinnedToCore(conectar_firebase, "Conectar ao firebase", 10000, NULL, 4, NULL, 0);
  xTaskCreatePinnedToCore(receber_nivel_turbidez, "receber turbidez", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(enviar_fb_nivel_turbidez, "Envia Turbidez", 10000, NULL, 2, NULL, 0);
}

void loop() {
  // Nada aqui, volte mais tarde!!
}

void enviar_fb_nivel_turbidez(void* params) {
  while (1) {
    if (Firebase.ready() && WiFi.status() == WL_CONNECTED) {
      Serial.printf("Set Turbidez... %s\n", Firebase.RTDB.setInt(&fbdo, F("/nivel_turbidez/"), nivel_turbidez) ? "Turbidez enviada para o FB" : fbdo.errorReason().c_str());
    }
    vTaskDelay(TEMPO_DELAY_TASK);
  }
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
  }
}

void conectar_firebase(void* params) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED && !Firebase.ready()) {
      Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
      /* Assign the api key (required) */
      config.api_key = API_KEY;
      /* Assign the user sign in credentials */
      auth.user.email = FIREBASE_EMAIL;
      auth.user.password = FIREBASE_SENHA;
      /* Assign the RTDB URL (required) */
      config.database_url = DATABASE_URL;
      /* Assign the callback function for the long running token generation task */
      config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
      fbdo.setResponseSize(2048);
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(false);
      Firebase.setDoubleDigits(5);
      config.timeout.serverResponse = 1000;
      vTaskSuspend(NULL);
    }
    vTaskDelay(1000);
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
    Serial.println();
  }
}

void criar_ap(void* params) {
  while (1) {
  
    WiFi.softAP(AP_NOME, AP_SENHA);
    Serial.println("OH MDS OBG CHATGPT");

    vTaskDelay(60000);
    vTaskDelete(NULL);
  }
}
