/**
 * @file Baseline.h
 * @brief class Baseline
 *
 *
 * @author Matthias Schartner
 * @date 23.06.2018
 */

#ifndef BASELINE_H
#define BASELINE_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

#include "VieVS_NamedObject.h"

namespace VieVS{
    /**
     * @class Baseline
     * @brief representation of a VLBI baseline
     *
     * @author Matthias Schartner
     * @date 23.06.2018
     */
    class Baseline: public VieVS_NamedObject {
    public:

        Baseline(std::string name, std::string alternativeName, unsigned long staid1, unsigned long staid2);

        bool hasStationIds(unsigned long staid1, unsigned long staid2) const noexcept;

        bool hasStationIds(const std::pair<unsigned long, unsigned long> &staids) const noexcept;

        /**
         * @brief baseline parameters
         */
        class Parameters: public VieVS_NamedObject{
        private:
            static unsigned long nextId;
        public:
            explicit Parameters(const std::string &name):VieVS_NamedObject(name,nextId++){}
            void setParameters(const Parameters &other);

            bool ignore = false;
            double weight = 1;
            unsigned int minScan = 30;
            unsigned int maxScan = 600;
            std::unordered_map<std::string, double> minSNR;

            /**
             * @brief output of the current parameters to out stream
             *
             * @param of out stream object
             */
            void output(std::ofstream &of) const {
//                if (ignore) {
//                    of << "    ignore: TRUE\n";
//                } else {
//                    of << "    ignore: FALSE\n";
//                }
//                of << "    weight:      " << weight << "\n";
//                of << "    min scan:      " << minScan << "\n";
//                of << "    max scan:      " << maxScan << "\n";
//                for (const auto &it:minSNR) {
//                    of << "    minSNR: " << it.first << " " << it.second << "\n";
//                }
            }
        };


        struct Event {
            Event(unsigned int time, bool softTransition, Parameters PARA): time{time},
                                                                            softTransition{softTransition},
                                                                            PARA{std::move(PARA)}{}

            unsigned int time; ///< time when new parameters should be used in seconds since start
            bool softTransition; ///< transition type
            Parameters PARA; ///< new parameters
        };

        struct Statistics{
            std::vector<unsigned int> scanStartTimes{};
            int totalObservingTime{0};
        };

        /**
         * @brief sets all upcoming events
         * @param EVENTS all upcoming events
         */
        void setEVENTS(std::vector<Event> &EVENTS) noexcept {
            events_ = std::make_shared<std::vector<Event>>(move(EVENTS));
            nextEvent_ = 0;
        }


        /**
         * @brief getter method for first station id
         *
         * @return first station id
         */
        unsigned long getStaid1() const noexcept {
            return staid1_;
        }

        /**
         * @brief getter method for second station id
         *
         * @return second station id
         */
        unsigned long getStaid2() const noexcept {
            return staid2_;
        }

        /**
         * @brief getter method for station ids
         *
         * @return pair of station ids
         */
        std::pair<unsigned long, unsigned long> getStaids() const noexcept {
            return {staid1_, staid2_};
        };

        const Parameters &getParameters() const {
            return parameters_;
        }

        Parameters &refParameters() {
            return parameters_;
        }


        bool checkForNewEvent(unsigned int time, bool &hardBreak) noexcept;

        void update(bool influence) noexcept;

        void setNextEvent(unsigned int idx) noexcept{
            nextEvent_ = idx;
        }

    private:
        static unsigned long nextId;

        unsigned long staid1_; ///< id of first antenna
        unsigned long staid2_; ///< id of second antenna

        std::shared_ptr<std::vector<Event>> events_; ///< list of all events

        Statistics statistics_;
        Parameters parameters_; ///< station parameters

        unsigned int nextEvent_{0}; ///< index of next event
        int nObs_{0}; ///< number of observations
        int nTotalObs_{0};

    };
}
#endif /* BASELINE_H */

