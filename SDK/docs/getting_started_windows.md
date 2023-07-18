# Getting started on Windows

This guide will explain how to get a development environment set up on your PC for Aquarius+ development.
These instructions assume you're running a 64-bit version of Windows 11, but should also work on older (64-bit) versions of Windows.

## Step 1: Installing SDCC

In this step we're going to install SDCC.

SDCC is a C compiler, assembler and linker that supports the Z80 processor that is on the Aquarius+.

The SDK was tested with SDCC version 4.2.0, so that is the version that we're going install here. You can use a newer version if you want at your own risk.

You can download the needed installer from:
https://sourceforge.net/projects/sdcc/files/sdcc-win64/4.2.0/sdcc-4.2.0-x64-setup.exe

Once downloaded, locate the file in your downloads folder and double click it. This will start the Setup process.

By default the installer will install libraries for many platforms, so if you want to save some disk space, you can deselect the libraries for all but the Z80 platform.

By default, the installer will install the compiler to C:\Program Files\SDCC.

**Allow the installer to add the compiler to the PATH. This will allow to run the compiler later from any directory. (This step can take a while to finish, so be patient.)**

## Step 2: Installing MSYS2

In this step we're going to install an environment called MSYS2. This will provide the necessary tools to be able to build projects.

Download the latest version from https://www.msys2.org/, at the time of writing this guide that is https://github.com/msys2/msys2-installer/releases/download/2023-07-18/msys2-x86_64-20230718.exe.

Once downloaded, locate the file in your downloads folder and double click it. This will start the setup process. Use the default installation location. At the last step of the installer, it will run MSYS2. If you skipped this step, open the MSYS2 from your Start menu by running MSYS2 UCRT64. This will open a MSYS2 terminal window.

Type the following command (followed by ENTER) in the terminal:
```
pacman -S make curl gnu-netcat
```
At the confirmation prompt type Y and ENTER.
This will install the **make** command needed to run Makefiles. It will also install the programs **curl** and **netcat**, used to remotely start programs on your real Aquarius+. You can now close the terminal window.

Now we are going to add MSYS2 to the PATH:

- Press the keyboard combination **WIN+R**. This will open the Run dialog.
- Type **sysdm.cpl** and press enter. This will open the **System Properties** dialog.
- Click on the **Advanced** tab page and then on the **Environment Variables** button. This will open the **Environment Variables** dialog.
- In the **User variables** section, double-click on the **Path** variable. This will open the **Edit environment variable** dialog for the PATH variable.
- Click the **New** button and type in **C:\msys64\usr\bin**. Then click the **OK** button. This will close the dialog.
- Click the **OK** button in the **Environment Variables** dialog to close this dialog and apply the new PATH variable.
- Click the **OK** button in the **System Properties** dialog to close this dialog.

## Step 3: Download the Aquarius+ SDK

Download the Aquarius+ SDK, either by cloning the repository via Git (if you're familiar with this) or by downloading a ZIP with the contents of the repository:

https://github.com/fvdhoef/aquarius-plus/archive/refs/heads/master.zip

Unpack this ZIP file and either, copy the SDK folder within or the whole contents of the ZIP, to a location of your liking (preferably to a location without any spaces in the path).

Now set the following environment variables:

| Name            | Description                                                                                                                                                                                               |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| AQPLUS_SDK      | Path to the Aquarius+ SDK folder, for example C:\Aquarius+\SDK                                                                                                                                            |
| AQPLUS_HOST     | (defaults to **aqplus**)<br>Hostname (or IP address) of your real Aquarius+. Used for easily remotely running code from your PC on your Aquarius+.                                                               |
| AQPLUS_EMU      | (defaults to **%USERPROFILE%\\Documents\\Aquarius+\\Emulator\\aquarius-emu.exe**)<br>Full path to the Aquarius+ emulator executable.                                                                                |
| AQPLUS_EMU_DISK | (defaults to **%USERPROFILE%\\Documents\\Aquarius+\\Emulator\\sdcard\\**)<br>Full path to a directory that the emulator uses as disk. The Makefile will install your application here when you run it via the Makefile. |

You can set these environment variables as follows:

- Press the keyboard combination **WIN+R**. This will open the Run dialog.
- Type **sysdm.cpl** and press enter. This will open the **System Properties** dialog.
- Click on the **Advanced** tab page and then on the **Environment Variables** button. This will open the **Environment Variables** dialog.
- In the **User variables** section, click on the **New...** button. This will open the **New User Variable** dialog. Enter the name and value in this dialog and click the **OK** button. Repeat this step to add more environment variables if necessary.
- Click the **OK** button in the **Environment Variables** dialog to close this dialog and apply the new PATH variable.
- Click the **OK** button in the **System Properties** dialog to close this dialog.

## Step 4: Setting up Visual Studio Code

In this step we're going to install and set up Visual Studio Code, which is a nice code editor and environment to do your software development in.

Download and install the latest version from https://code.visualstudio.com/

Once installed, start Visual Studio Code.
On the left side of the windows there is a row of icons. Click the one resembling 4 little squares, which is the **Extensions** side bar. Here search for and install the following extensions:

- **C/C++**, which is a Microsoft package with support for the C (and C++) programming language. Click on it and from the page that appears click on the **Install** button. This can take a while.
- **C/C++ Themes**, which provides some improved syntax highlighting for C files.
- **Z80 Assembly**, which provides syntax highlighting for Z80 assembler files.

There are many more useful extensions, but the ones mentioned above will suffice for now.

## Step 5: Try out your setup

- Start Visual Studio Code.
- Open an example project folder from the SDK folder using the **File > Open Folder...** menu item, for example **SDK\examples\hello_world**
- You can click the **main.c** file to see the source code for this example.
- Press CTRL+SHIFT+B to run a build task. A list will open where you can select several different options. Choose **Run project in emulator** from this menu. If everything is set up correctly, this will build your project. Copy the files into the emulator's sdcard directory, start the emulator and run your program.

Each of the example projects in the SDK contain a **.vscode** directory. This directory contains 2 important files:

| Name                  | Description                                                                                                                    |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| tasks.json            | This file contains the tasks that are available in the CTRL+SHIFT+B menu.                                                      |
| c_cpp_properties.json | This file contains settings used by Visual Studio Code for its Intellisense feature, providing code completion and navigation. |

When you copy an example project folder to create your own program make sure to include these files to get a working setup.

[Click here for more information on the project makefiles.](project_makefiles.md)
