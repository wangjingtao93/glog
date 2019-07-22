# syslog


## 1. 关于syslog
###1. 简介
syslog是Linux系统默认的日志守护进程。默认的主配置文件和辅助配置文件分别是/etc/syslog.conf和/etc/sysconfig/syslog文件。通常，syslog 接受来自系统的各种功能的信息，每个信息都包括重要级。/etc/syslog.conf 文件通知 syslogd 如何根据设备和信息重要级别来报告信息。

### 2. 配置文件
1. /etc/syslog.conf 文件

2. level

3. ation 动作域
- 日志信息可以记录到多个文件里
- 可以发送到命名管道
- 可以发送给其他程序
- 可以发送给另一台服务器
包括三类：
file   指定文件的绝对路径
terminal或prin 完全的穿黄或者并行设备标志符
@host(@ip地址)远程日志服务器


4. syslog APIs（可以调用）