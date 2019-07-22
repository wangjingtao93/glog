已经与2019年5月16日写完，在文件夹/glogSecExample/main.cpp函数里

该函数能够调用glog不断的生成日志文件，并自动清理一天前的日志文件。

可以直接使用 g++ main.cpp -o main -L/usr/local/lib -lglog -lpthread
进行编译