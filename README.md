# INS GTK4 NMEA Display

A small GTK4/Cairo C++20 application that displays marine INS screens:

1. Primary helm / attitude
2. Heave detail
3. Wave direction relative to heading
4. Rate of turn
5. INS status

It reads NMEA 0183-style lines from either TCP or serial:

```bash
./ins_gtk4_nmea --source tcp-nmea0183://127.0.0.1:10110
./ins_gtk4_nmea --source serial-nmea0183:///dev/ttyUSB0?baud=115200
```

If no source is supplied, it runs in demo mode.

## Build on Debian/Ubuntu/Raspberry Pi OS

```bash
sudo apt install build-essential cmake pkg-config libgtk-4-dev
cmake -S . -B build
cmake --build build -j
./build/ins_gtk4_nmea --source demo://
```

## Keyboard controls

- `1` Primary helm
- `2` Heave
- `3` Wave direction
- `4` Rate of turn
- `5` INS status
- Left/right arrows cycle screens
- `q` or `Esc` quit

## Supported input sentences

The app accepts standard-ish and proprietary sentences. Checksum is optional; if present, it is verified.

### Proprietary compact INS sentence

```text
$PINS,roll,pitch,yaw,heave,heaveVel,rotDegMin,waveRelDeg,waveConf,Hs,Tp,status*hh
```

Example without checksum:

```text
$PINS,-6.2,2.1,127,0.18,0.11,8.4,45,78,0.72,4.8,A
```

### Proprietary split sentences

```text
$PINS,ATT,roll,pitch,yaw
$PINS,HEAVE,heave,heaveVel,Hs,Tp
$PINS,WAVE,waveRelDeg,confidencePercent
$PINS,STATUS,ATT=GOOD,HEAVE=GOOD,WAVEDIR=FAIR,MAG=LOCKED,GYRO=LEARNING,ACC=STABLE,TEMP=36.8,SAMPLE=240,MAGRATE=100
```

### Common sentences

```text
$--HDT,heading,T
$--HDG,heading,...
$--ROT,rateOfTurnDegPerMin,A
$PRDID,pitch,roll,heading
$--XDR,A,value,D,ROLL,A,value,D,PITCH,A,value,D,YAW,...
```

## Notes

- Internally the UI displays heave positive upward.
- Rate of turn is displayed in degrees per minute.
- Wave direction is displayed relative to vessel heading: `0° = bow`, `90° = starboard beam`, `180° = stern`, `270° = port beam`.
- The serial parser supports common POSIX `termios` baud constants. If your URI says `baud=11520`, that is probably a typo for `115200`; `11520` is usually not a standard termios baud constant.
