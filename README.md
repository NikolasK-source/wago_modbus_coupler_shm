# WAGO Modbus Coupler SHM

Modbus Server that connects to a WAGO Modbus TCP Coupler and synchronizes the registers with a local shared memory.


## Build
```
git submodule init
git submodule update
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=$(which clang++) -DCMAKE_BUILD_TYPE=Release -DCLANG_FORMAT=OFF -DCOMPILER_WARNINGS=OFF
cmake --build .
```

## Use
```
wago_modbus_coupler_shm [OPTION...] host [service]

      --force             Force the use of the shared memory even if it already exists. Do not use this option per 
                          default! It should only be used if the shared memory of an improperly terminated instance 
                          continues to exist as an orphan and is no longer used.
  -q, --quiet             Disable output
  -d, --debug             Enable modbus debug output
      --read-start-image  do not initialize output registers with zero, but read values from coupler
  -p, --prefix arg        name prefix for the shared memories (default: wago_)
      --version           print application version
      --license           show licences
  -h, --help              print usage
      host                host or address of the WAGO Modbus TCP Coupler
      service             service or port of the WAGO Modbus TCP Coupler (default: 502)
```

## Libraries
This application uses the following libraries:
- cxxopts by jarro2783 (https://github.com/jarro2783/cxxopts)
- libmodbus by Stéphane Raimbault (https://github.com/stephane/libmodbus)
- cxxshm (https://github.com/NikolasK-source/cxxshm)
- cxxsignal (https://github.com/NikolasK-source/cxxsignal)

