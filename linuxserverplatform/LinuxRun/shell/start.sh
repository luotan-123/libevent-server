#!/bin/bash


program="LoaderServer"

pro_now=`ps aux | grep $program | grep -v grep | wc -l`
echo $pro_now
if [ $pro_now -lt 1 ];then
    ulimit -c unlimited
    ./$program  &
else
    eStr=${program}" is running!!!"
    echo $eStr
fi

eStr=${program}" is start!!!"
echo $eStr

