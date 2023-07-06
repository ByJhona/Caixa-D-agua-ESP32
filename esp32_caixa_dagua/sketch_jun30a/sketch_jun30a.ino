#include <WiFi.h>
#include <string.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

#define WIFI_NOME "OI"
#define WIFI_SENHA "12345678j"
#define TEMPO_ESPERA_MAXIMO_WIFI 20000

#define API_KEY "AIzaSyARa9LjX-_oE7nGRihgHg893UEw2YNAyGw"
#define DATABASE_URL "https://caixa-d-agua-esp32-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define FIREBASE_EMAIL "esp32@iot.com"
#define FIREBASE_SENHA "esp32iot"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
const char* ntpServer = "0.br.pool.ntp.org";
const long gmtOffset_sec = -10800;
const int daylightOffset_sec = 0;

#define TRIG 2
#define ECHO 4
#define TURB 35

// GLOBAIS
int nivel_turbidez = 0;
float nivel_agua = 0;
void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(TURB, INPUT);

  //pinMode(TURB, INPUT);
  xTaskCreate(conectar_wifi, "conectar_wifi", 2048, NULL, 1, NULL);
  xTaskCreate(receber_nivel_turbidez, "receber turbidez", 10000, NULL, 1, NULL);
  xTaskCreate(receber_nivel_agua, "receber ultrassonico", 2048, NULL, 1, NULL);


  //xTaskCreate(sensorHC, "tarefa1", 10000, NULL, 1, NULL);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {

  if (WiFi.status() == WL_CONNECTED && !Firebase.ready()) {
    conectar_firebase();
    delay(1000);
  }
  //Serial.println(analogRead(13));

  delay(1000);


  if (Firebase.ready() && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    Serial.printf("Set string... %s\n", Firebase.RTDB.setFloat(&fbdo, ("/nivel_agua/" + receber_data_hora()), nivel_agua) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set string... %s\n", Firebase.RTDB.setInt(&fbdo, F("/nivel_turbidez/"), nivel_turbidez) ? "ok" : fbdo.errorReason().c_str());
  }
}

void receber_nivel_agua(void* params) {

  while (1) {
    int distancia;
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    unsigned long tempoSom = pulseIn(ECHO, HIGH);
    nivel_agua = (tempoSom / 58) / 100;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void receber_nivel_turbidez(void* params) {
  while (1) {
    int media_turb = 0;
    for (int i = 0; i < 100; i++) {
      media_turb += analogRead(TURB);
    }
    media_turb /= 100;
    Serial.println(media_turb);

    if (media_turb >= 400 && media_turb <= 2100) {
      nivel_turbidez = map(media_turb, 400, 2100, 100, 0);
    } else if (media_turb < 400) {
      nivel_turbidez = map(400, 400, 2100, 100, 0);
    } else if (media_turb > 2100) {
      nivel_turbidez = map(2100, 400, 2100, 100, 0);
    }
    Serial.println(nivel_turbidez);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void conectar_firebase() {
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

  config.timeout.serverResponse = 10 * 1000;
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
    Serial.println();
  }
}

String receber_data_hora() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  char data[64];
  //strftime(data, 64, "%d-%m-%Y %H:%M:%S", &timeinfo);
  strftime(data, 64, "%H:%M", &timeinfo);
  return data;
}