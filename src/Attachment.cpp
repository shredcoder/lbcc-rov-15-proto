#include <Attachment.h>

MotorESC::MotorESC(Adafruit_PWMServoDriver pwm, int freq, char* name, int chan, int low, int zero, int high)
{
	this->name			= name;
	this->readonly		= false;
	this->pwm			= pwm;
	this->frequency		= freq;
	this->chan			= chan;
	this->value			= zero;
	this->zeroValue		= zero;
	this->minValue		= low;
	this->maxValue		= high;

	this->set(value);
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

	// some math for applying microseconds
	double pulselength;
	pulselength = 1000000;		// 1,000,000 us per second
	pulselength /= frequency;	// ??? Hz
	pulselength /= 4096;		// 12 bits of resolution
	double pulse = newValue / pulselength;
	pwm.setPWM(chan, 0, pulse);

	return true;
}
