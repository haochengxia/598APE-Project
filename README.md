# 598APE-Project

3L-Cache: Low Overhead and Precise Learning-based Eviction Policy for Web Caches
This is the implementation repository of 3L-CACHE: Low Overhead and Precise Learning-based Eviction Policy for Web Caches. This artifact provides the source code of 3L-Cache and scripts to reproduce experiment results in our paper.

3L Cache is implemented in the libCacheSim library, and its experimental environment configuration is consistent with libCacheSim.
Supported Platforms
Software Requirements: Ubuntu 18.04, cmake 3.28.6
Trace Format
Request traces are expected to be in a space-separated format with 3 columns.

time should be a long long int
id should be a long long int
size should be uint32
time	id	size
1	1	120
2	2	64
3	2	64
Build and Install
We provide some scripts for installation.

cd scripts && bash install_dependency.sh && bash install_libcachesim.sh
Usage
After building and installing, cachesim should be in the _build/bin/ directory.

~/libCacheSim/_build/bin/cachesim trace_path trace_type eviction_algo cache_size [OPTION...]
Traces
Dataset	Year	Type	Original Link
Tencent Photo	2018	object	link
WikiCDN	2019	object	link
WikiCDN 2018	2018	object	link
WikiCDN 2019	2019	object	link
Tencent CBS	2020	block	link
Alibaba Block	2020	block	link
Twitter	2020	key-value	link
MetaKV	2022	key-value	link
MetaCDN	2023	object	link
Run a single cache simulation
~/libCacheSim/_build/bin/cachesim /path/to/tencentBlock_ns3964.csv csv 3lcache 1347453593  -t "time-col=1, obj-id-col=2, obj-size-col=3"
Run multiple cache simulations
~/libCacheSim/_build/bin/cachesim /path/to/tencentBlock_ns3964.csv csv 3lcache 1347453593,13474535  -t "time-col=1, obj-id-col=2, obj-size-col=3"
Examples
/path/to/cachesim /data/csv/tencentBlock_ns3964.csv csv 3lcache-omr 1347453593 -t "time-col=1, obj-id-col=2, obj-size-col=3"
# Output object miss ratio
tencentBlock_ns3964.csv TLCache-OMR cache size     1GiB,         13625211 req, miss ratio 0.3380, throughput 0.59 MQPS

~/libCacheSim/_build/bin/cachesim /path/to/tencentBlock_ns3964.csv csv 3lcache 1347453593,13474535  -t "time-col=1, obj-id-col=2, obj-size-col=3"
#Output object miss ratio and byte miss ratio
result/tencentBlock_ns3964.csv                      TLCache-BMR cache size        1GiB, 13625211 req, miss ratio 0.3421, byte miss ratio 0.1034
result/tencentBlock_ns3964.csv                      TLCache-BMR cache size        0GiB, 13625211 req, miss ratio 0.5300, byte miss ratio 0.6377