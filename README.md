# V274X DAQ

The code for reading the data from the CAEN V2740/V2745 and converting it to a ROOT file.

## Requirements

- C++17 or later
- CMake Version 3.10 or later
- ROOT Version 6.26.08 or later
- CAEN V2740/V2745 Library
  - CAEN FELib & dig2 library: <https://www.caen.it/products/caen-felib-library/>

## Usage

- Compile the code

```bash
mkdir build
cd build
cmake ..
make
```

## Programs

- `daq`: CUI-based DAQ program for the CAEN V2740/V2745.
- `v274xm_daq`: GUI-based DAQ program for the CAEN V2740/V2745.
- `raw2root`: Read the raw data from the CAEN V2740/V2745 and convert it to a ROOT file.

### daq

- Run the code

```bash
./daq [-p] [-c] [-t <time>] [-o <file name>] <V2740_IP>
```

- Option Descriptions
  - `-p`: Option to load a parameter file.
  - `-c`: Option to read encoded events.
  - `-t <time>`: Specify the data acquisition time in seconds.
  - `-o <file name>`: Specify the output file name.
  - `<V2740_IP>`: Specify the IP address of the V2740 module.

### v274xm_daq

- Run the code

```bash
./v274xm_daq -c <IP_ADDRESS1>,<IP_ADDRESS2>,<IP_ADDRESS3>
```

- Option Descriptions
  - `-c <IP_ADDRESS1>,<IP_ADDRESS2>,<IP_ADDRESS3>`: Specify the IP addresses of the DAQ devices.

### raw2root

- Run the code

```bash
./raw2root <input file> [-c] [-o <output file>]
```

- Option Descriptions
  - `<input file>`: Specify the input file name to read data from.
  - `-c`: Option to read encoded events.
  - `-o <output file>`: Specify the output file name.
