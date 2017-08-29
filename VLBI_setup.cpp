//
// Created by mschartn on 24.08.17.
//

#include "VLBI_setup.h"

namespace VieVS {

    VLBI_setup::VLBI_setup() {
        VLBI_setup::start = 0;
        VLBI_setup::end = VieVS_time::duration;
    }

    VLBI_setup::VLBI_setup(unsigned int start, unsigned int end) :
            start{start}, end{end}, transition{TRANSITION::soft} {

    }

//    VLBI_setup::VLBI_setup(const std::string &parameterName, const std::string &memberName):
//            parameterName{parameterName}, memberName{memberName} {
//
//        if(memberName != "_all__"){
//            VLBI_setup::members.push_back(memberName);
//        }
//
//    }

    VLBI_setup::VLBI_setup(const std::string &parameterName, const std::string &memberName, unsigned int start,
                           unsigned int end, TRANSITION transition) :
            parameterName{parameterName}, memberName{memberName}, start{start}, end{end}, transition{transition} {

        VLBI_setup::members.push_back(memberName);
    }

//    VLBI_setup::VLBI_setup(const std::string &parameterName, const std::string &groupName,
//                           const std::vector<std::string> &groupMembers):
//            parameterName{parameterName}, memberName{groupName}{
//
//        VLBI_setup::members.insert(members.end(),groupMembers.begin(),groupMembers.end());
//    }

    VLBI_setup::VLBI_setup(const std::string &parameterName, const std::string &groupName,
                           const std::vector<std::string> &groupMembers, unsigned int start, unsigned int end,
                           TRANSITION transition) :
            parameterName{parameterName}, memberName{groupName}, start{start}, end{end}, transition{transition} {

        VLBI_setup::members.insert(members.end(), groupMembers.begin(), groupMembers.end());
    }


    bool VLBI_setup::isValidChild(const VLBI_setup &other) const {
        const std::vector<std::string> &otherMembers = other.getMembers();

        unsigned int otherStart = other.start;
        unsigned int otherEnd = other.end;

        if (members.empty()) {
            bool valid = otherStart >= start && otherEnd <= end;
            if (!valid) {
                return false;
            }
        } else {
            for (const auto &any:members) {
                if (find(otherMembers.begin(), otherMembers.end(), any) != otherMembers.end()) {
                    bool valid = otherStart >= start && otherEnd <= end;
                    if (!valid) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool VLBI_setup::isValidSibling(const VLBI_setup &other) const {

        const std::vector<std::string> &otherMembers = other.getMembers();
        unsigned int otherStart = other.start;
        unsigned int otherEnd = other.end;

        if (members.empty() || otherMembers.empty()) {
            return false;
        }

        for (const auto &any:members) {
            if (find(otherMembers.begin(), otherMembers.end(), any) != otherMembers.end()) {
                bool valid = otherEnd <= start || otherStart >= end;
                if (!valid) {
                    return false;
                }
            }
        }

        return true;
    }

    bool VLBI_setup::addChild(const VLBI_setup &child) {

        if (!isValidChild(child)) {
            return false;
        }
        for (const auto &any: children) {
            if (!any.isValidSibling(child)) {
                return false;
            }
        }

        children.push_back(std::move(child));

        return true;
    }


}