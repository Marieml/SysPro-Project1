#!/bin/bash

#Name: Maria Miliou
#A.M: 1115201300101

#	For each file and in directory out with suffix .out
#searches for emtries .<TLB>[space] using grep, sends output to cut
#through pipe. Cut takes the 2nd column(num_of_ap) with delim. [space]
#Output sends to tr to translate new line to +. Output will end with +
#so adds extra 0. e.g X=(1+6+3+7+8+0)

if [ "$#" -ne 0 ]
	then

	#List of files .out in directory out
	file_list=`ls ./out/*.out`

	for tlb  #Reads from arguments
	do
		echo -n "$tlb "
		j=0
		for file in ${file_list}
		do
			OLDIFS="$IFS"
			IFS='\n'
			X=( $(grep -a ."$tlb"' ' < "$file" | cut -f2 -d' '| tr "\n" "+" ;echo "0"))

			let j=j+${X[0]}
			IFS="$OLDIFS"
		done
		echo "$j"
	done
else
	echo -n "No arguments to search"
fi
