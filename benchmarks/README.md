install google benchmark: https://github.com/google/benchmark

by default google benchmark is built in Debug mode so you might want to specify Release mode when building/installing

build rl benchmarks:

```
make -j rl_benchmarks
```

run rl benchmarks:

```
./benchmarks/rl_benchmarks
```