# INS GTK4 NMEA Display

GTK4/C++20 marine INS display for NMEA 0183 streams.

Works best with INS sensors described here:

https://github.com/bareboat-necessities/ocean-imu

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
  - `A,<degrees>,D,WAVAXIS` or `A,<degrees>,D,WAVDIR`: relative wave axis / wave direction
  - `A,<degrees>,D,PTCH`: pitch
  - `A,<degrees>,D,ROLL`: roll
  - `D,<metres>,M,DRT1`: real-time heave displacement in metres; the Heave screen derives vertical speed from successive heave samples when the stream does not provide vertical speed
  - `V,<metres_per_second>,M,VHSPD`: vertical heave speed in metres per second, up-positive
  - `C,<degrees_celsius>,C,IMUT`: IMU chip temperature
- `IITXT,01,01,00,WAVSGN=<sign> POL=<polarity>`: optional wave-direction
  sign metadata applied to the latest and subsequent `WAVAXIS`/`WAVDIR` magnitudes
- `IITXT,01,01,00,WAVCONF=<percent>`: wave-direction estimate confidence displayed on the Wave screen
- `IITXT,01,01,00,ATT=<Y/N> HEV=<Y/N> MAG=<Y/N> GB=<Y/N> AB=<Y/N> IHz=<hz> MHz=<hz>`:
  INS status flags for attitude, heave, magnetic lock, gyro-bias learning, accel-bias estimating, IMU sample rate, and magnetometer sample rate
- `IIROT,<degrees_per_minute>,A`: valid rate of turn
- `IIHDM,<degrees>,M`: magnetic heading

Example stream:

```text
$IIXDR,A,1.8,D,PTCH*63
$IIXDR,A,0.9,D,ROLL*71
$IIXDR,D,0.0132,M,DRT1*2A
$IIXDR,V,-0.045,M,VHSPD*0E
$IIXDR,A,25.8,D,WAVAXIS*19
$IIXDR,A,25.8,D,WAVDIR*45
$IITXT,01,01,00,WAVSGN=1 POL=1*31
$IITXT,01,01,00,WAVCONF=82*2B
$IIXDR,C,31.5,C,IMUT*52
$IITXT,01,01,00,ATT=Y HEV=Y MAG=Y GB=N AB=Y IHz=199.8 MHz=10.0*40
$IIROT,3.8,V*3A
$IIXDR,A,1.8,D,PTCH*63
$IIXDR,A,0.9,D,ROLL*71
$IIXDR,D,0.0120,M,DRT1*29
$IIXDR,A,25.2,D,WAVAXIS*13
$IIXDR,A,25.2,D,WAVDIR*4F
$IITXT,01,01,00,WAVSGN=1 POL=1*31
$IIROT,4.1,V*34
```

Checksums are validated when present. Lines without checksums are still accepted for
lab/demo streams.
