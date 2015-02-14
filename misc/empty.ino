#include <Servo.h>

Servo beard;
int chin = 5;

void setup () 
{
	Serial.begin(9600);
	beard.attach(chin);
	beard.write(1000);
}

void loop () {
	
	delay(20);
}


