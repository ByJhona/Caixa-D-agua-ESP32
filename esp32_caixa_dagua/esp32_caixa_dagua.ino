#include <WiFi.h>
#include <string.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "time.h"
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>




#define WIFI_NOME "OI"
#define WIFI_SENHA "12345678j"
#define AP_NOME "ESP_32_PROJECT"
#define AP_SENHA "123456789"
#define TEMPO_ESPERA_MAXIMO_WIFI 10000
#define TEMPO_DELAY_TASK 3000
#define MAX_LITROS 10


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
float nivel_agua = 0.0;
int nivel_turbidez = 0;
bool estado_rele = false;
String data_hora = " ";
const char* serverNameTurb = "http://192.168.4.100/nivel_turbidez";

const char html[] PROGMEM = R"rawliteral( 
<!DOCTYPE html>
<html>

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="refresh" content="2">
    <title>SAAF</title>
</head>
<style>
    * {
        color: #433939;
        box-sizing: border-box;
        margin: 0;
        padding: 0;
        justify-content: center;
    }

    body {

        display: flex;
        flex-direction: column;
        align-items: center;
        width: 95vw;
        background-size: auto;
        background-color: rgb(247, 247, 247);
    }

    .nav {
        width: 90vw;
        height: 4rem;
        background-color: rgb(255, 255, 255);
        display: flex;
        justify-content: space-between;
        padding: 1rem;
        border: solid 1px rgb(201, 201, 201);
        border-radius: 0 0 1rem 1rem;

    }

    .botoes {
        margin-bottom: 1rem;
        display: flex;
        flex-direction: row;

        justify-content: space-between;
    }

    .botao-liga {
        border: solid 1px rgb(202, 202, 202);
        border-radius: 1rem;

        background-color: rgb(40, 219, 16);
        border: none;
        width: 10rem;
        height: 4rem;
    }

    .botao-desliga {
        border: solid 1px rgb(202, 202, 202);
        border-radius: 1rem;

        background-color: rgb(231, 95, 95);
        border: none;
        width: 10rem;
        height: 4rem;
    }

    .info {
        display: flex;

        justify-content: space-around;
        flex-direction: column;
        align-items: center;
        width: 80vw;
        height: 90vh;
        margin-top: 3rem;
        margin-bottom: 3rem;
    }

    .niveis {
        width: 70vw;
        display: flex;
        flex-direction: row;
        justify-content: space-between;

    }

    .nivel-agua {
        display: flex;
        align-items: center;

        font-size: 10rem;
        width: 30vw;
        height: 16rem;
        border: solid 1px rgb(202, 202, 202);
        background-color: rgb(255, 255, 255);
        border-radius: 1rem;
        overflow: hidden;
    }

    .nivel-turbidez {
        display: flex;
        align-items: center;
        font-size: 10rem;
        width: 30vw;
        height: 16rem;
        border: solid 1px rgb(202, 202, 202);
        background-color: rgb(255, 255, 255);

        border-radius: 1rem;
        overflow: hidden;
        margin-bottom: 1rem;
    }

    @media (max-width: 700px) {
        .niveis {

            flex-direction: column;


        }

        .nivel-agua {

            width: 70vw;


        }

        .botoes {

            display: flex;
            flex-direction: row;
            width: 70vw;
            justify-content: space-between;

        }

        .nivel-turbidez {

            width: 70vw;

        }
    }
</style>

<body>
    <div class="nav">
        <h2>SAAF</h2>
        <h2>DASHBOARD</h2>
    </div>
    </div>

    <div class="info">
        <div class="botoes">

            <button class="botao-liga" onclick="ligar_bomba()">Ligar Bomba D'Agua</button>
            <button class="botao-desliga" onclick="desligar_bomba()">Desligar Bomba D'Agua</button>
        </div>

        <div class="niveis">
            <div class="nivel-turbidez">

                %NIVEL_TURBIDEZ%
            </div>
            <div class="nivel-agua">

                %NIVEL_AGUA%
            </div>
        </div>
    </div>
    <script>function ligar_bomba() {
            
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/?ligar=true");
            xhr.send();
        }
        function desligar_bomba() {
            
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/?ligar=false");
            xhr.send();
        }
    </script>


</body>

</html>
)rawliteral";
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_APSTA);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  conectar_wifi();
  criar_ap();
  criar_server();


  conectar_firebase();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  receber_sensor_nivel_agua();
  receber_sensor_nivel_turbidez();
  verificar_maximo_caixa();
  receber_data_hora();
  receber_fb_estado_rele();
  if (WiFi.status() == WL_CONNECTED) {
    enviar_fb_nivel_agua();
    enviar_fb_nivel_turbidez();
    enviar_fb_estado_rele();
  }

  delay(1000);
}


void receber_fb_estado_rele(){
   if (Firebase.ready() && WiFi.status() == WL_CONNECTED) {
    Serial.printf("Get Rele... %s\n", Firebase.RTDB.getBool(&fbdo, F("/estado_rele/"), &estado_rele) ? "Estado do rele atualizado" : fbdo.errorReason().c_str());
  }
}

void enviar_fb_nivel_agua() {

  if (Firebase.ready() && WiFi.status() == WL_CONNECTED) {
    Serial.printf("Set Agua... %s\n", Firebase.RTDB.setFloat(&fbdo, ("/nivel_agua/" + data_hora), nivel_agua) ? "Agua enviada para o FB" : fbdo.errorReason().c_str());
  }
}

void enviar_fb_estado_rele() {

  if (Firebase.ready() && WiFi.status() == WL_CONNECTED) {
    Serial.printf("Set Agua... %s\n", Firebase.RTDB.setBool(&fbdo, F("/estado_rele/"), estado_rele) ? "Agua enviada para o FB" : fbdo.errorReason().c_str());
  }
}

void enviar_fb_nivel_turbidez() {
  if (Firebase.ready() && WiFi.status() == WL_CONNECTED) {
    Serial.printf("Set Turbidez... %s\n", Firebase.RTDB.setInt(&fbdo, F("/nivel_turbidez/"), nivel_turbidez) ? "Turbidez enviada para o FB" : fbdo.errorReason().c_str());
  }
}

void receber_sensor_nivel_agua() {

  int distancia = 0;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  unsigned long tempoSom = pulseIn(ECHO, HIGH);
  nivel_agua = (tempoSom / 58);
}

void receber_sensor_nivel_turbidez() {

  nivel_turbidez = (int)atof(httpGETRequest(serverNameTurb).c_str());
  Serial.println(nivel_turbidez);
}

void verificar_maximo_caixa() {

  if (nivel_agua >= MAX_LITROS && estado_rele == true) {
    estado_rele = false;
    Serial.printf("Set MAX... %s\n", Firebase.RTDB.setBool(&fbdo, F("/estado_rele/"), estado_rele) ? "Turbidez enviada para o FB" : fbdo.errorReason().c_str());
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
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  Firebase.setFloatDigits(5);
  config.timeout.serverResponse = 1000;
}

void conectar_wifi() {


  WiFi.disconnect();
  Serial.print("Conectando-se ao WiFi");
  WiFi.begin(WIFI_NOME, WIFI_SENHA);

  unsigned long TEMPO_ESPERA_ATUAL_WIFI = millis();
  // Essa função while pode ficar em loop, tentar limitar com um tempo limite
  while (WiFi.status() != WL_CONNECTED && (millis() - TEMPO_ESPERA_ATUAL_WIFI < TEMPO_ESPERA_MAXIMO_WIFI)) {
    delay(1000);
    Serial.print('.');
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Não foi possivel conectar!!");
    return;
  }


  Serial.println("");
  Serial.println("Conectado!!");
  Serial.print("Connected com IP: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
}

void receber_data_hora() {
  struct tm timeinfo;

  getLocalTime(&timeinfo);
  char data[64];
  //strftime(data, 64, "%d-%m-%Y %H:%M:%S", &timeinfo);
  strftime(data, 64, "%H:%M:%S", &timeinfo);
  data_hora = data;
}

void criar_ap() {
  WiFi.softAP(AP_NOME, AP_SENHA);
  delay(1000);
}
void criar_server() {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("ligar")) {
      String input = request->getParam("ligar")->value();
      Serial.println(input);
      if (input == "true") {
        estado_rele = true;
      } else if (input == "false") {
        estado_rele = false;
      }
    }
    request->send_P(200, "text/html", html, processor);
  });

  server.on("/estado_rele", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", String(estado_rele).c_str());
  });

  server.on("/estado_rele", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", String(estado_rele).c_str());
  });
  server.begin();
  delay(1000);
}
String processor(const String& var) {
  if (var == "NIVEL_AGUA") {
    return (String)nivel_agua;
  } else if (var == "NIVEL_TURBIDEZ") {
    return (String)nivel_turbidez;
  }
  return String();
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "--";

  if (httpResponseCode > 0) {

    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}