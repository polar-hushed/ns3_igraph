#!/bin/bash

testArray=("1" "2" "3" "4" "5")
currentValue=""

for i in ${testArray[@]};  do
        currentValue=$i

        echo "Processing " ${currentValue}


        echo `date`

done
