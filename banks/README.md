# BANKS
These are mainly a bunch of banks that try and test some explicit things. The idea is to run them on hardware (Dervish) and scope the signals.
Then a .wav can be generated and compared.

For way more in-depth tests, see here: https://github.com/ndf-zz/fv1testing

## Helpers
Generally everything can be done within this directory (`banks`)

- `derbanka` brings a bunch of utility scripts and makefile templates to create, build and manage banks.
- E.g. to create a new bank `./derbanka/newbank <bank name>`

## Build/upload
Building and uploading a bank can be done from the `banks` directory:
```
make -C <bank> upload
```
That should be it. This does assume that the `fv1-eeprom-host` executable is in the PATH, as well as other DERVISH environment variables
- `DERVISH_TTY` Should point to the programmer USB

(Setting these with `direnv` in the project folder is useful.)

