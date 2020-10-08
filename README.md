# libsequoia
an API for generative musical sequencing

## Build Instructions (C library)

First, install dependencies:
```
sudo apt-get install libjack-jackd2-dev libjson-c-dev
```

Then, compile/install with:

```
make
make install
```

## Build Instructions (Python bindings)

First, build and install the C library.

You'll also need:
```
sudo apt-get install python-dev
```

Then:
```
cd python
make
make install
```
