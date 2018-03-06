#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

typedef int INT;

INT g_lcd_ctl = 1;


void Lcd_control_handler(INT signum)
{
   printf("%s\n", __func__);
   if(g_lcd_ctl > 0)
   {
       //RefreshLcdDisplay();
   }
}


static void LcdControlThread(void *pArg)
{
	struct itimerval timerval;

	/* set loop timer for 5 seconds*/
	timerval.it_interval.tv_sec = 1;
	timerval.it_interval.tv_usec = 0;
	timerval.it_value.tv_sec = 1;
	timerval.it_value.tv_usec = 0;

	signal(SIGALRM, Lcd_control_handler);
	printf(" begin to launch a timer\n");
	
	setitimer(ITIMER_REAL, &timerval, NULL); 	
}


int main()
{
	pthread_t tid;

	pthread_create(&tid, NULL, LcdControlThread, NULL);

	while (1)
 	{
		sleep(100);
	}

	return 0;
}
