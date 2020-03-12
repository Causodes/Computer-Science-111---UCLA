#!/bin/bash

./lab4b --log=temp_log.txt --period=4 --scale="C" <<-EOF
PERIOD=1
SCALE=F
START
STOP
OFF
EOF

if [ $? -ne 0 ]
then
    echo "Error, failed smoke test."
else
    echo "Smoke test successfully passed."
fi
rm -f temp_log.txt