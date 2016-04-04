#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CACHE_UP_THRESH 80
#define CACHE_DOWN_THRESH 60

#define CACHE_MODE_ON 1
#define CACHE_MODE_OFF 0

#define CACHE1 1
#define CACHE2 2
#define CACHE3 3
#define CACHE4 4
#define CACHE5 5
char cpu_usage_cmd[128] = "top -b -n 1 | grep \"CPU\\:\" | awk '{print $2}' | sed s/\%//g";
char cache1_rw_cmd[128] = "iostat | grep md127 | awk '{print $5,$6}'";  
char cache2_rw_cmd[128] = "iostat | grep md126 | awk '{print $5,$6}'";  
char cache3_rw_cmd[128] = "iostat | grep md125 | awk '{print $5,$6}'";  
char cache4_rw_cmd[128] = "iostat | grep md124 | awk '{print $5,$6}'";  
char cache5_rw_cmd[128] = "iostat | grep dm- | awk '{print $5,$6}'";  

char clear_cmd[128] = "clear";

char cache1_show_cmd[128] = "echo -ne \"\\t\\t$(sysctl dev.flashcache.sdb1+md127.cache_all)\\n\"";
char cache2_show_cmd[128] = "echo -ne \"\\t\\t$(sysctl dev.flashcache.sdc1+md126.cache_all)\\n\"";
char cache3_show_cmd[128] = "echo -ne \"\\t\\t$(sysctl dev.flashcache.sdb2+md125.cache_all)\\n\"";
char cache4_show_cmd[128] = "echo -ne \"\\t\\t$(sysctl dev.flashcache.sdc2+md124.cache_all)\\n\"";

char cache1_off_cmd[128] = "sysctl -w dev.flashcache.sdb1+md127.cache_all=0";
char cache2_off_cmd[128] = "sysctl -w dev.flashcache.sdc1+md126.cache_all=0";
char cache3_off_cmd[128] = "sysctl -w dev.flashcache.sdb2+md125.cache_all=0";
char cache4_off_cmd[128] = "sysctl -w dev.flashcache.sdc2+md124.cache_all=0";

char cache1_on_cmd[128] = "sysctl -w dev.flashcache.sdb1+md127.cache_all=1";
char cache2_on_cmd[128] = "sysctl -w dev.flashcache.sdc1+md126.cache_all=1";
char cache3_on_cmd[128] = "sysctl -w dev.flashcache.sdb2+md125.cache_all=1";
char cache4_on_cmd[128] = "sysctl -w dev.flashcache.sdc2+md124.cache_all=1";

int cache1_mode = 1;
int cache2_mode = 1;
int cache3_mode = 1;
int cache4_mode = 1;
//disk
long long last_read_md127 = 0;
long long last_read_md126 = 0;
long long last_read_md125 = 0;
long long last_read_md124= 0;
long long last_write_md127 = 0;
long long last_write_md126 = 0;
long long last_write_md125 = 0;
long long last_write_md124 = 0;
//cache
long long last_read_dm127 = 0;
long long last_read_dm126 = 0;
long long last_read_dm125 = 0;
long long last_read_dm124= 0;
long long last_write_dm127 = 0;
long long last_write_dm126 = 0;
long long last_write_dm125 = 0;
long long last_write_dm124 = 0;


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
	int j=0;
	long long last_read = 0;
	long long last_write = 0;
	long long last_rw = 0;
	FILE *fp;
	int mode=0;
	buffer = (char*)malloc(32);
	fp = popen(i==CACHE1 ? cache1_rw_cmd :(i==CACHE2 ? cache2_rw_cmd : (i==CACHE3 ? cache3_rw_cmd : (i==CACHE4 ? cache4_rw_cmd : cache5_rw_cmd))), "r");

	if ( NULL == fp) {
		perror( "popen() 실패");
		return -1;
	}
	while( i != CACHE5 && fgets(buffer, 32, fp) ){
			output=(strtok(buffer," "));
			printf("\t\t%s  Read : %10d",i==CACHE1 ? "md127" : (i==CACHE2 ? "md126" : (i==CACHE3 ? "md125" : "md124" )),atoi(output)-(i==CACHE1 ? last_read_md127 :(i==CACHE2 ? last_read_md126 : (i==CACHE3 ? last_read_md125 : last_read_md124))));
			if(i==CACHE1)
			{
			   last_read = atoi(output);
			   last_read -= last_read_md127;
			   last_read_md127 += last_read;
			}
			else if(i==CACHE2)
			{
			   last_read = atoi(output);
               last_read -= last_read_md126;
			   last_read_md126 += last_read;
			}
			else if(i==CACHE3)
			{
			   last_read = atoi(output);
               last_read -= last_read_md125;
			   last_read_md125 += last_read;
			}
			else
			{
			   last_read = atoi(output);
               last_read -= last_read_md124;
			   last_read_md124 += last_read;
			}


            output=(strtok(NULL," "));
			printf("\t%s Write : %10d\n",(i==CACHE1 ? "md127" : (i==CACHE2 ? "md126" : (i==CACHE3 ? "md125" : "md124" ))),atoi(output)-(i==CACHE1 ? last_write_md127 : (i==CACHE2 ? last_write_md126 : (i==CACHE3 ? last_write_md125 : last_write_md124))));
			if(i==CACHE1)
			{
			   last_write = atoi(output);
			   last_write -= last_write_md127;
			   last_write_md127 += last_write;
			}
			else if(i==CACHE2)
			{
			   last_write = atoi(output);
               last_write -= last_write_md126;
			   last_write_md126 += last_write;
			}
			else if(i==CACHE3)
     		{
			   last_write = atoi(output);
               last_write -= last_write_md125;
			   last_write_md125 += last_write;

			}
			else
			{
			   last_write = atoi(output);
               last_write -= last_write_md124;
			   last_write_md124 += last_write;
			}

		}
	//Cache iostat moniter
	while(fgets(buffer,32,fp) != NULL && i == CACHE5)
	{
		output=(strtok(buffer," "));
	    printf("\t\t%s  Read  : %6d",j==0 ? "cache127" : (j==1 ? "cache126" : (j==2 ? "cache125" : "cache124" )),atoi(output)-(j==0 ? last_read_dm127 :(j==1 ? last_read_dm126 : (j==2 ? last_read_dm125 : last_read_dm124))));
			if(j==0)
			{
			   last_read = atoi(output);
			   last_read -= last_read_dm127;
			   last_read_dm127 += last_read;
			}
			else if(j==1)
			{
			   last_read = atoi(output);
               last_read -= last_read_dm126;
			   last_read_dm126 += last_read;
			}
			else if(j==2)
			{
			   last_read = atoi(output);
               last_read -= last_read_dm125;
			   last_read_dm125 += last_read;
			}
			else
			{
			   last_read = atoi(output);
               last_read -= last_read_dm124;
			   last_read_dm124 += last_read;
			}
        output=(strtok(NULL," "));
	    printf("\t%s  Write : %6d\n",j==0 ? "cache127" : (j==1 ? "cache126" : (j==2 ? "cache125" : "cache124" )),atoi(output)-(j==0 ? last_write_dm127 :(j==1 ? last_write_dm126 : (j==2 ? last_write_dm125 : last_write_dm124))));
			if(j==0)
			{
			   last_write = atoi(output);
			   last_write -= last_write_dm127;
			   last_write_dm127 += last_write;
			}
			else if(j==1)
			{
			   last_write = atoi(output);
               last_write -= last_write_dm126;
			   last_write_dm126 += last_write;
			}
			else if(j==2)
			{
			   last_write = atoi(output);
               last_write -= last_write_dm125;
			   last_write_dm125 += last_write;

			}
			else
			{
			   last_write = atoi(output);
               last_write -= last_write_dm124;
			   last_write_dm124 += last_write;
			}
	    
		
		
		
		j++;


	}

	

	

			

    last_rw = last_write + last_read;
	pclose(fp);
	return last_rw;
}

int main()
{
	int count=0;
	double cpu_usage = get_cpu_usage();
	int md127_rw, md126_rw, md125_rw, md124_rw, md123_rw;
    while(1)
	{
		printf("\t\t########################################################\n");
		printf("\t\tCache Mode Change Count : %d\n",count);
		printf("\t\t#####################CPU Usage##########################\n");
		cpu_usage=get_cpu_usage();
		printf("\t\t#####################Disk  IO Status####################\n");
		md127_rw = get_rw(CACHE1);
		md126_rw = get_rw(CACHE2);
		md125_rw = get_rw(CACHE3);
		md124_rw = get_rw(CACHE4);
		printf("\t\t#####################Cache IO Status#####################\n");
		md123_rw = get_rw(CACHE5);
		printf("\t\t#####################Cache Schedul Status################\n");
		
		system(cache1_show_cmd);
		system(cache2_show_cmd);
		system(cache3_show_cmd);
		system(cache4_show_cmd);
		printf("\t\t########################################################\n");
        printf("\t\tTEST = %d %d %d %d ",md127_rw, md126_rw,md125_rw,md124_rw);
		if (md127_rw > 1000000000 || md126_rw > 1000000000)  //1000000
			continue;
		
		if (cpu_usage > CACHE_UP_THRESH) {
			if(cache1_mode == CACHE_MODE_ON){
				system(cache1_off_cmd);
				cache1_mode = CACHE_MODE_OFF;
				count++;
			}
			if(cache2_mode == CACHE_MODE_ON){
				system(cache2_off_cmd);
				cache2_mode = CACHE_MODE_OFF;
				count++;
			}
			if(cache3_mode == CACHE_MODE_ON){
				system(cache3_off_cmd);
				cache3_mode = CACHE_MODE_OFF;
				count++;
			}
			if(cache4_mode == CACHE_MODE_ON){
				system(cache4_off_cmd);
				cache4_mode = CACHE_MODE_OFF;
				count++;
			}
			
			
			
		}

	/*	if(cache1_mode == CACHE_MODE_ON && md127_rw < 10000){
			system(cache1_off_cmd);
			cache1_mode = CACHE_MODE_OFF;
			count++;
		}
		if(cache2_mode == CACHE_MODE_ON && md126_rw < 10000){
			system(cache2_off_cmd);
			cache2_mode = CACHE_MODE_OFF;
			count++;
		}
		if(cache3_mode == CACHE_MODE_ON && md125_rw < 10000){
			system(cache3_off_cmd);
			cache3_mode = CACHE_MODE_OFF;
			count++;
		}
		if(cache4_mode == CACHE_MODE_ON && md124_rw < 10000){
			system(cache4_off_cmd);
			cache4_mode = CACHE_MODE_OFF;
			count++;
		}
		
	*/	
		
		if (cpu_usage < CACHE_DOWN_THRESH)
		{
           //작동 테스트
            if(cache1_mode == CACHE_MODE_OFF){
				system(cache1_on_cmd);
				cache1_mode = CACHE_MODE_ON;
			}
			if(cache2_mode == CACHE_MODE_OFF){
				system(cache2_on_cmd);
				cache2_mode = CACHE_MODE_ON;
			}
			if(cache3_mode == CACHE_MODE_OFF){
				system(cache3_on_cmd);
				cache3_mode = CACHE_MODE_ON;
			}
			if(cache4_mode == CACHE_MODE_OFF){
				system(cache4_on_cmd);
				cache4_mode = CACHE_MODE_ON;
			}
		
//io 조건 정하고 넣을것.
/*            if(cache1_mode == CACHE_MODE_OFF && md127_rw > 30000){
				system(cache1_on_cmd);
				cache1_mode = CACHE_MODE_ON;
			}
			if(cache2_mode == CACHE_MODE_OFF && md126_rw > 30000){
				system(cache2_on_cmd);
				cache2_mode = CACHE_MODE_ON;
			}
			if(cache3_mode == CACHE_MODE_OFF && md125_rw > 30000){
				system(cache3_on_cmd);
				cache3_mode = CACHE_MODE_ON;
			}
			if(cache4_mode == CACHE_MODE_OFF && md124_rw > 30000){
				system(cache4_on_cmd);
				cache4_mode = CACHE_MODE_ON;
			}*/
		}
		
		

		sleep(1); //1초 sleep
		system("clear");
	}

	return 0;
}
