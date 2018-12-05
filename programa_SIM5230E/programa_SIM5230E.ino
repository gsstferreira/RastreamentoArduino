#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define SIMCOM_3G

#define FONA_PWRKEY 8
#define FONA_RST 9
#define FONA_TX 2
#define FONA_RX 3

char url[250];
String tags;
uint16_t type;

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial);

  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH);
  pinMode(FONA_PWRKEY, OUTPUT);

  Serial.begin(115200);

  fonaSS.begin(115200);

  fonaSS.println("AT+IPR=4800");
  delay(100);
  fonaSS.begin(4800);
  if (! fona.begin(fonaSS)) {
    while (1);
  }
  type = fona.type();
  fona.setFunctionality(1);
  fona.setNetworkSettings(F("zap.vivo.com.br"), F("vivo"), F("vivo"));

  delay(3000);

  while(!fona.enableGPRS(true)){}
  while(!fona.enableGPS(true)) {}
}

void loop() {
  if (fona.available()) {
     fona.read();
  }

  if(Serial.available() > 0) {
    digitalWrite(LED_BUILTIN, LOW);
    tags = Serial.readString();

    sendRequest();

    
    while (fona.available()) {
      fona.read();
    }
  }
  
  digitalWrite(LED_BUILTIN, HIGH);

}
  
void flushSerial() {
  while (Serial.available())
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}

uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;
  return buffidx;
}

// Power on the module
void powerOn() {
  digitalWrite(FONA_PWRKEY, LOW);
  delay(200);
  digitalWrite(FONA_PWRKEY, HIGH);
}

void sendRequest() {

  char lat[16];
  char lng[16];
  
  float latitude,longitude,lixo;
    
  if (fona.getGPS(&latitude, &longitude, &lixo,&lixo,&lixo)) {
  
    dtostrf(latitude, 0, 6, lat);
    dtostrf(longitude, 0, 6, lng);
  
    sprintf(url,"GET /Localizacao/PostarPorQuery?T=%s&V=c7108c4b-5526-4ecf-ad2a-a97901703c16&lat=%s&lng=%s HTTP/1.1\r\nHost: 52.52.244.199\r\n\r\n",tags.c_str(), lat,lng);
    tags = "";
    fona.postData("52.52.244.199",80,"HTTP",url);    
  }
}
