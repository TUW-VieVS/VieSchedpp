#include <cstdlib>
#include <chrono>
#include <vector>

#include "VLBI_initializer.h"
#include "VLBI_scheduler.h"
#include "VLBI_output.h"
#include "VieVS_xmlWriter.h"

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


void createSkyCoverageLookup();

/**
 * Main function.
 *
 * @param argc currently unused
 * @param argv  currently unused
 * @return 0 if no errors occures
 */
int main(int argc, char *argv[])
{
//    createParameterFile();

    auto start = std::chrono::high_resolution_clock::now();
    run();
    auto finish = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
    std::cout << "execution time: " << (double) microseconds.count() / 1e6 << " [s]\n";


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

//    string path = "D:/VieVS/CATALOGS";

    string path = "/data/VieVS/CATALOGS";

    VieVS::VLBI_initializer init;

    init.initializeObservingMode();
    init.createStationsFromCatalogs(path);
    init.createSourcesFromCatalogs(path);
    init.initializeStations();
    init.initializeSources();
    init.initializeNutation();
    init.initializeEarth();
    init.initializeLookup();
    init.createSkyCoverages();
    init.initializeSkyCoverages();
    init.initializeWeightFactors();
    init.initializeBaselines();
//    init.displaySummary();

    VieVS::VLBI_scheduler scheduler(init);

    scheduler.precalcSubnettingSrcIds();
    scheduler.start();

    VieVS::VLBI_output output(scheduler);
    output.displayStatistics(true, true, true, true, true);
    output.writeNGS();
    cout << "Good Bye!" << endl;

}

/**
 * This function creates the .xml file. This will be replaced in a GUI in future!
 */
void createParameterFile(){

    VieVS::VieVS_xmlWriter para;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    para.software("Vie_SCHED2", "0.1", now);

    boost::posix_time::ptime start(boost::gregorian::date(2017, 01, 01), boost::posix_time::time_duration(12, 30, 00));
    boost::posix_time::ptime end = start + boost::posix_time::hours(24);
    boost::posix_time::time_duration a = end - start;
    unsigned int duration = a.total_seconds();

    vector<string> station_names{"HART15M", "NYALES20", "SEJONG", "WETTZ13N", "WETTZ13S", "WETTZELL", "YARRA12M",
                                 "KATH12M"};
    para.general("TEST", "test_description", start, end, 5000, true, true, station_names);


    vector<string> group_all;
    vector<string> group_sta{"WETTZ13N", "WETTZ13S", "WETTZELL"};
    para.group(VieVS::VieVS_xmlWriter::TYPE::station, "group:siteWettzell", group_sta);

    VieVS::VLBI_station::PARAMETERS sta_para1;
    sta_para1.minScan = 42;
    sta_para1.minSNR.insert({"X", 20});
    sta_para1.minSNR.insert({"S", 15});
    sta_para1.maxSlewtime = 600;
    para.parameters("para:general", sta_para1);

    VieVS::VLBI_station::PARAMETERS sta_para2;
    sta_para2.wait_source = 1;
    sta_para2.wait_tape = 0;
    para.parameters("para:wait", sta_para2);

    VieVS::VLBI_station::PARAMETERS sta_para3;
    sta_para3.minSNR.insert({"X", 22});
    para.parameters("para:SEJONG", sta_para3);

    VieVS::VLBI_station::PARAMETERS sta_para4;
    sta_para4.available = false;
    para.parameters("para:down", sta_para4);

    VieVS::VLBI_setup pp(0, std::numeric_limits<unsigned int>::max());
    VieVS::VLBI_setup pp1("para:general", "__all__", 0, duration);
    VieVS::VLBI_setup pp11("para:wait", "group:siteWettzell", group_sta, 0, duration);
    VieVS::VLBI_setup pp2("para:SEJONG", "SEJONG", 0, duration);
    VieVS::VLBI_setup pp21("para:down", "SEJONG", 3600, 7200, VieVS::VLBI_setup::TRANSITION::hard);
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
    valid = pp.addChild(pp1);
    if (!valid) {
        cout << "no valid child!\n";
    }
    para.setup(VieVS::VieVS_xmlWriter::TYPE::station, pp);


    vector<string> group_src{"2355-534", "2329-384"};
    para.group(VieVS::VieVS_xmlWriter::TYPE::source, "group:starSources", group_src);

    VieVS::VLBI_source::PARAMETERS src_para1;
    src_para1.minFlux = .8;
    src_para1.maxScan = 500;
    para.parameters("para:general", src_para1);

    VieVS::VLBI_source::PARAMETERS src_para2;
    src_para2.minFlux = 0;
    src_para2.minRepeat = 3600;
    src_para2.minScan = 100;
    src_para2.maxScan = 700;
    src_para2.fixedScanDuration = 500;
    para.parameters("para:star", src_para2);

    VieVS::VLBI_setup pp_src(0, std::numeric_limits<unsigned int>::max());
    VieVS::VLBI_setup pp_src1("para:general", "__all__", 0, duration);
    VieVS::VLBI_setup pp_src2("para:star", "group:starSources", group_src, 0, duration);
    valid = pp_src1.addChild(pp_src2);
    if (!valid) {
        cout << "no valid child!\n";
    }
    valid = pp_src.addChild(pp_src1);
    if (!valid) {
        cout << "no valid child!\n";
    }
    para.setup(VieVS::VieVS_xmlWriter::TYPE::source, pp_src);


    vector<string> group_bl{"WETTZ13N-WETTZ13S", "WETTZ13N-WETTZELL", "WETTZ13S-WETTZELL"};
    para.group(VieVS::VieVS_xmlWriter::TYPE::baseline, "group:siteWettzell", group_bl);

    VieVS::VLBI_baseline::PARAMETERS bl_para1;
    bl_para1.ignore = true;
    para.parameters("para:ignore", bl_para1);

    VieVS::VLBI_baseline::PARAMETERS bl_para2;
    bl_para2.minScan = 10;
    bl_para2.minSNR.insert({"X", 10});
    bl_para2.minSNR.insert({"S", 12});

    para.parameters("para:lessMinSNR", bl_para2);
    VieVS::VLBI_setup pp_bl(0, std::numeric_limits<unsigned int>::max());
    VieVS::VLBI_setup pp_bl1("para:ignore", "group:siteWettzell", group_bl, 0, duration);
    VieVS::VLBI_setup pp_bl2("para:lessMinSNR", "HART15M-YARRA12M", 0, duration);
    valid = pp_bl.addChild(pp_bl1);
    if (!valid) {
        cout << "no valid child!\n";
    }
    valid = pp_bl.addChild(pp_bl2);
    if (!valid) {
        cout << "no valid child!\n";
    }
    para.setup(VieVS::VieVS_xmlWriter::TYPE::baseline, pp_bl);


    para.skyCoverage(30, 3600);


    para.weightFactor(2, 1, 1, 0, 0.1);


    para.mode(16, 32, 1, 2);
    para.mode_band("X", 0.0349, VieVS::VLBI_obsMode::PROPERTY::required, 10);
    para.mode_band("S", 0.0349, VieVS::VLBI_obsMode::PROPERTY::required, 6);

    para.write("parameters.xml");

}
