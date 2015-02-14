// Native Arduino Hardware/Libraries
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>


// For some strange reason, this cpp is needed.
// This prevents an error about multiple definitions.
#include <ArduinoJson.h>
#include <ArduinoJson.cpp>

// The Attachment classes and code.
#include <attachments.h>

// ============================================================================
//	Settings
// ============================================================================

bool DEBUGGING = true;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip		(10,1,1,1);
IPAddress gateway	(10,0,0,1);
IPAddress subnet	(255,0,0,0);

unsigned int port = 21025;

int AttachmentCount = 1;
Attachment* attachments[] =
{
	new MotorESC("leftTopVector", 3, 500, 1000, 1500)
};

// ============================================================================
//	Main
// ============================================================================

long lastComm = 0;

EthernetUDP UDP;

void setup()
{
	// Spin up serial comms for debugging.
	// Beware Leonardo bug.
	Serial.begin(9600);

	// Attempt to get an IP from the router.
	if (Ethernet.begin(mac) == 0) {
		// DHCP will timeout after 1 minute.
		// If it fails, start with a static IP.
		Ethernet.begin(mac, ip, gateway, subnet);
	}

	// Open the listening socket on the right port.
	UDP.begin(port);

	// Print information about IP address and port.
	logListeningAt(Ethernet.localIP(), port);

	// Initialize the components that need to be.
	for (int i = 0; i < AttachmentCount; i++) {
		attachments[i]->init();
	}
}

// ============================================================================
//	Loop Tasks
// ============================================================================

void loop()
{
	tickIncomingPacket();
}

void tickIncomingPacket()
{
	static const int MAX_SIZE = 64;
	static char* json = new char[MAX_SIZE];

	int packetSize = UDP.parsePacket();

	if (packetSize) {

		UDP.read(json, MAX_SIZE);
		logPacket(UDP.remoteIP(), UDP.remotePort(), "RECEIVED", packetSize, json);

		const int BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(0);
		
		StaticJsonBuffer<BUFFER_SIZE> jsonBufferIn;
		JsonObject& req = jsonBufferIn.parseObject(json);

		StaticJsonBuffer<BUFFER_SIZE> jsonBufferOut;
		JsonObject& res = jsonBufferOut.createObject();

		if (req.success()) {

			if (String(req["ping"].asString()).equals("rov")) {

				res["okay"] = true;
				res["pong"] = "watching"; // controlling
				
			} else if (String(req["cmd"].asString()).equals("set")) {

				int c = req["channel"];
				int v = req["value"];

				if (c > 0 and c <= AttachmentCount) {

					if (attachments[c-1]->set(v)) {

						res["okay"] = true;
						res["channel"] = c;
						res["value"] = v;

					} else {

						res["okay"] = false;
						res["channel"] = c;
						res["value"] = v;
						res["msg"] = "invalid value";

					}

				} else {

					res["okay"] = false;
					res["channel"] = c;
					res["msg"] = "invalid channel";

				}

			}

			lastComm = millis();

		} else {

			res["okay"] = false;
			res["msg"] = "bad json";

		}

		res.printTo(json, MAX_SIZE);
		logPacket(UDP.remoteIP(), UDP.remotePort(), "SEND", sizeof(json), json);
		UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
		UDP.write(json);
		UDP.endPacket();
	}
}

// ============================================================================
//	Debugging
// ============================================================================

const char* ip_to_str(const IPAddress ipAddr)
{
	static char buf[16];
	sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	return buf;
}

void logListeningAt(IPAddress ip, int port)
{
	Serial.println();
	Serial.print("rov@");
	Serial.print(ip_to_str(ip));
	Serial.print(":");
	Serial.println(port, DEC);
	Serial.print("DEBUGGING: ");
	Serial.println(DEBUGGING ? "ON (Things will be slow!)" : "OFF" );
}

void logPacket(IPAddress ip, int port, char* message, int c, char* buffer)
{
	if (DEBUGGING) {
		Serial.print("[");
		Serial.print(message);
		Serial.print("] [");
		Serial.print(c);
		Serial.print(" bytes] ");
		Serial.print(ip_to_str(ip));
		Serial.print(":");
		Serial.println(UDP.remotePort());
		Serial.print("[PACKET] ");
		Serial.println(buffer);
	}
}
