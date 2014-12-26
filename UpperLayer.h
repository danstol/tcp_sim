#include<stdlib.h>

time_t t1;
int seconds;
int avail;

void upperLayerInit(int sec)
{
	seconds = sec;
	t1 = time(NULL);
	avail = 0;
}

int isDataAvailable()
{
	time_t t2;
	double elapsed;
	int retVal;

	t2 = time(NULL);
	elapsed = difftime(t2,t1);

	if (avail == 1)
		retVal = 1;
	else if (elapsed >= seconds)
	{
		avail = 1;
		retVal = 1;
	}
	else
	{
		retVal = 0;
	}
	return retVal;
}

int32_t retreiveData()
{
	avail = 0;			//data read, no longer available
	t1 = time(NULL);	

	return random() % 1000;
}


