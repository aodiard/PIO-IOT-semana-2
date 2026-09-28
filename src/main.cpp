#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";

const String RTDB_URL = "Completar con la URL de su proyecto";
const String TELEMETRIA_PATH = RTDB_URL + "/nodos/nodo_01/telemetria.json";

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastTelemetryTime = 0;
const unsigned long telemetryInterval = 5000;

void enviarTelemetria()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("[ERROR] Lectura fallida del sensor DHT22.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  if (https.begin(client, TELEMETRIA_PATH))
  {
    https.addHeader("Content-Type", "application/json");
    String payload = "{\"temperatura\":" + String(t, 1) +
                     ",\"humedad\":" + String(h, 1) +
                     ",\"dispositivo_id\":\"esp32_nodo_01\"}";

    int httpCode = https.PATCH(payload);
    if (httpCode > 0)
    {
      Serial.printf("[TELEMETRÍA] Enviada con éxito: %.1f °C | %.1f %%\n", t, h);
    }
    https.end();
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  Serial.print("\n[RED] Conectando a Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\n[RED] Conectado. IP: " + WiFi.localIP().toString());
}

void loop()
{
  if (millis() - lastTelemetryTime >= telemetryInterval)
  {
    lastTelemetryTime = millis();
    enviarTelemetria();
  }
}