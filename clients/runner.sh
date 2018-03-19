#!/bin/bash

pkill a.out

if [ "$#" -ne 6 ]; then
    echo "./a.out <threads> <hostname> <port> <rate> <#requests> <messageSize>"
    exit 0
fi

for i in `seq 1 $1`;
do
	./a.out $2 $3 $4 $5 $6 > data/$6/$6\_log_$i.txt &
done

wait
#aggregation
cat data/$6/$6\_log*.txt | (read; cat) > data/$6/$6\_agg_log.txt
