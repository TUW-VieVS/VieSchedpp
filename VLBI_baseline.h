/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   baseline.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 3:25 PM
 */

#ifndef BASELINE_H
#define BASELINE_H

#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std;
namespace VieVS{
    class VLBI_baseline {
    public:
        VLBI_baseline();

        VLBI_baseline(int staid1, int staid2, int srcid, unsigned int startTime);

        VLBI_baseline(const VLBI_baseline &other) = default;

        VLBI_baseline(VLBI_baseline &&other) = default;

        VLBI_baseline &operator=(const VLBI_baseline &other) = default;

        VLBI_baseline &operator=(VLBI_baseline &&other) = default;

        virtual ~VLBI_baseline() {}

        int getStaid1() const {
            return staid1;
        }

        int getStaid2() const {
            return staid2;
        }

        int getSrcid() const {
            return srcid;
        }

        unsigned int getStartTime() const {
            return startTime;
        }

        unordered_map<string, double> getObservedFlux() const {
            return observedFlux;
        }

        unsigned int getScanDuration() const {
            return scanDuration;
        }

        void setObservedFlux(const unordered_map<string, double> &observedFlux) {
            VLBI_baseline::observedFlux = observedFlux;
        }

        void setScanDuration(unordered_map<string,unsigned int>& scanDurations);

        string longestScanDurationBand();

    private:
        int staid1;
        int staid2;
        int srcid;
        unsigned int startTime;

        unordered_map<string,double> observedFlux;
        vector<string> observedFlux_name;
        vector<double> observedFlux_value;

        unordered_map<string,unsigned int> scanDurations;
        vector<string> scanDurations_name;
        vector<unsigned int> scanDurations_value;

        unsigned int scanDuration;
    };
}
#endif /* BASELINE_H */

