
#include <WiFi.h>
#define WIFI_NOME "OI"
#define WIFI_SENHA "12345678j"
#define TEMPO_ESPERA_MAXIMO_WIFI 20000

// Firebase
#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
#define API_KEY "AIzaSyARa9LjX-_oE7nGRihgHg893UEw2YNAyGw"
#define DATABASE_URL "https://caixa-d-agua-esp32-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define FIREBASE_EMAIL "esp32@iot.com"
#define FIREBASE_SENHA "esp32iot"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

//FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//TaskHandle_t conectar_wifi;

#define R 2
#define G 4
#define B 5

void setup() {
  Serial.begin(115200);

  xTaskCreate(conectar_wifi, "conectar_wifi", 2048, NULL, 1, NULL);
  //xTaskCreate(conectar_firebase, "conectar_firebase", 2048, NULL, 2, NULL);

  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
}

void loop() {

  if (WiFi.status() == WL_CONNECTED && !Firebase.ready()) {
    Serial.println("TESTE");
    conectar_firebase();
  }


  delay(1000);

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

int vermelho = Firebase.RTDB.getInt(&fbdo, F("/led/vermelho"));
int verde = Firebase.RTDB.getInt(&fbdo, F("/led/verde"));
int azul = Firebase.RTDB.getInt(&fbdo, F("/led/azul"));

    
      digitalWrite(R, vermelho);
    
      digitalWrite(G, verde);
      digitalWrite(B, azul);




    Serial.println();
  }
}


void conectar_firebase() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  Serial.println(Firebase.ready());
  Serial.println("Entrou aqui 2");

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_SENHA;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  Serial.println("Entrou aqui 3");

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
  Serial.println("Entrou aqui 4");
  Firebase.reconnectWiFi(false);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
  Serial.println(">>");
  Serial.println(Firebase.ready());
  Serial.println("<<");

  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void conectar_wifi(void* params) {
  //--------------------

  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }
    WiFi.disconnect();
    Serial.print("Conectando-se ao WiFi");
    WiFi.begin(WIFI_NOME, WIFI_SENHA);

    unsigned long TEMPO_ESPERA_ATUAL_WIFI = millis();
    // Essa função while pode ficar em loop, tentar limitar com um tempo limite
    while (WiFi.status() != WL_CONNECTED && (millis() - TEMPO_ESPERA_ATUAL_WIFI < TEMPO_ESPERA_MAXIMO_WIFI)) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
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