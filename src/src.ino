#include <SPI.h>
#include <Ethernet.h>

#include <SoftwareSerial.h>
#include <ParallaxLCD.h>

/* ============================================================================
	Settings
============================================================================ */

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const char* ip_to_str(const uint8_t*);

IPAddress ip		(192,168,1,177);
IPAddress gateway	(192,168,1,1);
IPAddress subnet	(255,255,0,0);

EthernetServer server(80); // port

ParallaxLCD lcd(7, 2, 16, 9600); //  pin, rows, cols, baud

/* ============================================================================
	Main
============================================================================ */

void setup()
{
	lcd.setup();
	delay(1000); // Wait for the LCD to start up
	lcd.backLightOn();

	lcd.at(0,0, "Waiting for DHCP lease...");

	if (Ethernet.begin(mac) == 0) {
		lcd.empty();
		lcd.at(0,0, "Static IP");
		Ethernet.begin(mac, ip, gateway, subnet);
	} else {
		lcd.empty();
		lcd.at(0,0, "DHCP IP");
	}

	lcd.at(1,0, ip_to_str(Ethernet.localIP()));
	server.begin();
}

void loop()
{

}

/* ============================================================================
	Networking Basics
============================================================================ */

void startDebugSerial ()
{
	Serial.begin(9600);
	// This loop is for compatibility with Leonardo.
	// Other models will still work fine too.
	while (!Serial) { }
}


const char* ip_to_str(const uint8_t* ipAddr)
{
	static char buf[16];
	sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	return buf;
}

const char* ip_to_str(const IPAddress ipAddr)
{
	static char buf[16];
	sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	return buf;
}








void handleClient ()
{
// listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
	Serial.println("new client");
	// an http request ends with a blank line
	boolean currentLineIsBlank = true;
	while (client.connected()) {
	  if (client.available()) {
		char c = client.read();
		Serial.write(c);
		// if you've gotten to the end of the line (received a newline
		// character) and the line is blank, the http request has ended,
		// so you can send a reply
		if (c == '\n' && currentLineIsBlank) {
		  // send a standard http response header
		  client.println("HTTP/1.1 200 OK");
		  client.println("Content-Type: text/html");
		  client.println("Connection: close");  // the connection will be closed after completion of the response
	  client.println("Refresh: 5");  // refresh the page automatically every 5 sec
		  client.println();
		  client.println("<!DOCTYPE HTML>");
		  client.println("<html>");
		  // output the value of each analog input pin
		  for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
			int sensorReading = analogRead(analogChannel);
			client.print("analog input ");
			client.print(analogChannel);
			client.print(" is ");
			client.print(sensorReading);
			client.println("<br />");
		  }
		  client.println("</html>");
		  break;
		}
		if (c == '\n') {
			// you're starting a new line
			currentLineIsBlank = true;
		}
		else if (c != '\r') {
			// you've gotten a character on the current line
			currentLineIsBlank = false;
		}
	  }
	}
	// give the web browser time to receive the data
	delay(1);
	// close the connection:
	client.stop();
	Serial.println("client disonnected");
  }
}
