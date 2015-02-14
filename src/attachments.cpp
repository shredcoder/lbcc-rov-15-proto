#include <Servo.h>

#include <attachments.h>

MotorESC::MotorESC(char* name, int pin, int low, int zero, int high)
{
	this->name		= name;
	this->readonly	= false;

	this->pin		= pin;
	zeroValue		= zero;
	minValue		= low;
	maxValue		= high;

	value			= zero;
}

void MotorESC::init()
{
	servo.attach(pin);
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
	servo.write(value);
	return true;
}
