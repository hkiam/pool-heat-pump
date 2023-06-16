
#include <stdio.h>
#include <ctype.h>
#include "decoder.h"

#ifdef WIN32
#include "windows.h"
#include <conio.h>
#include "ps2000.h"
#define PREF4 _stdcall
#else
#include <sys/types.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "libps2000/ps2000.h"
#define PREF4

#define Sleep(a) usleep(1000*a)
#define scanf_s scanf
#define fscanf_s fscanf
#define memcpy_s(a,b,c,d) memcpy(a,c,d)
typedef uint8_t BYTE;
typedef enum enBOOL
{
	FALSE, TRUE
} BOOL;
/* A function to detect a keyboard press on Linux */
int32_t _getch()
{
	struct termios oldt, newt;
	int32_t ch;
	int32_t bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	do
	{
		ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
		if (bytesWaiting)
			getchar();
	} while (bytesWaiting);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

int32_t _kbhit()
{
	struct termios oldt, newt;
	int32_t bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return bytesWaiting;
}

/* A function to get a single character on Linux */
#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)
#endif

/****************************************************************************
 *
 *
 ****************************************************************************/

int16_t	handle;
int32_t input_ranges [PS2000_MAX_RANGES] = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000};
int32_t count = 0;
FILE *		fp = NULL;

int threshold = 1000; // 1V
int lastState = -1;
int lastStateStart = -1;

pulsedecoder decoder;


/****************************************************************************
 * mv_to_adc
 *
 * Convert a millivolt value into a 12-bit ADC count
 *
 *  (useful for setting trigger thresholds)
 ****************************************************************************/
int16_t mv_to_adc (int16_t mv, int16_t ch)
{
	return ( ( mv * 32767 ) / input_ranges[ch] );
}

/****************************************************************************
 * adc_to_mv
 *
 * If the user selects scaling to millivolts,
 * Convert an 12-bit ADC count into millivolts
 ****************************************************************************/
int32_t adc_to_mv (int32_t raw, int32_t ch)
{
	return (raw * input_ranges[ch] ) / 32767.0;
}

/****************************************************************************
 *
 * Streaming callback
 *
 ****************************************************************************/
void  PREF4 ps2000FastStreamingReady( int16_t **overviewBuffers,
											int16_t overflow,
											uint32_t triggeredAt,
											int16_t triggered,
											int16_t auto_stop,
											uint32_t nValues)
{
    if(overflow){
        printf("warn: overflow");
    }

    if(overviewBuffers[0]){
        for (size_t i = 0; i< nValues; i++){
            int32_t value_mv =  adc_to_mv(overviewBuffers[0][i],9);
            int currentstate = value_mv > threshold; 
            if(lastState==-1){
                // init
                lastState = currentstate;
                lastStateStart = count + i;
                continue;
            }


            if(currentstate!=lastState){
                // change
                int duration_us = (((count + i) - lastStateStart) * 50);   
                fprintf(fp, "%d %d\n",duration_us,lastState);
				if(10 == decoder.add(lastState==1,duration_us)){

					byte buffer[10];
					memcpy(buffer,decoder.getframe(),10);
					if(buffer[0]<0x40){
						for(int i=0;i<10;i++) buffer[i]^=0xFF;
					}
					std::string data = byteArrayToHexString(buffer,10);
					//printf("dump: %s    cs:%s\n",data.c_str(), checksum(buffer+1,8)==buffer[9]?"valid":"invalid");
					
					
					if(stringHashSet.count(data)<=0){
						stringHashSet.insert(data);
						printf("dump: %s\n",data.c_str());
					}
				}

                lastState = currentstate;
                count = 0;
                lastStateStart = count + i;
            }       
        } 
    }
    count+=nValues;
}

/****************************************************************************
 *
 *
 ****************************************************************************/
int main(int argc, char** argv)
{
	int8_t	ch;

	printf ( "SimpleDataRecorder (PicoScope)\n" );
	printf ( "Version 1.0\n\n" );

	printf ( "\n\nOpening the device...\n");

	//open unit and show splash screen
	handle = ps2000_open_unit ();
	printf ( "Handler: %d\n", handle );
	if ( !handle )
	{
		printf ( "Unable to open device\n" );
        return 99;
	}

    printf ( "Device opened successfully\n\n" );
	
    /* disable ETS*/
    ps2000_set_ets ( handle, PS2000_ETS_OFF, 0, 0 );

    ps2000_set_trigger ( handle, PS2000_NONE, 0, 0, 0, 0 );

    
    /* short ps2000_set_channel (short handle,short channel,short enabled,short dc,short range) */
    ps2000_set_channel (handle,PS2000_CHANNEL_A,TRUE,TRUE,PS2000_10V);

    fp = fopen("dump.txt","wt");
    fprintf(fp, "duration(us) bitstate\n");

    /*samplerate 50us */
    ps2000_run_streaming_ns ( handle, 50, PS2000_US, 10000, 0, 1, 50000 ); 

    while (!_kbhit())
	{
		ps2000_get_streaming_last_values (handle, ps2000FastStreamingReady);
		Sleep (1000);  // 100ms pause
	}
    ps2000_stop (handle);
    ps2000_close_unit ( handle );



    fprintf(fp, "%d %d\n",-1,lastState);
    fclose(fp);
    return 0;
}
	
