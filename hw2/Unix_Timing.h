#include "stdio.h" // for printf, print time to stdout
#include "sys/time.h" // for timing function
#define TIMING_START 1 // define timing-start macro, DO NOT change
#define TIMING_END 2 // define timing-end macro, DO NOT change

void Timing(int task){
    static struct timeval start_time,end_time;
    static int init = 0;
    int sec,usec;
    switch(task){
    case TIMING_START:
        init = 1;
        gettimeofday(&start_time,0);
        break;
    case TIMING_END:
        if(init){
            gettimeofday(&end_time,0);
            sec=end_time.tv_sec-start_time.tv_sec;
            usec=end_time.tv_usec-start_time.tv_usec;
            printf("Elapsed Time:%lf second(s)\n",(sec*1000+(usec/1000.0))/1000);
        }
        else{
            printf("You MUST to call Timing(TIMING_START) before calling Timing(TIMING_END)\n");
        }
        break;
    }
}

