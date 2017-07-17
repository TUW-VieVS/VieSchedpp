//
// Created by mschartn on 14.07.17.
//

#ifndef VIEVS_SCHEDULER_CLION_VLBI_SCANTIMES_H
#define VIEVS_SCHEDULER_CLION_VLBI_SCANTIMES_H

#include <vector>

using namespace std;
namespace VieVS {

    class VLBI_scanTimes {
    public:
        VLBI_scanTimes();

        VLBI_scanTimes(int nsta);

        void setEndOfLastScan(vector<unsigned int> endOfLastScan) {
            VLBI_scanTimes::endOfLastScan = endOfLastScan;
        }

        void addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape, unsigned int calib);

        void addTimes(int idx, const VLBI_scanTimes &other, int idx_other);

        void removeElement(int idx);

        const vector<unsigned int> &getEndOfSlewTime() const {
            return endOfSlewTime;
        }

        const unsigned int getEndOfIdleTime(int idx) const {
            return endOfIdleTime[idx];
        }

        void alignStartTimes(unsigned int latestSlewtimes);

        void updateSlewtime(int idx, unsigned int new_slewtime);

    private:
        vector<unsigned int> endOfLastScan;
        vector<unsigned int> endOfSetupTime;
        vector<unsigned int> endOfSourceTime;
        vector<unsigned int> endOfSlewTime;
        vector<unsigned int> endOfIdleTime;
        vector<unsigned int> endOfTapeTime;
        vector<unsigned int> endOfCalibrationTime;
        vector<unsigned int> endOfScanTime;
    };
}

#endif //VIEVS_SCHEDULER_CLION_VLBI_SCANTIMES_H
