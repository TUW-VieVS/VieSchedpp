/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mask.h
 * Author: mschartn
 *
 * Created on June 27, 2017, 12:58 PM
 */

#ifndef VLBI_MASK_H
#define VLBI_MASK_H
#include <vector>
#include "VieVS_constants.h"

using namespace std;

namespace VieVS{
    class VLBI_mask {
    public:
        enum class category{step,line,none};
        
        VLBI_mask();
        
        VLBI_mask(vector<double> el_mask_deg);
        
        virtual ~VLBI_mask();
        
    private:
        vector<double> knots; 
        vector<double> values;
        category type ;
    };
}
#endif /* VLBI_MASK_H */

