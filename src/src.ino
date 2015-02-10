// For some strange reason, this cpp is needed.
// This prevents an error about multiple definitions.
#include <ArduinoJson.h>
#include <ArduinoJson.cpp>







StaticJsonBuffer<200> jsonBuffer;

class Commmand {
	public:
	int code;
	int channel;
	int value;
	bool load(char*);
};

bool Commmand::load(char json[]) {
	JsonObject& root = jsonBuffer.parseObject(json);
	if (!root.success()) { return false; }
	code	= root["code"];
	channel	= root["channel"];
	value	= root["value"];
	return true;
}





void setup()
{
	Serial.begin(9600); // Beware Leonardo bug.
	
	char json[] = "{\"code\":1,\"channel\":2,\"value\":3}";
	Commmand cmd;
	cmd.load(json);

	Serial.print("Code: ");
	Serial.println(cmd.code);
}

void loop() {}




