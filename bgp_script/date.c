#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

int main()
{
//	struct timeval tms;
//	char tstr[100];
//	timerclear(&tms);
//	gettimeofday(&tms,NULL);
//	strftime(tstr,100,"%S",localtime(&tms.tv_sec));
//	printf("%s.%ld", tstr, tms.tv_usec);

	float time_use = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("%ld.%ld", tv.tv_sec, tv.tv_usec);

	return 0;
}
