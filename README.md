# modbus_gateway
Modbus Gateway is an open source convertor modbus message.  
Features:
- Multi master
- TCP transport with TCP modbus message
- RTU transport with RTU and ASCII modbus message
- RTU support RS485 configuration
- Crosscompile ARM, ARM64

## Get and build
Get source
```sh
git clone https://github.com/petukhovtd/exchange.git
cd exchange
```
Build and run docker
```sh
docker build . -t mg-image

docker run --rm \
  -it \
  --name mg-build \
  -v "$(pwd)":/app \
  mg-image:latest
  
cd app
```
Configure and build
```sh
mkdir "build"
cd build
cmake .. 
# or select toolchain arm
cmake .. -DCMAKE_TOOLCHAIN_FILE=./toolchain/arm.cmake
# or select toolchain arm64
cmake .. -DCMAKE_TOOLCHAIN_FILE=./toolchain/arm64.cmake
# build
make -j$(nproc)
```
## Socat
You can use socat for create virtual serial port
```sh
socat -d -d pty,raw,echo=0,link=port1 pty,raw,echo=0,link=port2
```