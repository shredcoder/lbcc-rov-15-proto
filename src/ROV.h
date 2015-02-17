#include <Adafruit_PWMServoDriver.h>
#include <LinkedList.h>

#include <Attachment.h>

// Needed for Seeed Studio Ethernet Shield
#define W5200_CS  10
#define SDCARD_CS 4

class ROV
{
	public:
		ROV();
		int channelCount();
	private:
		Adafruit_PWMServoDriver pwm;
		LinkedList<Attachment*> list;
};
