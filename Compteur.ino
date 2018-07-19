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

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:	// display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

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
