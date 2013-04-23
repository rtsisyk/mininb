#!/bin/bash

MININB="../"

rm -rf nb-*-*/
make clean
rm -rf CMakeCache.txt CMakeFiles/ mininb
cmake ${MININB} -DCMAKE_BUILD_TYPE=Release
make

gen_keys() {
	echo "Generating keys..."
	dd if=/dev/urandom of=keys.bin bs=1M count=100
	echo "Generating keys..." " " "OK"
}

drop_caches() {
	if [ "${USER}" != "root" ]; then
		return
	fi

	echo "Dropping caches..."
	sync
	sync
	echo 3 > /proc/sys/vm/drop_caches
	sleep 1
	echo "Dropping caches..." " " "OK"
}

if [ "${USER}" != "root" ]; then
	echo "drop_caches is not supported when running from a non-root user."
fi

if [ ! -f keys.bin ]; then
	gen_keys
fi

for i in `seq 1 3`; do

for size in 500000 100000; do
	rm -f nb
	mkdir "nb-${i}-${size}"
	ln -s "nb-${i}-${size}" nb
	for driver in tokukv leveldb nessdb berkeleydb; do
		echo "Benchmark $size $driver ${logpath}"
		logname="${size}-${driver}"
		
		# Put
		echo "PUT"
		./mininb ${driver} ${size} 0 1> ./nb/${logname}-put.result 2> ./nb/${logname}-put.log
		drop_caches
		
		# Shuffle file
		echo "SHUFFLE"
		./mininb ${driver} ${size} 2 1> ./nb/${logname}-shuffle.log
		drop_caches
		
		# Get
		echo "GET"
		./mininb ${driver} ${size} 1 1> ./nb/${logname}-get.result 2> ./nb/${logname}-get.log
	done
done

done
