#include "mainwindow.h"
#include <QApplication>

#include <cstdlib>
#include "VLBI_initializer.h"
#include "VLBI_scheduler.h"

using namespace std;

void run();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    run();

//    MainWindow w;
//    w.show();

    return a.exec();
}

void run(){
    string path = "/data/VieVS/CATALOGS";

    VieVS::VLBI_initializer init;

    init.createStationsFromCatalogs(path);
    init.createSourcesFromCatalogs(path);
    init.createSkyCoverages();
    init.initializeStations();

//    init.displaySummary();


    VieVS::VLBI_scheduler scheduler(init);
    cout << "Good Bye!" << endl;

    scheduler.precalcSubnettingSrcIds();
    scheduler.start();

    cout << "Good Bye!" << endl;

}
