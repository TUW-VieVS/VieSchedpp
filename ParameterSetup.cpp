//
// Created by mschartn on 24.08.17.
//

#include "ParameterSetup.h"
using namespace std;
using namespace VieVS;


ParameterSetup::ParameterSetup(): start_{0}, end_{0}, transition_{Transition::soft} {
}

ParameterSetup::ParameterSetup(unsigned int start, unsigned int end) :
        start_{start}, end_{end}, transition_{Transition::soft} {

}


ParameterSetup::ParameterSetup(const std::string &parameterName, const std::string &memberName, unsigned int start,
                       unsigned int end, Transition transition) :
        parameterName_{parameterName}, memberName_{memberName}, start_{start}, end_{end}, transition_{transition} {

    ParameterSetup::members_.push_back(memberName);
}


ParameterSetup::ParameterSetup(const std::string &parameterName, const std::string &groupName,
                       const std::vector<std::string> &groupMembers, unsigned int start, unsigned int end,
                       Transition transition) :
        parameterName_{parameterName}, memberName_{groupName}, start_{start}, end_{end}, transition_{transition} {

    ParameterSetup::members_.insert(members_.end(), groupMembers.begin(), groupMembers.end());
}


bool ParameterSetup::isValidChild(const ParameterSetup &other) const {
    const std::vector<std::string> &otherMembers = other.getMembers();

    unsigned int otherStart = other.start_;
    unsigned int otherEnd = other.end_;

    if (members_.empty()) {
        bool valid = otherStart >= start_ && otherEnd <= end_;
        if (!valid) {
            return false;
        }
    } else {
        for (const auto &any:members_) {
            if (find(otherMembers.begin(), otherMembers.end(), any) != otherMembers.end()) {
                bool valid = otherStart >= start_ && otherEnd <= end_;
                if (!valid) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool ParameterSetup::isValidSibling(const ParameterSetup &other) const {

    const std::vector<std::string> &otherMembers = other.getMembers();
    unsigned int otherStart = other.start_;
    unsigned int otherEnd = other.end_;

    if (members_.empty() || otherMembers.empty()) {
        return false;
    }

    for (const auto &any:members_) {
        if (find(otherMembers.begin(), otherMembers.end(), any) != otherMembers.end()) {
            bool valid = otherEnd <= start_ || otherStart >= end_;
            if (!valid) {
                return false;
            }
        }
    }

    return true;
}

bool ParameterSetup::addChild(const ParameterSetup &child) {

    if (!isValidChild(child)) {
        return false;
    }
    for (const auto &any: childrens_) {
        if (!any.isValidSibling(child)) {
            return false;
        }
    }

    childrens_.push_back(child);

    return true;
}


