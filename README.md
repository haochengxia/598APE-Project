# 598APE-Project

The codebase for *"Optimized Feature Extraction for Learned Cache
Eviction"* is modified from [libCacheSim](https://github.com/1a1a11a/libCacheSim).

## How to run

- Install dependency

```
bash scripts/install_dependency.sh
```

- Download datasets

```
bash ae/download.sh
```

- Run experiments

```
bash ae/naive.sh  # walk through the link list
bash ae/optimized.sh  # use logic timestamp and binary search
```

- valgrind

```
bash ae/check.sh  # valgrind-clean
```

## More usages

The executable file also support more extra options for simulation. Please refer to the documentation of [libCacheSim](https://github.com/1a1a11a/libCacheSim).