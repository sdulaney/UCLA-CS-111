# NAME: Stewart Dulaney
# EMAIL: sdulaney@ucla.edu
# ID: 904064791
#
#!/bin/sh

echo "Executing smoke-test of application..."
{ echo "STOP"; echo "SCALE=F"; echo "SCALE=C"; echo "PERIOD=2"; echo "START"; echo "LOG hi"; sleep 8;echo "blah"; echo "OFF"; } | ./lab4b --log=LOGFILE

if [ $? -eq 0 ]
then
    echo "Success: got exit code 0"
else
    echo "Error: did not get exit code 0"
fi

for cmd in STOP SCALE=F SCALE=C PERIOD=2 START "LOG hi" blah SHUTDOWN OFF
do
    grep "$cmd" LOGFILE > /dev/null
    if [ $? -eq 0 ]
    then
	echo "Success: found $cmd in log file"
    else
	echo "Error: did not find $cmd in log file"
    fi
done
