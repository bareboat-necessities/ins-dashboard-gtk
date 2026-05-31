# INS GTK4 NMEA Display

GTK4/C++20 marine INS display for NMEA 0183 streams.

This refactored version splits the original monolithic `main.cpp` into logical modules:

```text
src/
  main.cpp                  CLI parsing + application startup only
  app/                      GTK application wiring, timer, keyboard handling
  input/                    TCP/serial/demo input readers and source URI parsing
  model/                    Shared INS data model
  nmea/                     NMEA checksum and sentence parser
  render/                   Cairo drawing helpers, icons, and screen drawing
  sim/                      Demo/synthetic INS data generator
  util/                     String and math helpers
```

## Build

Linux:

```bash
sudo apt install build-essential cmake pkg-config libgtk-4-dev
cmake -S . -B build
cmake --build build -j
```

Windows (MSYS2 MINGW64 shell):

```bash
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja mingw-w64-x86_64-pkgconf mingw-w64-x86_64-gtk4
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

CI also produces a portable `windows-x64` zip on GitHub Actions. Extract it and launch with `run.bat` so the bundled GTK runtime paths are configured before starting `ins_gtk4_nmea.exe`.

## Run

Demo mode:

```bash
./build/ins_gtk4_nmea --source demo://
```

TCP NMEA:

```bash
./build/ins_gtk4_nmea --source tcp-nmea0183://127.0.0.1:10110
```

Serial NMEA:

```bash
./build/ins_gtk4_nmea --source serial-nmea0183:///dev/ttyUSB0?baud=115200
```

On Windows, use a COM device path from the MSYS2 shell or `run.bat`:

```bash
./build/ins_gtk4_nmea.exe --source serial-nmea0183://COM3?baud=115200
```

## Keyboard controls

- `1` Helm / attitude
- `2` Heave
- `3` Wave direction
- `4` Rate of turn
- `5` INS status
- `Left` / `Right` / `Space` cycles screens
- `q` / `Esc` exits

## Supported sentences

Standard-ish:

- `HDT` / `HDG`: heading
- `ROT`: rate of turn, degrees/minute
- `XDR`: roll, pitch, yaw/heading, heave, ROT, wave, Hs, Tp by transducer name
- `PRDID`: pitch, roll, heading

Proprietary `$PINS` examples:

```text
$PINS,ATT,-6.2,2.1,127.0
$PINS,HEAVE,0.18,0.11,0.72,4.8
$PINS,WAVE,45.0,78.0
$PINS,STATUS,ATT=GOOD,HEAVE=GOOD,WAVE=FAIR,MAG=LOCKED,GYRO=LEARNING,ACC=STABLE,TEMP=36.8,SAMPLE=240,MAGRATE=100,SYS=INS GOOD
```

Compact numeric form:

```text
$PINS,roll,pitch,yaw,heave,heaveVel,rotDegMin,waveRelDeg,waveConf,Hs,Tp,status
```
