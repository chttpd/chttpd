# chttpd
Nonblocking IO HTTP server using Carrow.

#### Dependencies

- [Carrow](https://github.com/pylover/carrow)
- [Clog](https://github.com/pylover/clog)
- [Microhttpd](http://www.gnu.org/software/libmicrohttpd/)


##### Ubuntu

```bash
sudo apt install libmicrohttpd-dev
```


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
cd path/to/carrow/build
make lint
```
