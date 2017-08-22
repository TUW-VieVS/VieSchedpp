#include <cstdlib>
#include <chrono>
#include <vector>

#include "VLBI_initializer.h"
#include "VLBI_scheduler.h"
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

    cout << "Good Bye!" << endl;

}

/**
 * This function creates the .xml file. This will be replaced in a GUI in future!
 */
void createParameterFile(){

    boost::property_tree::ptree pt;

    boost::posix_time::ptime time(boost::gregorian::date(2017, 01, 01), boost::posix_time::time_duration(12, 30, 00));
    cout << time << "\n";

    pt.add("software.name", "VieVS Scheduler");
    pt.add("software.version", "1.0");
    boost::posix_time::ptime created(boost::gregorian::day_clock::local_day(), boost::posix_time::second_clock::local_time().time_of_day());
    pt.add("software.created_local_time", created);

    pt.add("general.experiment_name", "R1XXX");
    pt.add("general.experiment_description","This is this experiment R1XXX");
    pt.add("general.start",time);
    pt.add("general.end", time + boost::posix_time::hours(24));

    boost::property_tree::ptree station_name_tree;
    vector<string> station_names{"HART15M", "NYALES20", "SEJONG", "WETTZ13N", "WETTZ13S", "WETTZELL", "YARRA12M",
                                 "KATH12M"};
    for (auto &any: station_names) {
        boost::property_tree::ptree tmp;
        tmp.add("station", any);
        station_name_tree.add_child("stations.station", tmp.get_child("station"));
    }
    pt.add_child("general.stations", station_name_tree.get_child("stations"));
//    vector<string> sta = {"HART15M","NYALES20","SEJONG","WETTZ13N","WETTZ13S","WETTZELL","YARRA12M","KATH12M"};
//    pt.add("general.stations",boost::algorithm::join(sta, ","));

    pt.add("general.maxDistanceTwinTeleskopes",5000);
    pt.add("general.subnetting", false);
    pt.add("general.fillinmode", true);

    boost::property_tree::ptree station;

    boost::property_tree::ptree station_global;
    boost::property_tree::ptree flux1_sta;
    flux1_sta.add("minSNR",20);
    flux1_sta.put("minSNR.<xmlattr>.band","X");
    station_global.add_child("global.minSNR",flux1_sta.get_child("minSNR"));
    boost::property_tree::ptree flux2_sta;
    flux2_sta.add("minSNR",15);
    flux2_sta.put("minSNR.<xmlattr>.band","S");
    station_global.add_child("global.minSNR",flux2_sta.get_child("minSNR"));
    station_global.add("global.wait_setup",0);
    station_global.add("global.wait_source",5);
    station_global.add("global.wait_tape",1);
    station_global.add("global.wait_calibration",10);
    station_global.add("global.wait_corsynch",3);
    station_global.add("global.maxSlewtime",9999);
    station_global.add("global.maxWait",9999);
    station_global.add("global.maxScan",600);
    station_global.add("global.minScan",42);
    station_global.put("global.<xmlattr>.name","global");
    station_global.put("global.<xmlattr>.members","*");
    station.add_child("group",station_global.get_child("global"));

    boost::property_tree::ptree station_group1;
    station_group1.add("group1.maxScan",500);
    station_group1.add("group1.wait_setup",5);
    station_group1.put("group1.<xmlattr>.name","group1");
    station_group1.put("group1.<xmlattr>.members","HART15M,NYALES20,SEJONG");
    station.add_child("group",station_group1.get_child("group1"));

    boost::property_tree::ptree source;
    boost::property_tree::ptree source_global;
    source_global.add("group.minRepeat",1800);
    source_global.add("group.maxScan",500);
    source_global.add("group.minScan",42);
    source_global.add("group.minFlux", 0.5);
    boost::property_tree::ptree flux1_src;
    flux1_src.add("minSNR",20);
    flux1_src.put("minSNR.<xmlattr>.band","X");
    source_global.add_child("group.minSNR",flux1_src.get_child("minSNR"));
    boost::property_tree::ptree flux2_src;
    flux2_src.add("minSNR",15);
    flux2_src.put("minSNR.<xmlattr>.band","S");
    source_global.add_child("group.minSNR",flux2_src.get_child("minSNR"));
    source_global.put("group.<xmlattr>.name","global");
    source_global.put("group.<xmlattr>.members","*");
    source.add_child("group",source_global.get_child("group"));

    boost::property_tree::ptree source_group1;
    source_group1.add("group.minRepeat",3600);
    source_group1.add("group.maxScan",200);
    source_group1.add("group.minScan",20);
    source_group1.add("group.minFlux", 0.0);
    source_group1.put("group.<xmlattr>.name","group1");
    source_group1.put("group.<xmlattr>.members","2355-534,2329-384");
    source.add_child("group",source_group1.get_child("group"));

    pt.add_child("station",station);
    pt.add_child("source",source);

    boost::property_tree::ptree skyCoverage;
    skyCoverage.add("skyCoverageDistance", 30);
    skyCoverage.add("skyCoverageInterval", 3600);
    pt.add_child("skyCoverage", skyCoverage);

    boost::property_tree::ptree weightFactor;
    weightFactor.add("skyCoverage", 2);
    weightFactor.add("numberOfObservations", 1);
    weightFactor.add("duration", 1);
    weightFactor.add("averageSources", 0.0);
    weightFactor.add("averageStations", 0.1);
    pt.add_child("weightFactor", weightFactor);



    boost::property_tree::ptree obs_mode;

    obs_mode.add("bandwith",16);
    obs_mode.add("sample_rate",32);
    obs_mode.add("fanout",1);
    obs_mode.add("bits",2);

    boost::property_tree::ptree X;
    X.add("wavelength",0.0349);
    X.add("property","required");
    X.add("chanels",10);
    X.put("<xmlattr>.name","X");

    boost::property_tree::ptree S;
    S.add("wavelength",3.8000);
    S.add("property","required");
    S.add("chanels",6);
    S.put("<xmlattr>.name","S");

    obs_mode.add_child("band", X);
    obs_mode.add_child("band", S);
    pt.add_child("mode", obs_mode);

    boost::property_tree::ptree bl;
    bl.add("WETTZ13N_WETTZ13S.ignore",true);
    bl.add("WETTZ13N_WETTZELL.ignore",true);
    bl.add("WETTZ13S_WETTZELL.ignore",true);
    bl.add("HART15M_YARRA12M.minScan",10);
    bl.add("HART15M_YARRA12M.maxScan",700);
    boost::property_tree::ptree minSNR_X;
    minSNR_X.add("minSNR",10);
    minSNR_X.put("minSNR.<xmlattr>.band","X");
    boost::property_tree::ptree minSNR_S;
    minSNR_S.add("minSNR",12);
    minSNR_S.put("minSNR.<xmlattr>.band","S");
    bl.add_child("HART15M_YARRA12M.minSNR",minSNR_X.get_child("minSNR"));
    bl.add_child("HART15M_YARRA12M.minSNR",minSNR_S.get_child("minSNR"));

    pt.add_child("baseline",bl);


    boost::property_tree::ptree master;
    master.add_child("master.software", pt.get_child("software"));
    master.add_child("master.general", pt.get_child("general"));
    master.add_child("master.station", pt.get_child("station"));
    master.add_child("master.source", pt.get_child("source"));
    master.add_child("master.skyCoverage", pt.get_child("skyCoverage"));
    master.add_child("master.weightFactor", pt.get_child("weightFactor"));
    master.add_child("master.mode", pt.get_child("mode"));
    master.add_child("master.baseline",pt.get_child("baseline"));

    std::ofstream os("parameters.xml");
    boost::property_tree::xml_parser::write_xml(os, master,
                                                boost::property_tree::xml_writer_make_settings<string>('\t', 1));

}
