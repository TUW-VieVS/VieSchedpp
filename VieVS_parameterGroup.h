//
// Created by matth on 10.09.2017.
//

#ifndef VIEVS_PARAMETERGROUP_H
#define VIEVS_PARAMETERGROUP_H

#include <string>
#include <vector>

namespace VieVS {
    class VieVS_parameterGroup {
    public:
        std::string name;
        std::vector<std::string> members;

        VieVS_parameterGroup(const std::string &name, const std::vector<std::string> &members);
    };
}


#endif //VIEVS_PARAMETERGROUP_H
