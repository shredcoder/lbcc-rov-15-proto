// Native Arduino Hardware/Libraries
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
// Clever trick I discovered. :3
#define UDP_TX_PACKET_MAX_SIZE 256
#include <Servo.h>

// 3rd Party Libraries
#include <LinkedList.h>
// For some strange reason, this cpp is needed.
// This prevents an error about multiple definitions.
#include <ArduinoJson.h>
#include <ArduinoJson.cpp>

// Our Code
// The Attachment classes and code.
#include <attachments.h>



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
LinkedList<Attachment*> attachments;

void setup()
{
	// Spin up serial comms, mainly for debugging.
	// Beware a bug on Leonardo boards, which requires a loop here.
	Serial.begin(9600);
	Serial.println(F("\n\nROV start up..."));

	// The list of attachments. See "attachments.h" for options.
	// Keep this list up-to-date with real devices connected to the ROV.
	attachments.add(new MotorESC("leftTopVector", 3, 500, 1000, 1500));

	// Attempt to get an IP from the router.
	if (Ethernet.begin(MAC) == 0) {
		// DHCP will timeout after 1 minute.
		// If it fails, start with a static IP.
		Ethernet.begin(MAC, STATIC_IP, GATEWAY, SUBNET);
	}

	// Open the listening socket on the right port.
	UDP.begin(PORT);

	// Print information about IP address and port.
	logListeningAt();

	// Initialize the components that need it, like Servo-using attachments.
	// init() is an Attachment class function.
	for (int i = 0; i < attachments.size(); i++) {
		attachments.get(i)->init();
	}
}



// ============================================================================
//	Loop Tasks
// ============================================================================



const int MAX_INBOUND_PACKET_SIZE = 256;

const int INBOUND_BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(16);
const int OUTBOUND_BUFFER_SIZE = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(0);

char* reqJson = new char[MAX_INBOUND_PACKET_SIZE];
char* resJson = new char[UDP_TX_PACKET_MAX_SIZE];

long lastComm = 0;
IPAddress owner;



void loop ()
{
	handleInboundPacket();
}



// Deal with a single incoming packet from anywhere.
void handleInboundPacket()
{
	int packetSize = UDP.parsePacket();

	if (packetSize) {

		UDP.read(reqJson, MAX_INBOUND_PACKET_SIZE);
		logPacket("RECEIVED", packetSize, reqJson);

		StaticJsonBuffer<INBOUND_BUFFER_SIZE> jsonBufferIn;
		JsonObject& request = jsonBufferIn.parseObject(reqJson);

		StaticJsonBuffer<UDP_TX_PACKET_MAX_SIZE> jsonBufferOut;
		JsonObject& response = jsonBufferOut.createObject();

		if (request.success()) {
			handleRequest(request, response);
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



bool handleRequest (JsonObject& req, JsonObject& res)
{
	if (req.containsKey("cmd")) {

		const char* cmd = req["cmd"].asString();
		Serial.println(cmd);

		if (String("ping").equals(cmd)) { return handlePingCommand(res, res); }
		if (String("list").equals(cmd)) { return handleListCommand(res, res); }
		if (String("get").equals(cmd)) { return handleGetCommand(res, res); }
		if (String("set").equals(cmd)) { return handleSetCommand(res, res); }

		if (DEBUGGING) {
			Serial.println(F("Ignoring unknown command!"));
			return false;
		}

	} else if (DEBUGGING) {
		Serial.println(F("Ignoring request without command!"));
		return false;
	}
}



bool isSame(IPAddress ip1, IPAddress ip2)
{
	return ip1[0] == ip2[0]
		and ip1[1] == ip2[1]
		and ip1[2] == ip2[2]
		and ip1[3] == ip2[3];
}



// Currently, only accepts one controlling computer.
bool handlePingCommand (JsonObject& req, JsonObject& res)
{
	int isCtl = req["ctl"];
	Serial.println(isCtl);
	if (req["ctl"].as<bool>() and lastComm == 0) { owner = UDP.remoteIP(); }
	return sendPongCommand(res, isSame(owner, UDP.remoteIP()), attachments.size());
}



// Sends a pong command to the last UDP remote computer.
bool sendPongCommand (JsonObject& res, bool controlling, int channelCount)
{
	res["cmd"] = "pong";
	res["ctl"] = controlling;
	res["chn"] = channelCount;
	res.printTo(resJson, UDP_TX_PACKET_MAX_SIZE);
	logPacket("SEND", UDP_TX_PACKET_MAX_SIZE, resJson);
	UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
	UDP.write(resJson);
	UDP.endPacket();
	return true;
}



bool handleListCommand (JsonObject& req, JsonObject& res)
{
	return false;
}



bool handleGetCommand (JsonObject& req, JsonObject& res)
{
	return false;
}



bool handleSetCommand (JsonObject& req, JsonObject& res)
{
	return false;
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



void logListeningAt()
{
	Serial.print(F("rov@"));
	Serial.print(ip_to_str(Ethernet.localIP()));
	Serial.print(F(":"));
	Serial.println(PORT, DEC);
	Serial.print(F("DEBUGGING: "));
	Serial.println(DEBUGGING ? F("ON (Things will be slow!!!)") : F("OFF") );
}



void logPacket(char* message, int c, char* buffer)
{
	if (DEBUGGING) {
		Serial.print(F("["));
		Serial.print(message);
		Serial.print(F("] ["));
		Serial.print(c);
		Serial.print(F(" bytes] "));
		Serial.print(ip_to_str(UDP.remoteIP()));
		Serial.print(F(":"));
		Serial.println(UDP.remotePort());
		Serial.print(F("[PACKET] "));
		Serial.println(buffer);
	}
}
