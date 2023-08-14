# chttpd
Nonblocking IO HTTP server using Carrow.


#### Dependencies

- [Carrow](https://github.com/pylover/carrow)
- [Clog](https://github.com/pylover/clog)
- [mrb](https://github.com/pylover/mrb)


## Build
```bash
mkdir build
cd build
cmake ..
make
```


### Run
```bash
./chttpd
```


## Contribution

### Lint

```bash
pip install prettyc
cd path/to/chttpd/build
make lint
```
