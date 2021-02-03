# VieSched++

VLBI scheduling software

Written by Matthias Schartner

Contact: matthias.schartner@geo.tuwien.ac.at

Documentation: https://tuw-vievs.github.io/VieSchedpp/index.html

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

__Installers for Windows 10 and Ubuntu 18.04 are provided through the release page on GitHub!__

Visit https://github.com/TUW-VieVS/VieSchedpp/releases.

## Manual installation

This section descripes how to manually install __VieSched++__. Please note that we provide installers for several operating systems. It is a lot easier to use one of the installers instead of building __VieSched++__ manually! 

The software consists of several parts:
* VieSched++: the scheduler (this repository)
* VieSched++ GUI (optional): the graphical user interface, hostet on https://github.com/TUW-VieVS/VieSchedppGUI
* VieSched++ AUTO (optional): enables automated scheduling based on daily cronjob 

__It is highly recommended to use the GUI to create the VieSchedpp.xml input files!__

The following code shows how to install all components of VieSched++ as well as their dependencies on a plain Ubuntu installation (tested with 18.04 and 20.04):

	sudo apt update
	sudo apt install git
	sudo apt install build-essential
	sudo apt install cmake 
	sudo apt install libboost-all-dev

	mkdir VieSchedpp
	cd VieSchedpp

	# make IAU_SOFA (get source code from any repository you like)
	git clone https://github.com/Starlink/sofa.git --single-branch --branch=vendor IAU_SOFA 
	cd IAU_SOFA 
	mkdir Release 
	cd src/
	make 
	mv libsofa_c.a ../Release/
	make clean
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
	sudo apt install qt5-default libqt5charts5 libqt5charts5-dev libqt5network5 
	git clone https://github.com/TUW-VieVS/VieSchedppGUI.git 
	cd VieSchedppGUI
	mkdir Release 
	cd Release 
	qmake "IAU_SOFA=../../IAU_SOFA/Release/libsofa_c.a" ../VieSchedppGUI.pro 
	make 
	# [OPTIONAL] test installation: "$ ./VieSchedppGUI" 
	cd ../../

	# install VieSched++ AUTO (optional - auto scheduling program)
	# download VieSched++ AUTO
	git clone --recurse-submodules https://github.com/TUW-VieVS/VieSchedpp_AUTO.git 
	cd VieSchedpp_AUTO
	# create virtual environment (I'm using venv here):
	python3 -m venv venv
	source venv/bin/activate
	pip install -r requirements.txt
	# alternative using miniconda:
	wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
	bash Miniconda3-latest-Linux-x86_64.sh # follow the installation instructions
	conda env create -f ./VieSchedpp_AUTO/environment.yml # create new environment for VieSchedpp_AUTO
	conda activate VieSchedpp_AUTO # activate the new environment
	# [OPTIONAL] test installation: "$ python VieSchedpp_AUTO/VieSchedpp_AUTO.py -h"


### Tipps installing VieSchedpp
- `-DPATH_IAU_SOFA="./path/to/IAU_SOFA/"` --> set path to IAU_SOFA library (default: "../IAU_SOFA/Release")
- `-DLINK_BOOST=False` --> link against Boost libraries (default: "True")
- If you have troubles finding boost try to set path `-DBOOST_ROOT="./path/to/boost/"` 

### Tipps installing VieSchedppGUI:
- you can set path to `IAU_SOFA` library using `qmake "IAU_SOFA=../../IAU_SOFA/libsofa_c.a"` (default path is "../IAU_SOFA/Release")
 

## Troubleshooting installation

If your Output contains some `CMake` warnings like:

    CMake Warning at /usr/share/cmake-3.5/Modules/FindBoost.cmake:725 (message):
      Imported targets not available for Boost version 106400

check if your `CMake` version supports `boost` version and install the newest version of CMake from https://cmake.org/.

----

If you get error messages during linking with functions called: `*boost*` or `iau_*`:

Check if your `c++ boost libraries` and the `SOFA` library is found. If not set the path to this libraries in the `CMakeLists.txt` file.

----

If you have troubles getting `boost` to work simply try to build it without `c++ boost libraries`. (see Section "Tipps installing VieSchedpp")

----

If you still have troubles installing the software contact me: matthias.schartner@geo.tuwien.ac.at.

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

# Develop VieSched++

In case you want to develop __VieSched++__ have a look at the `doxygen` documentation and use an appropriate IDE.

* VieSchedpp was developed using the CLion. https://www.jetbrains.com/clion/
* VieSchedppGUI was developed using QtCreator http://doc.qt.io/qtcreator/
