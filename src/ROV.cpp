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

	list.add(new MotorESC(pwm, 100, "someServo", 0, 750, 1250, 2250));
	list.add(new MotorESC(pwm, 100, "otherServo", 0, 750, 1250, 2250));
}

int ROV::channelCount ()
{
	return list.size();
}
