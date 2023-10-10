# Example project: hello_world

This example project shows the minimum required to create an application for the Aq+.  
This can be a good starting point for your own project.

Building can be done using: `make`  
This will build the application and put the resulting .caq file in the build directory.

You can easily run the application on the emulator using: `make run_emu`  
Running on real hardware can be done using `make run_aqp`  
Cleaning the build can be done using `make clean`  

If you copy the project folder to another location please update the `AQPLUS_SDK` variable in the Makefile to the location of the Aq+ SDK directory. Alternatively you can set an environment variable with the same name that points to the location.
