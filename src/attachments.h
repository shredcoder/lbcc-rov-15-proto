
class Attachment
{
	public:
		char*	name;
		bool	readonly;
		virtual void	init() = 0;
		virtual int		get() = 0;
		virtual bool	set(int) = 0;
};


class MotorESC: public Attachment
{
	public:
		MotorESC(char*, int, int, int, int);
		// Repeated, so we can set them outside of the class.
		void init();
		int get();
		bool set(int);
	private:
		int pin; // this will become "channel" for the PWM shield
		int value;
		int zeroValue;
		int minValue;
		int maxValue;
		Servo servo;
};
