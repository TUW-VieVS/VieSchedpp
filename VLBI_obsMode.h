//
// Created by mschartn on 21.08.17.
//

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

        static unsigned int bandwith;
        static unsigned int sampleRate;
        static unsigned int fanout;
        static unsigned int bits;
        static std::unordered_map<std::string,unsigned int> num_channels;
        static std::unordered_map<std::string,double> wavelength;
        static std::unordered_map<std::string,PROPERTY> property;


    };
}


#endif //VLBI_OBSMODE_H
