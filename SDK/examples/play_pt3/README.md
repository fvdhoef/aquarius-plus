# Example project: play_pt3

This example project shows how to play a PT3 music file. The PT3 music file is included in the application binary. For this the .pt3 file was first converted to a C-array using `xxd -i ingarden.pt3 > ingarden.c`. Then the file was manually edited to replace
```c
unsigned char ingarden_pt3[] = {
```
with
```c
const unsigned char ingarden_pt3[] = {
```

Building can be done using: `make`  
This will build the application and put the resulting .caq file in the build directory.

You can easily run the application on the emulator using: `make run_emu`  
Running on real hardware can be done using `make run_aqp`  
Cleaning the build can be done using `make clean`  

If you copy the project folder to another location please update the `AQPLUS_SDK` variable in the Makefile to the location of the Aq+ SDK directory. Alternatively you can set an environment variable with the same name that points to the location.
