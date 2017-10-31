#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setApplicationName("VieVS Scheduler");
    QCoreApplication::setApplicationVersion("v0.8");
    QCoreApplication::setOrganizationName("TU Wien");
    QCoreApplication::setOrganizationDomain("http://hg.geo.tuwien.ac.at/");

    this->setWindowTitle("VieVS Scheduler");

    mainPath = QCoreApplication::applicationFilePath();

    QPushButton *savePara = new QPushButton("write xml",this);
    ui->statusBar->addPermanentWidget(savePara);
    ui->dateTimeEdit_sessionStart->setDate(QDate::currentDate());

    allStationModel = new QStandardItemModel(0,19,this);
    allSourceModel = new QStandardItemModel(0,3,this);
    allStationProxyModel = new QSortFilterProxyModel(this);
    allStationProxyModel->setSourceModel(allStationModel);
    allStationProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    allSourceProxyModel = new QSortFilterProxyModel(this);
    allSourceProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    allSourceProxyModel->setSourceModel(allSourceModel);


    selectedStationModel = new QStandardItemModel();
    selectedSourceModel = new QStandardItemModel();
    selectedBaselineModel = new QStandardItemModel();

    allSourcePlusGroupModel = new QStandardItemModel();
    allSourcePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/source_group.png"),"__all__"));

    allStationPlusGroupModel = new QStandardItemModel();
    allStationPlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/station_group_2.png"),"__all__"));

    allBaselinePlusGroupModel = new QStandardItemModel();
    allBaselinePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),"__all__"));

    allSkedModesModel = new QStringListModel();

    ui->treeView_allAvailabeStations->setModel(allStationProxyModel);
    ui->treeView_allAvailabeStations->setRootIsDecorated(false);
    ui->treeView_allAvailabeStations->setSortingEnabled(true);
    ui->treeView_allAvailabeStations->sortByColumn(0, Qt::AscendingOrder);

    ui->treeView_allAvailabeSources->setModel(allSourceProxyModel);
    ui->treeView_allAvailabeSources->setRootIsDecorated(false);
    ui->treeView_allAvailabeSources->setSortingEnabled(true);
    ui->treeView_allAvailabeSources->sortByColumn(0, Qt::AscendingOrder);

    ui->listView_allSelectedStations->setModel(selectedStationModel);
    ui->comboBox_skedObsModes->setModel(allSkedModesModel);
    ui->listView_allSelectedSources->setModel(selectedSourceModel);

    ui->comboBox_stationSettingMember->setModel(allStationPlusGroupModel);
    ui->comboBox_stationSettingMember_axis->setModel(allStationPlusGroupModel);
    ui->comboBox_stationSettingMember_wait->setModel(allStationPlusGroupModel);
    ui->comboBox_sourceSettingMember->setModel(allSourcePlusGroupModel);
    ui->comboBox_baselineSettingMember->setModel(allBaselinePlusGroupModel);

    ui->comboBox_calibratorBlock_calibratorSources->setModel(allSourcePlusGroupModel);

    ui->comboBox_setupStation->setModel(selectedStationModel);

    deleteModeMapper = new QSignalMapper(this);
    connect (deleteModeMapper, SIGNAL(mapped(QString)), this, SLOT(deleteModesCustomLine(QString))) ;
    multiSchedMapper = new QSignalMapper(this);
    connect (multiSchedMapper, SIGNAL(mapped(QString)), this, SLOT(multiSchedEditButton_clicked(QString))) ;


    connect(ui->pushButton_addGroupStationSetup, SIGNAL(clicked(bool)), this, SLOT(addGroupStation()));
    connect(ui->pushButton_addGroupStationWait, SIGNAL(clicked(bool)), this, SLOT(addGroupStation()));
    connect(ui->pushButton_addGroupStationCable, SIGNAL(clicked(bool)), this, SLOT(addGroupStation()));

    connect(ui->pushButton_addSourceGroup_Calibrator,SIGNAL(clicked(bool)), this, SLOT(addGroupSource()));
    connect(ui->pushButton_addSourceGroup_Sequence,SIGNAL(clicked(bool)), this, SLOT(addGroupSource()));
    connect(ui->pushButton_addGroupSourceSetup,SIGNAL(clicked(bool)), this, SLOT(addGroupSource()));

    connect(ui->pushButton_addGroupBaselineSetup,SIGNAL(clicked(bool)),this, SLOT(addGroupBaseline()));

    readAllSkedObsModes();
    readSkedCatalogs();
    readStations();
    readSources();

    createMultiSchedTable();
    createModesPolicyTable();
    createModesCustonBandTable();

    defaultParameters();

    ui->comboBox_stationSettingMember->installEventFilter(this);
    ui->ComboBox_parameterStation->installEventFilter(this);
    ui->comboBox_setupStation->installEventFilter(this);

    ui->comboBox_sourceSettingMember->installEventFilter(this);
    ui->ComboBox_parameterSource->installEventFilter(this);
    ui->comboBox_setupSource->installEventFilter(this);

    ui->comboBox_baselineSettingMember->installEventFilter(this);
    ui->ComboBox_parameterBaseline->installEventFilter(this);
    ui->comboBox_setupBaseline->installEventFilter(this);


    ui->treeWidget_2->expandAll();

    setupStation = new QChartView;
    setupSource = new QChartView;
    setupBaseline = new QChartView;
    prepareSetupPlot(setupStation, ui->verticalLayout_28);
    prepareSetupPlot(setupSource, ui->verticalLayout_36);
    prepareSetupPlot(setupBaseline, ui->verticalLayout_40);
    ui->comboBox_setupSource->setModel(selectedSourceModel);
    ui->comboBox_setupBaseline->setModel(selectedBaselineModel);

    ui->splitter->setStretchFactor(0,1);
    ui->splitter->setStretchFactor(1,3);
    ui->splitter_4->setStretchFactor(1,1);

    ui->spinBox_fontSize->setValue(QApplication::font().pointSize());
    ui->iconSizeSpinBox->setValue(ui->fileToolBar->iconSize().width());

    auto hv1 = ui->treeWidget_setupStationWait->header();
    hv1->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto hv2 = ui->treeWidget_setupStationAxis->header();
    hv2->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->pushButton_setupAxisAdd,SIGNAL(clicked(bool)),this,SLOT(setupStationAxisBufferAddRow()));
    connect(ui->pushButton_setupWaitAdd,SIGNAL(clicked(bool)),this,SLOT(setupStationWaitAddRow()));

    setupStationAxisBufferAddRow();
    setupStationWaitAddRow();

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event){
    if(obj == ui->comboBox_stationSettingMember){
        if(event->type() == QEvent::Enter){
            QString name = ui->comboBox_stationSettingMember->currentText();
            displayStationSetupMember(name);
        }
    }else if(obj == ui->ComboBox_parameterStation){
        if(event->type() == QEvent::Enter){
            QString name = ui->ComboBox_parameterStation->currentText();
            displayStationSetupParameter(name);
        }
    }else if(obj == ui->comboBox_setupStation){
        if(event->type() == QEvent::Enter){
            QString name = ui->comboBox_setupStation->currentText();
            if(!name.isEmpty()){
                displayStationSetupMember(name);
            }
        }
    }else if(obj == ui->comboBox_sourceSettingMember){
        if(event->type() == QEvent::Enter){
            QString name = ui->comboBox_sourceSettingMember->currentText();
            displaySourceSetupMember(name);
        }
    }else if(obj == ui->ComboBox_parameterSource){
        if(event->type() == QEvent::Enter){
            QString name = ui->ComboBox_parameterSource->currentText();
            displaySourceSetupParameter(name);
        }
    }else if(obj == ui->comboBox_setupSource){
        if(event->type() == QEvent::Enter){
            QString name = ui->comboBox_setupSource->currentText();
            if(!name.isEmpty()){
                displaySourceSetupMember(name);
            }
        }
    }else if(obj == ui->comboBox_baselineSettingMember){
        if(event->type() == QEvent::Enter){
            QString name = ui->comboBox_baselineSettingMember->currentText();
            displayBaselineSetupMember(name);
        }
    }else if(obj == ui->ComboBox_parameterBaseline){
        if(event->type() == QEvent::Enter){
            QString name = ui->ComboBox_parameterBaseline->currentText();
            displayBaselineSetupParameter(name);
        }
    }else if(obj == ui->comboBox_setupBaseline){
        if(event->type() == QEvent::Enter){
            QString name = ui->comboBox_setupBaseline->currentText();
            if(!name.isEmpty()){
                displayBaselineSetupMember(name);
            }
        }
    }
}

void MainWindow::displayStationSetupMember(QString name)
{
    auto t = ui->tableWidget_setupStation;
    t->clear();
    auto groups = para.getGroupStations();
    t->setColumnCount(1);
    if (name == "__all__"){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Group: %1").arg(name)));
        t->setRowCount(selectedStationModel->rowCount());
        for(int i=0; i<selectedStationModel->rowCount(); ++i){
            QString txt = selectedStationModel->index(i,0).data().toString();
            t->setItem(i,0,new QTableWidgetItem(txt));
        }
    }else if(groups.find(name.toStdString()) != groups.end()){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Group: %1").arg(name)));
        auto members = groups.at(name.toStdString());
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = QString::fromStdString(members.at(i));
            t->setItem(i,0,new QTableWidgetItem(txt));
        }
    }else{
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Station: %1").arg(name)));
        t->setRowCount(allStationModel->columnCount());
        QList<QStandardItem *> litm = allStationModel->findItems(name);
        QStandardItem * itm = litm.at(0);
        int row = itm->row();
        for(int i=0; i<allStationModel->columnCount(); ++i){
            QString txt = allStationModel->headerData(i,Qt::Horizontal).toString();
            QString txt2 = allStationModel->item(row,i)->text();
            t->setVerticalHeaderItem(i,new QTableWidgetItem(txt));
            t->setItem(i,0,new QTableWidgetItem(txt2));
        }
    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::displaySourceSetupMember(QString name)
{
    auto t = ui->tableWidget_setupSource;
    t->clear();
    auto groups = para.getGroupSources();
    t->setColumnCount(1);
    if (name == "__all__"){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Group: %1").arg(name)));
        t->setRowCount(selectedSourceModel->rowCount());
        for(int i=0; i<selectedSourceModel->rowCount(); ++i){
            QString txt = selectedSourceModel->index(i,0).data().toString();
            t->setItem(i,0,new QTableWidgetItem(txt));
        }
    }else if(groups.find(name.toStdString()) != groups.end()){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Group: %1").arg(name)));
        auto members = groups.at(name.toStdString());
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = QString::fromStdString(members.at(i));
            t->setItem(i,0,new QTableWidgetItem(txt));
        }
    }else{
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Source: %1").arg(name)));
        t->setRowCount(allSourceModel->columnCount());
        QList<QStandardItem *> litm = allSourceModel->findItems(name);
        QStandardItem * itm = litm.at(0);
        int row = itm->row();
        for(int i=0; i<allSourceModel->columnCount(); ++i){
            QString txt = allSourceModel->headerData(i,Qt::Horizontal).toString();
            QString txt2 = allSourceModel->item(row,i)->text();
            t->setVerticalHeaderItem(i,new QTableWidgetItem(txt));
            t->setItem(i,0,new QTableWidgetItem(txt2));
        }
    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::displayBaselineSetupMember(QString name)
{
    auto t = ui->tableWidget_setupBaseline;
    t->clear();
    auto groups = para.getGroupBaselines();
    t->setColumnCount(1);
    if (name == "__all__"){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Group: %1").arg(name)));
        t->setRowCount(selectedBaselineModel->rowCount());
        for(int i=0; i<selectedBaselineModel->rowCount(); ++i){
            QString txt = selectedBaselineModel->index(i,0).data().toString();
            t->setItem(i,0,new QTableWidgetItem(txt));
        }
    }else if(groups.find(name.toStdString()) != groups.end()){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Group: %1").arg(name)));
        auto members = groups.at(name.toStdString());
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = QString::fromStdString(members.at(i));
            t->setItem(i,0,new QTableWidgetItem(txt));
        }
    }else{
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Baseline: %1").arg(name)));
        t->setRowCount(2);
        QStringList list= name.split("-");
        t->setItem(0,0,new QTableWidgetItem(list.at(0)));
        t->setItem(0,1,new QTableWidgetItem(list.at(1)));
        t->setVerticalHeaderItem(0,new QTableWidgetItem("Station 1"));
        t->setVerticalHeaderItem(1,new QTableWidgetItem("Station 2"));
    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}



void MainWindow::displayStationSetupParameter(QString name)
{
    auto t = ui->tableWidget_setupStation;
    t->clear();
    t->setColumnCount(1);
    t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Parameter: %1").arg(name)));
    auto paraSta = para.getParaStations();
    VieVS::ParameterSettings::ParametersStations para = paraSta.at(name.toStdString());
    t->setRowCount(0);
    int r = 0;
    if(para.available.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.available ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("available"));
        ++r;
    }
    if(para.tagalong.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.tagalong ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("tagalong"));
        ++r;
    }
    if(para.maxScan.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxScan)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max scan time [s]"));
        ++r;
     }
    if(para.minScan.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minScan)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min scan time [s]"));
        ++r;
    }
    if(para.maxSlewtime.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxSlewtime)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max slew time [s]"));
        ++r;
    }
    if(para.maxWait.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxWait)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max wait time [s]"));
        ++r;
    }
    if(para.weight.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.weight)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("weight"));
        ++r;
    }
    if(para.minSNR.size() >0 ){
        for(const auto& any: para.minSNR){
            t->insertRow(r);
            t->setItem(r,0,new QTableWidgetItem(QString::number(any.second)));
            QString txt = QString("min SNR %1").arg(QString::fromStdString(any.first));
            t->setVerticalHeaderItem(r,new QTableWidgetItem(txt));
            ++r;
        }
    }
    if(para.ignoreSources_str.size() > 0){
        std::string txt = "";
        for(const auto &any: para.ignoreSources_str){
            txt.append(any).append(", ");
        }
        txt = txt.substr(0,txt.size()-2);
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::fromStdString(txt)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore sources"));
        ++r;

    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::displaySourceSetupParameter(QString name){
    auto t = ui->tableWidget_setupSource;
    t->clear();
    t->setColumnCount(1);
    t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Parameter: %1").arg(name)));
    auto paraSrc = para.getParaSources();
    VieVS::ParameterSettings::ParametersSources para = paraSrc.at(name.toStdString());
    t->setRowCount(0);
    int r = 0;
    if(para.available.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.available ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("available"));
        ++r;
    }
    if(para.minNumberOfStations.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minNumberOfStations)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min number of stations"));
        ++r;
    }
    if(para.maxScan.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxScan)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max scan time [s]"));
        ++r;
     }
    if(para.minScan.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minScan)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min scan time [s]"));
        ++r;
    }
    if(para.maxNumberOfScans.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxNumberOfScans)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max number of scans"));
        ++r;
    }
    if(para.minFlux.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minFlux)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min flux density [Jy]"));
        ++r;
    }
    if(para.minRepeat.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minRepeat)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min time between two scans [s]"));
        ++r;
    }
    if(para.weight.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.weight)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("weight"));
        ++r;
    }
    if(para.tryToFocusIfObservedOnce.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.tryToFocusIfObservedOnce ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("available"));
        ++r;
    }
    if(para.tryToObserveXTimesEvenlyDistributed.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.tryToObserveXTimesEvenlyDistributed)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("eavenly distributed scans over time"));
        ++r;
    }
    if(para.minSNR.size() >0 ){
        for(const auto& any: para.minSNR){
            t->insertRow(r);
            t->setItem(r,0,new QTableWidgetItem(QString::number(any.second)));
            QString txt = QString("min SNR %1").arg(QString::fromStdString(any.first));
            t->setVerticalHeaderItem(r,new QTableWidgetItem(txt));
            ++r;
        }
    }
    if(para.ignoreStationsString.size() > 0){
        std::string txt = "";
        for(const auto &any: para.ignoreStationsString){
            txt.append(any).append(", ");
        }
        txt = txt.substr(0,txt.size()-2);
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::fromStdString(txt)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore stations"));
        ++r;
    }
    if(para.requiredStationsString.size() > 0){
        std::string txt = "";
        for(const auto &any: para.requiredStationsString){
            txt.append(any).append(", ");
        }
        txt = txt.substr(0,txt.size()-2);
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::fromStdString(txt)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("required stations"));
        ++r;
    }
    if(para.ignoreBaselinesString.size() > 0){
        std::string txt = "";
        for(const auto &any: para.ignoreBaselinesString){
            std::string bl = any.first;
            bl.append("-").append(any.second);
            txt.append(bl).append(", ");
        }
        txt = txt.substr(0,txt.size()-2);
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::fromStdString(txt)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("ignored baselines"));
        ++r;
    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::displayBaselineSetupParameter(QString name)
{
    auto t = ui->tableWidget_setupBaseline;
    t->clear();
    t->setColumnCount(1);
    t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Parameter: %1").arg(name)));
    auto paraSta = para.getParaBaselines();
    VieVS::ParameterSettings::ParametersBaselines para = paraSta.at(name.toStdString());
    t->setRowCount(0);
    int r = 0;
    if(para.ignore.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.ignore ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("available"));
        ++r;
    }
    if(para.maxScan.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxScan)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max scan time [s]"));
        ++r;
     }
    if(para.minScan.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minScan)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min scan time [s]"));
        ++r;
    }
    if(para.weight.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.weight)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("weight"));
        ++r;
    }
    if(para.minSNR.size() >0 ){
        for(const auto& any: para.minSNR){
            t->insertRow(r);
            t->setItem(r,0,new QTableWidgetItem(QString::number(any.second)));
            QString txt = QString("min SNR %1").arg(QString::fromStdString(any.first));
            t->setVerticalHeaderItem(r,new QTableWidgetItem(txt));
            ++r;
        }
    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}


void MainWindow::on_actionWelcome_triggered()
{
    ui->main_stacked->setCurrentIndex(0);
}

void MainWindow::on_actionInput_triggered()
{
    ui->main_stacked->setCurrentIndex(1);
}

void MainWindow::on_actionMode_triggered()
{
    ui->main_stacked->setCurrentIndex(2);
}


void MainWindow::on_actionGeneral_triggered()
{
    ui->main_stacked->setCurrentIndex(3);
}


void MainWindow::on_actionStation_triggered()
{
    ui->main_stacked->setCurrentIndex(4);
}

void MainWindow::on_actionSource_triggered()
{
    ui->main_stacked->setCurrentIndex(5);
}

void MainWindow::on_actionBaseline_triggered()
{
    ui->main_stacked->setCurrentIndex(6);
}

void MainWindow::on_actionWeight_factors_triggered()
{
    ui->main_stacked->setCurrentIndex(7);
}

void MainWindow::on_actionRules_triggered()
{
    ui->main_stacked->setCurrentIndex(8);
}

void MainWindow::on_actionMulti_Scheduling_triggered()
{
    ui->main_stacked->setCurrentIndex(9);
}

void MainWindow::on_actionSky_Coverage_triggered()
{
    ui->main_stacked->setCurrentIndex(10);
}

void MainWindow::on_actionOutput_triggered()
{
    ui->main_stacked->setCurrentIndex(11);
}

void MainWindow::on_actionSettings_triggered()
{
    ui->main_stacked->setCurrentIndex(12);
}

void MainWindow::on_actionWhat_is_this_triggered()
{
    QWhatsThis::enterWhatsThisMode();
}


void MainWindow::on_pushButton_browseAntenna_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathAntenna->setText(path);
    }
}

void MainWindow::on_pushButton_browseEquip_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathEquip->setText(path);
    }
}

void MainWindow::on_pushButton_browsePosition_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathPosition->setText(path);
    }
}

void MainWindow::on_pushButton_browseMask_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathMask->setText(path);
    }
}

void MainWindow::on_pushButton_browseSource_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathSource->setText(path);
    }
}

void MainWindow::on_pushButton_browseFlux_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathFlux->setText(path);
    }
}

void MainWindow::on_pushButton_browsModes_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathModes->setText(path);
    }
}

void MainWindow::on_pushButton_browseFreq_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathFreq->setText(path);
    }
}

void MainWindow::on_pushButton_browseTracks_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathTracks->setText(path);
    }
}

void MainWindow::on_pushButton_browseLoif_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathLoif->setText(path);
    }
}

void MainWindow::on_pushButton_browseRec_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathRec->setText(path);
    }
}

void MainWindow::on_pushButton_browseRx_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathRx->setText(path);
    }
}

void MainWindow::on_pushButton_browseHdpos_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", mainPath);
    if( !path.isEmpty() ){
        ui->lineEdit_pathHdpos->setText(path);
    }
}

void MainWindow::readStations()
{
    QString antennaPath = ui->lineEdit_pathAntenna->text();
    QString equipPath = ui->lineEdit_pathEquip->text();
    QString positionPath = ui->lineEdit_pathPosition->text();
    QMap<QString,QStringList > antennaMap;
    QMap<QString,QStringList > equipMap;
    QMap<QString,QStringList > positionMap;

    allStationModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    allStationModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Id"));
    allStationModel->setHeaderData(2, Qt::Horizontal, QObject::tr("lat [deg]"));
    allStationModel->setHeaderData(3, Qt::Horizontal, QObject::tr("lon [deg]"));
    allStationModel->setHeaderData(4, Qt::Horizontal, QObject::tr("diam [m]"));
    allStationModel->setHeaderData(5, Qt::Horizontal, QObject::tr("SEFD X [Jy]"));
    allStationModel->setHeaderData(6, Qt::Horizontal, QObject::tr("SEFD S [Jy]"));
    allStationModel->setHeaderData(7, Qt::Horizontal, QObject::tr("axis offset [m]"));
    allStationModel->setHeaderData(8, Qt::Horizontal, QObject::tr("slew rate1 [deg/min]"));
    allStationModel->setHeaderData(9, Qt::Horizontal, QObject::tr("constant overhead1 [sec]"));
    allStationModel->setHeaderData(10, Qt::Horizontal, QObject::tr("lower axis limit1 [deg]"));
    allStationModel->setHeaderData(11, Qt::Horizontal, QObject::tr("upper axis limit1 [deg]"));
    allStationModel->setHeaderData(12, Qt::Horizontal, QObject::tr("slew rate2 [deg/min]"));
    allStationModel->setHeaderData(13, Qt::Horizontal, QObject::tr("constant overhead2 [sec]"));
    allStationModel->setHeaderData(14, Qt::Horizontal, QObject::tr("lower axis limit2 [deg]"));
    allStationModel->setHeaderData(15, Qt::Horizontal, QObject::tr("upper axis limit2 [deg]"));
    allStationModel->setHeaderData(16, Qt::Horizontal, QObject::tr("x [m]"));
    allStationModel->setHeaderData(17, Qt::Horizontal, QObject::tr("y [m]"));
    allStationModel->setHeaderData(18, Qt::Horizontal, QObject::tr("z [m]"));

    QFile antennaFile(antennaPath);
    if (antennaFile.open(QIODevice::ReadOnly)){
        QTextStream in(&antennaFile);
        while (!in.atEnd()){
            QString line = in.readLine();
            if(line.isEmpty() || line[0] == "*" || line[0] == "!" || line[0] == "&"){
                continue;
            }
            QStringList split = line.split(" ",QString::SplitBehavior::SkipEmptyParts);
            QString antennaName = split[1];
            antennaMap.insert(antennaName,split);
        }
        antennaFile.close();
    }

    QFile equipFile(equipPath);
    if (equipFile.open(QIODevice::ReadOnly)){
        QTextStream in(&equipFile);
        while (!in.atEnd()){
            QString line = in.readLine();
            if(line.isEmpty() || line[0] == "*" || line[0] == "!" || line[0] == "&"){
                continue;
            }
            QStringList split = line.split(" ",QString::SplitBehavior::SkipEmptyParts);
            QString equipName = split[1] + "|" + split[0];
            equipName = equipName.toUpper();
            equipMap.insert(equipName,split);
        }
        equipFile.close();
    }

    QFile positionFile(positionPath);
    if (positionFile.open(QIODevice::ReadOnly)){
        QTextStream in(&positionFile);
        while (!in.atEnd()){
            QString line = in.readLine();
            if(line.isEmpty() || line[0] == "*" || line[0] == "!" || line[0] == "&"){
                continue;
            }
            QStringList split = line.split(" ",QString::SplitBehavior::SkipEmptyParts);
            QString positionName = split[0];
            positionName = positionName.toUpper();
            positionMap.insert(positionName,split);
        }
        positionFile.close();
    }

    QMap<QString, QStringList>::iterator i;
    for (i = antennaMap.begin(); i != antennaMap.end(); ++i){
        try{
            QString antName = i.key();
            QStringList antList = i.value();

            QString antId,eqKey,posKey;
            double offset, rate1, con1, axis1_low, axis1_up, rate2, con2, axis2_low, axis2_up, diam;
            if(antList.size()>14){
                antId = antList.at(13);

                offset = antList.at(3).toDouble();
                rate1 = antList.at(4).toDouble();
                con1 = antList.at(5).toDouble();
                axis1_low = antList.at(6).toDouble();
                axis1_up = antList.at(7).toDouble();
                rate2 = antList.at(8).toDouble();
                con2 = antList.at(9).toDouble();
                axis2_low = antList.at(10).toDouble();
                axis2_up = antList.at(11).toDouble();
                diam = antList.at(12).toDouble();

                eqKey = antList.at(14) + "|" +antName;
                eqKey = eqKey.toUpper();
                posKey = antList.at(13);
                posKey = posKey.toUpper();
            }else{
                continue;
            }

            QStringList eqList = equipMap.value(eqKey);
            double SEFD_X, SEFD_S;
            if(eqList.size()>8){
                SEFD_X = 0;
                if(eqList.at(5) == "X"){
                    SEFD_X = eqList.at(6).toDouble();
                }

                SEFD_S = 0;
                if(eqList.at(7) == "S"){
                    SEFD_S = eqList.at(8).toDouble();
                }
            }else{
                continue;
            }

            QStringList posList = positionMap.value(posKey);
            double x, y, z, lon, lat, h;
            if(posList.size()>8){
                x = posList.at(2).toDouble();
                y = posList.at(3).toDouble();
                z = posList.at(4).toDouble();

                double a = 6378136.6;
                double f = 1/298.25642;
                double e2 = 2*f-f*f;

                lon = atan2(y,x);
                double r = sqrt(x*x+y*y);
                lat = atan2(z,r);

                for(int i=0; i<6; ++i){
                    double N=a/sqrt(1-e2*sin(lat)*sin(lat));
                    h=r/cos(lat)-N;
                    lat=atan2(z*(N+h),r*((1-e2)*N+h));
                }
            }else{
                continue;
            }

            allStationModel->insertRow(0);
            allStationModel->setData(allStationModel->index(0, 0), antName);
            allStationModel->item(0,0)->setIcon(QIcon(":/icons/icons/station.png"));
            allStationModel->setData(allStationModel->index(0, 1), antId);
            allStationModel->setData(allStationModel->index(0, 2), (double)((int)(qRadiansToDegrees(lat)*100 +0.5))/100.0);
            allStationModel->setData(allStationModel->index(0, 3), (double)((int)(qRadiansToDegrees(lon)*100 +0.5))/100.0);
            allStationModel->setData(allStationModel->index(0, 4), (double)((int)(diam*10 +0.5))/10.0);

            allStationModel->setData(allStationModel->index(0, 5), SEFD_X);
            allStationModel->setData(allStationModel->index(0, 6), SEFD_S);

            allStationModel->setData(allStationModel->index(0, 7), offset);

            allStationModel->setData(allStationModel->index(0, 8), rate1);
            allStationModel->setData(allStationModel->index(0, 9), con1);
            allStationModel->setData(allStationModel->index(0, 10), axis1_low);
            allStationModel->setData(allStationModel->index(0, 11), axis1_up);

            allStationModel->setData(allStationModel->index(0, 12), rate2);
            allStationModel->setData(allStationModel->index(0, 13), con2);
            allStationModel->setData(allStationModel->index(0, 14), axis2_low);
            allStationModel->setData(allStationModel->index(0, 15), axis2_up);


            allStationModel->setData(allStationModel->index(0, 16), x);
            allStationModel->setData(allStationModel->index(0, 17), y);
            allStationModel->setData(allStationModel->index(0, 18), z);

        }catch(...){

        }
    }

    for(int i=0; i<19; ++i){
        ui->treeView_allAvailabeStations->resizeColumnToContents(i);
    }

    plotWorldMap();
    worldMapCallout = new Callout(worldmap->chart());
    worldMapCallout->hide();
}

void MainWindow::readSources()
{
    QString sourcePath = ui->lineEdit_pathSource->text();


    allSourceModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    allSourceModel->setHeaderData(1, Qt::Horizontal, QObject::tr("RA [deg]"));
    allSourceModel->setHeaderData(2, Qt::Horizontal, QObject::tr("DC [deg]"));

    QFile sourceFile(sourcePath);
    if (sourceFile.open(QIODevice::ReadOnly)){
        QTextStream in(&sourceFile);
        while (!in.atEnd()){
            QString line = in.readLine();
            if(line.isEmpty() || line[0] == "*" || line[0] == "!" || line[0] == "&"){
                continue;
            }
            QStringList split = line.split(" ",QString::SplitBehavior::SkipEmptyParts);
            QString sourceName = split.at(0);
            QString rah = split.at(2);
            QString ram = split.at(3);
            QString ras = split.at(4);
            double ra = (rah.toDouble() + ram.toDouble()/60 + ras.toDouble()/3600)*15;
            QString ded = split.at(5);
            QString dem = split.at(6);
            QString des = split.at(7);
            double de = ded.toDouble() + dem.toDouble()/60 + des.toDouble()/3600;

            allSourceModel->insertRow(0);
            allSourceModel->setData(allSourceModel->index(0,0), sourceName);
            allSourceModel->item(0,0)->setIcon(QIcon(":/icons/icons/source.png"));
            allSourceModel->setData(allSourceModel->index(0, 1), (double)((int)(ra*100 +0.5))/100.0);
            allSourceModel->setData(allSourceModel->index(0, 2), (double)((int)(de*100 +0.5))/100.0);

            selectedSourceModel->insertRow(0);
            selectedSourceModel->setItem(0,new QStandardItem(QIcon(":/icons/icons/source.png"),sourceName));
            selectedSourceModel->sort(0);


            auto map = para.getGroupSources();
            int r = 0;
            for(int i = 0; i<allSourcePlusGroupModel->rowCount(); ++i){
                QString txt = allSourcePlusGroupModel->item(i)->text();
                if(map.find(txt.toStdString()) != map.end() || txt == "__all__"){
                    ++r;
                    continue;
                }
                if(txt>sourceName){
                    break;
                }else{
                    ++r;
                }
            }

            allSourcePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/source.png"),sourceName));
        }
        sourceFile.close();
    }
    plotSkyMap();
    skyMapCallout = new Callout(skymap->chart());
    skyMapCallout->hide();

}

void MainWindow::readAllSkedObsModes()
{
    QString modesPath = ui->lineEdit_pathModes->text();
    QFile modesFile(modesPath);
    QStringList modes;
    if (modesFile.open(QIODevice::ReadOnly)){
        QTextStream in(&modesFile);
        while (!in.atEnd()){
            QString line = in.readLine();
            if(line.isEmpty() || line[0] == "*" || line[0] == "!" || line[0] == "&"){
                continue;
            }
            QStringList split = line.split(" ",QString::SplitBehavior::SkipEmptyParts);
            QString obsModeName = split[0];

            modes << obsModeName;
        }
        modesFile.close();
    }
    allSkedModesModel->setStringList(modes);
    if(ui->comboBox_skedObsModes->count()>0){
        ui->comboBox_skedObsModes->setCurrentIndex(0);
    }
}

void MainWindow::plotWorldMap()
{
    ui->pushButton_worldmapZoomFull->setSizeIncrement(45,45);
    ui->pushButton_worldmapZoomFull->setIconSize(ui->pushButton_worldmapZoomFull->size());
    QChart *worldChart = new QChart();
    worldChart->setAcceptHoverEvents(true);


    QFile coastF(":/plotting/coast.txt");
    if (coastF.open(QIODevice::ReadOnly)){
        QTextStream in(&coastF);

        int c = 0;
        while (!in.atEnd()){

            QLineSeries *coast = new QLineSeries(worldChart);
            coast->setColor(Qt::gray);
            coast->setName("coast" + QString::number(c));

            while(!in.atEnd()){
                QString line = in.readLine();

                if(line == "NaN,NaN"){
                    ++c;
                    worldChart->addSeries(coast);
                    break;
                }
                QStringList split = line.split(",",QString::SplitBehavior::SkipEmptyParts);
                QString lat = split[0];
                QString lon = split[1];
                coast->append(lon.toDouble(),lat.toDouble());
            }
        }
        coastF.close();
    }

    availableStations = new QScatterSeries(worldChart);
    availableStations->setColor(Qt::red);
    availableStations->setMarkerSize(10);

    selectedStations = new QScatterSeries(worldChart);
    QImage img(":/icons/icons/station_white.png");
    img = img.scaled(30,30);
    selectedStations->setBrush(QBrush(img));
    selectedStations->setMarkerSize(30);
    selectedStations->setPen(QColor(Qt::transparent));
//    selectedStations->setColor(Qt::darkGreen);
//    selectedStations->setMarkerSize(15);

    worldChart->addSeries(availableStations);
    worldChart->addSeries(selectedStations);

    connect(availableStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));
    connect(availableStations,SIGNAL(clicked(QPointF)),this,SLOT(worldmap_clicked(QPointF)));
    connect(selectedStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));


    for(int row = 0; row<allStationModel->rowCount(); ++row){
        double lat = allStationModel->index(row,2).data().toDouble();
        double lon = allStationModel->index(row,3).data().toDouble();
        availableStations->append(lon,lat);
    }

    worldChart->createDefaultAxes();
    worldChart->setAcceptHoverEvents(true);
    worldChart->legend()->hide();
    worldChart->axisX()->setRange(-180,180);
    worldChart->axisY()->setRange(-90,90);
    worldChart->setAnimationOptions(QChart::NoAnimation);

    worldmap = new ChartView(worldChart);
    worldmap->setMinMax(-180,180,-90,90);
    worldmap->setRenderHint(QPainter::Antialiasing);
    worldmap->setFrameStyle(QFrame::Raised | QFrame::StyledPanel);
    worldmap->setBackgroundBrush(QBrush(Qt::white));
    worldmap->setMouseTracking(true);

    ui->verticalLayout_worldmap->addWidget(worldmap,8);

}

void MainWindow::plotSkyMap(){
    ui->pushButton_skymapZoomFull->setSizeIncrement(45,45);
    ui->pushButton_skymapZoomFull->setIconSize(ui->pushButton_skymapZoomFull->size());
    QChart *skyChart = new QChart();

    for(int ra = -180; ra<=180; ra+=60){
        QLineSeries *ral = new QLineSeries(skyChart);
        ral->setColor(Qt::gray);
        double lambda = qDegreesToRadians((double) ra);

        for(int de = -90; de<=90; de+=5){
            double phi = qDegreesToRadians((double) de);
            double hn = qSqrt( 1 + qCos(phi)*qCos(lambda/2) );

            double x = (2 * qSqrt(2) *qCos(phi) *qSin(lambda/2) ) / hn;
            double y = (qSqrt(2) *qSin(phi) ) / hn;
            ral->append(x,y);
        }
        skyChart->addSeries(ral);
    }

    for(int de = -60; de<=60; de+=30){
        QLineSeries *del = new QLineSeries(skyChart);
        del->setColor(Qt::gray);
        double phi = qDegreesToRadians((double) de);

        for(int ra = -180; ra<=180; ra+=5){
            double lambda = qDegreesToRadians((double) ra);
            double hn = qSqrt( 1 + qCos(phi)*qCos(lambda/2) );

            double x = (2 * qSqrt(2) *qCos(phi) *qSin(lambda/2) ) / hn;
            double y = (qSqrt(2) *qSin(phi) ) / hn;
            del->append(x,y);
        }
        skyChart->addSeries(del);
    }


    availableSources = new QScatterSeries(skyChart);
    availableSources->setColor(Qt::red);
    availableSources->setMarkerSize(10);

    selectedSources = new QScatterSeries(skyChart);
    selectedSources->setColor(Qt::darkGreen);
    selectedSources->setMarkerSize(12);

    skyChart->addSeries(availableSources);
    skyChart->addSeries(selectedSources);

    connect(availableSources,SIGNAL(hovered(QPointF,bool)),this,SLOT(skymap_hovered(QPointF,bool)));
    connect(selectedSources,SIGNAL(hovered(QPointF,bool)),this,SLOT(skymap_hovered(QPointF,bool)));

    for(int i = 0; i< allSourceModel->rowCount(); ++i){
        double ra = allSourceModel->item(i,1)->text().toDouble();
        ra -=180;
        double lambda = qDegreesToRadians(ra);

        double dc = allSourceModel->item(i,2)->text().toDouble();
        double phi = qDegreesToRadians(dc);

        double hn = qSqrt( 1 + qCos(phi)*qCos(lambda/2) );

        double x = (2 * qSqrt(2) *qCos(phi) *qSin(lambda/2) ) / hn;
        double y = (qSqrt(2) *qSin(phi) ) / hn;

        availableSources->append(x,y);
        selectedSources->append(x,y);
    }


    skyChart->createDefaultAxes();
    skyChart->setAcceptHoverEvents(true);
    skyChart->legend()->hide();
    skyChart->axisX()->setRange(-2.85,2.85);
    skyChart->axisY()->setRange(-1.45,1.45);
    skyChart->axisX()->hide();
    skyChart->axisY()->hide();
    skyChart->setAnimationOptions(QChart::NoAnimation);

    skymap = new ChartView(skyChart);
    skymap->setMinMax(-2.85,2.85,-1.45,1.45);
    skymap->setRenderHint(QPainter::Antialiasing);
    skymap->setFrameStyle(QFrame::Raised | QFrame::StyledPanel);
    skymap->setBackgroundBrush(QBrush(Qt::white));
    skymap->setMouseTracking(true);


    ui->verticalLayout_skymap->addWidget(skymap,8);
}



void MainWindow::defaultParameters()
{
    VieVS::ParameterSettings::ParametersStations sta;
    sta.available = true;
    sta.maxScan = 600;
    sta.minScan = 20;
    sta.maxSlewtime = 600;
    sta.maxWait = 600;
    sta.weight = 1;

    VieVS::ParameterSettings::ParametersSources src;
    src.available = true;
    src.minRepeat = 1800;
    src.minScan = 20;
    src.maxScan = 600;
    src.weight = 1;
    src.maxNumberOfScans = 999;

    VieVS::ParameterSettings::ParametersBaselines bl;
    bl.ignore = false;
    bl.maxScan = 600;
    bl.minScan = 20;
    bl.weight = 1;


    para.parameters("default",sta);
    ui->ComboBox_parameterStation->addItem("default");

    para.parameters("default",src);
    ui->ComboBox_parameterSource->addItem("default");

    para.parameters("default",bl);
    ui->ComboBox_parameterBaseline->addItem("default");

    QDateTime e = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    QTreeWidgetItem *wsta = new QTreeWidgetItem();
    wsta->setText(0,"__all__");
    wsta->setText(1,"default");
    wsta->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
    wsta->setText(3,e.toString("dd.MM.yyyy hh:mm"));
    wsta->setText(4,"hard");
    wsta->setIcon(0,QIcon(":/icons/icons/station_group_2.png"));
    ui->treeWidget_setupStation->insertTopLevelItem(0,wsta);

    QTreeWidgetItem *wsrc = new QTreeWidgetItem();
    wsrc->setText(0,"__all__");
    wsrc->setText(1,"default");
    wsrc->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
    wsrc->setText(3,e.toString("dd.MM.yyyy hh:mm"));
    wsrc->setText(4,"hard");
    wsrc->setIcon(0,QIcon(":/icons/icons/source_group.png"));
    ui->treeWidget_setupSource->insertTopLevelItem(0,wsrc);

    QTreeWidgetItem *wbl = new QTreeWidgetItem();
    wbl->setText(0,"__all__");
    wbl->setText(1,"default");
    wbl->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
    wbl->setText(3,e.toString("dd.MM.yyyy hh:mm"));
    wbl->setText(4,"hard");
    wbl->setIcon(0,QIcon(":/icons/icons/baseline_group.png"));
    ui->treeWidget_setupBaseline->insertTopLevelItem(0,wbl);

    QHeaderView * hvsta = ui->treeWidget_setupStation->header();
    hvsta->setSectionResizeMode(QHeaderView::ResizeToContents);
    QHeaderView * hvsrc = ui->treeWidget_setupSource->header();
    hvsrc->setSectionResizeMode(QHeaderView::ResizeToContents);
    QHeaderView * hvbl = ui->treeWidget_setupBaseline->header();
    hvbl->setSectionResizeMode(QHeaderView::ResizeToContents);

    std::string parameterName = "default";
    std::string member = "__all__";
    QDateTime sessionStart = ui->dateTimeEdit_sessionStart->dateTime();
    unsigned int startt = sessionStart.secsTo(ui->DateTimeEdit_startParameterStation->dateTime());
    unsigned int endt = sessionStart.secsTo(ui->DateTimeEdit_endParameterStation->dateTime());

    setupStationTree = VieVS::ParameterSetup(parameterName,
                                  member,
                                  startt,
                                  endt,
                                  VieVS::ParameterSetup::Transition::hard);

    setupSourceTree = VieVS::ParameterSetup(parameterName,
                                  member,
                                  startt,
                                  endt,
                                  VieVS::ParameterSetup::Transition::hard);

    setupBaselineTree = VieVS::ParameterSetup(parameterName,
                                  member,
                                  startt,
                                  endt,
                                  VieVS::ParameterSetup::Transition::hard);


}

void MainWindow::on_listView_allSelectedStations_clicked(const QModelIndex &index)
{

    QString name = selectedStationModel->item(index.row())->text();
    selectedStationModel->removeRow(index.row());

    int row;
    double x;
    double y;
    for(int i = 0; i < allStationModel->rowCount(); ++i){
        if (allStationModel->index(i,0).data().toString() == name){
            row = i;
            x = allStationModel->index(row,3).data().toDouble();
            y = allStationModel->index(row,2).data().toDouble();
            break;
        }
    }

    for(int i = 0; i<selectedStations->count(); ++i){
        double xn = selectedStations->at(i).x();
        double yn = selectedStations->at(i).y();
        if( (x-xn)*(x-xn) + (y-yn)*(y-yn) < 1e-3 ){
            selectedStations->remove(i);
            break;
        }
    }

    for(int i = 0; i<allStationPlusGroupModel->rowCount(); ++i){
        if (allStationPlusGroupModel->index(i,0).data().toString() == name) {
            allStationPlusGroupModel->removeRow(i);
            break;
        }
    }

    createBaselineModel();

}

void MainWindow::on_listView_allSelectedSources_clicked(const QModelIndex &index)
{
    QString name = selectedSourceModel->item(index.row())->text();
    selectedSourceModel->removeRow(index.row());

    int row;
    double x;
    double y;
    for(int i = 0; i < allSourceModel->rowCount(); ++i){
        if (allSourceModel->index(i,0).data().toString() == name){
            row = i;
            double ra = allSourceModel->index(i,1).data().toDouble();
            double dc = allSourceModel->index(i,2).data().toDouble();
            ra -=180;

            double lambda = qDegreesToRadians(ra);
            double phi = qDegreesToRadians(dc);
            double hn = qSqrt( 1 + qCos(phi)*qCos(lambda/2) );

            x = (2 * qSqrt(2) *qCos(phi) *qSin(lambda/2) ) / hn;
            y = (qSqrt(2) *qSin(phi) ) / hn;
            break;
        }
    }

    for(int i = 0; i<selectedSources->count(); ++i){
        double xn = selectedSources->at(i).x();
        double yn = selectedSources->at(i).y();
        if( (x-xn)*(x-xn) + (y-yn)*(y-yn) < 1e-3 ){
            selectedSources->remove(i);
            break;
        }
    }

    for(int i = 0; i<allSourcePlusGroupModel->rowCount(); ++i){
        if (allSourcePlusGroupModel->index(i,0).data().toString() == name) {
            allSourcePlusGroupModel->removeRow(i);
            break;
        }
    }

}


void MainWindow::on_groupBox_modeSked_toggled(bool arg1)
{
    ui->groupBox_modeCustom->setChecked(!arg1);
}

void MainWindow::on_groupBox_modeCustom_toggled(bool arg1)
{
    ui->groupBox_modeSked->setChecked(!arg1);
}

void MainWindow::on_lineEdit_allStationsFilter_textChanged(const QString &arg1)
{
    allStationProxyModel->setFilterRegExp(ui->lineEdit_allStationsFilter->text());
}

void MainWindow::on_treeView_allAvailabeStations_clicked(const QModelIndex &index)
{
    int row = index.row();
    QString name = allStationProxyModel->index(row,0).data().toString();

    if(selectedStationModel->findItems(name).isEmpty()){
        selectedStationModel->insertRow(0);
        selectedStationModel->setItem(0,new QStandardItem(QIcon(":/icons/icons/station.png"),name));
        selectedStationModel->sort(0);
        selectedStations->append(allStationProxyModel->index(row,3).data().toDouble(),
                                 allStationProxyModel->index(row,2).data().toDouble());

        auto map = para.getGroupStations();
        int r = 0;
        for(int i = 0; i<allStationPlusGroupModel->rowCount(); ++i){
            QString txt = allStationPlusGroupModel->item(i)->text();
            if(map.find(txt.toStdString()) != map.end() || txt == "__all__"){
                ++r;
                continue;
            }
            if(txt>name){
                break;
            }else{
                ++r;
            }
        }

        allStationPlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/station.png"),name));
        createBaselineModel();
    }
}

void MainWindow::on_treeView_allAvailabeSources_clicked(const QModelIndex &index)
{
    int row = index.row();
    QString name = allSourceProxyModel->index(row,0).data().toString();

    if(selectedSourceModel->findItems(name).isEmpty()){
        selectedSourceModel->insertRow(0);
        selectedSourceModel->setItem(0,new QStandardItem(QIcon(":/icons/icons/source.png"),name));
        selectedSourceModel->sort(0);

        double ra = allSourceProxyModel->index(row,1).data().toDouble();
        double dc = allSourceProxyModel->index(row,2).data().toDouble();
        ra -=180;

        double lambda = qDegreesToRadians(ra);
        double phi = qDegreesToRadians(dc);
        double hn = qSqrt( 1 + qCos(phi)*qCos(lambda/2) );

        double x = (2 * qSqrt(2) *qCos(phi) *qSin(lambda/2) ) / hn;
        double y = (qSqrt(2) *qSin(phi) ) / hn;
        selectedSources->append(x,y);

        auto map = para.getGroupSources();
        int r = 0;
        for(int i = 0; i<allSourcePlusGroupModel->rowCount(); ++i){
            QString txt = allSourcePlusGroupModel->item(i)->text();
            if(map.find(txt.toStdString()) != map.end() || txt == "__all__"){
                ++r;
                continue;
            }
            if(txt>name){
                break;
            }else{
                ++r;
            }
        }

        allSourcePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/source.png"),name));
        createBaselineModel();
    }
}



void MainWindow::on_checkBox_fillinMode_clicked(bool checked)
{
    if(checked){
        ui->checkBox_fillinModeInfluence->setEnabled(true);
        ui->checkBox_fillinModeInfluence->setChecked(true);
    }else{
        ui->checkBox_fillinModeInfluence->setEnabled(false);
        ui->checkBox_fillinModeInfluence->setChecked(false);
    }
}

void MainWindow::on_doubleSpinBox_weightLowDecStart_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_weightLowDecEnd->value() > arg1){
        ui->doubleSpinBox_weightLowDecEnd->setValue(arg1);
    }
}

void MainWindow::on_doubleSpinBox_weightLowDecEnd_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_weightLowDecStart->value() < arg1){
        ui->doubleSpinBox_weightLowDecStart->setValue(arg1);
    }
}

void MainWindow::on_doubleSpinBox_weightLowElStart_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_weightLowElEnd->value() > arg1){
        ui->doubleSpinBox_weightLowElEnd->setValue(arg1);
    }
}

void MainWindow::on_doubleSpinBox_weightLowElEnd_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_weightLowElStart->value() < arg1){
        ui->doubleSpinBox_weightLowElStart->setValue(arg1);
    }
}

void MainWindow::on_spinBox_scanSequenceCadence_valueChanged(int arg1)
{
    if(ui->tableWidget_scanSequence->rowCount()>arg1){
        while(ui->tableWidget_scanSequence->rowCount()>arg1){
            ui->tableWidget_scanSequence->removeRow(ui->tableWidget_scanSequence->rowCount()-1);
        }
    }else{
        while(ui->tableWidget_scanSequence->rowCount()<arg1){
            ui->tableWidget_scanSequence->insertRow(ui->tableWidget_scanSequence->rowCount());
            QComboBox *cBox = new QComboBox(this);
            cBox->setModel(allSourcePlusGroupModel);
            ui->tableWidget_scanSequence->setCellWidget(ui->tableWidget_scanSequence->rowCount()-1,0, cBox);
        }
    }
}

void MainWindow::on_doubleSpinBox_calibratorLowElStart_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_calibratorLowElEnd->value() > arg1){
        ui->doubleSpinBox_calibratorLowElEnd->setValue(arg1);
    }
}

void MainWindow::on_doubleSpinBox_calibratorLowElEnd_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_calibratorLowElStart->value() < arg1){
        ui->doubleSpinBox_calibratorLowElStart->setValue(arg1);
    }
}

void MainWindow::on_doubleSpinBox_calibratorHighElStart_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_calibratorHighElEnd->value() < arg1){
        ui->doubleSpinBox_calibratorHighElEnd->setValue(arg1);
    }
}

void MainWindow::on_doubleSpinBox_calibratorHighElEnd_valueChanged(double arg1)
{
    if(ui->doubleSpinBox_calibratorHighElStart->value() > arg1){
        ui->doubleSpinBox_calibratorHighElStart->setValue(arg1);
    }
}

void MainWindow::createModesPolicyTable()
{
    QHeaderView *hv = ui->tableWidget_ModesPolicy->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::addModesPolicyTable(QString name){

    ui->tableWidget_ModesPolicy->insertRow(ui->tableWidget_ModesPolicy->rowCount());
    ui->tableWidget_ModesPolicy->setVerticalHeaderItem(ui->tableWidget_ModesPolicy->rowCount()-1,new QTableWidgetItem(name));
    QComboBox *psta = new QComboBox(this);
    psta->addItem("required");
    psta->addItem("optional");
    QComboBox *psrc = new QComboBox(this);
    psrc->addItem("required");
    psrc->addItem("optional");
    QComboBox *bsta = new QComboBox(this);
    bsta->addItem("none");
    bsta->addItem("value");
    bsta->addItem("minValueTimes");
    bsta->addItem("maxValueTimes");
    QComboBox *bsrc = new QComboBox(this);
    bsrc->addItem("none");
    bsrc->addItem("value");
    bsrc->addItem("minValueTimes");
    bsrc->addItem("maxValueTimes");
    QDoubleSpinBox *vsta = new QDoubleSpinBox(this);
    vsta->setMinimum(0);
    vsta->setMaximum(100000);
    vsta->setSingleStep(.1);
    vsta->setValue(1);
    QDoubleSpinBox *vsrc = new QDoubleSpinBox(this);
    vsrc->setMinimum(0);
    vsrc->setMaximum(100000);
    vsrc->setSingleStep(.1);
    vsrc->setValue(1);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,0,psta);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,1,bsta);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,2,vsta);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,3,psrc);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,4,bsrc);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,5,vsrc);


}

void MainWindow::createModesCustonBandTable()
{
    addModesCustomTable("X",8.6,10);
    addModesCustomTable("S",2.3,6);

    ui->tableWidget_modeCustonBand->resizeColumnsToContents();
    ui->tableWidget_modeCustonBand->verticalHeader()->show();

    QHeaderView *hv = ui->tableWidget_modeCustonBand->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);

}

void MainWindow::addModesCustomTable(QString name, double freq, int nChannel){
    name = name.trimmed();

    if(name.isEmpty()){
        QMessageBox warning;
        warning.warning(this,"missing band name","Band name is missing!");
        return;
    }
    for(int i = 0; i<ui->tableWidget_modeCustonBand->rowCount(); ++i ){
        QString tmp = ui->tableWidget_modeCustonBand->verticalHeaderItem(i)->text();
        if(name == tmp ){
            QMessageBox warning;
            warning.warning(this,"already used band name","Band name " + tmp + " is already used!");
            return;
        }
    }


    ui->tableWidget_modeCustonBand->insertRow(ui->tableWidget_modeCustonBand->rowCount());
    ui->tableWidget_modeCustonBand->setVerticalHeaderItem(ui->tableWidget_modeCustonBand->rowCount()-1,new QTableWidgetItem(name));

    QDoubleSpinBox *freqSB = new QDoubleSpinBox(this);
    freqSB->setMinimum(0);
    freqSB->setMaximum(100);
    freqSB->setSingleStep(.1);
    freqSB->setValue(freq);
    freqSB->setDecimals(4);
    freqSB->setSuffix(" [GHz]");

    QSpinBox *nChannelSB = new QSpinBox(this);
    nChannelSB->setMinimum(1);
    nChannelSB->setMaximum(100);
    nChannelSB->setValue(nChannel);

    QPushButton *d = new QPushButton("delete",this);
    connect(d,SIGNAL(clicked(bool)),deleteModeMapper,SLOT(map()));
    deleteModeMapper->setMapping(d,name);

    ui->tableWidget_modeCustonBand->setCellWidget(ui->tableWidget_modeCustonBand->rowCount()-1,0,freqSB);
    ui->tableWidget_modeCustonBand->setCellWidget(ui->tableWidget_modeCustonBand->rowCount()-1,1,nChannelSB);
    ui->tableWidget_modeCustonBand->setCellWidget(ui->tableWidget_modeCustonBand->rowCount()-1,2,d);
    addModesPolicyTable(name);
}

void MainWindow::deleteModesCustomLine(QString name)
{
    int row;
    for(int i = 0; i<ui->tableWidget_modeCustonBand->rowCount(); ++i ){
        QString tmp = ui->tableWidget_modeCustonBand->verticalHeaderItem(i)->text();
        if(name == tmp ){
            row = i;
            break;
        }
    }

    ui->tableWidget_modeCustonBand->removeRow(row);
    ui->tableWidget_ModesPolicy->removeRow(row);
}


void MainWindow::createMultiSchedTable()
{
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("session start"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("subnetting"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("fillin mode"));

    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Weight: sky coverage"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Weight: number of observations"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Weight: duration"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Weight: average stations"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Weight: average sources"));

    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Station: max slew time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Station: max wait time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Station: max scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Station: min scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Station: weight"));

    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Source: min number of stations"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Source: min flux"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Source: min repeat time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Source: max scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Source: min scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Source: weight"));

    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Baseline: max scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Baseline: min scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("Baseline: weight"));

    QBrush brush(Qt::black,Qt::HorPattern);
    QSet<int> breakes = {3,9,15,22};
    for(auto i:breakes){
        for(int j = 0; j<3; ++j){
            ui->tableWidget_multiSched->setItem(i, j, new QTableWidgetItem);
            // ui->tableWidget_multiSched->item(i, j)->setBackground(brush);
        }
    }

    for(int i = 0; i< ui->tableWidget_multiSched->rowCount(); ++i){
        if (breakes.find(i) != breakes.end()){
            continue;
        }

        ui->tableWidget_multiSched->verticalHeader()->setDefaultSectionSize(40);

        QListWidget *list = new QListWidget(this);
        list->setFlow(QListWidget::LeftToRight);
        ui->tableWidget_multiSched->setCellWidget(i, 0, list);

        ui->tableWidget_multiSched->setItem(i,1,new QTableWidgetItem);

        QString txt = "edit";
        if (i==1 || i==2){
            txt = "toggle";
        }
        QPushButton *button = new QPushButton(txt,this);
        ui->tableWidget_multiSched->setCellWidget(i, 2, button);

        connect(button,SIGNAL(clicked(bool)),multiSchedMapper,SLOT(map()));
        multiSchedMapper->setMapping(button,ui->tableWidget_multiSched->verticalHeaderItem(i)->text());
    }

    ui->tableWidget_multiSched->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);

}


void MainWindow::on_spinBox_fontSize_valueChanged(int arg1)
{
    QFont myFont = ui->fontComboBox_font->currentFont();
    myFont.setPointSize(arg1);
    worldMapCallout->setFont(myFont);
    skyMapCallout->setFont(myFont);
    QApplication::setFont(myFont);
}

void MainWindow::on_fontComboBox_font_currentFontChanged(const QFont &f)
{
    QFont myFont = f;
    myFont.setPointSize(ui->spinBox_fontSize->value());
    worldMapCallout->setFont(myFont);
    skyMapCallout->setFont(myFont);
    QApplication::setFont(myFont);
}

void MainWindow::readSkedCatalogs()
{
    skdCatalogReader.setCatalogFilePathes(ui->lineEdit_pathAntenna->text().toStdString(), ui->lineEdit_pathEquip->text().toStdString(),
                                          ui->lineEdit_pathFlux->text().toStdString(), ui->lineEdit_pathFreq->text().toStdString(),
                                          ui->lineEdit_pathHdpos->text().toStdString(), ui->lineEdit_pathLoif->text().toStdString(),
                                          ui->lineEdit_pathMask->text().toStdString(), ui->lineEdit_pathModes->text().toStdString(),
                                          ui->lineEdit_pathPosition->text().toStdString(), ui->lineEdit_pathRec->text().toStdString(),
                                          ui->lineEdit_pathRx->text().toStdString(), ui->lineEdit_pathSource->text().toStdString(),
                                          ui->lineEdit_pathTracks->text().toStdString());

    skdCatalogReader.initializeStationCatalogs();
    skdCatalogReader.initializeSourceCatalogs();

    skdCatalogReader.initializeModesCatalogs(ui->comboBox_skedObsModes->currentText().toStdString());
}

void MainWindow::on_pushButton_worldmapZoomFull_clicked()
{
    worldmap->chart()->zoomReset();
}

void MainWindow::on_treeView_allAvailabeStations_entered(const QModelIndex &index)
{
    int row = index.row();
    QString name = allStationProxyModel->index(row,0).data().toString();

    double x = allStationProxyModel->index(row,3).data().toDouble();
    double y = allStationProxyModel->index(row,2).data().toDouble();
    QString text = QString("%1 \nlat: %2 [deg] \nlon: %3 [deg] ").arg(name).arg(x).arg(y);
    worldMapCallout->setText(text);
    worldMapCallout->setAnchor(QPointF(x,y));
    worldMapCallout->setZValue(11);
    worldMapCallout->updateGeometry();
    worldMapCallout->show();
}

void MainWindow::on_treeView_allAvailabeSources_entered(const QModelIndex &index)
{
    int row = index.row();
    QString name = allSourceProxyModel->index(row,0).data().toString();

    double ra = allSourceProxyModel->index(row,1).data().toDouble();
    ra -=180;
    double dc = allSourceProxyModel->index(row,2).data().toDouble();

    double lambda = qDegreesToRadians(ra);
    double phi = qDegreesToRadians(dc);
    double hn = qSqrt( 1 + qCos(phi)*qCos(lambda/2) );

    double x = (2 * qSqrt(2) *qCos(phi) *qSin(lambda/2) ) / hn;
    double y = (qSqrt(2) *qSin(phi) ) / hn;


    QString text = QString("%1 \nra: %2 [deg] \ndc: %3 [deg] ").arg(name).arg(ra+180).arg(dc);
    skyMapCallout->setText(text);
    skyMapCallout->setAnchor(QPointF(x,y));
    skyMapCallout->setZValue(11);
    skyMapCallout->updateGeometry();
    skyMapCallout->show();
}


void MainWindow::worldmap_hovered(QPointF point, bool state)
{
    if (state) {
        QString sta;
        for(int i = 0; i<allStationModel->rowCount();++i){
            double x = allStationModel->index(i,3).data().toDouble();
            double y = allStationModel->index(i,2).data().toDouble();

            auto dx = x-point.x();
            auto dy = y-point.y();
            if(dx*dx+dy*dy < 1e-3){
                if(sta.size()==0){
                    sta.append(allStationModel->index(i,0).data().toString());
                }else{
                    sta.append(","+allStationModel->index(i,0).data().toString());
                }
            }

        }

        QString text = QString("%1 \nlat: %2 [deg] \nlon: %3 [deg] ").arg(sta).arg(point.x()).arg(point.y());
        worldMapCallout->setText(text);
        worldMapCallout->setAnchor(point);
        worldMapCallout->setZValue(11);
        worldMapCallout->updateGeometry();
        worldMapCallout->show();
    } else {
        worldMapCallout->hide();
    }

}

void MainWindow::skymap_hovered(QPointF point, bool state){

    if (state) {

        double px = point.x();
        double py = point.y();


        double z = qSqrt(1 - (1./4.*px)*(1./4.*px) - (1./2.*py)*(1./2.*py));
        double pra = qRadiansToDegrees(2* qAtan( (z*px) / (2*(2*z*z-1)) ));
        pra +=180;
        double pde = qRadiansToDegrees(qAsin(z*py));

        QString src;
        for(int i = 0; i<allSourceModel->rowCount();++i){
            double ra = allSourceModel->index(i,1).data().toDouble();
            double de = allSourceModel->index(i,2).data().toDouble();

            auto dra = ra-pra;
            auto dde = de-pde;
            if(dra*dra+dde*dde < 10){
                if(src.size()==0){
                    src.append(allSourceModel->index(i,0).data().toString());
                }else{
                    src.append(","+allSourceModel->index(i,0).data().toString());
                }
            }
        }

        QString text = QString("%1 \nRA: %2 [deg] \nDC: %3 [deg] ").arg(src).arg(pra).arg(pde);
        skyMapCallout->setText(text);
        skyMapCallout->setAnchor(point);
        skyMapCallout->setZValue(11);
        skyMapCallout->updateGeometry();
        skyMapCallout->show();
    } else {
        skyMapCallout->hide();
    }

}


//void MainWindow::worldmap_clicked()
//{
//    int i = 0;
//}

void MainWindow::on_pushButton_modeCustomAddBAnd_clicked()
{
    QDialog *d = new QDialog();
    QVBoxLayout *mainLayout = new QVBoxLayout(d);
    QFormLayout *layout = new QFormLayout(d);

    QLineEdit *name = new QLineEdit(d);
    QDoubleSpinBox *freqSB = new QDoubleSpinBox(d);
    freqSB->setMinimum(0);
    freqSB->setMaximum(100);
    freqSB->setSingleStep(.1);
    freqSB->setValue(1);
    freqSB->setDecimals(4);
    freqSB->setSuffix(" [GHz]");

    QSpinBox *nChannelSB = new QSpinBox(d);
    nChannelSB->setMinimum(1);
    nChannelSB->setMaximum(100);
    nChannelSB->setValue(1);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                        | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, SIGNAL(accepted()), d, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), d, SLOT(reject()));

    layout->addRow(tr("&Name"),name);
    layout->addRow(tr("&Frequency"),freqSB);
    layout->addRow(tr("&Channels"),nChannelSB);

    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);
    d->setLayout(mainLayout);

    int result = d->exec();
    if(result == QDialog::Accepted){
        addModesCustomTable(name->text(),freqSB->value(),nChannelSB->value());
    }

    delete(d);
}

void MainWindow::on_listView_allSelectedStations_entered(const QModelIndex &index)
{
    int row = index.row();
    QString name = selectedStationModel->index(row,0).data().toString();

    for(int i = 0; i < allStationModel->rowCount(); ++i){
        QString newName = allStationModel->index(i,0).data().toString();
        if (newName == name){
            double x = allStationModel->index(i,3).data().toDouble();;
            double y = allStationModel->index(i,2).data().toDouble();;
            QString text = QString("%1 \nlat: %2 [deg] \nlon: %3 [deg] ").arg(name).arg(x).arg(y);
            worldMapCallout->setText(text);
            worldMapCallout->setAnchor(QPointF(x,y));
            worldMapCallout->setZValue(11);
            worldMapCallout->updateGeometry();
            worldMapCallout->show();
            break;
        }
    }
}

void MainWindow::on_listView_allSelectedSources_entered(const QModelIndex &index)
{

    int row = index.row();
    QString name = selectedSourceModel->index(row,0).data().toString();

    for(int i = 0; i < allSourceModel->rowCount(); ++i){
        QString newName = allSourceModel->index(i,0).data().toString();

        if (newName == name){
            double ra = allSourceModel->index(i,1).data().toDouble();
            ra -=180;
            double dc = allSourceModel->index(i,2).data().toDouble();

            double lambda = qDegreesToRadians(ra);
            double phi = qDegreesToRadians(dc);
            double hn = qSqrt( 1 + qCos(phi)*qCos(lambda/2) );

            double x = (2 * qSqrt(2) *qCos(phi) *qSin(lambda/2) ) / hn;
            double y = (qSqrt(2) *qSin(phi) ) / hn;


            QString text = QString("%1 \nra: %2 [deg] \ndc: %3 [deg] ").arg(name).arg(ra+180).arg(dc);
            skyMapCallout->setText(text);
            skyMapCallout->setAnchor(QPointF(x,y));
            skyMapCallout->setZValue(11);
            skyMapCallout->updateGeometry();
            skyMapCallout->show();
            break;
        }
    }
}


void MainWindow::multiSchedEditButton_clicked(QString name)
{

    QStringList row2toggle{"subnetting",
                           "fillin mode"};

    QStringList row2intDialog {"Station: max slew time",
                              "Station: max wait time",
                              "Station: max scan time",
                              "Station: max scan time",
                              "Station: min scan time",
                              "Source: min number of stations",
                              "Source: min repeat time",
                              "Source: max scan time",
                              "Source: min scan time",
                              "Baseline: max scan time",
                              "Baseline: min scan time"};

    QStringList row2doubleDialog {"Weight: sky coverage",
                                  "Weight: number of observations",
                                  "Weight: duration",
                                  "Weight: average stations",
                                  "Weight: average sources",
                                  "Station: weight",
                                  "Source: min flux",
                                  "Source: weight",
                                  "Baseline: weight"};

    QStringList row2dateTimeDialog {"session start"};

    for(int i = 0; i<ui->tableWidget_multiSched->rowCount(); ++i){
        QString rname = ui->tableWidget_multiSched->verticalHeaderItem(i)->text();
        if(rname == name){

            if(row2intDialog.indexOf(name) != -1){
                multiSchedEditDialogInt *dialog = new multiSchedEditDialogInt(this);

                int result = dialog->exec();
                if(result == QDialog::Accepted){
                    QVector<int> val = dialog->getValues();
                    int n = val.size();

                    QListWidget *tmpList = qobject_cast<QListWidget*>(ui->tableWidget_multiSched->cellWidget(i,0));
                    tmpList->clear();
                    for(int j = 0; j<n; ++j){
                        tmpList->addItem(QString::number(val.at(j),'f',2));
                    }

                    QTableWidgetItem *tmpNr = ui->tableWidget_multiSched->item(i,1);
                    tmpNr->setText("");
                    if (n>0){
                        tmpNr->setText(QString::number(n,'f',0));
                    }
                }
                delete(dialog);

            } else if(row2doubleDialog.indexOf(name) != -1){
                multiSchedEditDialogDouble *dialog = new multiSchedEditDialogDouble(this);

                int result = dialog->exec();
                if(result == QDialog::Accepted){
                    QVector<double> val = dialog->getValues();
                    int n = val.size();

                    QListWidget *tmpList = qobject_cast<QListWidget*>(ui->tableWidget_multiSched->cellWidget(i,0));
                    tmpList->clear();
                    for(int j = 0; j<n; ++j){
                        tmpList->addItem(QString::number(val.at(j),'f',2));
                    }

                    QTableWidgetItem *tmpNr = ui->tableWidget_multiSched->item(i,1);
                    tmpNr->setText("");
                    if (n>0){
                        tmpNr->setText(QString::number(n,'f',0));
                    }
                }
                delete(dialog);

            } else if (row2dateTimeDialog.indexOf(name) != -1){
                multiSchedEditDialogDateTime *dialog = new multiSchedEditDialogDateTime(this);

                int result = dialog->exec();
                if(result == QDialog::Accepted){
                    QVector<QDateTime> val = dialog->getValues();
                    int n = val.size();

                    QListWidget *tmpList = qobject_cast<QListWidget*>(ui->tableWidget_multiSched->cellWidget(i,0));
                    tmpList->clear();
                    for(int j = 0; j<n; ++j){
                        tmpList->addItem(val.at(j).toString());
                    }

                    QTableWidgetItem *tmpNr = ui->tableWidget_multiSched->item(i,1);
                    tmpNr->setText("");
                    if (n>0){
                        tmpNr->setText(QString::number(n,'f',0));
                    }
                }
                delete(dialog);
            } else if (row2toggle.indexOf(name) != -1){
                QListWidget *tmpList = qobject_cast<QListWidget*>(ui->tableWidget_multiSched->cellWidget(i,0));
                QTableWidgetItem *tmpNr = ui->tableWidget_multiSched->item(i,1);
                if(tmpList->count() == 0){
                    tmpList->addItem("True");
                    tmpList->addItem("False");
                    tmpNr->setText("2");
                }else {
                    tmpList->clear();
                    tmpNr->setText("");
                }
            }

            break;
        }
    }

    int nsched = 1;
    for(int i = 0; i<ui->tableWidget_multiSched->rowCount(); ++i){
        QTableWidgetItem *tmpNr = ui->tableWidget_multiSched->item(i,1);
        if (tmpNr->text() != ""){
            int tmpNr_int = tmpNr->text().toInt();
            nsched *= tmpNr_int;
        }
    }

    ui->label_multiSchedulingNsched->setText(QString::number(nsched));
}

void MainWindow::on_actionExit_triggered()
{
    closeEvent(new QCloseEvent);
}


void MainWindow::closeEvent(QCloseEvent *event)  // show prompt when user wants to close app
{
    event->ignore();
    if (QMessageBox::Yes == QMessageBox::question(this, "Exit?", "Do you really want to exit?", QMessageBox::Yes | QMessageBox::No))
    {
        QSettings settings("TU Wien","VieVS Scheduler");
        settings.setValue("geometry", saveGeometry());
        event->accept();
    }

}

void MainWindow::on_iconSizeSpinBox_valueChanged(int arg1)
{
    ui->fileToolBar->setIconSize(QSize(arg1,arg1));
    ui->basicToolBar->setIconSize(QSize(arg1,arg1));
    ui->advancedToolBar->setIconSize(QSize(arg1,arg1));
    ui->helpToolBar->setIconSize(QSize(arg1,arg1));
}

void MainWindow::on_treeWidget_2_itemChanged(QTreeWidgetItem *item, int column)
{
    for(int i = 0; i<item->childCount(); ++i){
        if(item->checkState(0) == Qt::Checked){
            item->child(i)->setDisabled(false);
        }else{
            item->child(i)->setDisabled(true);
        }
    }

    if(item->text(0) == "Files"){
        if(item->checkState(0) == Qt::Checked){
            ui->fileToolBar->show();
        }else{
            ui->fileToolBar->hide();
        }
    } else if(item->text(0) == "Basic"){
        if(item->checkState(0) == Qt::Checked){
            ui->basicToolBar->show();
        }else{
            ui->basicToolBar->hide();
        }
    } else if(item->text(0) == "Advanced"){
        if(item->checkState(0) == Qt::Checked){
            ui->advancedToolBar->show();
        }else{
            ui->advancedToolBar->hide();
        }
    } else if(item->text(0) == "Help"){
        if(item->checkState(0) == Qt::Checked){
            ui->helpToolBar->show();
        }else{
            ui->helpToolBar->hide();
        }
    } else if(item->text(0) == "Welcome"){
        auto actions = ui->fileToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Welcome"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Settings"){
        auto actions = ui->fileToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Settings"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "New"){
        auto actions = ui->fileToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "New"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Open"){
        auto actions = ui->fileToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Open"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Save"){
        auto actions = ui->fileToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Save"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Run"){
        auto actions = ui->fileToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Run"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Exit"){
        auto actions = ui->fileToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Exit"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Input"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Input"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Mode"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Mode"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "General"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "General"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Parameter Stations"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Station"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Parameter Sources"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Source"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Parameter Baselines"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Baseline"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Weight Factors"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Weight Factors"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Output"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Output"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Rules"){
        auto actions = ui->advancedToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Rules"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Multi Scheduling"){
        auto actions = ui->advancedToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Multi Scheduling"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Sky Coverage"){
        auto actions = ui->advancedToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Sky Coverage"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "What is this?"){
        auto actions = ui->helpToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "What is this?"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Current Release"){
        auto actions = ui->helpToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Current Release"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "About"){
        auto actions = ui->helpToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "About"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "About Qt"){
        auto actions = ui->helpToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "About Qt"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    }
}

void MainWindow::addGroupStation()
{
    AddGroupDialog *dial = new AddGroupDialog(this);
    dial->addModel(selectedStationModel);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::vector<std::string> stdlist = dial->getSelection();
        std::string stdname = dial->getGroupName();
        VieVS::ParameterGroup newGroup(stdname, stdlist);

        auto map = para.getGroupStations();
        int r = 0;
        for(int i = 0; i<allStationPlusGroupModel->rowCount(); ++i){
            QString txt = allStationPlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++r;
                continue;
            }
            if(map.find(txt.toStdString()) == map.end()){
                break;
            }
            if(txt>QString::fromStdString(stdname)){
                break;
            }else{
                ++r;
            }
        }


        para.group(VieVS::ParameterSettings::Type::station,newGroup);
        allStationPlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/station_group_2.png"),QString::fromStdString(stdname) ));
    }
    delete(dial);
}

void MainWindow::addGroupSource()
{
    AddGroupDialog *dial = new AddGroupDialog(this);
    dial->addModel(selectedSourceModel);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::vector<std::string> stdlist = dial->getSelection();
        std::string stdname = dial->getGroupName();
        VieVS::ParameterGroup newGroup(stdname, stdlist);

        auto map = para.getGroupSources();
        int r = 0;
        for(int i = 0; i<allSourcePlusGroupModel->rowCount(); ++i){
            QString txt = allSourcePlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++r;
                continue;
            }
            if(map.find(txt.toStdString()) == map.end()){
                break;
            }
            if(txt>QString::fromStdString(stdname)){
                break;
            }else{
                ++r;
            }
        }
        para.group(VieVS::ParameterSettings::Type::source,newGroup);
        allSourcePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/source_group.png"),QString::fromStdString(stdname) ));
    }
    delete(dial);
}

void MainWindow::addGroupBaseline()
{
    AddGroupDialog *dial = new AddGroupDialog(this);
    dial->addModel(selectedBaselineModel);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::vector<std::string> stdlist = dial->getSelection();
        std::string stdname = dial->getGroupName();
        VieVS::ParameterGroup newGroup(stdname, stdlist);

        auto map = para.getGroupSources();
        int r = 0;
        for(int i = 0; i<allBaselinePlusGroupModel->rowCount(); ++i){
            QString txt = allBaselinePlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++r;
                continue;
            }
            if(map.find(txt.toStdString()) == map.end()){
                break;
            }
            if(txt>QString::fromStdString(stdname)){
                break;
            }else{
                ++r;
            }
        }
        para.group(VieVS::ParameterSettings::Type::baseline,newGroup);
        allBaselinePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),QString::fromStdString(stdname) ));
    }
    delete(dial);
}

void MainWindow::on_pushButton_stationParameter_clicked()
{
    stationParametersDialog *dial = new stationParametersDialog(this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);

    dial->addSourceNames(allSourcePlusGroupModel);

    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersStations> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersStations parameter = res.second;

        para.parameters(name,parameter);

        ui->ComboBox_parameterStation->addItem(QString::fromStdString(name));

    }
    delete(dial);
}

void MainWindow::on_pushButton_sourceParameter_clicked()
{
    sourceParametersDialog *dial = new sourceParametersDialog(this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);

    dial->addStationModel(allStationPlusGroupModel);
    dial->addBaselineModel(allBaselinePlusGroupModel);

    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersSources> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersSources parameter = res.second;

        para.parameters(name,parameter);

        ui->ComboBox_parameterSource->addItem(QString::fromStdString(name));

    }
    delete(dial);
}

void MainWindow::on_pushButton__baselineParameter_clicked()
{
    baselineParametersDialog *dial = new baselineParametersDialog(this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);

    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersBaselines> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersBaselines parameter = res.second;

        para.parameters(name,parameter);

        ui->ComboBox_parameterBaseline->addItem(QString::fromStdString(name));

    }
    delete(dial);
}


void MainWindow::on_dateTimeEdit_sessionStart_dateTimeChanged(const QDateTime &dateTime)
{
    QDateTime dateTimeEnd = dateTime.addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);

    ui->DateTimeEdit_startParameterStation->setMinimumDateTime(dateTime);
    ui->DateTimeEdit_endParameterStation->setMinimumDateTime(dateTime);
    ui->DateTimeEdit_startParameterStation->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterStation->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_startParameterStation->setDateTime(dateTime);
    ui->DateTimeEdit_endParameterStation->setDateTime(dateTimeEnd);

    ui->DateTimeEdit_startParameterSource->setMinimumDateTime(dateTime);
    ui->DateTimeEdit_endParameterSource->setMinimumDateTime(dateTime);
    ui->DateTimeEdit_startParameterSource->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterSource->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_startParameterSource->setDateTime(dateTime);
    ui->DateTimeEdit_endParameterSource->setDateTime(dateTimeEnd);

    ui->DateTimeEdit_startParameterBaseline->setMinimumDateTime(dateTime);
    ui->DateTimeEdit_endParameterBaseline->setMinimumDateTime(dateTime);
    ui->DateTimeEdit_startParameterBaseline->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterBaseline->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_startParameterBaseline->setDateTime(dateTime);
    ui->DateTimeEdit_endParameterBaseline->setDateTime(dateTimeEnd);

}

void MainWindow::on_doubleSpinBox_sessionDuration_valueChanged(double arg1)
{
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(arg1*3600);

    ui->DateTimeEdit_startParameterStation->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterStation->setMaximumDateTime(dateTimeEnd);

    ui->DateTimeEdit_startParameterSource->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterSource->setMaximumDateTime(dateTimeEnd);

    ui->DateTimeEdit_startParameterBaseline->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterBaseline->setMaximumDateTime(dateTimeEnd);
}


void MainWindow::on_DateTimeEdit_startParameterStation_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime > ui->DateTimeEdit_endParameterStation->dateTime()){
        ui->DateTimeEdit_endParameterStation->setDateTime(dateTime);
    }
}

void MainWindow::on_DateTimeEdit_endParameterStation_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime < ui->DateTimeEdit_startParameterStation->dateTime()){
        ui->DateTimeEdit_startParameterStation->setDateTime(dateTime);
    }
}

void MainWindow::on_DateTimeEdit_startParameterSource_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime > ui->DateTimeEdit_endParameterSource->dateTime()){
        ui->DateTimeEdit_endParameterSource->setDateTime(dateTime);
    }
}

void MainWindow::on_DateTimeEdit_endParameterSource_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime < ui->DateTimeEdit_startParameterSource->dateTime()){
        ui->DateTimeEdit_startParameterSource->setDateTime(dateTime);
    }
}

void MainWindow::on_DateTimeEdit_startParameterBaseline_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime > ui->DateTimeEdit_endParameterBaseline->dateTime()){
        ui->DateTimeEdit_endParameterBaseline->setDateTime(dateTime);
    }
}

void MainWindow::on_DateTimeEdit_endParameterBaseline_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime < ui->DateTimeEdit_startParameterBaseline->dateTime()){
        ui->DateTimeEdit_startParameterBaseline->setDateTime(dateTime);
    }
}


void MainWindow::createBaselineModel()
{
    selectedBaselineModel->clear();

    allBaselinePlusGroupModel->clear();
    allBaselinePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),"__all__"));


    int n = selectedStationModel->rowCount();
    for(int i = 0; i<n; ++i){
        for(int j = i+1; j<n; ++j){
            QString bl = selectedStationModel->index(i,0).data().toString();
            bl.append("-").append(selectedStationModel->index(j,0).data().toString());
            allBaselinePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/baseline.png"),bl));
            int row = selectedBaselineModel->rowCount();
            selectedBaselineModel->insertRow(row);
            selectedBaselineModel->setItem(row,new QStandardItem(QIcon(":/icons/icons/baseline.png"),bl));
        }
    }
}


void MainWindow::on_pushButton_3_clicked()
{

    addSetup(ui->treeWidget_setupStation, ui->DateTimeEdit_startParameterStation, ui->DateTimeEdit_endParameterStation,
             ui->comboBox_parameterStationTransition, ui->comboBox_stationSettingMember, ui->ComboBox_parameterStation,
             setupStationTree, setupStation, ui->comboBox_setupStation);
}

void MainWindow::on_pushButton_addSetupSource_clicked()
{
    addSetup(ui->treeWidget_setupSource, ui->DateTimeEdit_startParameterSource, ui->DateTimeEdit_endParameterSource,
             ui->comboBox_parameterSourceTransition, ui->comboBox_sourceSettingMember, ui->ComboBox_parameterSource,
             setupSourceTree, setupSource, ui->comboBox_setupSource);
}

void MainWindow::on_pushButton_addSetupBaseline_clicked()
{
    addSetup(ui->treeWidget_setupBaseline, ui->DateTimeEdit_startParameterBaseline, ui->DateTimeEdit_endParameterBaseline,
             ui->comboBox_parameterBaselineTransition, ui->comboBox_baselineSettingMember, ui->ComboBox_parameterBaseline,
             setupBaselineTree, setupBaseline, ui->comboBox_setupBaseline);
}


void MainWindow::addSetup(QTreeWidget *targetTreeWidget, QDateTimeEdit *paraStart, QDateTimeEdit *paraEnd,
                          QComboBox *transition, QComboBox *member, QComboBox *parameter,
                          VieVS::ParameterSetup &paraSetup, QChartView *setupChartView, QComboBox *targetStationPlot){

    QList<QTreeWidgetItem *> sel = targetTreeWidget->selectedItems();

    if(sel.size() != 1){
        QMessageBox *ms = new QMessageBox;
        ms->warning(this,"Wrong selection","Please select one parent in the right window!");
    }else{

        VieVS::ParameterSetup ps;

        QDateTime sessionStart = ui->dateTimeEdit_sessionStart->dateTime();
        unsigned int startt = sessionStart.secsTo(paraStart->dateTime());
        unsigned int endt = sessionStart.secsTo(paraEnd->dateTime());
        VieVS::ParameterSetup::Transition trans;
        if(transition->currentText() == "soft"){
            trans = VieVS::ParameterSetup::Transition::soft;
        }else{
            trans = VieVS::ParameterSetup::Transition::hard;
        }

        std::map<std::string, std::vector<std::string>> groups;
        if(targetTreeWidget == ui->treeWidget_setupStation){
            groups = para.getGroupStations();
        }else if(targetTreeWidget == ui->treeWidget_setupSource){
            groups = para.getGroupSources();
        }else if(targetTreeWidget == ui->treeWidget_setupBaseline){
            groups = para.getGroupBaselines();
        }
        bool isGroup = groups.find(member->currentText().toStdString() ) != groups.end();
        if(isGroup){
            std::string parameterName = parameter->currentText().toStdString();
            std::string groupName = member->currentText().toStdString();
            std::vector<std::string> groupMembers = groups.at(member->currentText().toStdString());
            ps = VieVS::ParameterSetup(parameterName,
                                          groupName,
                                          groupMembers,
                                          startt,
                                          endt,
                                          trans);
        }else{
            std::string parameterName = parameter->currentText().toStdString();
            std::string stationName = member->currentText().toStdString();
            ps = VieVS::ParameterSetup(parameterName,
                                          stationName,
                                          startt,
                                          endt,
                                          trans);
        }

        int level=0;
        QTreeWidgetItem * t = sel.at(0);
        while(t->parent()){
            t = t->parent();
            ++level;
        }


        QString txt2 = sel.at(0)->text(2);
        QString txt3 = sel.at(0)->text(3);
        QDateTime start2 = QDateTime::fromString(txt2,"dd.MM.yyyy hh:mm");
        QDateTime start3 = QDateTime::fromString(txt3,"dd.MM.yyyy hh:mm");

        unsigned int startt2 = sessionStart.secsTo(start2);
        unsigned int endt2 = sessionStart.secsTo(start3);
        std::string parameterName2 = sel.at(0)->text(1).toStdString();
        std::string memberName2 = sel.at(0)->text(0).toStdString();
        std::vector<std::string> members2;
        if(groups.find(memberName2) != groups.end()){
            members2 = groups.at(memberName2);
        }else{
            members2.push_back(memberName2);
        }
        VieVS::ParameterSetup::Transition trans2;
        if(sel.at(0)->text(4) == "soft"){
            trans2 = VieVS::ParameterSetup::Transition::soft;
        }else{
            trans2 = VieVS::ParameterSetup::Transition::hard;
        }

        boost::optional<VieVS::ParameterSetup &> root = paraSetup.search(0,level, parameterName2, memberName2, members2, trans2, startt2, endt2);

        int errorCode = root->addChild(ps);

        if (errorCode != 0) {
            QString txt;
            switch (errorCode) {
            case 1: txt = "Conflict with parent: child contains all stations but parent object does not! Always make sure that all stations in child are also part of parent."; break;
            case 2: txt = "Conflict with parent: time span of child is not part of time span of parent!"; break;
            case 3: txt = "Conflict with parent: at least one of the stations in child are not part of parent! Always make sure that all stations in child are also part of parent."; break;
            case 4: txt = "Conflict with sibling: overlapping time series with at least one sibling and at least one of the siblings or new setup contains all stations"; break;
            case 5: txt = "Conflict with sibling: overlapping time series with at least one sibling and somehow there are no members in at least one sibling or in the new setup... maybe error with a group."; break;
            case 6: txt = "Conflict with sibling: overpassing time series with at least one sibling and at least one station is part of a sibling! "; break;
            default: txt = "Child could not be added... wired error... please report to developers! This should not have happened :-) "; break;
            }

            QMessageBox ms;
            ms.warning(this,"Invalid child",txt);
        }else{
            QTreeWidgetItem *c = new QTreeWidgetItem();
            QIcon ic;
            if(isGroup || member->currentText() == "__all__"){
                if(targetTreeWidget == ui->treeWidget_setupStation){
                    ic = QIcon(":/icons/icons/station_group_2.png");
                }else if(targetTreeWidget == ui->treeWidget_setupSource){
                    ic = QIcon(":/icons/icons/source_group.png");
                }else if(targetTreeWidget == ui->treeWidget_setupBaseline){
                    ic = QIcon(":/icons/icons/baseline_group.png");
                }
            }else{
                if(targetTreeWidget == ui->treeWidget_setupStation){
                    ic = QIcon(":/icons/icons/station.png");
                }else if(targetTreeWidget == ui->treeWidget_setupSource){
                    ic = QIcon(":/icons/icons/source.png");
                }else if(targetTreeWidget == ui->treeWidget_setupBaseline){
                    ic = QIcon(":/icons/icons/baseline.png");
                }
            }

            c->setIcon(0,ic);
            c->setText(0,member->currentText());
            c->setText(1,parameter->currentText());
            c->setText(2,paraStart->dateTime().toString("dd.MM.yyyy hh:mm"));
            c->setText(3,paraEnd->dateTime().toString("dd.MM.yyyy hh:mm"));
            c->setText(4,transition->currentText());

            sel.at(0)->addChild(c);
            sel.at(0)->setExpanded(true);
            drawSetupPlot(setupChartView, targetStationPlot, targetTreeWidget);
        }
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    deleteSetupSelection(setupStationTree, setupStation, ui->comboBox_setupStation, ui->treeWidget_setupStation);
}

void MainWindow::on_pushButton_removeSetupSource_clicked()
{
    deleteSetupSelection(setupSourceTree, setupSource, ui->comboBox_setupSource, ui->treeWidget_setupSource);
}

void MainWindow::on_pushButton_removeSetupBaseline_clicked()
{
    deleteSetupSelection(setupBaselineTree, setupBaseline, ui->comboBox_setupBaseline, ui->treeWidget_setupBaseline);
}


void MainWindow::deleteSetupSelection(VieVS::ParameterSetup &setup, QChartView *setupChartView, QComboBox *setupCB, QTreeWidget *setupTW){
    QList<QTreeWidgetItem *> sel = setupTW->selectedItems();
    for(int i = 0; i<sel.size(); ++i){
        if(sel.at(0)->parent()){

            std::map<std::string, std::vector<std::string>> groups = para.getGroupStations();
            QString txt2 = sel.at(0)->text(2);
            QString txt3 = sel.at(0)->text(3);
            QDateTime start2 = QDateTime::fromString(txt2,"dd.MM.yyyy hh:mm");
            QDateTime start3 = QDateTime::fromString(txt3,"dd.MM.yyyy hh:mm");

            QDateTime sessionStart = ui->dateTimeEdit_sessionStart->dateTime();
            unsigned int startt2 = sessionStart.secsTo(start2);
            unsigned int endt2 = sessionStart.secsTo(start3);
            std::string parameterName2 = sel.at(0)->text(1).toStdString();
            std::string memberName2 = sel.at(0)->text(0).toStdString();
            std::vector<std::string> members2;
            if(groups.find(memberName2) != groups.end()){
                members2 = groups.at(memberName2);
            }else{
                members2.push_back(memberName2);
            }
            VieVS::ParameterSetup::Transition trans2;
            if(sel.at(0)->text(4) == "soft"){
                trans2 = VieVS::ParameterSetup::Transition::soft;
            }else{
                trans2 = VieVS::ParameterSetup::Transition::hard;
            }

            int level=0;
            QTreeWidgetItem * t = sel.at(0);
            while(t->parent()){
                t = t->parent();
                ++level;
            }

            bool successful = setup.deleteChild(0,level, parameterName2, memberName2, members2, trans2, startt2, endt2);

            delete(sel.at(0));
            drawSetupPlot(setupChartView, setupCB, setupTW);
        }else{
            QMessageBox *ms = new QMessageBox;
            ms->warning(this,"Wrong selection","You can not delete top level default parameter item!");
        }
    }
}


void MainWindow::on_treeWidget_setupStation_itemEntered(QTreeWidgetItem *item, int column)
{
    if(column == 0){
        displayStationSetupMember(item->text(column));
    }else if(column == 1){
        displayStationSetupParameter(item->text(column));
    }
}

void MainWindow::on_treeWidget_setupSource_itemEntered(QTreeWidgetItem *item, int column)
{
    if(column == 0){
        displaySourceSetupMember(item->text(column));
    }else if(column == 1){
        displaySourceSetupParameter(item->text(column));
    }
}

void MainWindow::on_treeWidget_setupBaseline_itemEntered(QTreeWidgetItem *item, int column)
{
    if(column == 0){
        displayBaselineSetupMember(item->text(column));
    }else if(column == 1){
        displayBaselineSetupParameter(item->text(column));
    }
}


void MainWindow::prepareSetupPlot(QChartView *figure, QVBoxLayout *container)
{
    QChart *chart = new QChart();

    QLineSeries *series = new QLineSeries();
    QDateTime start = ui->DateTimeEdit_startParameterStation->dateTime();
    QDateTime end = ui->DateTimeEdit_endParameterStation->dateTime();

    series->append(start.toMSecsSinceEpoch(),-5);
    series->append(end.toMSecsSinceEpoch(),-5);
    chart->addSeries(series);

    chart->setTitle("Setup");

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTitleText("time");
    axisX->setFormat("hh:mm");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Parameters");
    axisY->setTickCount(10);
    chart->addAxis(axisY,Qt::AlignLeft);
    series->attachAxis(axisY);

    figure->setRenderHint(QPainter::Antialiasing);
    container->addWidget(figure,10);
    axisY->hide();
    axisX->show();
    chart->legend()->hide();
    figure->setChart(chart);
}

void MainWindow::drawSetupPlot(QChartView *cv, QComboBox *cb, QTreeWidget *tw)
{
    QChart * ss = cv->chart();
    ss->removeAllSeries();
    QString name = cb->currentText();
    ss->setTitle(QString("Setup %1").arg(name));

    std::map<std::string,std::vector<std::string>> map;
    if(cv == setupStation){
        map = para.getGroupStations();
    }else if(cv == setupSource){
        map = para.getGroupSources();
    }else if(cv == setupBaseline){
        map = para.getGroupBaselines();
    }

    QTreeWidgetItem *root = tw->topLevelItem(0);
    plotParameter(ss,root,0,0,name,map);
    QDateTime start = QDateTime::fromString(root->text(2),"dd.MM.yyyy hh:mm");
    QDateTime end = QDateTime::fromString(root->text(3),"dd.MM.yyyy hh:mm");

    auto axes = ss->axes();
    axes.at(0)->setMin(start);
    axes.at(0)->setMax(end);
    axes.at(1)->setMin(-10);
    axes.at(1)->setMax(1);
}

int MainWindow::plotParameter(QChart* chart, QTreeWidgetItem *root, int level, int plot, QString target, const std::map<std::string, std::vector<std::string> > &map){
    QDateTime start = QDateTime::fromString(root->text(2),"dd.MM.yyyy hh:mm");
    QDateTime end = QDateTime::fromString(root->text(3),"dd.MM.yyyy hh:mm");

    QLineSeries *series = new QLineSeries();

//    connect(series,SIGNAL(clicked(QPointF)),this,SLOT(worldmap_clicked(QPointF)));
    series->setName(root->text(1));

    QColor c;
    switch (plot%9) {
    case 0: c = QColor(228,26,28); break;
    case 1: c = QColor(55,126,184); break;
    case 2: c = QColor(77,175,74); break;
    case 3: c = QColor(152,78,163); break;
    case 4: c = QColor(255,127,0); break;
    case 5: c = QColor(255,255,51); break;
    case 6: c = QColor(166,86,40); break;
    case 7: c = QColor(247,129,191); break;
    case 8: c = QColor(153,153,153); break;
    default:c = QColor(153,153,153);break;
    }
    root->setBackgroundColor(5,c);
    series->setPen(QPen(QBrush(c),10,Qt::SolidLine,Qt::RoundCap));

    QDateTime i = start;
    while( i <= end){
        series->append(i.toMSecsSinceEpoch(),0-level);
        i = i.addSecs(60);
    }
    chart->addSeries(series);
    auto axes = chart->axes();
    if(level>9){
        axes.at(1)->setMin(-(level+1));
    }
    series->attachAxis(axes.at(1));
    series->attachAxis(axes.at(0));

    if(chart == setupStation->chart()){
        connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(displayStationSetupParameterFromPlot()));
    }else if(chart == setupSource->chart()){
        connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(displaySourceSetupParameterFromPlot()));
    }else if(chart == setupBaseline->chart()){
        connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(displayBaselineSetupParameterFromPlot()));
    }

    for(int i=0; i<root->childCount(); ++i ){
        auto itm = root->child(i);
        QString member = itm->text(0);
        bool inMap = false;
        if(map.find(member.toStdString())!=map.end()){
            auto members = map.at(member.toStdString());
            if (std::find(members.begin(),members.end(),target.toStdString()) != members.end()){
                inMap = true;
            }
        }
        if(member == "__all__" || inMap || member == target){
            plot = plotParameter(chart,itm,level+1, plot+1,target,map);
        }else{
            itm->setBackgroundColor(5,Qt::white);
        }
    }
    return plot;
}

void MainWindow::on_comboBox_stationSettingMember_currentTextChanged(const QString &arg1)
{
    displayStationSetupMember(arg1);
}

void MainWindow::displayStationSetupParameterFromPlot(){
    QLineSeries* series = qobject_cast<QLineSeries*>(sender());
    QString name = series->name();
    displayStationSetupParameter(name);
}

void MainWindow::displaySourceSetupParameterFromPlot(){
    QLineSeries* series = qobject_cast<QLineSeries*>(sender());
    QString name = series->name();
    displaySourceSetupParameter(name);
}

void MainWindow::displayBaselineSetupParameterFromPlot(){
    QLineSeries* series = qobject_cast<QLineSeries*>(sender());
    QString name = series->name();
    displayBaselineSetupParameter(name);
}

void MainWindow::on_ComboBox_parameterStation_currentTextChanged(const QString &arg1)
{
    displayStationSetupParameter(arg1);
}

void MainWindow::on_comboBox_setupStation_currentTextChanged(const QString &arg1)
{
    drawSetupPlot(setupStation, ui->comboBox_setupStation, ui->treeWidget_setupStation);
    if(!arg1.isEmpty()){
        displayStationSetupMember(arg1);
    }
}

void MainWindow::on_comboBox_setupSource_currentTextChanged(const QString &arg1)
{
    drawSetupPlot(setupSource, ui->comboBox_setupSource, ui->treeWidget_setupSource);
    if(!arg1.isEmpty()){
        displaySourceSetupMember(arg1);
    }
}

void MainWindow::on_comboBox_setupBaseline_currentTextChanged(const QString &arg1)
{
    drawSetupPlot(setupBaseline, ui->comboBox_setupBaseline, ui->treeWidget_setupBaseline);
    if(!arg1.isEmpty()){
        displayBaselineSetupMember(arg1);
    }
}

void MainWindow::setupStationWaitAddRow()
{
    auto t = ui->treeWidget_setupStationWait;
    int row = t->topLevelItemCount();
    QString name = ui->comboBox_stationSettingMember_wait->currentText();
    auto groups = para.getGroupStations();
    QIcon ic;
    bool inGroup = groups.find(name.toStdString()) != groups.end();
    if( inGroup || name == "__all__"){
        ic = QIcon(":/icons/icons/station_group_2.png");
    }else{
        ic = QIcon(":/icons/icons/station.png");
    }

    QString errorStation;
    bool valid = true;
    for(int i = 0; i<row; ++i){
        QString itmName = t->topLevelItem(i)->text(0);
        if(name == "__all__" || itmName == "__all__"){
            valid = false;
            errorStation = QString("__all__");
            break;
        }

        if(groups.find(itmName.toStdString()) != groups.end()){
            std::vector<std::string> itmMembers = groups.at(itmName.toStdString());
            if(inGroup){
                std::vector<std::string> members = groups.at(name.toStdString());
                for(const auto &any:members){
                    if(std::find(itmMembers.begin(),itmMembers.end(),any) != itmMembers.end()){
                        valid = false;
                        errorStation = QString::fromStdString(any);
                        break;
                    }
                }
            }else{
                if(std::find(itmMembers.begin(),itmMembers.end(),name.toStdString()) != itmMembers.end()){
                    valid = false;
                    errorStation = name;
                    break;
                }
            }
        }else{
            if(inGroup){
                std::vector<std::string> members = groups.at(name.toStdString());
                if(std::find(members.begin(),members.end(),itmName.toStdString()) != members.end()){
                    valid = false;
                    errorStation = itmName;
                    break;
                }
            }else{
                if(itmName.toStdString() == name.toStdString()){
                    valid = false;
                    errorStation = name;
                    break;
                }
            }
        }
    }

    if(valid){
        t->insertTopLevelItem(row,new QTreeWidgetItem());
        t->topLevelItem(row)->setText(0,name);
        t->topLevelItem(row)->setIcon(0,ic);
        t->topLevelItem(row)->setText(1,QString::number(ui->SpinBox_setup->value()).append(" [sec]"));
        t->topLevelItem(row)->setText(2,QString::number(ui->SpinBox_source->value()).append(" [sec]"));
        t->topLevelItem(row)->setText(3,QString::number(ui->SpinBox_tape->value()).append(" [sec]"));
        t->topLevelItem(row)->setText(4,QString::number(ui->SpinBox_calibration->value()).append(" [sec]"));
        t->topLevelItem(row)->setText(5,QString::number(ui->SpinBox_corrSynch->value()).append(" [sec]"));
    }else{
        QMessageBox *ms = new QMessageBox(this);
        QString txt;
        if(errorStation != "__all__"){
            txt = QString("Setup for station %1 already defined!").arg(errorStation);
        }else{
            txt = QString("Setup for all stations is already defined! \nRemove selection and try again!");
        }
        ms->warning(this,"Multiple setup for station",txt);
        delete(ms);
    }
}

void MainWindow::setupStationAxisBufferAddRow()
{
    auto t = ui->treeWidget_setupStationAxis;
    int row = t->topLevelItemCount();
    QString name = ui->comboBox_stationSettingMember_axis->currentText();
    auto groups = para.getGroupStations();
    QIcon ic;
    bool inGroup = groups.find(name.toStdString()) != groups.end();
    if( inGroup || name == "__all__"){
        ic = QIcon(":/icons/icons/station_group_2.png");
    }else{
        ic = QIcon(":/icons/icons/station.png");
    }

    QString errorStation;
    bool valid = true;
    for(int i = 0; i<row; ++i){
        QString itmName = t->topLevelItem(i)->text(0);
        if(name == "__all__" || itmName == "__all__"){
            valid = false;
            errorStation = QString("__all__");
            break;
        }

        if(groups.find(itmName.toStdString()) != groups.end()){
            std::vector<std::string> itmMembers = groups.at(itmName.toStdString());
            if(inGroup){
                std::vector<std::string> members = groups.at(name.toStdString());
                for(const auto &any:members){
                    if(std::find(itmMembers.begin(),itmMembers.end(),any) != itmMembers.end()){
                        valid = false;
                        errorStation = QString::fromStdString(any);
                        break;
                    }
                }
            }else{
                if(std::find(itmMembers.begin(),itmMembers.end(),name.toStdString()) != itmMembers.end()){
                    valid = false;
                    errorStation = name;
                    break;
                }
            }
        }else{
            if(inGroup){
                std::vector<std::string> members = groups.at(name.toStdString());
                if(std::find(members.begin(),members.end(),itmName.toStdString()) != members.end()){
                    valid = false;
                    errorStation = itmName;
                    break;
                }
            }else{
                if(itmName.toStdString() == name.toStdString()){
                    valid = false;
                    errorStation = name;
                    break;
                }
            }
        }
    }

    if(valid){
        t->insertTopLevelItem(row,new QTreeWidgetItem());
        t->topLevelItem(row)->setText(0,name);
        t->topLevelItem(row)->setIcon(0,ic);
        t->topLevelItem(row)->setText(1,QString::number(ui->DoubleSpinBox_axis1low->value()).append(" [deg]"));
        t->topLevelItem(row)->setText(2,QString::number(ui->DoubleSpinBox_axis1up->value()).append(" [deg]"));
        t->topLevelItem(row)->setText(3,QString::number(ui->DoubleSpinBox_axis2low->value()).append(" [deg]"));
        t->topLevelItem(row)->setText(4,QString::number(ui->DoubleSpinBox_axis2up->value()).append(" [deg]"));
    }else{
        QMessageBox *ms = new QMessageBox(this);
        QString txt;
        if(errorStation != "__all__"){
            txt = QString("Setup for station %1 already defined!").arg(errorStation);
        }else{
            txt = QString("Setup for all stations is already defined! \nRemove selection and try again!");
        }
        ms->warning(this,"Multiple setup for station",txt);
        delete(ms);
    }
}

void MainWindow::on_pushButton_14_clicked()
{
    auto t = ui->treeWidget_setupStationWait;
    auto sel = t->selectedItems();
    for(int i=0; i<sel.count(); ++i){
        delete(sel.at(i));
    }
}

void MainWindow::on_pushButton_16_clicked()
{
    auto t = ui->treeWidget_setupStationAxis;
    auto sel = t->selectedItems();
    for(int i=0; i<sel.count(); ++i){
        delete(sel.at(i));
    }
}



void MainWindow::on_pushButton_13_clicked()
{
    for(int i=0; i<allSourceModel->rowCount(); ++i){
        on_treeView_allAvailabeSources_clicked(allSourceModel->index(i,0));
    }
}


void MainWindow::on_pushButton_15_clicked()
{
    selectedSourceModel->clear();
    selectedSources->clear();
}

void MainWindow::worldmap_clicked(QPointF point)
{
    bool x = true;
}

void MainWindow::on_pushButton_skymapZoomFull_clicked()
{
    skymap->chart()->axisX()->setRange(-2.85,2.85);
    skymap->chart()->axisY()->setRange(-1.45,1.45);
}
