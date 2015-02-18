#include <ROV.h>

ROV::ROV ()
{
	// Deselect the Seeed Studio SD card.
	pinMode(SDCARD_CS, OUTPUT);
	digitalWrite(SDCARD_CS, HIGH);

	// Start the PWM shield.
	pwm = Adafruit_PWMServoDriver();
	pwm.begin();
	pwm.setPWMFreq(100);

	list.add(new MotorESC(pwm, 100, "vector1", 0, 500, 1000, 1500));
	list.add(new MotorESC(pwm, 100, "vector2", 1, 500, 1000, 1500));
}

int ROV::channelCount ()
{
	return list.size();
}

Attachment* ROV::getChannel (int num)
{
	return list.get(num-1);
}
