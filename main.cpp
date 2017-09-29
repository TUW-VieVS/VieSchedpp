#include <cstdlib>
#include <chrono>
#include <vector>
#include <boost/format.hpp>
#include <iostream>
#include <thread>
//#include <boost/filesystem.hpp>

#include "Initializer.h"
#include "Scheduler.h"
#include "Output.h"
#include "ParameterSettings.h"

/**
 * @file main.cpp
 * @brief main file
 *
 * @author Matthias Schartner
 * @date 21.06.2017
 */

/**
 * @namespace VieVS
 * @brief namespace VieVS is used for all "VieVS_*" and "VLBI_*" classes and files.
 */

using namespace std;
/**
 * starts the scheduling software
 */
void run();

/**
 * creates the corresponding .xml file (will be replaced later by GUI.
 */
void createParameterFile();

/**
 * Main function.
 *
 * @param argc currently unused
 * @param argv  currently unused
 * @return 0 if no error occurred
 */
int main(int argc, char *argv[])
{
    createParameterFile();

    auto start = std::chrono::high_resolution_clock::now();
    run();
    auto finish = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
    std::cout << "execution time: " << static_cast<double>(microseconds.count()) / 1e6 << " [s]\n";


    return 0;
}

/**
 * First a VLBI_initializer is created, than the following steps are executed:
 * - stations are created
 * - sources are creted
 * - stations are initialized
 * - sources are initialized
 * - nutation is calculated
 * - earth velocity is calculated
 * - lookup tabels are created
 * - skyCoverage objects are created
 * After this, the VLBI Scheduler is created, some a prioiri calculations are done and the scheduler is started.
 */
void run(){

    VieVS::Initializer init("parameters.xml");
    cout << "log file is written in this file: header.txt\n";

    ofstream headerLog("header.txt");

    VieVS::SkdCatalogReader skdCatalogReader = init.createSkdCatalogReader();

    init.initializeObservingMode(skdCatalogReader, headerLog);
    init.initializeLookup();
    init.initializeSkyCoverages();

    init.createStations(skdCatalogReader, headerLog);
    init.createSkyCoverages(headerLog);

    init.createSources(skdCatalogReader, headerLog);
    init.precalcSubnettingSrcIds();
    init.initializeSourceSequence();

    init.initializeCalibrationBlocks( headerLog );

    bool flag_multiSched = false;
    unsigned long nsched = 1;


    vector<VieVS::MultiScheduling::Parameters> all_multiSched_PARA = init.readMultiSched();
    if (!all_multiSched_PARA.empty()) {
        flag_multiSched = true;
        nsched = all_multiSched_PARA.size();
        headerLog << "multi scheduling found ... creating " << nsched << " schedules!\n";
    }

    headerLog.close();

    unsigned long numberOfCreatedScans = 0;


//    unsigned int concurentThreadsSupported = std::thread::hardware_concurrency();
//    omp_set_num_threads(concurentThreadsSupported);
//    omp_set_num_threads(1);
//    #pragma omp parallel for
    for (int i = 0; i < nsched; ++i) {

        VieVS::Scheduler scheduler;

        ofstream bodyLog;
        if (flag_multiSched) {
            VieVS::Initializer newinit = init;
            string fname = (boost::format("body_%04d.txt") % (i + 1)).str();
            bodyLog.open(fname);

            string txt = (boost::format("########## multiSched number: %4d of %4d ##########\n") % (i + 1) %
                          nsched).str();
            string txt2 = (boost::format("log file is written in this file: %s \n") % fname).str();
            cout << txt;
            cout << txt2;

            newinit.initializeGeneral(bodyLog);

            newinit.initializeStations();
            newinit.initializeSources();
            newinit.initializeBaselines();

            newinit.initializeWeightFactors();

            newinit.applyMultiSchedParameters(all_multiSched_PARA[i], bodyLog);

            newinit.initializeNutation();
            newinit.initializeEarth();

            scheduler = VieVS::Scheduler(newinit);
        } else {
            cout << "log file is written in this file: body.txt\n";
            bodyLog.open("body.txt");
            init.initializeGeneral(bodyLog);

            init.initializeStations();
            init.initializeSources();
            init.initializeBaselines();

            init.initializeWeightFactors();

            init.initializeNutation();
            init.initializeEarth();

            scheduler = VieVS::Scheduler(init);
        }


        scheduler.start(bodyLog);

        unsigned long createdScans = scheduler.numberOfCreatedScans();

//        #pragma omp atomic
        numberOfCreatedScans += createdScans;

        bodyLog.close();

        VieVS::Output output(scheduler);
        if (flag_multiSched) {
            output.setIsched(i + 1);
        } else {
            output.setIsched(0);
        }

        output.writeStatistics(true, true, true, true, true);

        output.writeNGS();

        output.writeSkd(skdCatalogReader);

        string txt3 = (boost::format("thread %4d finished\n") % (i + 1)).str();
        cout << txt3;

    }
    cout << "everything finally finished!!!\n";
    cout << "created total scans:" << numberOfCreatedScans << "\n";
}

/**
 * This function creates the .xml file. This will be replaced in a GUI in future!
 */
void createParameterFile(){

    VieVS::ParameterSettings para;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    para.software("Vie_SCHED2", "0.1", now);

    boost::posix_time::ptime start(boost::gregorian::date(2017, 01, 01), boost::posix_time::time_duration(12, 30, 00));
    boost::posix_time::ptime end = start + boost::posix_time::hours(24);
    boost::posix_time::time_duration a = end - start;
    auto duration = static_cast<unsigned int>(a.total_seconds());

    vector<string> station_names{"HART15M", "NYALES20", "SEJONG", "WETTZ13N", "WETTZ13S", "WETTZELL", "YARRA12M",
                                 "KATH12M"};
    para.general(start, end, true, true, true, 5.0, station_names);


    para.output("TEST", "test_description", "VIEN", "SHAO", true, true, true, false);

//    std::string root = "D:/VieVS/CATALOGS";
    std::string root = "/data/VieVS/CATALOGS";
    std::string antenna = "antenna.cat";
    std::string equip = "equip.cat";
    std::string flux = "flux.cat";
    std::string freq = "freq.cat";
    std::string hdpos = "hdpos.cat";
    std::string loif = "loif.cat";
    std::string mask = "mask.cat";
    std::string modes = "modes.cat";
    std::string position = "position.cat";
    std::string rec = "rec.cat";
    std::string rx = "rx.cat";
    std::string source = "source_geod.cat";
    std::string tracks = "tracks.cat";

    para.catalogs(root, antenna, equip, flux, freq, hdpos,
                  loif, mask, modes, position, rec, rx,
                  source, tracks);


    VieVS::ParameterGroup group_sta("group:siteWettzell",
                                          std::vector<std::string>{"WETTZ13N", "WETTZ13S", "WETTZELL"});
    para.group(VieVS::ParameterSettings::Type::station, group_sta);

    VieVS::Station::PARAMETERS sta_para1;
    sta_para1.minScan = 42;
    sta_para1.minSNR.insert({"X", 20});
    sta_para1.minSNR.insert({"S", 15});
    sta_para1.maxSlewtime = 600;
    para.parameters("para:general", sta_para1);

    VieVS::Station::PARAMETERS sta_para2;
    sta_para2.maxSlewtime = 400;
    sta_para2.maxScan = 400;
    para.parameters("para:wettzell", sta_para2);

    VieVS::Station::PARAMETERS sta_para3;
    sta_para3.minSNR.insert({"X", 22});
    sta_para3.ignoreSources_str.emplace_back("0104-408");
    sta_para3.ignoreSources_str.emplace_back("0111+021");
    para.parameters("para:SEJONG", sta_para3);

    VieVS::Station::PARAMETERS sta_para4;
    sta_para4.available = false;
    para.parameters("para:down", sta_para4);

    VieVS::Station::PARAMETERS sta_para5;
    sta_para5.tagalong = true;
    para.parameters("para:tagalong", sta_para5);

    VieVS::ParameterSetup pp(0, std::numeric_limits<unsigned int>::max());
    VieVS::ParameterSetup pp1("para:general", "__all__", 0, duration);
    VieVS::ParameterSetup pp11("para:wettzell", "group:siteWettzell",
                               para.getGroupMembers(VieVS::ParameterSettings::Type::station, "group:siteWettzell"), 0,
                               duration);
    VieVS::ParameterSetup pp2("para:SEJONG", "SEJONG", 0, duration);
    VieVS::ParameterSetup pp21("para:down", "SEJONG", 3600, 7200, VieVS::ParameterSetup::Transition::hard);
    VieVS::ParameterSetup pp3("para:tagalong", "HART15M", 3600, 7200);

    bool valid;
    valid = pp2.addChild(pp21);
    if (!valid) {
        cout << "no valid child!\n";
    }
    valid = pp1.addChild(pp11);
    if (!valid) {
        cout << "no valid child!\n";
    }
    valid = pp1.addChild(pp2);
    if (!valid) {
        cout << "no valid child!\n";
    }
    valid = pp1.addChild(pp3);
    if (!valid) {
        cout << "no valid child!\n";
    }
    valid = pp.addChild(pp1);
    if (!valid) {
        cout << "no valid child!\n";
    }
    para.setup(VieVS::ParameterSettings::Type::station, pp);

    para.stationWaitTimes("group:siteWettzell", 0, 2, 0, 5, 3);
    para.stationWaitTimes("SEJONG", 0, 5, 1, 10, 3);

    para.stationCableWrapBuffer("group:siteWettzell", 0, 0, 0, 0);
    para.stationCableWrapBuffer("SEJONG", 5, 5, 5, 5);

    VieVS::ParameterGroup group_src("group:starSources", std::vector<std::string>{"2355-534", "2329-384","0003-066","1615+029","1920-211"});
    para.group(VieVS::ParameterSettings::Type::source, group_src);

    VieVS::Source::PARAMETERS src_para1;
    src_para1.minFlux = .5;
    src_para1.maxScan = 500;
    para.parameters("para:general", src_para1);

    VieVS::Source::PARAMETERS src_para2;
    src_para2.minFlux = 0;
    src_para2.minRepeat = 3600;
    src_para2.minScan = 100;
    src_para2.maxScan = 700;
    src_para2.fixedScanDuration = 500;
    src_para2.ignoreStationsString.emplace_back("WETTZ13N");
    src_para2.ignoreStationsString.emplace_back("WETTZ13S");
    src_para2.ignoreBaselinesString.emplace_back("WETTZ13S", "WETTZ13N");
    src_para2.ignoreBaselinesString.emplace_back("WETTZELL", "WETTZ13N");
    src_para2.ignoreBaselinesString.emplace_back("WETTZELL", "WETTZ13S");
    para.parameters("para:star", src_para2);

    VieVS::ParameterSetup pp_src(0, std::numeric_limits<unsigned int>::max());
    VieVS::ParameterSetup pp_src1("para:general", "__all__", 0, duration);
    VieVS::ParameterSetup pp_src2("para:star", "group:starSources",
                              para.getGroupMembers(VieVS::ParameterSettings::Type::source, "group:starSources"), 0,
                              duration);

    valid = pp_src1.addChild(pp_src2);
    if (!valid) {
        cout << "no valid child!\n";
    }
    valid = pp_src.addChild(pp_src1);
    if (!valid) {
        cout << "no valid child!\n";
    }
    para.setup(VieVS::ParameterSettings::Type::source, pp_src);


    VieVS::ParameterGroup group_bl("group:siteWettzell",
                                         std::vector<std::string>{"WETTZ13N-WETTZ13S", "WETTZ13N-WETTZELL",
                                                                  "WETTZ13S-WETTZELL"});
    para.group(VieVS::ParameterSettings::Type::baseline, group_bl);

    VieVS::Baseline::PARAMETERS bl_para1;
    bl_para1.ignore = true;
    para.parameters("para:ignore", bl_para1);

    VieVS::Baseline::PARAMETERS bl_para2;
    bl_para2.minScan = 10;
    bl_para2.minSNR.insert({"X", 10});
    bl_para2.minSNR.insert({"S", 12});

    para.parameters("para:lessMinSNR", bl_para2);
    VieVS::ParameterSetup pp_bl(0, std::numeric_limits<unsigned int>::max());
//    VieVS::ParameterSetup pp_bl1("para:ignore", "group:siteWettzell",
//                             para.getGroupMembers(VieVS::ParameterSettings::Type::baseline, "group:siteWettzell"), 0,
//                             duration);
    VieVS::ParameterSetup pp_bl2("para:lessMinSNR", "HART15M-YARRA12M", 0, duration);
//    valid = pp_bl.addChild(pp_bl1);
//    if (!valid) {
//        cout << "no valid child!\n";
//    }
    valid = pp_bl.addChild(pp_bl2);
    if (!valid) {
        cout << "no valid child!\n";
    }
    para.setup(VieVS::ParameterSettings::Type::baseline, pp_bl);


    para.skyCoverage(30, 3600, 5000);


    para.weightFactor(2, 1, 1, 0, 0.1, 0, -20, -50, 0, 20, 10);

//    para.ruleScanSequence(10, std::vector<unsigned int>{0,1},
//                          std::vector<string>{"group:starSources","group:starSources"});

    para.ruleCalibratorBlockNScanSelections(10,"__all__",3,120);

    para.mode("1024-16(AU)");
//    para.mode(32, 2);
//    para.mode_band("X", 0.0349, VieVS::ObservationMode::Property::required, VieVS::ObservationMode::Backup::none,0,
//                   VieVS::ObservationMode::Property::required, VieVS::ObservationMode::Backup::none,0, 10);
//    para.mode_band("S", 3.8000, VieVS::ObservationMode::Property::required, VieVS::ObservationMode::Backup::none,0,
//                   VieVS::ObservationMode::Property::required, VieVS::ObservationMode::Backup::none,0, 6);


//    VieVS::MultiScheduling multiSched;
//    multiSched.setWeight_skyCoverage(std::vector<double>{1, 2});
//    multiSched.setStation_maxSlewtime(group_sta, std::vector<unsigned int>{100, 200, 300});
//    multiSched.setStation_maxSlewtime("HART15M", std::vector<unsigned int>{100, 200});
//    para.multisched(multiSched);

    para.write("parameters.xml");

}
