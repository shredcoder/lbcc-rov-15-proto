# ROV Commands v0.1

The ROV software works by sending requests to the Arduino, and the robot will reply with relevant information. The robot is aggressive about ignoring broken data.



## ping commmand

- First, the **computer** broadcasts to everything on the network.
- 25 or 26 Bytes Per Packet
- Computer wants control by default.

```json
{
	"cmd"	: "ping",
	"ctl"	: false					// true, false
}
```



## pong command

- The **ROV** replies directly to the computer.
- 31 Bytes Max Per Packet
- Used to maintain contact with ROV if controlling.

```json
{
	"cmd"	: "pong",
	"ctl"	: 1,					// 0 or 1, if controlling
	"chn"	: 12					// 1 thru 99 only
}
```



## list command

- The **computer** requests details about each channel *if it needs them*.
- 14 Bytes Always Per Packet

```json
{
	"cmd"	: "list"
}
```



## chn command

- The **ROV** replies with a series of packets that each describe one channel.
- 88 Bytes Max Per Packet
- 99 packets maximum to receive from Arduino.
- Packets only need to be exchanged once during initial connection.

```json
{
	"cmd"	: "chn",
	"num"	: 12,					// 1 thru 99 only
	"name"	: "bowStarboardVector",	// 1 thru 20 letters
	"min"	: -500,					// -999 thru 9999
	"max"	: 1500,					// -999 thru 9999
	"now"	: 1234					// -999 thru 9999
}
```



## set command

- The **computer** sets out-of-date channel values in bundles.
- Max Bytes = 23 + 17(1) + 18(n-1)
- Max size is setting 99 channels, at 1804 bytes.
- 16 channels = 310 bytes, which is our first maximum.

```json
{
	"cmd"	: "set",
	"list"	: [
		{
			"c"	: 14,				// 1 thru 99 only
			"v"	: 1234				// -999 thru 9999
		},
		{
			"c"	: 10,				// order does not matter
			"v"	: 4321				// bad values are simply ignored
		}
	]
}
```



## get command

- The **computer** gets any values by requesting with a list of channels.
- Max Bytes = 23 + 2(1) + 3(n-1)
- Max size is setting 99 channels, at 319 bytes.
- 16 channels = 70 bytes, which is our first maximum.

```json
{
	"cmd"	: "get",
	"list"	: [
		12,						// -999 thru 9999
		17						// bad channels are ignored
	]
}
```



## is command

- The **ROV** explains current values with another array.
- Max Bytes = 22 + 17(1) + 18(n-1)
- Max size is setting 99 channels, at 1803 bytes.
- 16 channels = 309 bytes, which is our frist maximum.

```json
{
	"cmd"	: "is",
	"list"	: [
		{
			"c"	: 14,				// 1 thru 99 only
			"v"	: 1234				// -999 thru 9999
		},
		{
			"c"	: 10,				// order does not matter
			"v"	: 4321				// bad values are simply ignored
		}
	]
}
```



