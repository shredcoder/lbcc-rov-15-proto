// Native Arduino Hardware/Libraries
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>

// For some strange reason, this cpp is needed.
// This prevents an error about multiple definitions.
#include <ArduinoJson.h>
#include <ArduinoJson.cpp>

// ============================================================================
//	Settings
// ============================================================================

bool DEBUGGING = true;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip		(10,1,1,1);
IPAddress gateway	(10,0,0,1);
IPAddress subnet	(255,0,0,0);

unsigned int port = 21025;

// ============================================================================
//	Attachment Class
// ============================================================================

class Attachment
{
	public:
		char*	name;
		bool	readonly;
		virtual int		get() { return -1; }
		virtual bool	set(int) { return false; }
		virtual void	update() {}
};

// ============================================================================
//	Motor Attachment Class
// ============================================================================

class MotorESC: public Attachment
{
	public:
		MotorESC(char*, int, int, int, int);
		// Repeated, so we can set them outside of the class.
	private:
		int pin; // this will become "channel" for the PWM shield
		int value;
		int zeroValue;
		int minValue;
		int maxValue;
		Servo servo;
};

MotorESC::MotorESC(char* name, int pin, int low, int zero, int high)
{
	this->name		= name;
	this->readonly	= false;

	this->pin		= pin;
	zeroValue		= zero;
	minValue		= low;
	maxValue		= high;

	value			= zero;

	servo.attach(pin);
}

int MotorESC::get()
{
	return value;
}

bool MotorESC::set(int newValue)
{
	if (newValue > maxValue) return false;
	if (newValue < minValue) return false;
	value = newValue;
	return true;
}

void MotorESC::update()
{
	// Using a PWM board, this should do nothing, really.
	// Using the Servo library, when need to write here.
	servo.writeMicroseconds(value);
}

// ============================================================================
//	Main
// ============================================================================

EthernetUDP UDP;

Attachment attachments[] =
{
	MotorESC("servo1", 2, 1000, 1500, 2000),
	MotorESC("servo2", 3, 1000, 1500, 2000)
};

void setup()
{
	// Spin up serial comms for debugging.
	Serial.begin(9600); // Beware Leonardo bug.

	// Attempt to get an IP from the router.
	if (Ethernet.begin(mac) == 0) {
		// DHCP will timeout after 1 minute.
		// If it fails, start with a static IP.
		Ethernet.begin(mac, ip, gateway, subnet);
	}

	// Open the listening socket on the right port.
	UDP.begin(port);


	Serial.println();
	Serial.print("rov@");
	Serial.print(ip_to_str(Ethernet.localIP()));
	Serial.print(":");
	Serial.println(port, DEC);
	Serial.print("DEBUGGING: ");
	Serial.println(DEBUGGING ? "ON (Things will be slow!)" : "OFF" );
}

// ============================================================================
//	Loop Tasks
// ============================================================================

long lastComm = 0;

void loop()
{
	tickIncomingPacket();
	//tickLostMode();
	tickPulseMotors();
}

void tickIncomingPacket()
{
	static const int MAX_SIZE = 64;
	static char* json = new char[MAX_SIZE];
	static bool reply = false;

	// Grab the next packet available.
	int packetSize = UDP.parsePacket();

	// If the packet exists, process it.
	if (packetSize) {

		if (DEBUGGING) {
			Serial.print("[RECEIVED] [");
			Serial.print(packetSize);
			Serial.print(" bytes] ");
			Serial.print(ip_to_str(UDP.remoteIP()));
			Serial.print(":");
			Serial.println(UDP.remotePort());
		}

		// Read the packet into the json buffer.
		UDP.read(json, MAX_SIZE);

		if (DEBUGGING) {
			Serial.print("[RECEIVED] ");
			Serial.println(json);
		}

		const int BUFFER_SIZE = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(0);
		StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(json);

		if (root.success()) {

			Serial.println(root["ping"].asString());

			if (String("rov").equals(root["ping"].asString())) {

				const int BUFFER_SIZE = JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(0);
				StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
				JsonObject& res = jsonBuffer.createObject();
				res["pong"] = "watching";
				res.printTo(json, MAX_SIZE);
				reply = true;

			}

			lastComm = millis();

		} else {

			const int BUFFER_SIZE = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(0);
			StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
			JsonObject& res = jsonBuffer.createObject();

			res["error"] = true;
			res["msg"] = "bad json";

			res.printTo(json, MAX_SIZE);
			reply = true;

		}

		if (reply) {

			if (DEBUGGING) {
				Serial.print(F("[SENT] ["));
				Serial.print(sizeof(json));
				Serial.print(F(" bytes] "));
				Serial.print(ip_to_str(UDP.remoteIP()));
				Serial.print(F(":"));
				Serial.println(UDP.remotePort());
				Serial.print(F("[SENT] "));
				Serial.println(json);
			}

			UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
			UDP.write(json);
			UDP.endPacket();

		} else if (DEBUGGING) {

			Serial.println(F("[NOT REPLYING]"));

		}
	}
}

void tickPulseMotors()
{
	for (int i = 0; i < sizeof(attachments); i++) {
		Serial.println("Updating ");
		Serial.println(attachments[i].name);
		attachments[i].update();
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
