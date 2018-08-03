//################# LIBRARIES ##########################
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//################# Settings ##########################
#define ssid "SFR_9C08"
#define password "cerolutenocks7arally"

#define serverUrl "http://requestbin.net/r/1iqgqlx1"

#define interval 30000 // soit 30 secondes            // interval between data requests in milliseconds
#define millisMax 86400000                            // maximum interval scince system start

#define refreshMax 60 // = 1,8e+6 / interval (soit toutes les 30 minutes)


//################# Other variables ##########################

// Variables used by the display
int const pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int const numberOfHorizontalDisplays = 4;
int const numberOfVerticalDisplays   = 1;
String message;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
int const wait   = 300;               // In milliseconds between scroll movements
int const spacer = 1;                 // space between two char
int const width  = 5 + spacer;        // The font width is 5 pixels


// Variables used for control
long previousMillis = 0;          // will store last time Message was updated
//const int interval = 30000;          // interval between data requests in milliseconds
//const int intervalMax = 864;         // maximum interval scince system start
int failCount = 0;                // will store number of time the server didn't answer
int refreshCount = 0;             // will store number of time 'message' variable was refresh

//################# Functions ##########################

// Print a string on the display
void display_message(String message){
  int ind = matrix.width() / width - message.length();
  if (ind >= 0)
  {
    display_message_without_scroll(message);
  }
  else
  {
    display_message_with_scroll(message);
  }
}

// Display a message using scrolling
void display_message_with_scroll(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);

    //width  = 5 + spacer; // The font width is 5 pixels

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    //int y = (matrix.height() - 8) / 2; // center the text vertically if more than 1 matrix vertically else y=0
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, 0 /*or 'y' */, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background OFF, reverse these to invert the display!
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}

void display_message_without_scroll(String message){
  int x = matrix.width();
  for ( int i = message.length()-1 ; i>=0; i-- ) { //tester peut être décalage de 1 ???
    matrix.drawChar(x - width, 0, message[i], HIGH, LOW, 1);
    x -= width;
  }
  matrix.write();
}


// Get the number from the server
void GetServerMessage(){
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

      Serial.println(F("Try to connect server"));

      HTTPClient http;  //Declare an object of class HTTPClient

      http.begin(serverUrl);  //Specify request destination
      int httpCode = http.GET();                       //Send the request

      Serial.println(httpCode);

      if (httpCode > 0) { //Check the returning code
        Serial.println(F("Data received"));

        message = http.getString();   //Get the request response message
        Serial.println(message);             //Print the response message
        } else {
          // Server didn't respond properly
          Serial.println(F("Server didn't respond properly"));
          //faire qqc jsp quoi led ou message
        }
    http.end();   //Close connection

  } else {
    // If wifi connection is down
    Serial.println(F("Wifi connection down"));
    //faire qqc
    while (true){
      display_message("Votre connection wifi semble défaillante : vérifiez la et redémarrer l'afficheur");
    }
  }
}

// Declare reset function at adress 0
void(* resetFunc) (void) = 0;


//################# Program ##########################
void setup() {
  // Start connection with pc
  Serial.begin(115200); // initialize serial communications
  Serial.println(F("Lancement"));

  // Set up display
  matrix.setIntensity(2);    // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);  // The first display is position upside down
  matrix.setRotation(1, 1);  // The first display is position upside down
  matrix.setRotation(2, 1);  // The first display is position upside down
  matrix.setRotation(3, 1);  // The first display is position upside down

  //
  message = "...";              // écrire un message qui s'affiche au lancement
  display_message(message);
  Serial.println(message);

  // Set up wifi connection
  WiFi.begin(ssid, password);
  // The client will try only 20 times to connect to wifi
  int count = 0;
  while (WiFi.status() != WL_CONNECTED || count == 20) { // '||' c'est un 'ou'
    count++;
    delay(1000);
    Serial.println(F("Connecting.."));
  }

  GetServerMessage();
}


void loop(){
  display_message(message); // Display the message
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    GetServerMessage();
    refreshCount++;

    // We will not test the following condition each time that's why we used a counter
    if (refreshCount == refreshMax){
      refreshCount = 0;
      // In order to limit the memory usage to store the time data, we periodically restart the card
      if (currentMillis >= millisMax){
        resetFunc();
      }
    }
  }


/*
à faire

on vérifie tous les x requêtes au serv :
  si temps max atteint : on redémarre le système

  où x = 1,8e+6 / interval (soit toutes les 30 minutes
OK
à refaire je suis trop con
faut juste remplacer le test par un ajustement du compteur en changeant le max

si nombre max d'échecs atteint, on affiche un message pour dire de nous contacter

si co wifi pas bonne afficher un message
OK (peut être revoir en fonction de ce qu'on veut)
 */

}
