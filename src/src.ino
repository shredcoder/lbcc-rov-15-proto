// Native Arduino Hardware/Libraries
#include <SPI.h>
#include <Wire.h>

// 3rd Party Libraries
#include <Adafruit_PWMServoDriver.h>
#include <EthernetV2_0.h>
#include <EthernetUdpV2_0.h>
#include <LinkedList.h>
// For some strange reason, this cpp is needed.
// This prevents an error about multiple definitions.
#include <ArduinoJson.h>
#include <ArduinoJson.cpp>

#include <ROV.h>

// Clever trick I discovered: you can redefine this. :3
// UNTESTED VALUE! This might be WAY too high.
#define UDP_TX_PACKET_MAX_SIZE 2048

// ============================================================================
//	Settings
// ============================================================================

// If true, you must suffer the wrath of a million Serial.println()s.
// Please leave false unless you are developing or debugging.
bool DEBUGGING = true;

// MAC address--generally don't touch this, but can be different.
byte MAC[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Port to communicate on--matches desktop app.
unsigned int PORT = 21025;

// IP to use if DHCP fails.
IPAddress STATIC_IP	(10,1,1,1);
IPAddress GATEWAY	(10,0,0,1);
IPAddress SUBNET	(255,0,0,0);

// ============================================================================
//	Core
// ============================================================================

EthernetUDP UDP;
ROV * rov;

void setup()
{
	// Spin up serial comms, mainly for debugging.
	// Beware a bug on Leonardo boards, which requires a loop here.
	Serial.begin(9600);
	Serial.println(F("\n\nROV start up..."));

	rov = new ROV();

	// Attempt to get an IP from the router.
	if (Ethernet.begin(MAC) == 0) {
		// DHCP will timeout after 1 minute.
		// If it fails, start with a static IP.
		Ethernet.begin(MAC, STATIC_IP, GATEWAY, SUBNET);
	}

	// Open the communications socket on the right port.
	UDP.begin(PORT);

	// Log information about our IP address and port.
	logListeningAt();
}


// ============================================================================
//	Loop Tasks
// ============================================================================

void loop ()
{
	readPacket();
	delay(1);
}

const int CHAR_BUFFER_SIZE = UDP_TX_PACKET_MAX_SIZE;
const int JSON_BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(16);
char incoming [CHAR_BUFFER_SIZE];

long lastComm = 0;
IPAddress owner;

void readPacket()
{
	// Get the size of the next available packet, 0 if there is no packet.
	int packetSize = UDP.parsePacket();

	if (packetSize) {

		// Read the packet into the char array.
		UDP.read(incoming, CHAR_BUFFER_SIZE);

		// Transform the char array into an object.
		StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
		JsonObject& request = jsonBuffer.parseObject(incoming);

		// If it was valid JSON, try to use it as a command.
		if (request.success()) {

			handle(request);

		} else if (DEBUGGING) {

			Serial.println("Ignoring bad json!");
			request.printTo(Serial);
			Serial.println();

		}
	}
}

// ============================================================================
//	Commands
// ============================================================================

bool handle (JsonObject& request)
{
	if (request.containsKey("cmd")) {

		String cmd = request["cmd"].asString();

		if (cmd.equals("ping")) { return handlePingCommand(request); }
		if (cmd.equals("list")) { return handleListCommand(request); }
		if (cmd.equals("get")) { return handleGetCommand(request); }
		if (cmd.equals("set")) { return handleSetCommand(request); }

		if (DEBUGGING) {
			Serial.println(F("Ignoring unknown command!"));
		}
		return false;

	} else if (DEBUGGING) {
		Serial.println(F("Ignoring request without command!"));
	}
	return false;
}

bool isSame(IPAddress ip1, IPAddress ip2)
{
	return ip1[0] == ip2[0]
		and ip1[1] == ip2[1]
		and ip1[2] == ip2[2]
		and ip1[3] == ip2[3];
}

// Currently, only accepts one controlling computer.
bool handlePingCommand (JsonObject& req)
{
	if (req["ctl"] == 1 and lastComm == 0) {
		if (DEBUGGING) {
			Serial.println("Assigning owner.");
		}
		owner = UDP.remoteIP();
		lastComm = millis();
	}
	return sendPongCommand(isSame(owner, UDP.remoteIP()), rov->channelCount());
}


// Sends a pong command to the last UDP remote computer.
bool sendPongCommand (bool controlling, int channelCount)
{
	const int BUFFER_SIZE = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(0);
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	JsonObject& response = jsonBuffer.createObject();

	response["cmd"] = "pong";
	response["ctl"] = controlling;
	response["chn"] = channelCount;

	response.printTo(incoming, UDP_TX_PACKET_MAX_SIZE);
	UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
	UDP.write(incoming);
	UDP.endPacket();
	return true;
}


bool handleListCommand (JsonObject& request)
{
	return false;
}


bool handleGetCommand (JsonObject& request)
{
	return false;
}


bool handleSetCommand (JsonObject& request)
{
	return false;
}

// ============================================================================
//	Debugging
// ============================================================================

int freeRam () {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

const char* ip_to_str(const IPAddress ipAddr)
{
	static char buf[16];
	sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	return buf;
}

void logListeningAt()
{
	Serial.print(F("ROV joined network: "));
	Serial.print(ip_to_str(Ethernet.localIP()));
	Serial.print(F(":"));
	Serial.println(PORT, DEC);
	Serial.print("Free RAM: ");
	Serial.print(freeRam(), DEC);
	Serial.println(" bytes");
	Serial.print(F("DEBUGGING: "));
	Serial.println(DEBUGGING ? F("ON (Things will be slow!!!)") : F("OFF") );
}
