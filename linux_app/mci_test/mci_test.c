#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <math.h>

#include "mci_pef.h"
#include "mci_sensor.h"
#include "mci_chassis.h"
#include "mci_lan.h"
#include "mci_smbios.h"
#include "mci_smtp.h"
#include "mci_sel.h"
#include "mci_datetime.h"
#include "mci_standard_ipmi.h"
#include "mci_ns_port.h"
#include "mci_app.h"
#include "mci_network_service.h"
#include "mci_lan_auth.h"
#include "mci_nm.h"
#include "mci_syslog.h"
#include "mci_sensor.h"
#include "sel_decoder.h"
#include "mci_string.h"
#include "mci_systeminfo.h"

#include <GEN_DBGLOG/dbglog.h>
#include "rcm_api.h"


#define THREAD_MAX   50

#define MAX_BUF_LEN      256
#define MAX_COLOR_LEN    16

#define TO_SIGNED32(val, bits)   (((val) & (1 << ((bits) - 1))) ? \
                                  (-((1 << ((bits) - 1))) | (val)) : (val))
#define IS_SCANNING_DISABLED(val)   (((val) & 0x40) == 0)
#define IS_READING_UNAVAILABLE(val) (((val) & 0x20) != 0)

pthread_t pid_list[THREAD_MAX] = {0};
static int 	thread_index[THREAD_MAX];

extern unsigned char *g_sdr_repo_ptr;
#define gettid() syscall(__NR_gettid)

static void debug_msg(const char *degmsg, ...)
{	
#if 0
    va_list ap;
    char msg[512] = {0};	
	int len;
	int stdout_fd;
	char time_str[16] = {0};

	sprintf(time_str, "%u    ", (unsigned int)time(NULL));
	stdout_fd = open("/dev/ttyS0", O_WRONLY);
	if (stdout_fd <= 0)
		return;
    va_start(ap, degmsg);
    vsprintf(msg, degmsg, ap);
    va_end(ap); 
	len = strlen(msg);
	msg[len] = '\n';	
	write(stdout_fd, time_str, strlen(time_str));
	write(stdout_fd, msg, (len + 1));
	close(stdout_fd);
#else
	;
#endif
}

static void get_thresholds_only(int owner_id, int sensor_id, int sdr_id)
{
	uint8_t thresholds[7] = {0};
	uint8_t *sdr_record = NULL;
	char buff[MAX_BUF_LEN] = {0};
	char output_format[8] = {0};
	float human_read_data = 0.0;
	int ret = 0;
	int i = 0;

	sdr_record = g_sdr_repo_ptr + (sdr_id << 6);

	if (sdr_record[13] == 0x1)     // Threshold-based sensor only
	{
		ret = mci_get_raw_thresholds(owner_id, sensor_id, thresholds);

		if (sdr_record[12] == 0x02)     // Sensor Type code 0x02 : Voltage
		{
			strcpy(output_format, "%.3f");
		}
		else
		{
			strcpy(output_format, "%.0f");
		}

		for (i = 0; i < 6; i++)
		{
			memset(buff, 0, sizeof (buff));

			if (0 == ret && thresholds[0] & (1 << i))
			{
				human_read_data = convert_raw_reading(sdr_record, thresholds[i + 1]);
				snprintf(buff, sizeof (buff),
				         (const char *) output_format, human_read_data);
			}
			else
			{
				strcpy(buff, "N/A");
			}
	
		}
	}
}


static int get_sensor_info(void)
{
	int i, ret;
	unsigned char *sdr_record = NULL;
	int sdr_cnt = 0, sdr_type = 0;
	int owner_id = 0, sensor_number = 0, sensor_type = 0;
	int event_reading_type = 0;
	int event_severity = 0;
	unsigned char rsp_data[4] = {0};
	char buff[MAX_BUF_LEN] = {0};

	sdr_cnt = mci_update_sdr_cache();
	//debug_msg("sdr_cnt = %d\n", sdr_cnt);
	for (i = 0; i < sdr_cnt; i++)
	{
		//debug_msg("sdr_cnt[%d]\n", i);
		sdr_record = g_sdr_repo_ptr + (i << 6);

		sdr_type = sdr_record[3];

		if (sdr_type != 0x01 && sdr_type != 0x02)
		{
			continue;
		}

		owner_id = sdr_record[5];
		sensor_number = sdr_record[7];
		sensor_type = (unsigned int) sdr_record[12];
		event_reading_type = sdr_record[13];


		/* init sensor healthy*/
		memset(buff, 0, sizeof (buff));

		/************* NAME ***************/
		if (0x01 == sdr_type)
		{
			;
		}
		else if (0x02 == sdr_type)
		{
			;
		}



		/************* NUMBER ***************/
		//snprintf_s_i(buff, sizeof (buff), "%02x", sensor_number);
	
		/************* Sensor Type: STYPE ***************/
		//snprintf_s_i(buff, sizeof (buff), "%02x", sensor_type);

		/************* Event/Reading Type: ERTYPE ***************/
		//snprintf_s_i(buff, sizeof (buff), "%02x", event_reading_type);

		/************* OPTION ***************/
		//option = (((sdr_record[10] << 2) & 0x80) | (sdr_record[10] & 0x40));
		//snprintf_s_i (buff, sizeof (buff), "%02x", option);

		/************* Thresholds ***************/
		if (0x01 == event_reading_type)     // Threshold-based sensor only
		{
			get_thresholds_only(owner_id, sensor_number, i);
		}

		/************* SENSOR READING ***************/
		memset(rsp_data, 0, sizeof (rsp_data));
		ret = mci_get_raw_reading(owner_id, sensor_number, rsp_data);

		if (0 == ret &&
		        !IS_SCANNING_DISABLED(rsp_data[1]) &&
		        !IS_READING_UNAVAILABLE(rsp_data[1]))
		{
			/************* RAW READING ***************/
			

			/************* HUMAN READING ***************/
			if (event_reading_type == 0x01)                      //threshold sensor
			{
				convert_raw_reading(sdr_record, rsp_data[0]);

				if (sdr_record[12] == 0x02)                  //Sensor Type code 0x02 : Voltage
				{
					;

				}
				else                                                //Sensor Type: Degree
				{
					;					
				}

				/************* UNIT1 ***************/
				//snprintf_s_i(buff, sizeof (buff), "%02x", unit_1);

				/************* UNIT ***************/
				//snprintf_s_i(buff, sizeof (buff), "%02x", unit);
				memset(buff, 0, sizeof (buff));
				//convert_sensor_reading_unit(unit, buff);
			}
			else           //discrete sensor
			{
				;
			}


			/************* SENSOR STATUS ***************/
			memset(buff, 0, sizeof (buff));
			//strcpy(sensor_state_color, MAX_COLOR_LEN, "white");
			
			event_severity = mci_decode_sensor_status(buff,
			                 sensor_type,
			                 event_reading_type,
			                 rsp_data[2], rsp_data[3]);

			ret = mci_get_sensor_severity(owner_id, sensor_number, &event_severity);

			switch (event_severity)
			{
				case 0:
					//strcpy_s(sensor_state_color, MAX_COLOR_LEN, "green");
					break;

				case 1:
					//strcpy_s(sensor_state_color, MAX_COLOR_LEN, "yellow");
					break;

				case 2:
				case 3:
					//strcpy_s(sensor_state_color, MAX_COLOR_LEN, "red");
					break;
			}

			/************* SENSOR STATE COLOR ***************/
		}
	}

	return 0;
}


static int thread_proc(void *arg)
{
	int thread_index = *((int *)arg);
	debug_msg("thread[%d], pid(%u), tid(%u) start\n", thread_index, getpid(), gettid());
	while (1)
	{
		debug_msg("thread[%d], pid(%u), tid(%u) get begin\n", thread_index, getpid(), gettid());
		get_sensor_info();
		debug_msg("thread[%d], pid(%u), tid(%u) get end\n", thread_index, getpid(), gettid());
		sleep(1);
	}
	return 0;
}



static int thread_init(void)
{
	int i;

	for (i = 0; i < THREAD_MAX; i++)
	{
		thread_index[i] = i;
		if(0 != pthread_create(&pid_list[i], NULL, (void *)thread_proc, &thread_index[i]))
		{
			debug_msg("create thread[%d] failed\n", i);
		}
		else 
		{
			pthread_detach(pid_list[i]);
			debug_msg("create thread[%d] sucess\n", i);
		}
	}

	return 0;
}



int main()
{
	/* init socket */
	debug_msg("mci_test: pid(%u)\n", getpid());
	if (thread_init() != 0)
	{
		debug_msg("msg_proc: socket_init failed\n");
		return -1;
	}
	
	while (1)
	{
		sleep(1);
	}


	return 0;
}














