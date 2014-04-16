#!/bin/bash

pmodel=("ns3::JakesPropagationLossModel"  "ns3::Kun2600MhzPropagationLossModel"  "ns3::OkumuraHataPropagationLossModel"  "ns3::ItuR1411LosPropagationLossModel"  "ns3::ItuR1411NlosOverRooftopPropagationLossModel")
for i in "${pmodel[@]}"; do
	echo $i
	echo "DONE"
	$PBS_O_WORKDIR/waf --run "scratch/wifi-ap --pmodel=$i --verbose=false"
done
