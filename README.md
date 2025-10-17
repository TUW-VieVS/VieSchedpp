# VieSched++

VLBI scheduling software

Written by Matthias Schartner

Contact: mschartner@ethz.ch

Documentation: https://vievswiki.geo.tuwien.ac.at/

# Reference

doi: https://doi.org/10.1088/1538-3873/ab1820

see: https://iopscience.iop.org/article/10.1088/1538-3873/ab1820

# Getting started

Have a look at our YouTube channel 
https://www.youtube.com/channel/UCl2VPe7OrnznNtrh0_lwrqQ where we provide video tutorials. 
Additionally, the graphical user interface of VieSched++ contains a build in help. 
Read the text on the welcome page for more information. 

# License
> VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
>
> Copyright (C) 2018  Matthias Schartner
>
> This program is free software: you can redistribute it and/or modify
> it under the terms of the GNU General Public License as published by
> the Free Software Foundation, either version 3 of the License, or
> (at your option) any later version.
>
> This program is distributed in the hope that it will be useful,
> but WITHOUT ANY WARRANTY; without even the implied warranty of
> MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
> GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License
> along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Installation

## VieSched++ installers 

__Installers for Windows and Linux are provided through the release page on GitHub!__

Visit https://github.com/TUW-VieVS/VieSchedpp/releases.

## Manual installation

This section descripes how to manually install __VieSched++__. Please note that we provide installers for several operating systems. It is a lot easier to use one of the installers instead of building __VieSched++__ manually! 

The software consists of several parts:
* VieSched++: the scheduler (this repository)
* VieSched++ GUI (optional): the graphical user interface, hostet on https://github.com/TUW-VieVS/VieSchedppGUI
* VieSched++ AUTO (optional): enables automated scheduling based on daily cronjob 

__It is highly recommended to use the GUI to create the VieSchedpp.xml input files!__

### build software on Linux

The following code shows how to install all components of VieSched++ as well as their dependencies on a plain Linux installation:

	sudo apt update
	sudo apt install git
	sudo apt install build-essential
	sudo apt install cmake 
	sudo apt install libboost-all-dev

	mkdir VieSchedpp
	cd VieSchedpp

	# make IAU_SOFA (get source code from any repository you like - see https://www.iausofa.org/)
	git clone https://github.com/Starlink/sofa.git --single-branch --branch=vendor IAU_SOFA 
	cd IAU_SOFA 
	mkdir Release 
	cd src/
	make 
	mv libsofa_c.a ../Release/
	make clean
	cd ../../
	
	# make SGP4 (for satellite scheduling - see https://www.danrw.com/sgp4/)
	git clone https://github.com/dnwrnr/sgp4.git
	cd sgp4/
	git checkout f5cb54b38
	mkdir Release
	cd Release
	cmake -DCMAKE_BUILD_TYPE=Release ..
	make
	cd ../../

	# make VieSched++
	git clone https://github.com/TUW-VieVS/VieSchedpp.git 
	cd VieSchedpp 
	mkdir Release 
	cd Release 
	cmake -DCMAKE_BUILD_TYPE=Release .. 
	make 
	# [OPTIONAL] test installation: "$ ./VieSchedpp" 
	cd ../../

	# make VieSched++ GUI (optional - graphical user interface)
	sudo apt install qt6-base-dev libqt6charts6 libqt6charts6-dev libqt6network6 
	git clone https://github.com/TUW-VieVS/VieSchedppGUI.git 
	cd VieSchedppGUI
	mkdir Release 
	cd Release 
	cmake -DCMAKE_BUILD_TYPE=Release .. 
	make 
	# [OPTIONAL] test installation: "$ ./VieSchedppGUI" 
	cd ../../

	# install VieSched++ AUTO (optional - auto scheduling program)
	# download VieSched++ AUTO
	git clone --recurse-submodules https://github.com/TUW-VieVS/VieSchedpp_AUTO.git 
	cd VieSchedpp_AUTO
	python3 -m venv venv
	source venv/bin/activate
	pip install -r requirements.txt

### Build software on Windows

You sure you want to do that? 
I am not an expert on Windows but this is how I got it to work.
I recognize that this is not the most elegant way and that there are many alternatives. 
Please use whatever works for you. 

#### Preparations

    Install GIT: https://git-scm.com/downloads
    - Follow the recommended GIT installation procedure. 
    - At the end, you should have access to a GIT Bash terminal 
    
    Install C++ compiler 
    - In the following, I will use MSYS2 for installing and managing packages. 
    - Visit https://www.msys2.org and download installer msys2-x86_64-<version>.exe (or similar)
    - I will install everything into C:\msys64
    - Start MSYS2 MSYS, this should opern the MSYS2 MSYS terminal 
    
    Install Boost and CMake via the MSYS2 MSYS terminal
    - Run: 
      MSYS~ $ pacman -Syu 
    - It might be that the terminal closes. If this is the case, restart MSYS2 MSYS. 
    - Run:
      MSYS~ $ pacman -Su 
    - Install the C++ compiler toolchain
      MSYS~ $ pacman -S --needed base-devel mingw-w64-x86_64-toolchain
      This should now install a new terminal MSYS2 MINGW64. Start this termianl and close the MSYS2 MSYS to avoid confusion
    -  Verify that the installation of the MinGW compiler was successful by running the following command in MSYS2 MINGW64
      MINGW64~ $ g++ --version
      Which should return something like 15.2.0 
    - Install the boost libraries 
      MINGW64~ $ pacman -S mingw-w64-x86_64-boost
      This should place files in C:\msys64\mingw64\include\boost C:\msys64\mingw64\lib\libboost* etc. 
    - Install CMake
      MINGW64~ $ pacman -S --needed mingw-w64-x86_64-cmake
    - Verify installation using 
      MINGW64~ $ cmake --version 
      Which should return something like 4.1.2
    
    Now, everything is set up and you can start installing VieSched++
    You will need two support libraries: SOFA and SGP4
    Let's assume you will install everything to C:\VieSched++
    
    PART 1: Install SOFA
    - In your GIT Bash terminal:
      GIT Bash $ cd C:/VieSched++
      GIT Bash $ git clone https://github.com/Starlink/sofa.git --single-branch --branch=vendor IAU_SOFA
    - In your MSYS2 MINGW64 terminal:
      MINGW64~ $ cd C:/VieSched++/IAU_SOFA
      MINGW64~ $ mkdir Release
      MINGW64~ $ cd src
      MINGW64~ $ make 
      MINGW64~ $ mv libsofa_c.a ../Release/
      MINGW64~ $ make clean
      You should now see a file located in C:\IAU_SOFA\Release\libsofa_c.a
    
    PART 2: Install SGP4
    - In your GIT Bash terminal:
      GIT Bash $ cd C:/VieSched++
      GIT Bash $ git clone https://github.com/dnwrnr/sgp4.git
      GIT Bash $ cd sgp4
      GIT Bash $ git checkout f5cb54b38
    - In your MSYS2 MINGW64 terminal:
      MINGW64~ $ cd C:/VieSched++/sgp4
      MINGW64~ $ mkdir -p build Release/libsgp4
      MINGW64~ $ cd build
      MINGW64~ $ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 .. 
      MINGW64~ $ ninja
      MINGW64~ $ mv libsgp4/libsgp4* ../Release/libsgp4
      MINGW64~ $ ninja clean
      You should now see files located in C:\VieSched++\sgp4\Release\libsgp4.*
    
#### For Users
    
    PART 3: Install VieSched++
    - In your GIT Bash terminal: 
      GIT Bash $ cd C:/VieSched++
      GIT Bash $ git clone https://github.com/TUW-VieVS/VieSchedpp.git
    - In your MSYS2 MINGW64 terminal:
      MINGW64~ $ cd C:/VieSched++/VieSchedpp
      MINGW64~ $ mkdir Release 
      MINGW64~ $ cd Release
      MINGW64~ $ cmake -DCMAKE_BUILD_TYPE=Release .. 
      MINGW64~ $ ninja 
    
    PART 4: Install VieSched++ GUI
    - In your GIT Bash terminal: 
      GIT Bash $ cd C:/VieSched++
      GIT Bash $ git clone https://github.com/TUW-VieVS/VieSchedppGUI.git
    - In your MSYS2 MINGW64 terminal:
      MINGW64~ $ pacman -S mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-charts mingw-w64-x86_64-openssl 
      MINGW64~ $ cd C:/VieSched++/VieSchedppGUI
      MINGW64~ $ mkdir Release 
      MINGW64~ $ cd Release
      MINGW64~ $ cmake -DCMAKE_BUILD_TYPE=Release .. 
      MINGW64~ $ ninja 
    
#### For Developers

    PART 3: Install VieSched++
    - If you want to use a proper IDE, take the following steps:
    - For VieSched++, I recommend JetBrain's CLion
    - Install CLion and open: C:/VieSched++/VieSchedpp/CMakeLists.txt as project
    - Define toolchain with MSYS2 compiler:
      Settings -> Build, Execution, Deployment -> Toolchains
      Add a new one "+" and name it MSYS2
      Toolset: C:\msys64\mingw64 
      Debugger: C:\msys64\mingw64\bin\gdb.exe 
      Should find compilers in C:\sys64\mingw64\bin\cc.exe and c++.exe 
    - Add CMake profiles:
      Settings -> Build, Execution, Deployment -> CMake
      Add two new ones, one Debug and one Release. 
      Make sure to select the MSYS Toolchain for both.
    
    PART4: Install VieSched++ GUI 
    - Install QT6 (I use v.6.9.3):
      - I recommend to use the QT6 online installer (account might be needed)
      - log into my.qt.io 
      - On the left, select "Get Qt Open Source" -> Download -> "Windows x64" -> "Qt Online Installer for Windows (x64)"
      - Start installer and follow steps
      - Under "Installation options" select "Custom Installation" 
      - Under "Customize" select: "Qt/Qt <version>/Additinal Libraries/Qt Graphs" (not strictly necessary now but will be in the future)
      - Finish installation. 
      - I installed everything into C:\Qt
    - Open QT Creator (C:\Qt\Tools\QtCreator\bin\qtcreator.exe)
      - Open Project: C:\VieSched++\VieSchedppGUI\CMakeLists.txt 
      - In Qt Creator: Edit -> Preferences -> Kits
      - Compilers -> Add -> MinGW 
        Name: MSYS2 
        C compiler path: C:\msys64\mingw64\bin\gcc.exe
      - Debuggers -> Add 
        Name: MSYS2 
        Path: C:\msys64\mingw64\bin\gdb.exe
      - Kits -> Add 
        Name: MSYS2 
        Compiler: MSYS2 
        DEbugger: MSYS2 
        Qt version: Qt 6.9.3 MinGW 64-bit
      - Close Preferences 
      - At the right, click on Projects
        Build & Run: select MSYS2
        At the top, add Debug and Release build configuration 
        I specified the Build directories to 
        C:\VieSched++\VieSchedppGUI_MSYS2-Debug
        C:\VieSched++\VieSchedppGUI_MSYS2-Release
        This way, they will automatically find the libraries you compiled earlier. 
        Otherwise, use the CMake parameters 

### What about MacOS?

I do not own a Mac, so I cannot provide detailed instructions on how to install VieSched++ on MacOS.
However, I heard from colleagues that they managed to build the software from scratch so it should be possible.
Good luck and have fun! 

### Tipps installing VieSchedpp
There are several CMake variables you can set to customize your installation and provide paths to libraries if CMake does not find them automatically.
- PATH_BOOST_ROOT
- PATH_IAU_SOFA
- PATH_SGP4


----

If you get error messages during linking with functions called: `*boost*` or `iau_*`:

Check if your `c++ boost libraries` and the `SOFA` library is found. If not set the path to this libraries in the `CMakeLists.txt` file.

----

If you still have troubles installing the software contact me: mschartner@ethz.ch.

## Software errors

__VieSched++__ is still in developing, there might be bugs or crashes.

Please always have a look at the log file and have a look if you get some [warning], [error] or [fatal] messages.

In case __VieSched++__ crashes or it reports an error in the log files please raise an issue at https://github.com/TUW-VieVS/VieSchedpp.

Follow the "Bug report" issue template and attach:
* your current software version number (you can find it in the beginning of your log file, or in the .skd or .vex files - it is a hash code like `f20be1498274232acb46cf44121f9e60278c6528`)
* your VieSchedpp.xml file
* your catalog files
* your log file `VieSchedpp_yyyy-mm-dd_hh-mm-ss.sss.log`
* any other helpful information

