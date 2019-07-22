#glog日志管理

## 1. 概述

从官方文档中暂时没有看到glog是如何处理过期的或者多余的日志文件的， 查到资料说：glog的缺点是没有回转覆写、轮转切割功能，日志文件会一直增长，需要其他方式清理过期日志或者管理日志[here](http://www.cppblog.com/pizzx/archive/2014/06/18/207320.html)。

## 2. 工作内容

1. 研究了glog库相关知识，并尝试使用各类flags
2. 研究使用脚本方式，自动删除过期文件（已经实现）
3. 研究linux系统的logrotate，以及如何管理syslog日志
4. 写了一个程序，可以删除过期的glog日志文件（已经实现）
5. 自己配置了一个GlogConf，[here](./GlogConf)，可以放到/etc/logrotate.d/路径下直接使用（日志的路径根据情况要修改）


### 2. 使用脚本，自动删除过期文件
在文件夹下有一个写好的脚本可以参考，auto-del-1-days-ago-log.sh

####1. 删除文件命令
find 对应目录 -mtime +天数 -name "文件名" -exec -rf {} \;
实例：
find /opt/soft/log -mtime +30 -name "*.log" -exec rm -rf {} \ ;
*说明*：
-mtime：标准语法
+30：查找30天前的文件
-exec：固定写法
rm -rf:强制删除命令，包括目录；
{} \; :固定写法，**一对大括号+空格+\+;

####2. 计划任务
每次手动很麻烦，可以写一个shell脚本，再设置cron调度执行，就可以让系统自动去清理相关文件。

1. 创建shell
新建一个可执行文件auto-del-30-days-ago-log.sh,并分配可执行权限
```
touch /opt/soft/bin/auto-del-30-days-ago-log.sh

chmod +x auto-del-30-days-ago-log.sh
```
2. 编辑shell脚本
```
vi auto-del-30-days-ago-log.sh
```
文件内如下
```
#!/bin/sh

find /opt/soft/log/ -mtime +30 -name "*.log" -exec rm -rf {} \;
```
3. 计划任务
```
#crontab -e
```
将auto-del-30-days-ago-log.sh执行脚本加入到系统计划任务，到点自动执行

输入：
```
10 0 * * * /opt/soft/log/auto-del-7-days-ago-log.sh
```
这里的设置是每天凌晨0点10分执行auto-del-7-days-ago-log.sh文件进行数据清理任务了。

完成以上三步，你就再也不每天惦记是否硬盘空间满了，该清理日志文件了，

### 3. 使用logrotate管理glog日志

#### 概述 logrotate
linux 日志定时轮询流程详解(logrotate)
[here](https://www.cnblogs.com/kevingrace/p/6307298.html)

#### 1. logrotate介绍
当日志文件不断增长的时候，就需要定时切割。logrotate它可以自动对日志进行自动轮替、删除、压缩和mail日志。例如，你可以设置logrotate，让/var/log/foo日志文件每30天轮循，并删除超过6个月的日志。配置完后，logrotate的运作完全自动化，不必进行任何进一步的人为干预。

#### 2. 轮替（rotate）
即系统管理中一个自动化的归档过期文件的过程。

#### 3. 切割操作
比如以系统日志/var/log/message做切割来简单说明下：

第一次执行完rotate(轮转)之后，原本的messages会变成messages.1，而且会制造一个空的messages给系统来储存日志；

第二次执行之后，messages.1会变成messages.2，而messages会变成messages.1，又造成一个空的messages来储存日志！
如果仅设定保留三个日志（即轮转3次）的话，那么执行第三次时，则 messages.3这个档案就会被删除，并由后面的较新的保存日志所取代！也就是会保存最新的几个日志。

日志究竟rotate几次，这个是根据配置文件logrotate.conf中的`dateext` 参数来判定的

#### 4. logrotate配置文件

Linux系统默认安装logrotate工具，它默认的配置文件在：
/etc/logrotate.conf
/etc/logrotate.d/

logrotate.conf 才主要的配置文件，logrotate.d 是一个目录，该目录里的所有文件都会被主动的读入/etc/logrotate.conf中执行。

另外，如果 /etc/logrotate.d/ 里面的文件中没有设定一些细节，则会以/etc/logrotate.conf这个文件的设定来作为默认值。

Logrotate是基于CRON来运行的，其脚本是/etc/cron.daily/logrotate，日志轮转是系统自动完成的。
实际运行时，Logrotate会调用配置文件/etc/logrotate.conf

可以在/etc/logrotate.d目录里放置自定义好的配置文件，用来覆盖Logrotate的缺省值。

下面是logrotate的脚本内容
```
[root@localhost cron.daily]# cat /etc/cron.daily/logrotate 
#!/bin/sh

/usr/sbin/logrotate -s /var/lib/logrotate/logrotate.status /etc/logrotate.conf
EXITVALUE=$?
if [ $EXITVALUE != 0 ]; then
    /usr/bin/logger -t logrotate "ALERT exited abnormally with [$EXITVALUE]"
fi
exit 0

```
如果等不及cron自动执行日志轮转，想手动强制切割日志，需要加-f参数；不过正式执行前最好通过Debug选项来验证一下（-d参数），这对调试也很重要：
```
/usr/sbin/logrotate -f /etc/logrotate.d/glogfile
/usr/sbin/logrotate -d -f /etc/logrotate.d/glogfile
```
logrotate命令格式：
logrotate [OPTION...] <configfile>
- -d, --debug ：debug模式，测试配置文件是否有错误。
- -f, --force ：强制转储文件。
- -m, --mail=command ：压缩日志后，发送日志到指定邮箱。
- -s, --state=statefile ：使用指定的状态文件。
- -v, --verbose ：显示转储过程。



  *更多操作参见*[here](https://www.jb51.net/article/115694.htm)

  *注意*：使用/usr/sbin/logrotate -f /etc/logrotate.d/glogfile时，只执行/glogfile里的参数，不执行logrotate.conf的默认参数

#### 5. 其他操作命令
1. 根据日志切割设置进行操作，并显示详细信息
  ```
  /usr/sbin/logrotate -v /etc/logrotate.conf 
  /usr/sbin/logrotate    -v /etc/logrotate.d/glogfile
   ```

2. 根据日志切割设置进行执行，并显示详细信息,但是不进行具体操作，debug模式
 ```
 /usr/sbin/logrotate -d /etc/logrotate.conf 
 /usr/sbin/logrotate -d /etc/logrotate.d/glogfile
```
3. 查看各log文件的具体执行情况
```
 cat /var/lib/logrotate.status
 ```


**logrotate.d下文件格式**
```
日志文件路径 ...{ #多个文件绝对路径路径可以用空格、换行分隔，
  参数配置
}
```

#### 6 logrotate.conf配置

cat /etc/logrotate.conf
```
# 底下的设定是 "logrotate 的默认值" ，如果別的文件设定了其他的值，就会以其它文件的设定为主

# see "man logrotate" for details，详情可“man logratate"
# rotate log files weekly,默认每周一次rotate
weekly

# keep 4 weeks worth of backlogs,保留4个轮替日志
#保留多少个日志文件备份，默认保留四个。就是指定日志文件删除之前轮转的次数，0是指没有备份
rotate 4 

# create new (empty) log files after rotating old ones，轮替后创建新的日志文件
# 新的日志文件具有和原来的文件相同的权限；因为日志被改名,因此要创建一个新的来继续存储之前的日志
create

# use date as a suffix of the rotated file，使用时间作为轮替文件的后缀
# 这个参数很重要！就是切割后的日志文件以当前日期为格式结尾，
# 如xxx.log-20131216这样,如果注释掉,切割出来就是按数字递增,即前面说的 xxx.log-1这种格式
dateext

# uncomment this if you want your log files compressed,如果需要压缩日志，去除注释
# 通过gzip压缩转储以后的日志文件，如xxx.log-20131216.gz 
#compress

# RPM packages drop log rotation information into this directory,让/etc/logrotate.d目录下面配置文件内容参与轮替
# 将 /etc/logrotate.d/ 目录中的所有文件都加载进来
include /etc/logrotate.d

# no packages own wtmp and btmp -- we'll rotate them here
# 这个 wtmp 可记录用户登录系统及系统重启的时间
/var/log/wtmp {             #轮替对象为/var/log/中的wtmp文件
    monthly                 #每个月轮替一次
    create 0664 root utmp   #创建新的日志文件 权限 所属用户 所属组
        minsize 1M          #日志大小大于1M后才能参与轮替
    rotate 1                #保留一个轮替日志文件
}

/var/log/btmp {
    missingok               #如果日志文件不存在，继续进行下一个操作，不报错
    monthly
    create 0600 root utmp
    rotate 1
}

# system-specific logs may be also be configured here.
# 这里还可以配置特定于系统的日志。
```

#### 7. 其他重要参数说明
比如，在/etc/logrotate.d/文件夹下定义了glogfile,比如现在我的glog日志路径在`/root/glogBrpc/glogSecExample/glogfile`文件夹下，所有的日志文件格式是例如：logwarn20190516-095155.12991

vim /etc/logrotate.d/glogfile
```
/root/glogBrpc/glogSecExample/glogfile/log* {
    missingok
    notifempty
    sharedscripts

    postrotate
    kill -HUP ‘cat /root/glogBrpc/glogSecExample/glogfile/filename #最后跟文件名
    endscript
}

```

参数|解释
---|---
compress|通过gzip 压缩转储以后的日志
nocompress|不做gzip压缩处理
copytruncate|用于还在打开中的日志文件，把当前日志备份并截断；是先拷贝再清空的方式，拷贝和清空之间有一个时间差，可能会丢失部分日志数据。
nocopytruncate|备份日志文件，不过不截断
create mode owner group|轮转时指定创建新文件的属性，如create 0777 nobody nobody
nocreate|不建立新的日志文件
delaycompress|和compress 一起使用时，转储的日志文件到下一次转储时才压缩
nodelaycompress|覆盖 delaycompress 选项，转储同时压缩。
errors address|专储时的错误信息发送到指定的Email 地址
ifempty|即使日志文件为空文件也做轮转，这个是logrotate的缺省选项。
notifempty|当日志文件为空时，不进行轮转
mail address|把转储的日志文件发送到指定的E-mail 地址
nomail|轮替时不发送日志文件
olddir|将轮替的文件移至指定目录下，必须和当前日志文件在同一个文件系统，如olddir /root/glogBrpc/glogSecExample/oldglogfile
noolddir|rotate后的日志文件和当前日志文件放在同一个目录下
sharedscripts|运行postrotate脚本，作用是在所有日志都轮转后统一执行一次脚本。如果没有配置这个，那么每个日志轮转后都会执行一次脚本
prerotate...endscript|在logrotate rotate之前前需要执行的指令，例如修改文件的属性等动作；必须独立成行
postrotate...endscript|在logrotate转储之后需要执行的指令，(kill -HUP)打开某个服务，必须独立成行。上述的作用一般主要是为了让使用该日志文件的程序知道这个日志被替换了，需要重新打开这个文件！
daily|指定转储周期为每天
weekly|指定转储周期为每周
monthly|指定转储周期为每月
rotate|指定日志文件删除之前转储的次数，意思是：ratate 0 指没有备份，rotate 2 指最多保留2个备份,多余的老旧日志就被覆盖了。保留的日志文件个数，当达到指定个数，会按照日志新旧把最旧的删除
dateext|使用当期日期作为命名格式。轮替旧日志文件时，文件名添加-%Y %m %d形式日期，可用dateformat选项扩展配置。
nodateext|旧日志文件不使用dateext扩展名，后面序数自增如"*.log.1"
dateformat .%s|配合dateext使用，紧跟在下一行出现，定义文件切割后的文件名，必须配合dateext使用，只支持 %Y %m %d %s 这四个参数。
size|日志达到多少后进行rotate。size=5 或 size 5 （>= 5 个字节就转储）size = 100M 或 size 100M
minsize|文件容量一定要超过多少后才进行rotate
missingok|意思是如果找不到载上述定义的log*日志，也不会报错，直接跳过
sharedscripts|作用域下文件存在至少有一个满足轮替条件的时候，完成轮替后执行一次postrotate内的脚本。



#### 8. 使用logroate管理glog日志操作演练
参照[here](https://www.jb51.net/article/115694.htm)

注意：使用的命令是：`/usr/sbin/logrotate -v /etc/logrotate.d/glogfile`,logrotate.conf的默认配置不会被加载进来。
本人操作记录：
1. 使用copytruncate，感觉像是使用了creat，都是存到oldGlogfile之后，在glogfile创建一个新的空的相同的日志文件。只是说在没定义rotate时copytruncate默认是1（备份一份），creat不会备份

2. 
