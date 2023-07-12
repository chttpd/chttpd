# chttpd
Nonblocking IO HTTP server using Carrow.

#### Dependencies
- [Carrow](https://github.com/pylover/carrow)
- [Clog](https://github.com/pylover/clog)
- Microhttpd
    - `sudo apt install libmicrohttpd-dev`

## Build
```bash
mkdir build
cd build
cmake ..
make
```

### Run
```bash
./chttpd           // Listening on port 8080...
```
