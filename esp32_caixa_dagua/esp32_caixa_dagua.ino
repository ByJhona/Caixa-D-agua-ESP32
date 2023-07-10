#include <WiFi.h>
#include <string.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"
#include "ESPAsyncWebServer.h"



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

const char* ntpServer = "0.br.pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

#define TRIG 2
#define ECHO 4


// GLOBAIS
String data_hora = " ";
float nivel_agua = 0.0;
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_APSTA);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  xTaskCreatePinnedToCore(conectar_wifi, "conectar_wifi", 2048, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(criar_ap, "conectar AP", 2048, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(criar_server, "criar servidor", 2048, NULL, 5, NULL, 0);


  xTaskCreatePinnedToCore(receber_sensor_nivel_agua, "receber ultrassonico", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(receber_data_hora, "receber horario", 10000, NULL, 1, NULL, 1);


  xTaskCreatePinnedToCore(conectar_firebase, "Conectar ao firebase", 10000, NULL, 4, NULL, 0);
  xTaskCreatePinnedToCore(enviar_fb_nivel_agua, "Envia agua", 10000, NULL, 2, NULL, 0);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {

  
}




void enviar_fb_nivel_agua(void* params) {
  while (1) {
    if (Firebase.ready() && data_hora != " " && WiFi.status() == WL_CONNECTED) {
      Serial.printf("Set Agua... %s\n", Firebase.RTDB.setFloat(&fbdo, ("/nivel_agua/" + data_hora), nivel_agua) ? "Agua enviada para o FB" : fbdo.errorReason().c_str());
    }
    vTaskDelay(5000);
  }
}

void receber_sensor_nivel_agua(void* params) {
  while (1) {
    int distancia;
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    unsigned long tempoSom = pulseIn(ECHO, HIGH);
    //nivel_agua = (tempoSom / 58);

    for (int i = 0; i < 100; i++) {
      nivel_agua = i;
      vTaskDelay(5000);
    }
  }
}

void conectar_firebase(void* params) {

  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
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
      Firebase.setFloatDigits(5);
      config.timeout.serverResponse = 1000;
      vTaskSuspend(NULL);
    }
    vTaskDelay(100);
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
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.softAPIP());
  }
}

void receber_data_hora(void* params) {
  struct tm timeinfo;
  while (1) {
    getLocalTime(&timeinfo);
    char data[64];
    //strftime(data, 64, "%d-%m-%Y %H:%M:%S", &timeinfo);
    strftime(data, 64, "%H:%M:%S", &timeinfo);
    data_hora = data;
    vTaskDelay(1000);
  }
}

void criar_ap(void* params) {
  while (1) {

    WiFi.softAP(AP_NOME, AP_SENHA);

    vTaskDelay(60000);
    vTaskDelete(NULL);
  }
}
void criar_server(void* params) {
  
  while (1) {
    
    server.on("/nivel_agua", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send_P(200, "text/plain", String(nivel_agua).c_str());
    });
    server.begin();
    vTaskDelay(50000);
    vTaskDelete(NULL);
  }
}