/**
 * @file VLBI_obsMode.h
 * @brief class VLBI_obsMode
 *
 *
 * @author Matthias Schartner
 * @date 21.08.2017
 */

#ifndef VLBI_OBSMODE_H
#define VLBI_OBSMODE_H

#include <unordered_map>

namespace VieVS{
    class VLBI_obsMode {
    public:
        /**
         * @brief all possible flux information type
         */
        enum class PROPERTY {
            required,    ///< this band information is required. If this information is missing this object is not used.
            optional,    ///< this band information is only optional. If information is available it is used, otherwise it is interpolated
        };

        static unsigned int bandwith; ///< bandwith of one channel
        static unsigned int sampleRate; ///< sample rate of one channel
        static unsigned int fanout; ///< fanout
        static unsigned int bits; ///< number of recorded bits
        static std::unordered_map<std::string, unsigned int> num_channels; ///< number of channels per band
        static std::unordered_map<std::string, double> wavelength; ///< wavelength per band
        static std::unordered_map<std::string, PROPERTY> property; ///< is band required or optional


    };
}


#endif //VLBI_OBSMODE_H
