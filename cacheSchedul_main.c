#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CACHE_UP_THRESH 80
#define CACHE_DOWN_THRESH 60

#define CACHE_MODE_ON 1
#define CACHE_MODE_OFF 0

#define CACHE1 1
#define CACHE2 2

char cpu_usage_cmd[128] = "top -b -n 1 | grep \"CPU\\:\" | awk '{print $2}' | sed s/\%//g";
//char cache1_read_cmd[128] = "iostat | grep md127 | awk '{print $5}'";
//char cache1_write_cmd[128] = "iostat | grep md127 | awk '{print $6}'";
//char cache2_read_cmd[128] = "iostat | grep md126 | awk '{print $5}'";
//char cache2_write_cmd[128] = "iostat | grep md126 | awk '{print $6}'";
char cache1_rw_cmd[128] = "iostat | grep md127 | awk '{print $5,$6}'";  
char cache2_rw_cmd[128] = "iostat | grep md126 | awk '{print $5,$6}'";  

char clear_cmd[128] = "clear";

char cache1_show_cmd[128] = "echo -ne \"\\t\\t$(sysctl dev.flashcache.sdb1+md127.cache_all)\\n\"";
char cache2_show_cmd[128] = "echo -ne \"\\t\\t$(sysctl dev.flashcache.sdc1+md126.cache_all)\\n\"";

char cache1_off_cmd[128] = "sysctl -w dev.flashcache.sdb1+md127.cache_all=0";
char cache2_off_cmd[128] = "sysctl -w dev.flashcache.sdc1+md126.cache_all=0";
//char * cache3_off_cmd = "sysctl -w dev.flashcache.sdb2+md125.cache_all=0";
//char * cache4_off_cmd = "sysctl -w dev.flashcache.sdc2+md124.cache_all=0";

char cache1_on_cmd[128] = "sysctl -w dev.flashcache.sdb1+md127.cache_all=1";
char cache2_on_cmd[128] = "sysctl -w dev.flashcache.sdc1+md126.cache_all=1";
//char * cache3_on_cmd = "sysctl -w dev.flashcache.sdb2+md125.cache_all=1";
//char * cache4_on_cmd = "sysctl -w dev.flashcache.sdc2+md124.cache_all=1";

int cache1_mode = 1;
int cache2_mode = 1;

long long last_read_md127 = 0;
long long last_read_md126 = 0;
long long last_write_md127 = 0;
long long last_write_md126 = 0;

double get_cpu_usage(){
	char *buffer = NULL;
	double cpu_usage = 0;
	FILE *fp;
	
	buffer = (char*)malloc(8);
	fp = popen(cpu_usage_cmd, "r");

	if ( NULL == fp) {
		perror( "popen() 실패");
		return -1;
	}

	while( fgets( buffer, 8, fp) ){
		printf("\t\tCPU_Usage : %s", buffer);
	}

	cpu_usage = atof(buffer);

	pclose(fp);
	return cpu_usage;
}

int get_rw(int i){
	char *output = NULL;
	char *buffer = NULL;
	long long last_read = 0;
	long long last_write = 0;
	long long last_rw = 0;
	FILE *fp;
	FILE *fp2;
	int mode=0;
	buffer = (char*)malloc(32);
	fp = popen(i==CACHE1 ? cache1_rw_cmd : cache2_rw_cmd, "r");
//	fp = popen(i==CACHE1 ? cache1_read_cmd : cache2_read_cmd, "r");
//	fp2 = popen(i==CACHE1 ? cache1_write_cmd : cache2_write_cmd, "r");

	if ( NULL == fp) {
		perror( "popen() 실패");
		return -1;
	}

	while( fgets( buffer, 32, fp) ){

//		if(i==CACHE1 && mode == 0)
//		{
			output=(strtok(buffer," "));
			printf("\t\t%s  Read : %d\n",(i==CACHE1 ? "md127" : "md126"),atoi(output)-(i==CACHE1 ? last_read_md127 : last_read_md126));
			if(i==CACHE1)
			{
			   last_read = atoi(output);
			   last_read -= last_read_md127;
			   last_read_md127 += last_read;
			}
			else
			{
			   last_read = atoi(output);
               last_read -= last_read_md126;
			   last_read_md126 += last_read;
			}

            output=(strtok(NULL," "));
			printf("\t\t%s Write : %d\n",(i==CACHE1 ? "md127" : "md126"),atoi(output)-(i==CACHE1 ? last_write_md127 : last_write_md126));

			if(i==CACHE1)
			{
			   last_write = atoi(output);
			   last_write -= last_write_md127;
			   last_write_md127 += last_write;
			}
			else
			{
			   last_write = atoi(output);
               last_write -= last_write_md126;
			   last_write_md126 += last_write;
			}



				
			

		}
	




    last_rw = last_write + last_read;
	//printf("\t\t%s  R+W : %d\n",(i==CACHE1 ? "md127" : "md126"),last_rw);
	pclose(fp);
	return last_rw;
}

int main()
{
    while(1)
	{
		double cpu_usage = get_cpu_usage();
		long long md127_rw = get_rw(CACHE1);
		long long md126_rw = get_rw(CACHE2);
		system(cache1_show_cmd);
		system(cache2_show_cmd);
		printf("\n\n");

		if (md127_rw > 1000000000 || md126_rw > 1000000000)  //1000000
			continue;
		
		if (cpu_usage > CACHE_UP_THRESH) {
			//system("clear");
			//printf("\tCPU_Usage : Up_Threshold(%.2lf\%:%d\%), Cache OFF\n\
\tmd127 RW  : %ldkB/s\n\
\tmd126 RW  : %ldkB/s\n", cpu_usage, CACHE_UP_THRESH, md127_rw, md126_rw);
			if(cache1_mode == CACHE_MODE_ON){
				system(cache1_off_cmd);
				cache1_mode = CACHE_MODE_OFF;
			}
			if(cache2_mode == CACHE_MODE_ON){
				system(cache2_off_cmd);
				cache2_mode = CACHE_MODE_OFF;
			}
		}

		if(cache1_mode == CACHE_MODE_ON && md127_rw < 10000){
			system(cache1_off_cmd);
			cache1_mode = CACHE_MODE_OFF;
		}
		if(cache2_mode == CACHE_MODE_ON && md126_rw < 10000){
			system(cache2_off_cmd);
			cache2_mode = CACHE_MODE_OFF;
		}
		
		if (cpu_usage < CACHE_DOWN_THRESH)
		{
			//system("clear");
			//printf("\tCPU_Usage : Down_Threshold(%.2lf\%:%d\%), Cache ON\n\
\tmd127 RW  : %ldkB/s\n\
\tmd126 RW  : %ldkB/s\n", cpu_usage, CACHE_DOWN_THRESH, md127_rw, md126_rw);

            if(cache1_mode == CACHE_MODE_OFF && md127_rw > 30000){
				system(cache1_on_cmd);
				cache1_mode = CACHE_MODE_ON;
			}
			if(cache2_mode == CACHE_MODE_OFF && md127_rw > 30000){
				system(cache2_on_cmd);
				cache2_mode = CACHE_MODE_ON;
			}
		}

		sleep(1); //1초 sleep
	}

	return 0;
}
