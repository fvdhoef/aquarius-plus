# Getting started on MacOS

This guide will explain how to get a development environment set up on your Mac for Aquarius+ development.
These instructions assume you're running MacOS Ventura, but probably work on old versions.

## Step 1: Installing SDCC

In this step we're going to install SDCC.

SDCC is a C compiler, assembler and linker that supports the Z80 processor that is on the Aquarius+.

The SDK was tested with SDCC version 4.2.0, but the latest version as of this writing was 4.3.0, so that's what we'll use here.

The easy way to install SDCC is with Homebrew. Head over to [brew.sh](https://brew.sh) and install the latest. If you already have brew installed, run `brew install` first.

To install SDCC, do the following:

```bash
brew install sdcc
```

By default, Homebrew will install the compiler to `/usr/local/bin`.

## Step 2: There is no Step 2

You don't need to install MSYS2. It Just Works.

(Actually, I lied. If you never installed XCode, commands like `make` don't work, and you'll need to install XCode Command Line Tools. Do a `xcode-select --install` and then go take a long lunch, because it will be a while. It Almost Just Works...)

## Step 3: Download the Aquarius+ SDK

Download the Aquarius+ SDK, either by cloning the repository via Git (if you're familiar with this) or by downloading a ZIP with the contents of the repository:

https://github.com/fvdhoef/aquarius-plus/archive/refs/heads/master.zip

Unpack this ZIP file and either copy the SDK folder within or the whole contents of the ZIP to a location of your liking (preferably to a location without any spaces in the path).

Now set the following environment variables:

| Name            | Description                                                                                                                                                                                               |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `AQPLUS_SDK`      | Path to the Aquarius+ SDK folder, for example `/Users/jkonrath/Documents/GitHub/aquarius-plus/SDK`                                                                                                                                            |
| `AQPLUS_HOST`     | (defaults to `aqplus`)<br>Hostname (or IP address) of your real Aquarius+. Used for easily remotely running code from your PC on your Aquarius+.                                                               |
| `AQPLUS_EMU`      | (defaults to `open -a AquariusPlusEmu --args`)<br>Full path to the Aquarius+ emulator executable. (The default doesn't work for me; I had to set this to `"open -a /Users/jkonrath/Documents/GitHub/aquarius-plus/System/emulator/build/AquariusPlusEmu.app --args"`)                                                                                |
| `AQPLUS_EMU_DISK` | (defaults to `$(HOME)/Documents/AquariusPlusDisk/`)<br>Full path to a directory that the emulator uses as disk. The Makefile will install your application here when you run it from the Makefile. |

In bash, you can set these environment variables like this:
```sh
export AQPLUS_SDK=/Users/jkonrath/Documents/GitHub/aquarius-plus/SDK
export AQPLUS_EMU_DISK=/Users/jkonrath/Documents/AquariusPlusDisk
export AQPLUS_EMU="open -a /Users/jkonrath/Documents/GitHub/aquarius-plus/System/emulator/build/AquariusPlusEmu.app --args"
```

You can then add the lines to your `~/.bashrc`.


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
- Open an example project folder from the SDK folder using the **File > Open Folder...** menu item, for example **SDK/examples/hello_world**
- You can click the **main.c** file to see the source code for this example.
- Press CTRL+SHIFT+B to run a build task. A list will open where you can select several different options. Choose **Run project in emulator** from this menu. If everything is set up correctly, this will build your project. Copy the files into the emulator's sdcard directory, start the emulator and run your program.

Each of the example projects in the SDK contain a `.vscode` directory. This directory contains two important files:

| Name                  | Description                                                                                                                    |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| `tasks.json`           | This file contains the tasks that are available in the CTRL+SHIFT+B menu.                                                      |
| `c_cpp_properties.json` | This file contains settings used by Visual Studio Code for its Intellisense feature, providing code completion and navigation. |

When you copy an example project folder to create your own program make sure to include these files to get a working setup.

[Click here for more information on the project makefiles.](project_makefiles.md)
