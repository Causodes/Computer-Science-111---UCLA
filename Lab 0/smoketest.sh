#!/bin/sh

echo "test phrase" > input.txt;
./lab0 --input=input.txt --output=output.txt;
if [[ $? -ne 0 ]]
then
    echo "Error: Unable to read or write from file."
    exit 1;
fi
diff -q input.txt output.txt > /dev/null;
if [[ $status ]]
then
    echo "Error: Input and output do not match."
    exit 1;
fi
rm -f input.txt output.txt;
./lab0 --arg1 &> /dev/null;
if [[ $? -ne 1 ]]
then
    echo "Error: Invalid argument not caught."
    exit 1;
fi
echo "test phrase" > input.txt;
chmod u-r input.txt;
echo | ./lab0 -i input.txt &> /dev/null;
if [[ $? -ne 2 ]]
then
    echo "Error: File is not readable but error is not caught."
    exit 1;
fi
rm -f input.txt;
touch output.txt;
chmod -w output.txt;
echo "test phrase" | ./lab0 --output=output.txt &> /dev/null;
if [[ $? -ne 3 ]]
then
    echo "Error: File is not writable but error is not caught."
    exit 1;
fi
rm -f output.txt;
echo | ./lab0 -s -c &> /dev/null;
if [[ $? -ne 4 ]]
then
    echo "Error: Segmentation fault triggered but was not caught. Error Code: $?"
    exit 1;
fi
echo "Smoke Tests Passed.";
