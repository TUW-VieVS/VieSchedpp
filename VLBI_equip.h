/**
 * @file VLBI_equip.h
 * @brief class VLBI_equip
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef VLBI_EQUIP_H
#define VLBI_EQUIP_H

#include <vector>
#include <unordered_map>
#include <iostream>
#include <limits>

using namespace std;

namespace VieVS{
    class VLBI_equip {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_equip();

        /**
         * @brief constructor
         *
         * @param all_channelNames all channel names
         * @param corresponding_SEFDs all corresponding channel SEFDs
         */
        VLBI_equip(vector<string> all_channelNames, vector<double> corresponding_SEFDs);

        /**
         * @brief destructor
         */
        virtual ~VLBI_equip();

        /**
         * @brief getter function for antenna SEFD information
         *
         * @param band name of band
         * @return SEFD of this band
         */
        double getSEFD(string band) const {
            return SEFD.at(band);
        }

        /**
         * @brief returns maximum SEFD of this antenna
         *
         * @return maximum SEFD of this antenna
         */
        double getMaxSEFD() const;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param equip equipment information that should be printed to stream
         * @return stream object
         */
        friend ostream& operator<<(ostream& out, const VLBI_equip& equip);
        
    private:
        unordered_map<string,double> SEFD;
    };
}
#endif /* VLBI_EQUIP_H */

