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
./build/ins_gtk4_nmea.exe --source serial-nmea0183://COM9?baud=115200
```

The source URI may also be passed as the first positional argument, which is useful
when launching from shells or shortcuts that do not use `--source`:

```bash
./build/ins_gtk4_nmea.exe serial-nmea0183://COM9?baud=115200
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

The dashboard is configured for the incoming `II` talker stream that reports attitude,
magnetic heading, rate of turn, and wave axis using standard NMEA 0183 sentence
types:

- `IIXDR` transducer measurements:
  - `A,<degrees>,D,WAVAXIS`: relative wave axis / wave direction
  - `A,<degrees>,D,PTCH`: pitch
  - `A,<degrees>,D,ROLL`: roll
  - `D,<metres>,M,DRT1`: accepted distance transducer sentence; currently not shown
    because the dashboard has no dedicated draft/distance field
- `IIROT,<degrees_per_minute>,A`: valid rate of turn
- `IIHDM,<degrees>,M`: magnetic heading

Example stream:

```text
$IIXDR,A,25.5,D,WAVAXIS*14
$IIROT,-1.8,A*02
$IIHDM,197.4,M*29
$IIXDR,A,-16.9,D,PTCH*79
$IIXDR,A,15.4,D,ROLL*48
$IIXDR,D,0.0000,M,DRT1*2A
$IIXDR,A,25.5,D,WAVAXIS*14
$IIROT,-1.0,A*0A
$IIHDM,197.4,M*29
$IIXDR,A,-16.9,D,PTCH*79
$IIXDR,A,15.4,D,ROLL*48
$IIXDR,D,0.0000,M,DRT1*2A
$IIXDR,A,25.5,D,WAVAXIS*14
$IIROT,-1.6,A*0C
$IIHDM,197.5,M*28
```

Checksums are validated when present. Lines without checksums are still accepted for
lab/demo streams.
