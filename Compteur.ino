//Libraries used to display
#include <MD_MAX72xx.h>
#include <SPI.h>

//Libraries used to get data
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW  //Choisir en fonction du matériel utilisé https://majicdesigns.github.io/MD_MAX72XX/class_m_d___m_a_x72_x_x.html#a88ea7aada207c02282d091b7be7084e6
#define MAX_DEVICES 5

#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN 10 // or SS

#define MAX_NB_CHAR 5

String message

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Text parameters
#define CHAR_SPACING 1 // pixels between characters


// WiFi login parameters - network name and password
#define ssid ""
#define password ""

// Server connection settings
#define serverUrl ""



void GetServerMessage() {
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin(serverUrl);
    int httpCode = http.GET();
    if (httpCode == 200) { // if all is right, then we update message variable
      message = http.getString();
      Serial.println(message);
    }
    http.end();
}

/*
-Preparer le buffer à partir du message à afficher
exemple 'uint8_t globalBuf[2*(8*MAX_DEVICES)]' : il faut définir sa taille directement car pas pratique dynamiquement quand peu d'espace

on insère x premières colonnes vides dans le buffer
pour chaque caractère du texte :
  on charge le caractère dans un buffer temp
  on copie le contenu du buffer à la suite des infos précédentes dans le buffer global
  on insère x colonne vide d'espace avant le prochain caractère

-Affiche le texte
si message plus long que place dispo : on fait dérouler le texte
  affichage ça défile vers la gauche une fois en entier, puis après petit temps ça recommence
si message plus court on centre le texte à droite ou à gauche (paramètre à définir)
*/

void setup() {
  message = "error"

  // initialize serial communications
  Serial.begin(115200);

  // initialize wifi connection
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);

  if(!wifiManager.autoConnect(ssid, password)) {
    Serial.println(F("failed to connect and timeout occurred"));
    delay(6000);
    ESP.reset(); //reset and try again
    delay(180000);
  }

  // initialize display
  mx.begin();

}

void loop() {
  GetServerMessage()
  printText(0, MAX_DEVICES-1, message);
}
