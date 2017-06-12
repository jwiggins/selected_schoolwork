#!/bin/sh

if [ $# -lt 2 ]
then
	echo "usage: $0 <filename> <numprocs>"
	exit -1
fi

machines=`cat machines`
limit=$2
base=$1
counter=1

blocks=`printf "/tmp/%s_000.blks" $base`
info=`printf "/tmp/%s.info" $base`
ids=`printf "/tmp/%s.ids" $base`
echo "rm -f $blocks $info $ids"
rm -f $blocks $info $ids

for mach in $machines
do
	blocks=`printf "/tmp/%s_%03d.blks" $base $counter`
	info=`printf "/tmp/%s.info" $base`
	ids=`printf "/tmp/%s.ids" $base`
	echo "$mach: rm -f $blocks $info $ids"
	ssh $mach rm -f $blocks $info $ids
	
	let "counter = counter + 1" 
	if [ $counter -eq $limit ]
	then
		exit 0;
	fi
done
