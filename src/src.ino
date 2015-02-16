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

bool DEBUGGING = false;

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
	tickInboundPacket();
	//tickOutboundPacket();
}

// Deal with a single incoming packet from anywhere.
void tickInboundPacket()
{
	const int MAX_INBOUND_PACKET_SIZE = 256;
	const int MAX_OUTBOUND_PACKET_SIZE = 64;

	const int INBOUND_BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(16);
	const int OUTBOUND_BUFFER_SIZE = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(0);

	static char* reqJson = new char[MAX_INBOUND_PACKET_SIZE];
	static char* resJson = new char[MAX_INBOUND_PACKET_SIZE];

	int packetSize = UDP.parsePacket();

	if (packetSize) {

		UDP.read(reqJson, MAX_INBOUND_PACKET_SIZE);
		logPacket(UDP.remoteIP(), UDP.remotePort(), "RECEIVED", packetSize, reqJson);
		StaticJsonBuffer<INBOUND_BUFFER_SIZE> jsonBufferIn;
		JsonObject& request = jsonBufferIn.parseObject(reqJson);

		if (request.success()) {

			if (String(request["ping"].asString()).equals("rov")) {

				StaticJsonBuffer<OUTBOUND_BUFFER_SIZE> jsonBufferOut;
				JsonObject& response = jsonBufferOut.createObject();
				response["pong"] = "controlling"; // watchings
				response.printTo(resJson, MAX_OUTBOUND_PACKET_SIZE);
				logPacket(UDP.remoteIP(), UDP.remotePort(), "SEND", sizeof(resJson), resJson);
				UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
				UDP.write(resJson);
				UDP.endPacket();
				
			} else if (String(request["cmd"].asString()).equals("set")) {

				JsonArray& list = request["list"];
				int count;

				if (request["list"].is<JsonArray&>()) {
					count = list.size();
					Serial.print("Setting ");
					Serial.print(count, DEC);
					Serial.println(" values.");
				}

				if (count > 0 && count <= AttachmentCount) {
					for (int i = 0; i < count; i++) {

						int c = list[i]["c"];
						int v = list[i]["v"];

						if (c > 0 and c <= AttachmentCount) {
							if (attachments[c-1]->set(v)) {

								StaticJsonBuffer<OUTBOUND_BUFFER_SIZE> jsonBufferOut;
								JsonObject& response = jsonBufferOut.createObject();
								response["cmd"] = "is";
								response["c"] = c;
								response["v"] = v;
								response.printTo(resJson, MAX_OUTBOUND_PACKET_SIZE);
								logPacket(UDP.remoteIP(), UDP.remotePort(), "SEND", sizeof(resJson), resJson);
								UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
								UDP.write(resJson);
								UDP.endPacket();

							} else {

								if (DEBUGGING) {
									Serial.println("Illegal channel value!");
								}

							}
						
						} else {

							if (DEBUGGING) {
								Serial.print("Illegal channel! ");
								Serial.println(c);
							}

						}
					}
				}
			}

			lastComm = millis();

		} else {

			if (DEBUGGING) {
				Serial.println("Ignoring bad json.");
			}

		}
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
