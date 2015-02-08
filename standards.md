

	[byte]	[byte]	[int]
	[cmd]	[index]	[value]

# Commands

	0	Error
	1	Set
	2	Get
	3	Range

## Error Command

	[byte]	[byte]	[int]
	[0]		[cmd]	[extra]

	0		0					Unknown Command

	0		1		0			Can't set
	0		1		1			Too Low ?
	0		1		2			Too High ?

## Set Command

	[byte]	[byte]		[int]
	[1]		[channel]	[value]

	1		0			488		Set channel to 

## Get Command




## Range Command