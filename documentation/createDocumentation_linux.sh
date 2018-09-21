#!/bin/bash

doxygen
cd latex
make
cd ../
ln -s html/index.html index.html
mv latex/refman.pdf .

