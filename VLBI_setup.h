//
// Created by mschartn on 24.08.17.
//

#ifndef VIEVS_VLBI_SETUP_H
#define VIEVS_VLBI_SETUP_H

#include <string>
#include <boost/optional.hpp>
#include <utility>
#include <vector>
#include <memory>

#include "VieVS_time.h"

namespace VieVS {
    class VLBI_setup {
    public:
        enum class TRANSITION {
            soft,
            hard
        };


        VLBI_setup();

        VLBI_setup(unsigned int start, unsigned int end);

        VLBI_setup(const std::string &parameterName, const std::string &memberName,
                   unsigned int start, unsigned int end, TRANSITION transition = TRANSITION::soft);

        VLBI_setup(const std::string &parameterName, const std::string &groupName,
                   const std::vector<std::string> &groupMembers, unsigned int start, unsigned int end,
                   TRANSITION transition = TRANSITION::soft);

        const std::string &getParameterName() const {
            return parameterName;
        }

        const std::string &getMemberName() const {
            return memberName;
        }

        unsigned int getStart() const {
            return start;
        }

        unsigned int getEnd() const {
            return end;
        }

        const std::vector<std::string> &getMembers() const {
            return members;
        }

        const std::vector<VLBI_setup> &getChildren() const {
            return children;
        }

        TRANSITION getTransition() const {
            return transition;
        }

        bool isValidChild(const VLBI_setup &other) const;

        bool isValidSibling(const VLBI_setup &other) const;

        bool addChild(const VLBI_setup &child);

    private:
        std::string parameterName;
        std::string memberName;

        std::vector<std::string> members;

        TRANSITION transition;

        unsigned int start;
        unsigned int end;

        std::vector<VLBI_setup> children;
    };
}


#endif //VIEVS_VLBI_SETUP_H
