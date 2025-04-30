# apply
cp s3fifo/S3FIFO_link.c libCacheSim/cache/eviction/S3FIFO.c
cp s3fifo/FIFO_link.c libCacheSim/cache/eviction/FIFO.c

bash scripts/install_libcachesim.sh

# run
./_build/bin/cachesim ./data/cluster26.oracleGeneral.sample10.zst oracleGeneral s3fifo 0.001 --ignore-obj=1
./_build/bin/cachesim ./data/cluster26.oracleGeneral.sample10.zst oracleGeneral s3fifo 0.01 --ignore-obj=1
./_build/bin/cachesim ./data/cluster26.oracleGeneral.sample10.zst oracleGeneral s3fifo 0.1 --ignore-obj=1
./_build/bin/cachesim ./data/1082.oracleGeneral.zst oracleGeneral s3fifo 0.001 --ignore-obj=1
./_build/bin/cachesim ./data/1082.oracleGeneral.zst oracleGeneral s3fifo 0.01 --ignore-obj=1
./_build/bin/cachesim ./data/1082.oracleGeneral.zst oracleGeneral s3fifo 0.1 --ignore-obj=1
