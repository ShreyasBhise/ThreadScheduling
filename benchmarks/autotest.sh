#!/bin/bash
output_file=benchmark
echo "start"
for i in {1..5}
do
  ./parallel_cal 5 >> $output_file 
done
echo "done 1"
for i in {1..5}
do
  ./parallel_cal 10 >> $output_file 
done
echo "done 2"
for i in {1..5}
do
  ./parallel_cal 50 >> $output_file 
done
echo "done 3"
for i in {1..5}
do
  ./parallel_cal 100 >> $output_file 
done
echo "done 4"
for i in {1..5}
do
  ./parallel_cal 200 >> $output_file 
done

python analyzetest.py