#!/bin/sh
find /root/glogBrpc/glogSecExample/glogfile/ -mtime +0 -name "mylog*" -exec rm -rf {} \;
