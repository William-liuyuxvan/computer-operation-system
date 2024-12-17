#define __LIBRARY__
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <sys/wait.h>

#define HZ 100

void cpuio_bound(int last, int cpu_time, int io_time);

int main(int argc, char *argv[])
{
    pid_t pid1, pid2, pid3, pid4;
    /* 第1个子进程。*/
    /* fork失败->负数;fork成功->父进程返回的是子进程的pid（>0），子进程返回0*/
    pid1 = fork();
    if (pid1 < 0) /* 负数->出现了错误。 */
        printf("error in fork!， errno=%d\n", pid1);
    else if (pid1 == 0) /* 0->子进程。 */
    {
        printf("Child process 1 is running.\n");
        cpuio_bound(10, 1, 0); /* 占用10秒的CPU时间 */
        printf("Child process 1 is end.\n");
        return 0;
    }

    /* 第2个子进程*/
    pid2 = fork();
    if (pid2 < 0)
        printf("error in fork!， errno=%d\n", pid2);
    else if (pid2 == 0)
    {
        printf("Child process 2 is running.\n");
        cpuio_bound(10, 0, 1); /* 以IO为主要任务 */
        printf("Child process 2 is end.\n");
        return 0;
    }

    /* 第3个子进程*/
    pid3 = fork();
    if (pid3 < 0)
        printf("error in fork!， errno=%d\n", pid3);
    else if (pid3 == 0)
    {
        printf("Child process 3 is running.\n");
        cpuio_bound(10, 1, 1); /* CPU和IO各1秒钟轮回 */
        printf("Child process 3 is end.\n");
        return 0;
    }

    /* 第4个子进程*/
    pid4 = fork();
    if (pid4 < 0)
        printf("error in fork!， errno=%d\n", pid4);
    else if (pid4 == 0)
    {
        printf("Child process 4 is running.\n");
        cpuio_bound(10, 1, 9); /* IO时间是CPU的9倍 */
        printf("Child process 4 is end.\n");
        return 0;
    }

    printf("This process's pid is %d\n", getpid());
    printf("Pid of child process 1 is %d\n", pid1);
    printf("Pid of child process 2 is %d\n", pid2);
    printf("Pid of child process 3 is %d\n", pid3);
    printf("Pid of child process 4 is %d\n", pid4);

    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);

    printf("end.\n");
    return 0;
}

/*
 * 此函数按照参数占用CPU和I/O时间
 * last: 函数实际占用CPU和I/O的总时间，不含在就绪队列中的时间，>=0是必须的
 * cpu_time: 一次连续占用CPU的时间，>=0是必须的
 * io_time: 一次I/O消耗的时间，>=0是必须的
 * 如果last > cpu_time + io_time，则往复多次占用CPU和I/O
 * 所有时间的单位为秒
 */
void cpuio_bound(int last, int cpu_time, int io_time)
{
	struct tms start_time, current_time;
	clock_t utime, stime;
	int sleep_time;

	while (last > 0)
	{
		/* CPU Burst */
		times(&start_time);
		/* 其实只有t.tms_utime才是真正的CPU时间。但我们是在模拟一个
		 * 只在用户状态运行的CPU大户，就像“for(;;);”。所以把t.tms_stime
		 * 加上很合理。*/
		do
		{
			times(&current_time);
			utime = current_time.tms_utime - start_time.tms_utime;
			stime = current_time.tms_stime - start_time.tms_stime;
		} while ( ( (utime + stime) / HZ )  < cpu_time );
		last -= cpu_time;

		if (last <= 0 )
			break;

		/* IO Burst */
		/* 用sleep(1)模拟1秒钟的I/O操作 */
		sleep_time=0;
		while (sleep_time < io_time)
		{
			sleep(1);
			sleep_time++;
		}
		last -= sleep_time;
	}
}

