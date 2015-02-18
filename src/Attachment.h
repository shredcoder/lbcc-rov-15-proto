#include <Adafruit_PWMServoDriver.h>

class Attachment
{
	public:
		char* name;
		bool readonly;
		int minValue;
		int maxValue;
		virtual int get() = 0;
		virtual bool set(int) = 0;
};


class MotorESC: public Attachment
{
	public:
		MotorESC(Adafruit_PWMServoDriver, int, char*, int, int, int, int);
		int get();
		bool set(int);
	private:
		Adafruit_PWMServoDriver pwm;
		int frequency;
		int chan;
		int value;
		int zeroValue;
};
