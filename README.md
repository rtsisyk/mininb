Mini NoSQL Benchmark
====================

**mininb** - is a simple and easy to use benchmark for embedded NoSQL database libraries.

Core Features
-------------

 + GET and PUT operations support
 + Using external source of random keys
 + Histogram output
 + Percentilies calculation
 
Supported databases
-------------------

 + LevelDB by Google - [README.LevelDB](README.LevelDB)
 + TokuKV by Tokutek - [README.TokuKV](README.TokuKV)
 + KyotoCabinet by FAL Labs - [README.KyotoCabinet](README.KyotoCabinet)
 + BerkeleyDB by Oracle [README.BerkeleyDB](README.BerkeleyDB)
 + nessDB - [README.NessDB](README.NessDB)

Plugin API is pretty simple and new engines can be added very quickly.

Installation
------------

Prerequisites:

 + Linux host (OS X and FreeBSD is not tested but might work)
 + CMake 2.6+
 + GCC 4.7+ or clang 3.1+

```
roman@work:~$ git clone --recursive git://github.com/rtsisyk/mininb.git
roman@work:~$ cd mininb
roman@work:~/mininb$ cmake . -DCMAKE_BUILD_TYPE=Release .
roman@work:~/mininb$ make
```

Please use `--recursive` flag to clone if you want to retrive all bundled databases in `third_party` directory.

Usage
-----

Generate a file with random keys using your most favorite random number generator:

```
roman@work:~/mininb$ dd if=/dev/urandom of=keys.bin bs=1M count=100
100+0 records in
100+0 records out
104857600 bytes (105 MB) copied, 8.48439 s, 12.4 MB/s
```

Create a temporary directory for database files:

```
roman@work:~/mininb$ mkdir nb
```

Save keys into database (PUT benchmark):
```
roman@work:~/mininb$ ./mininb --path ./nb --action=put --driver=leveldb -k 16 -v 100 -c 100000
```

Drop disk caches (Linux):

```
roman@work:/# sync
roman@work:/# echo 3 > /proc/sys/vm/drop_caches
```

Shuffle keys:

```
roman@work:~/mininb$ ./mininb --path ./nb --action=shuffle -k 16 -c 100000
```

Read keys from the database (GET benchmark)

```
roman@work:~/mininb$ ./mininb --path ./nb --action=get --driver=leveldb -k 16 -v 100 -c 100000
```

See `extra/run.sh` script for additional examples.

Output
------

mininb outputs histograms and percentiles.

Histogram is a representation of the distribution of data.

```
Action: get
Driver: leveldb
Key Len: 16
Val Len: 100
Count: 500000

Histogram:
[  t min,   t max)        ops count           %
--------------------------------------------------
[      1,       2)             2554        0.51 
[      2,       3)           151209       30.24 
[      3,       4)            59414       11.88 
[      4,       5)           153407       30.68 
[      5,       6)            60251       12.05 
[      6,       7)             9442        1.89 
[      7,       8)            15489        3.10 
[      8,       9)            23270        4.65 
[      9,      10)            11691        2.34 
[     10,      12)             7566        1.51 
[     12,      14)             2538        0.51 
[     14,      16)             1592        0.32 
[     16,      18)              363        0.07 
[     18,      20)              348        0.07 
[     20,      25)              253        0.05 
[     25,      30)              112        0.02 
[     30,      35)               48        0.01 
[     35,      40)               38        0.01 
[     40,      45)               21        0.00 
[     45,      50)               14        0.00 
[     50,      60)               25        0.01 
[     60,      70)               17        0.00 
[     70,      80)               13        0.00 
[     80,      90)               22        0.00 
[     90,     100)               12        0.00 
[    100,     120)                8        0.00 
[    120,     140)                2        0.00 
[    140,     160)                4        0.00 
[    160,     180)                9        0.00 
[    180,     200)                1        0.00 

[    200,     250)               33        0.01
# 33 request from 500000 take 200-250 usec to execute 

[    300,     350)                1        0.00 
[    450,     500)                1        0.00 
[    500,     600)                2        0.00 
[    600,     700)                1        0.00 
[    700,     800)                1        0.00 
[    800,     900)                2        0.00 
[   1000,    1200)                2        0.00 
[   1200,    1400)                1        0.00 
[   1400,    1600)                1        0.00 
[   1800,    2000)                1        0.00 
[   2000,    2500)                1        0.00 
[   2500,    3000)                3        0.00 
[   3000,    3500)                1        0.00 
[   3500,    4000)                5        0.00 
[   4000,    4500)                7        0.00 
[   4500,    5000)                2        0.00 
[   5000,    6000)                9        0.00 
[   6000,    7000)                5        0.00 
[   7000,    8000)                8        0.00 
[   8000,    9000)                9        0.00 
[   9000,   10000)                8        0.00 
[  10000,   12000)               15        0.00 
[  12000,   14000)               13        0.00 
[  14000,   16000)               12        0.00 
[  16000,   18000)                9        0.00 
[  18000,   20000)               14        0.00 
[  20000,   25000)               22        0.00 
[  25000,   30000)               15        0.00 
[  30000,   35000)               15        0.00 
[  35000,   40000)               11        0.00 
[  40000,   45000)               12        0.00 
[  45000,   50000)                7        0.00 
[  50000,   60000)                5        0.00 
[  60000,   70000)                5        0.00 
[  70000,   80000)                3        0.00 
[  80000,   90000)                2        0.00 
[  90000,  100000)                1        0.00 
[ 100000,  120000)                1        0.00 
[ 120000,  140000)                1        0.00 
--------------------------------------------------
Total:     7561191           500000        100%
Min latency       : 1.586974 * 1e-6 sec/op
Avg latency       : 15.122383 * 1e-6 sec/op
Max latency       : 124253.062066 * 1e-6 sec/op
5.0000%  latency  : 2.148444 * 1e-6 sec/op
50.0000%  latency  : 4.240035 * 1e-6 sec/op
95.0000%  latency  : 8.998453 * 1e-6 sec/op
96.0000%  latency  : 9.424600 * 1e-6 sec/op
97.0000%  latency  : 9.852280 * 1e-6 sec/op
98.0000%  latency  : 10.865186 * 1e-6 sec/op
99.0000%  latency  : 12.557132 * 1e-6 sec/op
99.5000%  latency  : 14.840452 * 1e-6 sec/op
99.9000%  latency  : 30.104167 * 1e-6 sec/op
99.9500%  latency  : 225.757576 * 1e-6 sec/op
99.9900%  latency  : 34333.333333 * 1e-6 sec/op <!- DISK SEEKS here
Avg throughput    :   66127 ops/sec
```
