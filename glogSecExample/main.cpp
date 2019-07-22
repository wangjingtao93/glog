/*
 * main.cpp
 *  调用glog打印日志，并自动删除过期日志
 *  Created on: 2019-5-16
 *      Author: root
 */

#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <libio.h>
#include <pthread.h>

#include "glog/logging.h"

using namespace std;

//调用的函数
void AutoDelLog();
time_t GetMorningTime();

/*************************************************************************
函数名称：*ThreadAutoDelLogEntry
功能描述：线程函数入口
输入参数：void* arg
输出参数：无
返 回 值： 无
 *************************************************************************/
void *ThreadAutoDelLogEntry(void *arg)
{
	time_t OldTime = 0;

	//每天凌晨启动一次
	while (true)
	{
		if (time(NULL) - OldTime >= 24 * 60 * 60)
		{
			AutoDelLog();

			OldTime = GetMorningTime();
		}

		usleep(1000);
	}
}

/*************************************************************************
函数名称：main
功能描述：主函数
输入参数：无
输出参数：无
返 回 值： 无
 *************************************************************************/
int main()
{

	//起清理日志线程
	pthread_t threadLogDel;
	pthread_create(&threadLogDel, NULL, ThreadAutoDelLogEntry, NULL);

	//调用glog函数打印日志
	google::InitGoogleLogging("KLH");

	//为不同级别的日志设置不同的文件basename。
	google::SetLogDestination(google::INFO, "./glogfile/loginfo");
	google::SetLogDestination(google::WARNING, "./glogfile/logwarn");
	google ::SetLogDestination(google::GLOG_ERROR, "./glogfile/logerror");

	FLAGS_logbufsecs = 60; //缓存的最大时长，超时会写入文件
	FLAGS_max_log_size = 10; //单个日志文件最大，单位M
	FLAGS_logtostderr = true; //设置为true，就不会写日志文件了

	int i = 0;
	while (i < 1000000)
	{
		i++;

		LOG(INFO) << "info" << endl;
		LOG(WARNING) << "warning" << endl;
		LOG(ERROR) << "error : " << i << endl;
		// LOG(FATAL) << "fatal" << endl;

		//重至i，可以一直打印日志
		if (i == 1000000)
		{
			i = 0;
			sleep(24 * 60 * 60); //每隔24小时循环打印一次，一次打印一百万条
		}
	}

	//线程释放
	if (0 != threadLogDel)
	{
		pthread_join(threadLogDel, NULL);
		threadLogDel = 0;
	}

	google::ShutdownGoogleLogging();

	return 0;
}

/*************************************************************************
函数名称：MySystem
功能描述：调用linux系统指令函数
输入参数：const char *szCpcmd
输出参数：无
返 回 值： bool
 *************************************************************************/
bool MySystem(const char *szCpcmd)
{
	bool bMoveFlag = false;
	pid_t status = system(szCpcmd);
	if (-1 == status)
	{
		cout << "系统错误，执行命令失败[ " << szCpcmd << "]" << endl;
		//fprintf(m_fpLog, "系统错误，执行命令失败[%s]\n", szCpcmd);
	}
	else
	{
		//printf("命令[%s]执行结果[0x%x]\n", szCpcmd, status);
		cout << "命令[" << szCpcmd << "]"
			 << "执行结果" << hex << status << endl;
		//fprintf(m_fpLog, "命令[%s]执行结果[0x%x]\n", szCpcmd, status);
		if (WIFEXITED(status))
		{
			if (0 == WEXITSTATUS(status))
			{
				//printf("执行命令脚本“成功”\n");
				//fprintf(m_fpLog, "执行命令脚本“成功”\n");
				cout << "执行命令脚本“成功" << endl;
				bMoveFlag = true;
			}
			else
			{
				//printf("执行命令脚本“失败”，脚本退出代码[%d]\n", WEXITSTATUS(status));
				//fprintf(m_fpLog, "执行命令脚本“失败”，脚本退出代码[%d]\n", WEXITSTATUS(status));
				cout << "执行命令脚本[失败]，脚本退出代码[" << WEXITSTATUS(status) << "]" << endl;
			}
		}
		else
		{
			//printf("执行命令“失败”，退出代码[%d]\n", WIFEXITED(status));
			//fprintf(m_fpLog, "执行命令“失败”，退出代码[%d]\n", WIFEXITED(status));
			cout << "执行命令[失败]，退出代码[" << WIFEXITED(status) << "]" << endl;
		}
	}
	return bMoveFlag;
}

/*************************************************************************
函数名称：AutoDelLog
功能描述：自动清理日志
输入参数：无
输出参数：无
返 回 值： 无
 *************************************************************************/
void AutoDelLog()
{
	char szComDelLog[1024] = {0};
	snprintf(szComDelLog, 1024, "find %s -mtime +0 -name \"%s\" -exec rm -rf {} \\;", "./glogfile/", "log*");
	if (!MySystem(szComDelLog))
	{
		cout << "自动清理日志[失败]" << endl;
	}
}

/*************************************************************************
函数名称：GetMorningTime
功能描述：获取每天凌晨时间
输入参数：无
输出参数：无
返 回 值： time_t
 *************************************************************************/
time_t GetMorningTime()
{
	time_t t = time(NULL);				//获得现在的时间
	struct tm *zeroTom = localtime(&t); //转化为“2018-01-01 20：08：08这种格式
	zeroTom->tm_hour = 0;
	zeroTom->tm_min = 0;
	zeroTom->tm_sec = 0;

	time_t zeroTime = mktime(zeroTom);
	return zeroTime; //今天凌晨的时间
}