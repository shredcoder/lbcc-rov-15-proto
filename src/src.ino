#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

/* ============================================================================
	Settings
============================================================================ */

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip		(10,1,1,1);
IPAddress gateway	(10,0,0,1);
IPAddress subnet	(255,0,0,0);

unsigned int localPort = 8888;

EthernetUDP Udp;

/* ============================================================================
	Main
============================================================================ */

void setup()
{
	startDebugSerial();

	if (Ethernet.begin(mac) == 0) {
		// DHCP will timeout after 1 minute.
		Ethernet.begin(mac, ip, gateway, subnet);
	}
	Serial.print("IP: ");
	Serial.println(Ethernet.localIP());

	Udp.begin(localPort);
}

void loop()
{
	handleIncomingData();
	broadcast(4000);
	delay(10);
}

/* ============================================================================
	Networking
============================================================================ */

void handleIncomingData ()
{
	static char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
	static char replyBuffer[] = "acknowledged";

	int packetSize = Udp.parsePacket();

	if (packetSize) {

		Serial.print("Received packet of size ");
		Serial.println(packetSize);
		Serial.print("From ");
		Serial.print(ip_to_str(Udp.remoteIP()));
		Serial.print(", port ");
		Serial.println(Udp.remotePort());

		// read the packet into packetBufffer
		Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
		Serial.println("Contents:");
		Serial.println(packetBuffer);

		// send a reply, to the IP address and port that sent us the packet we received
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		Udp.write(replyBuffer);
		Udp.endPacket();
	}
}

void broadcast (int gap) {

	static unsigned long last = -gap;
	static const IPAddress BROADCAST_IP (255,255,255,255);
	static const int BROADCAST_PORT (8888);

	if (millis() - last > gap) {
		last = millis();
		Serial.println("Broadcast!");
		Serial.print("IP: ");
		Serial.println(ip_to_str(BROADCAST_IP));
		Serial.print("PORT: ");
		Serial.println(BROADCAST_PORT, DEC);
	}
}

/* ============================================================================
	Debugging
============================================================================ */

void startDebugSerial ()
{
	Serial.begin(9600);
	// This loop is for compatibility with Leonardo.
	// Other models will still work fine too.
	while (!Serial) { }
	Serial.println("Serial logging started.");
}

const char* ip_to_str(const IPAddress ipAddr)
{
	static char buf[16];
	sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	return buf;
}
