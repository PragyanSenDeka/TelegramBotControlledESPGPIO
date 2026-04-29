#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Put Your WiFi Name";
const char* password = "Put Your WiFi Password";

#define BOTtoken "Put your bot token"  // your Bot Token (Get from Botfather)
#define CHAT_ID "YOUR_CHAT_ID" // from ID bot, 10 digit User ID

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
//paku
const int fanPin = 2;  // Gate of IRLZ44N connected to GPIO 2
bool fanState = LOW;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control the fan.\n\n";
      welcome += "/fan_on to turn the Fan ON\n";
      welcome += "/fan_off to turn the Fan OFF\n";
      welcome += "/state to get the current Fan state\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/fan_on") {
      fanState = HIGH;
      digitalWrite(fanPin, fanState);
      bot.sendMessage(chat_id, "Fan is now ON", "");
    }

    if (text == "/fan_off") {
      fanState = LOW;
      digitalWrite(fanPin, fanState);
      bot.sendMessage(chat_id, "Fan is now OFF", "");
    }

    if (text == "/state") {
      if (digitalRead(fanPin)) {
        bot.sendMessage(chat_id, "Fan is currently ON", "");
      } else {
        bot.sendMessage(chat_id, "Fan is currently OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");
    client.setTrustAnchors(&cert);
  #endif

  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);  // Ensure fan is OFF on boot

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}