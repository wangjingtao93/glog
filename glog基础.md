

#glog基础

## 工作记录
1. 从官方文档中暂时没有看到glog是如何处理过期的日志文件的
3. 研究linux是怎么清理日志的
4. 写程序，自动清理日志。
5. 查询更多的日制管理方式。比如syslog
6. 研究如何处理一定的size之后，管理多余的日志
7. 深入研究logrotate


## 1. 安装和编译

###1. 正确步骤

参照[Ubuntu14.04下安装glog](https://www.cnblogs.com/flyinggod/p/10120906.html)

1. 先apt search glog-，apt install一下，再继续下面的步骤。

2. Clone Source Code  glog

  ```
  git clone https://github.com/google/glog
  ```
3. Install dependencies and tools
```
sudo apt-get install autoconf automake libtool
```
4. install glog
```
./autogen.sh
./configure
make -j 6
sudo make install
```
5 .示例使用

[示例](https://blog.csdn.net/u012203028/article/details/79493067)

直接g++ 编译，连接到库即可，例如：g++ main.cpp -o test -L/usr/local/lib/ -lglog



## 2. 知识点

### 1. 概述
1.  日志文件名格式为：
basename+时间搓（年月日-HH:MM:SS.主线程ID）
要为不同级别日志文件设置不同的basename，**不能相同**。
google::SetLogDestination(google::INFO,"c:\\log");  
*注*：这里log是文件名的一部分，并不是指定文件夹log。最终日志文件名类似：log20140611-095200.9520


2. 程序每次重新启动后，会重新生成新的日志文件。

 

3. 日志是分级别存放的，低级别的日志文件包含高级别的日志信息。
如INFO级别的日志包含其他高级别的所有日志，ERROR级别的日志只包含ERROR和FATAL两个级别。


4.  LOG(LEVEL)使用方式是线程安全。

### 2. 日志级别
1. 打印FATAL时，打印完这一条后，程序会终止
### 3. 设置Flags

有几个flags会影响到glog的输出。安装了gflags,可以直接输入下面命令让所有日志打印到屏幕
```
./your_application --logtostderr=1
```
没有安装gflags,则输入命令：
```
GLOG_logtostderr=1 ./your_application
```

**下面的flags是经常用到的**：
1. `logtostderr (bool, default=false)`
  打印消息到stderr而不是logfiles
  *注意*：值可以是1，true，或者yes，不区分大小写，并且值可以使0，false或者no，同样也不区分大小写

2. `stderrthreshold (int, default=2, which is ERROR)` 
打印日志到logfiles同时拷贝一份级别高的日志在屏幕上显示，日志的级别INFO, WARNING, ERROR、FATAL对应的值是0,1,2，3。这个flag将被淘汰

3. `minloglevel (int, default=0, which is INFO)`
打印级别之上的日志。同样日志的级别INFO, WARNING, ERROR、FATAL对应的值是0,1,2，3

4. `log_dir (string, default="")`
日志路径，如果指定了，则写入指定的路径，不再写入默认路径

5. v (int, default=0)
显示所有的`VLOG(m)`小于或等于`m`的message。关于vmodule这块的知识，详情参见

6. `vmodule (string, default="")`
每个模块的详细级别，

还有一些其他的flags,在loggin.cc定义了。请grep“Defined_”的源代码查看所有标志的完整列表,下面将列举粗所有的flags：
1. `max_log_size（int,default=1800）`
日志文件的最大默认为1800M
2. `stop_logging_if_full_disk(string,default="false")`
磁盘满了停止写日志到磁盘上，默认是false状态

3. `alsologtostderr(bool, default = false)`
打印日志到文件和屏幕上（stderr）
4. `colorlogtostderr(bool, default = false)`

5. `drop_log_memory(bool, default = true)` 
删除日志内容的内存缓冲区。日志可以增长得非常快，并且在需要从内存中删除日志之前很少被读取。只要它们被flush到磁盘就把它们从内存中删除。
6. `alsologtoemail (string, default="")`
日志消息到这些电子邮件地址，同时写到文件中
7. `log_prefix(bool, default = true)` 
将日志前缀前置到每个日志行开头
8.  `minloglevel（int,default=0）`
在比这更低级别上记录的消息实际上不会在任何地方被记录
9.  `logbuflevel（int,default=0）`
缓冲区日志消息记录在此级别或更低级别，-1示不缓冲;0只表示仅缓存INFO
10.  `logbufsecs（int,default=30）`
缓冲区日志消息最多持续这么多秒
11.  `logbufsecs（int,default=999）`
邮件日志消息记录在这个级别或更高,0表示所有，3表示仅FATAL
12.  `logmailer (string, default="/bin/mail")`
用于发送日志邮件
11. `log_backtrace_at (string, default="")` 
在文件linenum上进行日志记录时发出回溯跟踪

也可以通过修改全局变量`FLAGS_*`，修改程序中的flags的值。大部分的设置在你更新`FLAGS_*`后会立刻生效。例外的情况是与目标文件相关的flags。例如，也许你想设置` FLAGS_log_dir`在调用`google::InitGoogleLogging`。下面有个例子
```C++
   LOG(INFO) << "file";
   // Most flags work immediately after updating values.
   FLAGS_logtostderr = 1;
   LOG(INFO) << "stderr";
   FLAGS_logtostderr = 0;
   // This won't change the log destination. If you want to set this
   // value, you should do this before google::InitGoogleLogging .
   FLAGS_log_dir = "/some/log/directory";
   LOG(INFO) << "the same file";
```


### 4. 有条件的/偶尔的打印日志

如果只想记录某些特定情况下的日志，可以通过下面的宏定义运行有条件的日志打印
```
LOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
```
上面的输出是在执行日志消息的第1、11、21次时输出日志消息。注意这个特殊的`google::COUNTER`是用来识别正在发生的重复。

可以将条件日志记录和偶尔日志记录与以下宏组合在一起：
```
 LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << google::COUNTER << "th big cookie";
```
不用每次都输出message,可以限制为...,例如：
```
LOG_FIRST_N(INFO, 20) << "Got the " << google::COUNTER << "th cookie"
```
在执行前20次输出message

### 5.Debug Mode support
特殊的“Debug Mode”日志宏只在Debug Mode 下起作用，而对于非Debug Mode编译则被编译为空。使用这些宏可避免由于过度日志记录而降低生产应用程序的速度:
```
   DLOG(INFO) << "Found cookies";

   DLOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";

   DLOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th cookie";
```
### 4. CHECK Macros
经常检查程序中的预期条件以尽早检测错误是一种很好的做法,Check宏提供了在不满足条件时中止应用程序的能力，类似于标准C库中定义的assert宏。

### 5. Verbose Logging(更细致的logging)
当您在跟踪复杂的bug时，更详尽的日志消息非常有用。

### 6. Failure Signal Handler
故障信号处理程序，该库提供了一个方便的信号处理程序，当程序在某些信号(如SIGSEGV)上崩溃时，将转储有用的信息。类似下面：
```
*** Aborted at 1225095260 (unix time) try "date -d @1225095260" if you are using GNU date ***
*** SIGSEGV (@0x0) received by PID 17711 (TID 0x7f893090a6f0) from PID 0; stack trace: ***
PC: @           0x412eb1 TestWaitingLogSink::send()
    @     0x7f892fb417d0 (unknown)
    @           0x412eb1 TestWaitingLogSink::send()
    @     0x7f89304f7f06 google::LogMessage::SendToLog()
    @     0x7f89304f35af google::LogMessage::Flush()
    @     0x7f89304f3739 google::LogMessage::~LogMessage()
    @           0x408cf4 TestLogSinkWaitTillSent()
    @           0x4115de main
    @     0x7f892f7ef1c4 (unknown)
    @           0x4046f9 (unknown)
```
默认情况下，信号处理程序将故障转储写入标准错误。但可以通过InstallFailureWriter()自定义目标。

### 7. 附注

#### 1. Performance of Messages

Glog提供的conditional logging被carefully执行，并且当条件为false时，不执行右侧表达式，所以，下面的`CHECK`可能不会牺牲应用程序的性能。
```
CHECK(obj.ok) << obj.CreatePrettyFormattedStringButVerySlow();
```
#### 2.User-defined Failure Function
用户定义的故障函数，FATAL级别的消息或者不满足CHECK条件，将会终止程序，你可以修改这种behavior通过`InstallFailureFunction`。
```
   void YourFailureFunction() {
     // Reports something...
     exit(1);
   }

   int main(int argc, char* argv[]) {
     google::InstallFailureFunction(&YourFailureFunction);
   }
```
默认情况下，glog尝试转储堆栈跟踪并使程序以状态1退出。

#### 3. Raw Logging

#### 4. Syslog 

` SYSLOG, SYSLOG_IF, SYSLOG_EVERY_N`宏定义是可用的。 除了普通日志之外，这些日志还可以登录（log to） syslog。但是注意这样会极大的影响性能，特别是当syslog被配置为远程日志记录。在使用这些宏之前，一定要了解输出到syslog的含义，要谨慎的使用这些宏

#### 5. Strip Logging Messages
日志消息中使用的字符串可以增加二进制文件的大小，并表示对隐私的关注。因此，您可以指示Glog通过使用GOOGLE_LOG宏删除低于一定严重级别的所有字符串：
如果你的程序代码像这样：
```
  #define GOOGLE_STRIP_LOG 1    // this must go before the #include!
  #include <glog/logging.h>
```
编译器将删除其严重程度小于指定整数值的日志消息。因为vlog日志记录在严重级别info(数值0)，所以将google_strig_log设置为1或更高,将删除与vlog和info log语句关联的所有日志消息。