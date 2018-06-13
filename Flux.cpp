/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Flux.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 11:22 AM
 */

#include "Flux.h"
using namespace std;
using namespace VieVS;

unsigned long Flux::nextId = 0;

Flux::Flux(double wavelength): VieVS_Object{nextId++} {
    wavelength_ = wavelength;
}

//std::unique_ptr<Flux> Flux::clone() const {
//    std::unique_ptr<Flux> other(this->do_clone());
//    assert(typeid(*this) == typeid(*other));
//    return other;
//};

