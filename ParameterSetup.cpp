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


int ParameterSetup::isValidChild(const ParameterSetup &other) const {
    const std::vector<std::string> &otherMembers = other.getMembers();

    unsigned int otherStart = other.start_;
    unsigned int otherEnd = other.end_;

    if(other.getMemberName() == "__all__" && memberName_ != "__all__"){
        return 1;
    }

    bool valid = otherStart >= start_ && otherEnd <= end_;
    if (!valid) {
        return 2;
    }

    for (const auto &any:otherMembers) {
        if (memberName_ != "__all__" && find(members_.begin(), members_.end(), any) == members_.end()) {
            return 3;
        }
    }
    return 0;
}

int ParameterSetup::isValidSibling(const ParameterSetup &other) const {

    unsigned int otherStart = other.start_;
    unsigned int otherEnd = other.end_;
    bool seperate = otherEnd <= start_ || otherStart >= end_;
    if(seperate){
        return 0;
    }

    if(memberName_ == "__all__" || other.getMemberName() == "__all__"){
        return 4;
    }

    const std::vector<std::string> &otherMembers = other.getMembers();

    if (members_.empty() || otherMembers.empty()) {
        return 5;
    }

    for (const auto &any:members_) {
        if (find(otherMembers.begin(), otherMembers.end(), any) != otherMembers.end()) {
            return 6;
        }
    }

    return 0;
}

int ParameterSetup::addChild(const ParameterSetup &child) {

    int errorCode = isValidChild(child);
    if (errorCode != 0) {
        return errorCode;
    }
    for (const auto &any: childrens_) {
        errorCode = any.isValidSibling(child);
        if (errorCode != 0) {
            return errorCode;
        }
    }

    childrens_.push_back(child);

    return errorCode;
}


boost::optional<ParameterSetup &>
ParameterSetup::search(int thisLevel, int level, const std::string &parameterName, const std::string &memberName, const std::vector<std::string> &members,
                       ParameterSetup::Transition transition, unsigned int start, unsigned int end) {

    if(thisLevel == level && this->isEqual(parameterName,memberName,members,transition,start,end)){
        return *this;
    }else{
        for(auto &any: childrens_){
            auto ans = any.search(thisLevel+1, level, parameterName, memberName, members, transition, start, end);
            if(ans.is_initialized()){
                return ans;
            }
        }
    }
    return boost::none;
}

bool ParameterSetup::deleteChild(int thisLevel, int level, const string &parameterName, const string &memberName, const std::vector<string> &members, ParameterSetup::Transition transition, unsigned int start, unsigned int end)
{
    int i=0;
    for(auto &any: childrens_){
        if(thisLevel+1 == level && any.isEqual(parameterName,memberName,members,transition,start,end)){
            childrens_.erase(childrens_.begin()+i);
            return true;
        }
        ++i;
    }
    bool found = false;
    for(auto &any: childrens_){
        found = any.deleteChild(thisLevel+1,level,parameterName,memberName,members,transition,start,end);
        if(found == true){
            return found;
        }
    }
}

bool
ParameterSetup::isEqual(std::string parameterName, std::string memberName, std::vector<std::string> members,
                        ParameterSetup::Transition transition, unsigned int start, unsigned int end) {

    if(parameterName_ != parameterName){
        return false;
    }
    if(memberName_ != memberName){
        return false;
    }
    if(members_ != members){
        return false;
    }
    if(transition_ != transition){
        return false;
    }
    if(start_ != start){
        return false;
    }
    if(end_ != end){
        return  false;
    }

    return true;
}


