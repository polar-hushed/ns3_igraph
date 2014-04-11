#!/bin/bash
#script to parse and find queueing delays

awk '/TxQueue/ {print $1 $2 $3 " " $19}' $1 | sed '1,$s/\/NodeList\//   \t/g'| sed '1,$s/\/DeviceList\//\t/g'| sed '1,$s/\/\$ns3\:\:PointToPointNetDevice\/TxQueue\//\t/g'

