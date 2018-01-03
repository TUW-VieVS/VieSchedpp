#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCoreApplication::setApplicationName("VieVS Scheduler");
    QCoreApplication::setApplicationVersion("v0.8");
    ui->label_version->setText("Version: " + QCoreApplication::applicationVersion());
    ui->label_version->setFont(QFont(QApplication::font().family(),8));
    QCoreApplication::setOrganizationName("TU Wien");
    QCoreApplication::setOrganizationDomain("http://hg.geo.tuwien.ac.at/");

    this->setWindowTitle("VieVS Scheduler");

    mainPath = QCoreApplication::applicationFilePath();
    QStringList mainPathSplit = mainPath.split("/");
    mainPathSplit.removeLast();
    mainPathSplit.removeLast();
    mainPath = mainPathSplit.join("/");
    ui->pathToGUILineEdit->setText(mainPath);
    ui->defaultSettingsFileLineEdit->setText(mainPath+"/settings.txt");
    QLabel *il = new QLabel;
    QPixmap ic(":/icons/icons/emblem-important-4.png");
    ic = ic.scaled(16,16);
    il->setPixmap(ic);
    ui->horizontalLayout->insertWidget(0,il);

    QFile file;
    file.setFileName("settings.xml");
    if(!file.exists()){
        QMessageBox mb;
        QString txt = "Before you start, please make sure to set the path to the VieVS Scheduling executeable in the settings <img src=\":/icons/icons/emblem-system-2.png\" height=\"30\" width=\"30\"/> page!";
        mb.information(this,"Before you start!",txt);
        createDefaultParameterSettings();
    }

    std::ifstream iSettings("settings.xml");
    boost::property_tree::read_xml(iSettings,settings,boost::property_tree::xml_parser::trim_whitespace);
    readSettings();

    setupChanged = false;
    setupStation = new QChartView;
    setupSource = new QChartView;
    setupBaseline = new QChartView;
    prepareSetupPlot(setupStation, ui->verticalLayout_28);
    prepareSetupPlot(setupSource, ui->verticalLayout_36);
    prepareSetupPlot(setupBaseline, ui->verticalLayout_40);

    QPushButton *savePara = new QPushButton(QIcon(":/icons/icons/document-export.png"),"",this);
    savePara->setToolTip("save parameter file");
    savePara->setStatusTip("save parameter file");
    connect(savePara,SIGNAL(clicked(bool)),this,SLOT(writeXML()));
    savePara->setMinimumSize(30,30);
    ui->statusBar->addPermanentWidget(savePara);

    QPushButton *createSchedule = new QPushButton(QIcon(":/icons/icons/arrow-right-3.png"),"",this);
    createSchedule->setToolTip("save parameter file and create schedule");
    createSchedule->setStatusTip("save parameter file and create schedule");
    connect(createSchedule,SIGNAL(clicked(bool)),this,SLOT(on_actionRun_triggered()));
    createSchedule->setMinimumSize(30,30);
    ui->statusBar->addPermanentWidget(createSchedule);

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

    connect(selectedStationModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(networkSizeChanged()));
    connect(selectedStationModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(networkSizeChanged()));

    connect(selectedSourceModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(sourceListChanged()));
    connect(selectedSourceModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(sourceListChanged()));

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

    ui->comboBox_setupSource->setModel(selectedSourceModel);
    ui->comboBox_setupBaseline->setModel(selectedBaselineModel);


    connect(ui->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved()));
    connect(ui->splitter_2, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved()));
    connect(ui->splitter_3, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved()));

    ui->splitter->setStretchFactor(1,3);
    ui->splitter_5->setStretchFactor(1,3);

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

    connect(ui->pushButton_addGroupStationSetup_2,SIGNAL(clicked(bool)),this,SLOT(addGroupStation()));
    connect(ui->pushButton_addGroupsourceSetup_2,SIGNAL(clicked(bool)),this,SLOT(addGroupSource()));
    connect(ui->pushButton_addGroupBaselineSetup_2,SIGNAL(clicked(bool)),this,SLOT(addGroupBaseline()));

    connect(ui->lineEdit_faqSearch,SIGNAL(textChanged(QString)),this,SLOT(on_pushButton_faqSearch_clicked()));

    ui->spinBox_scanSequenceCadence->setValue(2);
    ui->spinBox_scanSequenceCadence->setMinimum(2);

    setupStatisticView();


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
    t->setColumnCount(1);
    if (name == "__all__"){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/station_group_2.png"),QString("Group: %1").arg(name)));
        t->setRowCount(selectedStationModel->rowCount());
        for(int i=0; i<selectedStationModel->rowCount(); ++i){
            QString txt = selectedStationModel->index(i,0).data().toString();
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),txt));
        }
    }else if(groupSta.find(name.toStdString()) != groupSta.end()){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/station_group_2.png"),QString("Group: %1").arg(name)));
        auto members = groupSta.at(name.toStdString());
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = QString::fromStdString(members.at(i));
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),txt));
        }
    }else{
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),QString("Station: %1").arg(name)));
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
    t->setColumnCount(1);
    if (name == "__all__"){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/source_group.png"),QString("Group: %1").arg(name)));
        t->setRowCount(selectedSourceModel->rowCount());
        for(int i=0; i<selectedSourceModel->rowCount(); ++i){
            QString txt = selectedSourceModel->index(i,0).data().toString();
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/source.png"),txt));
        }
    }else if(groupSrc.find(name.toStdString()) != groupSrc.end()){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/source_group.png"),QString("Group: %1").arg(name)));
        auto members = groupSrc.at(name.toStdString());
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = QString::fromStdString(members.at(i));
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/source.png"),txt));
        }
    }else{
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/source.png"),QString("Source: %1").arg(name)));
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
    t->setColumnCount(1);
    if (name == "__all__"){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/baseline_group.png"),QString("Group: %1").arg(name)));
        t->setRowCount(selectedBaselineModel->rowCount());
        for(int i=0; i<selectedBaselineModel->rowCount(); ++i){
            QString txt = selectedBaselineModel->index(i,0).data().toString();
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/baseline.png"),txt));
        }
    }else if(groupBl.find(name.toStdString()) != groupBl.end()){
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/baseline_group.png"),QString("Group: %1").arg(name)));
        auto members = groupBl.at(name.toStdString());
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = QString::fromStdString(members.at(i));
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/baseline.png"),txt));
        }
    }else{
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/baseline.png"),QString("Baseline: %1").arg(name)));
        t->setRowCount(2);
        QStringList list= name.split("-");
        t->setItem(0,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),list.at(0)));
        t->setItem(0,1,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),list.at(1)));
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
    if(para.ignoreSourcesString.size() > 0){
          for(const auto &any: para.ignoreSourcesString){
              t->insertRow(r);
              if(groupSrc.find(any) != groupSrc.end() || any == "__all__"){
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore source group"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/source_group.png"),QString::fromStdString(any)));
              }else{
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore source"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/source.png"),QString::fromStdString(any)));
              }
              ++r;
          }

//        std::string txt = "";
//        for(const auto &any: para.ignoreSources_str){
//            txt.append(any).append(", ");
//        }
//        txt = txt.substr(0,txt.size()-2);
//        t->insertRow(r);
//        t->setItem(r,0,new QTableWidgetItem(QString::fromStdString(txt)));
//        t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore sources"));
//        ++r;

    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::displaySourceSetupParameter(QString name){
    auto t = ui->tableWidget_setupSource;
    t->clear();
    t->setColumnCount(1);
    t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Parameter: %1").arg(name)));
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
          for(const auto &any: para.ignoreStationsString){
              t->insertRow(r);
              if(groupSta.find(any) != groupSta.end() || any == "__all__"){
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore station group"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/station_group_2.png"),QString::fromStdString(any)));
              }else{
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore station"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),QString::fromStdString(any)));
              }
              ++r;
          }
    }
    if(para.requiredStationsString.size() > 0){
          for(const auto &any: para.requiredStationsString){
              t->insertRow(r);
              if(groupSta.find(any) != groupSta.end() || any == "__all__"){
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("required station group"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/station_group_2.png"),QString::fromStdString(any)));
              }else{
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("required stations"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),QString::fromStdString(any)));
              }
              ++r;
          }

    }
    if(para.ignoreBaselinesString.size() > 0){
          for(const auto &any: para.ignoreBaselinesString){
              t->insertRow(r);
              if(groupBl.find(any) != groupBl.end() || any == "__all__"){
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore baseline group"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/baseline_group.png"),QString::fromStdString(any)));
              }else{
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore baseline"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/baseline.png"),QString::fromStdString(any)));
              }
              ++r;
          }

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
    VieVS::ParameterSettings::ParametersBaselines para = paraBl.at(name.toStdString());
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

void MainWindow::on_actionSettings_triggered()
{
    ui->main_stacked->setCurrentIndex(1);
}

void MainWindow::on_actionInput_triggered()
{
    ui->main_stacked->setCurrentIndex(2);
}

void MainWindow::on_actionMode_triggered()
{
    ui->main_stacked->setCurrentIndex(3);
}

void MainWindow::on_actionGeneral_triggered()
{
    ui->main_stacked->setCurrentIndex(4);
}

void MainWindow::on_actionStation_triggered()
{
    ui->main_stacked->setCurrentIndex(5);
}

void MainWindow::on_actionSource_triggered()
{
    ui->main_stacked->setCurrentIndex(6);
}

void MainWindow::on_actionBaseline_triggered()
{
    ui->main_stacked->setCurrentIndex(7);
}

void MainWindow::on_actionWeight_factors_triggered()
{
    ui->main_stacked->setCurrentIndex(8);
}

void MainWindow::on_actionOutput_triggered()
{
    ui->main_stacked->setCurrentIndex(9);
}

void MainWindow::on_actionRules_triggered()
{
    ui->main_stacked->setCurrentIndex(10);
}

void MainWindow::on_actionMulti_Scheduling_triggered()
{
    ui->main_stacked->setCurrentIndex(11);
}

void MainWindow::on_actionSky_Coverage_triggered()
{
    ui->main_stacked->setCurrentIndex(12);
}

void MainWindow::on_actionsummary_triggered()
{
    ui->main_stacked->setCurrentIndex(13);
}

void MainWindow::on_actionFAQ_triggered()
{
    ui->main_stacked->setCurrentIndex(14);
}

void MainWindow::on_actionWhat_is_this_triggered()
{
    QWhatsThis::enterWhatsThisMode();
}

QString MainWindow::on_actionSave_triggered()
{
    QString txt = "Do you want to save xml file?";

    QString result;
    if (QMessageBox::Yes == QMessageBox::question(this, "Save?", txt, QMessageBox::Yes | QMessageBox::No)){
        result = writeXML();
    }
    return result;
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

            int r = 0;
            for(int i = 0; i<allSourcePlusGroupModel->rowCount(); ++i){
                QString txt = allSourcePlusGroupModel->item(i)->text();
                if(groupSrc.find(txt.toStdString()) != groupSrc.end() || txt == "__all__"){
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

    auto stationTree = settings.get_child_optional("settings.station.parameters");
    if(stationTree.is_initialized()){
        for(const auto& it: *stationTree){
            std::string parameterName = it.second.get_child("<xmlattr>.name").data();
            if(parameterName == "default"){
                for (auto &it2: it.second) {
                    std::string paraName = it2.first;
                    if (paraName == "<xmlattr>") {
                        continue;
                    } else if (paraName == "weight") {
                        sta.weight = it2.second.get_value<double>();
                    } else if (paraName == "minScan") {
                        sta.minScan = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "maxScan") {
                        sta.maxScan = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "maxSlewtime") {
                        sta.maxSlewtime = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "maxWait") {
                        sta.maxWait = it2.second.get_value < unsigned int > ();
                    } else {
                        QString txt = "Ignoring parameter: ";
                        txt.append(QString::fromStdString(paraName)).append(" in station default parameters!\nCheck settings.xml file!");
                        QMessageBox::warning(this,"Wrong default setting!",txt,QMessageBox::Ok);
                    }
                }
            }
        }
    }else{
        QMessageBox::warning(this,"Missing default parameters!","You have no entry for your default station parameters in settings.xml file! Internal backup values are used!",QMessageBox::Ok);
    }

    VieVS::ParameterSettings::ParametersSources src;
    src.available = true;
    src.minRepeat = 1800;
    src.minScan = 20;
    src.maxScan = 600;
    src.weight = 1;
    src.minFlux = 0.05;
    src.maxNumberOfScans = 999;
    src.minNumberOfStations = 2;

    auto sourceTree = settings.get_child_optional("settings.source.parameters");
    if(sourceTree.is_initialized()){
        for(const auto& it: *sourceTree){
            std::string parameterName = it.second.get_child("<xmlattr>.name").data();
            if(parameterName == "default"){
                for (auto &it2: it.second) {
                    std::string paraName = it2.first;
                    if (paraName == "<xmlattr>") {
                        continue;
                    } else if (paraName == "weight") {
                        src.weight = it2.second.get_value<double>();
                    } else if (paraName == "minScan") {
                        src.minScan = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "maxScan") {
                        src.maxScan = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "minRepeat") {
                        src.minRepeat = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "minFlux"){
                        src.minFlux = it2.second.get_value<double>();
                    } else if (paraName == "maxNumberOfScans"){
                        src.maxNumberOfScans = it2.second.get_value<double>();
                    } else if (paraName == "minNumberOfStations"){
                        src.minNumberOfStations = it2.second.get_value<double>();
                    } else {
                        QString txt = "Ignoring parameter: ";
                        txt.append(QString::fromStdString(paraName)).append(" in source default parameters!\nCheck settings.xml file!");
                        QMessageBox::warning(this,"Wrong default setting!",txt,QMessageBox::Ok);
                    }
                }
            }
        }
    }else{
        QMessageBox::warning(this,"Missing default parameters!","You have no entry for your default source parameters in settings.xml file! Internal backup values are used!",QMessageBox::Ok);
    }

    VieVS::ParameterSettings::ParametersBaselines bl;
    bl.ignore = false;
    bl.maxScan = 600;
    bl.minScan = 20;
    bl.weight = 1;
    auto baselineTree = settings.get_child_optional("settings.baseline.parameters");
    if(baselineTree.is_initialized()){
        for(const auto& it: *baselineTree){
            std::string parameterName = it.second.get_child("<xmlattr>.name").data();
            if(parameterName == "default"){
                for (auto &it2: it.second) {
                    std::string paraName = it2.first;
                    if (paraName == "<xmlattr>") {
                        continue;
                    } else if (paraName == "weight") {
                        src.weight = it2.second.get_value<double>();
                    } else if (paraName == "minScan") {
                        src.minScan = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "maxScan") {
                        src.maxScan = it2.second.get_value < unsigned int > ();
                    } else {
                        QString txt = "Ignoring parameter: ";
                        txt.append(QString::fromStdString(paraName)).append(" in baseline default parameters!\nCheck settings.xml file!");
                        QMessageBox::warning(this,"Wrong default setting!",txt,QMessageBox::Ok);
                    }
                }
            }
        }
    }else{
        QMessageBox::warning(this,"Missing default parameters!","You have no entry for your default baseline parameters in settings.xml file! Internal backup values are used!",QMessageBox::Ok);
    }

    paraSta["default"] = sta;
    ui->ComboBox_parameterStation->addItem("default");

    paraSrc["default"] = src;
    ui->ComboBox_parameterSource->addItem("default");

    paraBl["default"] = bl;
    ui->ComboBox_parameterBaseline->addItem("default");

    clearSetup(true,true,true);

    bool waitTimeMissing = false;
    boost::optional<int> setup = settings.get_optional<int>("settings.station.waitTimes.setup");
    if(setup.is_initialized()){
        ui->SpinBox_setup->setValue(*setup);
    }else{
        waitTimeMissing = true;
    }
    boost::optional<int> source = settings.get_optional<int>("settings.station.waitTimes.source");
    if(source.is_initialized()){
        ui->SpinBox_source->setValue(*source);
    }else{
        waitTimeMissing = true;
    }
    boost::optional<int> tape = settings.get_optional<int>("settings.station.waitTimes.tape");
    if(tape.is_initialized()){
        ui->SpinBox_tape->setValue(*tape);
    }else{
        waitTimeMissing = true;
    }
    boost::optional<int> calibration = settings.get_optional<int>("settings.station.waitTimes.calibration");
    if(calibration.is_initialized()){
        ui->SpinBox_calibration->setValue(*calibration);
    }else{
        waitTimeMissing = true;
    }
    boost::optional<int> corsynch = settings.get_optional<int>("settings.station.waitTimes.corsynch");
    if(corsynch.is_initialized()){
        ui->SpinBox_corrSynch->setValue(*corsynch);
    }else{
        waitTimeMissing = true;
    }
    if(waitTimeMissing){
        QMessageBox::warning(this,"Missing default parameters!","You have no entry for your default station wait times in settings.xml file! Internal backup values are used!",QMessageBox::Ok);
    }


    bool bufferMissing = false;
    boost::optional<int> ax1low = settings.get_optional<int>("settings.station.cableWrapBuffers.axis1LowOffset");
    if(ax1low.is_initialized()){
        ui->DoubleSpinBox_axis1low->setValue(*ax1low);
    }else{
        bufferMissing = true;
    }
    boost::optional<int> ax1up = settings.get_optional<int>("settings.station.cableWrapBuffers.axis1UpOffset");
    if(ax1up.is_initialized()){
        ui->DoubleSpinBox_axis1up->setValue(*ax1up);
    }else{
        bufferMissing = true;
    }
    boost::optional<int> ax2low = settings.get_optional<int>("settings.station.cableWrapBuffers.axis2LowOffset");
    if(ax2low.is_initialized()){
        ui->DoubleSpinBox_axis2low->setValue(*ax2low);
    }else{
        bufferMissing = true;
    }
    boost::optional<int> ax2up = settings.get_optional<int>("settings.station.cableWrapBuffers.axis2UpOffset");
    if(ax2up.is_initialized()){
        ui->DoubleSpinBox_axis2up->setValue(*ax2up);
    }else{
        bufferMissing = true;
    }
    if(bufferMissing){
        QMessageBox::warning(this,"Missing default parameters!","You have no entry for your default station axis limit buffer in settings.xml file! Internal backup values are used!",QMessageBox::Ok);
    }

}

void MainWindow::on_listView_allSelectedStations_clicked(const QModelIndex &index)
{

    QString name = selectedStationModel->item(index.row())->text();
    selectedStationModel->removeRow(index.row());
    clearGroup(true,false,true, name);

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
    ui->treeWidget_multiSchedSelected->clear();
    for(int i=0; i<ui->treeWidget_multiSched->topLevelItemCount(); ++i){
        ui->treeWidget_multiSched->topLevelItem(i)->setDisabled(false);
    }
}

void MainWindow::on_listView_allSelectedSources_clicked(const QModelIndex &index)
{
    QString name = selectedSourceModel->item(index.row())->text();
    if(ui->comboBox_calibratorBlock_calibratorSources->currentText() == name){
        QMessageBox::warning(this,"Calibration block error!","Deleted source was choosen as calibrator source!\nCheck calibrator block!");
        ui->comboBox_calibratorBlock_calibratorSources->setCurrentIndex(0);
    }
    for(int i=0; i<ui->tableWidget_scanSequence->rowCount(); ++i){
        QComboBox* cb = qobject_cast<QComboBox*>(ui->tableWidget_scanSequence->cellWidget(i,0));
        if(cb->currentText() == name){
            QMessageBox::warning(this,"Scan sequence error!","Deleted source was in scan sequence!\nCheck scan sequence!");
            cb->setCurrentIndex(0);
        }
    }
    selectedSourceModel->removeRow(index.row());

    clearGroup(false,true,false,name);

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
    ui->treeWidget_multiSchedSelected->clear();
    for(int i=0; i<ui->treeWidget_multiSched->topLevelItemCount(); ++i){
        ui->treeWidget_multiSched->topLevelItem(i)->setDisabled(false);
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

        int r = 0;
        for(int i = 0; i<allStationPlusGroupModel->rowCount(); ++i){
            QString txt = allStationPlusGroupModel->item(i)->text();
            if(groupSta.find(txt.toStdString()) != groupSta.end() || txt == "__all__"){
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

        int r = 0;
        for(int i = 0; i<allSourcePlusGroupModel->rowCount(); ++i){
            QString txt = allSourcePlusGroupModel->item(i)->text();
            if(groupSrc.find(txt.toStdString()) != groupSrc.end() || txt == "__all__"){
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
    QDoubleSpinBox *dsp = new QDoubleSpinBox(this);
    if(name == "S"){
        dsp->setValue(15.);
    }else{
        dsp->setValue(20.);
    }
    dsp->setMaximum(1000);
    QComboBox *psta = new QComboBox(this);
    psta->addItem("required");
    psta->addItem("optional");
    QComboBox *psrc = new QComboBox(this);
    psrc->addItem("required");
    psrc->addItem("optional");
    QComboBox *bsta = new QComboBox(this);
    bsta->addItem("none");
    bsta->addItem("value");
    bsta->addItem("min value Times");
    bsta->addItem("max value Times");
    QComboBox *bsrc = new QComboBox(this);
    bsrc->addItem("none");
    bsrc->addItem("value");
    bsrc->addItem("min value Times");
    bsrc->addItem("max value Times");
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
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,0,dsp);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,1,psta);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,2,bsta);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,3,vsta);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,4,psrc);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,5,bsrc);
    ui->tableWidget_ModesPolicy->setCellWidget(ui->tableWidget_ModesPolicy->rowCount()-1,6,vsrc);


}

void MainWindow::createModesCustonBandTable()
{
    addModesCustomTable("X",8.590,10);
    addModesCustomTable("S",2.260,6);

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

    QTreeWidget *t = ui->treeWidget_multiSched;

    t->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    t->expandAll();
    ui->treeWidget_multiSchedSelected->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
    addBandDialog *dial = new addBandDialog(settings,this);
    int result = dial->exec();

    if(result == QDialog::Accepted){
        QString name = dial->getBandName();
        double freq = dial->getFrequency();
        int channels = dial->getChannels();
        addModesCustomTable(name,freq,channels);
    }

    delete(dial);
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

void MainWindow::on_pushButton_multiSchedAddSelected_clicked()
{
    auto tall = ui->treeWidget_multiSched;
    auto list = tall->selectedItems();

    for(const auto&any:list){

        if(any->parent()){
            QString name = any->text(0);
            QString parameterType = any->parent()->text(0);

            QStringList row2toggle{"subnetting",
                                   "fillin mode"};

            QStringList row2intDialog {"max slew time",
                                      "max wait time",
                                      "max scan time",
                                      "min scan time",
                                      "min number of stations",
                                      "min repeat time"};

            QStringList row2doubleDialog {"sky coverage",
                                          "number of observations",
                                          "duration",
                                          "average stations",
                                          "average sources",
                                          "min flux",
                                          "weight"};

            QStringList row2dateTimeDialog {"session start"};

            QIcon ic;
            if(parameterType == "General"){
                ic = QIcon(":/icons/icons/applications-internet-2.png");
            }else if(parameterType == "Weight factor"){
                ic = QIcon(":/icons/icons/weight.png");
            }else if(parameterType == "Station"){
                ic = QIcon(":/icons/icons/station.png");
            }else if(parameterType == "Source"){
                ic = QIcon(":/icons/icons/source.png");
            }else if(parameterType == "Baseline"){
                ic = QIcon(":/icons/icons/baseline.png");
            }

            auto t = ui->treeWidget_multiSchedSelected;

            QTreeWidgetItem * itm = new QTreeWidgetItem();

            if(row2toggle.indexOf(name) != -1){
                if(parameterType == "General" || parameterType == "Weight factor"){
                    any->setDisabled(true);
                }
                QString valuesString = "True, False";

                itm->setText(0,name);
                itm->setIcon(0,ic);
                itm->setText(1,"global");
                itm->setIcon(1,QIcon(":/icons/icons/applications-internet-2.png"));
                itm->setText(2,"2");

                QComboBox *cb = new QComboBox(this);
                cb->addItem("True");
                cb->addItem("False");

                t->addTopLevelItem(itm);
                t->setItemWidget(itm,3,cb);

            }else if(row2intDialog.indexOf(name) != -1){
                multiSchedEditDialogInt *dialog = new multiSchedEditDialogInt(this);
                if(parameterType == "Station"){
                    dialog->addMember(allStationPlusGroupModel);
                }else if(parameterType == "Source"){
                    dialog->addMember(allSourcePlusGroupModel);
                }else if(parameterType == "Baseline"){
                    dialog->addMember(allBaselinePlusGroupModel);
                }
                int result = dialog->exec();
                if(result == QDialog::Accepted){
                    if(parameterType == "General" || parameterType == "Weight factor"){
                        any->setDisabled(true);
                    }
                    QVector<int> val = dialog->getValues();
                    int n = val.size();
                    if(parameterType == "Station" || parameterType == "Source" || parameterType == "Baseline"){
                        QStandardItem* member = dialog->getMember();
                        itm->setText(1,member->text());
                        itm->setIcon(1,member->icon());
                    }else{
                        itm->setText(1,"global");
                        itm->setIcon(1,QIcon(":/icons/icons/applications-internet-2.png"));
                    }
                    QComboBox *cb = new QComboBox(this);
                    for(const auto& any:val){
                        cb->addItem(QString::number(any));
                    }

                    itm->setText(2,QString::number(n));
                    itm->setText(0,name);
                    itm->setIcon(0,ic);
                    t->addTopLevelItem(itm);
                    t->setItemWidget(itm,3,cb);

                }
                delete(dialog);

            }else if(row2doubleDialog.indexOf(name) != -1){
                multiSchedEditDialogDouble *dialog = new multiSchedEditDialogDouble(this);
                if(parameterType == "Station"){
                    dialog->addMember(allStationPlusGroupModel);
                }else if(parameterType == "Source"){
                    dialog->addMember(allSourcePlusGroupModel);
                }else if(parameterType == "Baseline"){
                    dialog->addMember(allBaselinePlusGroupModel);
                }else{
                    itm->setText(1,"global");
                    itm->setIcon(1,QIcon(":/icons/icons/applications-internet-2.png"));
                }
                int result = dialog->exec();
                if(result == QDialog::Accepted){
                    if(parameterType == "General" || parameterType == "Weight factor"){
                        any->setDisabled(true);
                    }
                    QVector<double> val = dialog->getValues();
                    int n = val.size();

                    if(parameterType == "Station" || parameterType == "Source" || parameterType == "Baseline"){
                        QStandardItem* member = dialog->getMember();
                        itm->setText(1,member->text());
                        itm->setIcon(1,member->icon());
                    }else{
                        itm->setText(1,"global");
                        itm->setIcon(1,QIcon(":/icons/icons/applications-internet-2.png"));
                    }
                    QComboBox *cb = new QComboBox(this);
                    for(const auto& any:val){
                        cb->addItem(QString::number(any));
                    }

                    itm->setText(2,QString::number(n));
                    itm->setText(0,name);
                    itm->setIcon(0,ic);
                    t->addTopLevelItem(itm);
                    t->setItemWidget(itm,3,cb);
                }
                delete(dialog);

            }else if(row2dateTimeDialog.indexOf(name) != -1){
                multiSchedEditDialogDateTime *dialog = new multiSchedEditDialogDateTime(this);

                int result = dialog->exec();
                if(result == QDialog::Accepted){
                    if(parameterType == "General" || parameterType == "Weight factor"){
                        any->setDisabled(true);
                    }
                    QVector<QDateTime> val = dialog->getValues();
                    int n = val.size();

                    itm->setText(1,"global");
                    itm->setIcon(1,QIcon(":/icons/icons/applications-internet-2.png"));
                    QComboBox *cb = new QComboBox(this);
                    for(const auto& any:val){
                        cb->addItem(any.toString("dd.MM.yyyy hh:mm"));
                    }

                    itm->setText(2,QString::number(n));
                    itm->setText(0,name);
                    itm->setIcon(0,ic);
                    t->addTopLevelItem(itm);
                    t->setItemWidget(itm,3,cb);
                }
                delete(dialog);
            }

            int nsched = 1;
            QVector<double>weightSkyCoverage_;
            QVector<double>weightNumberOfObservations_;
            QVector<double>weightDuration_;
            QVector<double>weightAverageSources_;
            QVector<double>weightAverageStations_;
            for(int i = 0; i<t->topLevelItemCount(); ++i){
                if(t->topLevelItem(i)->text(0) == "sky coverage"){
                    QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
                    for(int ilist = 0; ilist<list->count(); ++ilist){
                        weightSkyCoverage_.push_back( QString(list->itemText(ilist)).toDouble());
                    }
                }
                if(t->topLevelItem(i)->text(0) == "number of observations"){
                    QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
                    for(int ilist = 0; ilist<list->count(); ++ilist){
                        weightNumberOfObservations_.push_back( QString(list->itemText(ilist)).toDouble());
                    }
                }
                if(t->topLevelItem(i)->text(0) == "duration"){
                    QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
                    for(int ilist = 0; ilist<list->count(); ++ilist){
                        weightDuration_.push_back( QString(list->itemText(ilist)).toDouble());
                    }
                }
                if(t->topLevelItem(i)->text(0) == "average stations"){
                    QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
                    for(int ilist = 0; ilist<list->count(); ++ilist){
                        weightAverageSources_.push_back( QString(list->itemText(ilist)).toDouble());
                    }
                }
                if(t->topLevelItem(i)->text(0) == "average sources"){
                    QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
                    for(int ilist = 0; ilist<list->count(); ++ilist){
                        weightAverageStations_.push_back( QString(list->itemText(ilist)).toDouble());
                    }
                }
            }

            QVector<QVector<double> > weightFactors;
            if(!weightSkyCoverage_.empty() ||
                    !weightNumberOfObservations_.empty() ||
                    !weightDuration_.empty() ||
                    !weightAverageSources_.empty() ||
                    !weightAverageStations_.empty()) {

                if (weightSkyCoverage_.empty()) {
                    if(ui->checkBox_weightCoverage->isChecked()){
                        weightSkyCoverage_.push_back(ui->doubleSpinBox_weightSkyCoverage->value());
                    }else{
                        weightSkyCoverage_.push_back(0);
                    }
                }
                if (weightNumberOfObservations_.empty()) {
                    if(ui->checkBox_weightNobs->isChecked()){
                        weightNumberOfObservations_.push_back(ui->doubleSpinBox_weightNumberOfObservations->value());
                    }else{
                        weightNumberOfObservations_.push_back(0);
                    }
                }
                if (weightDuration_.empty()) {
                    if(ui->checkBox_weightDuration->isChecked()){
                        weightDuration_.push_back(ui->doubleSpinBox_weightDuration->value());
                    }else{
                        weightDuration_.push_back(0);
                    }
                }
                if (weightAverageSources_.empty()) {
                    if(ui->checkBox_weightAverageSources->isChecked()){
                        weightAverageSources_.push_back(ui->doubleSpinBox_weightAverageSources->value());
                    }else{
                        weightAverageSources_.push_back(0);
                    }
                }
                if (weightAverageStations_.empty()) {
                    if(ui->checkBox_weightAverageStations->isChecked()){
                        weightAverageStations_.push_back(ui->doubleSpinBox_weightAverageStations->value());
                    }else{
                        weightAverageStations_.push_back(0);
                    }
                }

                for (double wsky: weightSkyCoverage_) {
                    for (double wobs: weightNumberOfObservations_) {
                        for (double wdur: weightDuration_) {
                            for (double wasrc: weightAverageSources_) {
                                for (double wsta: weightAverageStations_) {
                                    double sum = wsky + wobs + wdur + wasrc + wsta;

                                    if(sum == 0){
                                        continue;
                                    }

                                    QVector<double> wf {wsky/sum, wobs/sum, wdur/sum, wasrc/sum, wsta/sum};

                                    weightFactors.push_back(wf);
                                }
                            }
                        }
                    }
                }

                int i1 = 0;
                while (i1 < weightFactors.size()) {
                    const QVector<double> &v1 = weightFactors[i1];
                    int i2 = i1 + 1;

                    while (i2 < weightFactors.size()) {
                        const QVector<double> &v2 = weightFactors[i2];
                        int equal = 0;
                        for (int i3 = 0; i3 < v1.size(); ++i3) {
                            if (abs(v1[i3] - v2[i3]) < 1e-10) {
                                ++equal;
                            }
                        }
                        if (equal == v1.size()) {
                            weightFactors.erase(std::next(weightFactors.begin(), i2));
                        } else {
                            ++i2;
                        }
                    }
                    ++i1;
                }

            }

            if (!weightFactors.empty()) {
                nsched = weightFactors.count();
            }

            QStringList weightFactorsStr {"sky coverage",
                                          "number of observations",
                                          "duration",
                                          "average stations",
                                          "average sources"};

            for(int i = 0; i<t->topLevelItemCount(); ++i){
                if(weightFactorsStr.indexOf(t->topLevelItem(i)->text(0)) != -1){
                    continue;
                }
                nsched *= t->topLevelItem(i)->text(2).toInt();
            }
            ui->label_multiSchedulingNsched->setText(QString::number(nsched));
        }
    }
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

QString MainWindow::writeXML()
{
    para = VieVS::ParameterSettings();
    para.software(QApplication::applicationName().toStdString(), QApplication::applicationVersion().toStdString());

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    para.created(now, ui->nameLineEdit->text().toStdString(), ui->emailLineEdit->text().toStdString());

    int startYear = ui->dateTimeEdit_sessionStart->date().year();
    int startMonth = ui->dateTimeEdit_sessionStart->date().month();
    int startDay = ui->dateTimeEdit_sessionStart->date().day();
    int startHour = ui->dateTimeEdit_sessionStart->time().hour();
    int startMinute = ui->dateTimeEdit_sessionStart->time().minute();
    int startSecond = ui->dateTimeEdit_sessionStart->time().second();
    boost::posix_time::ptime start(boost::gregorian::date(startYear, startMonth, startDay), boost::posix_time::time_duration(startHour, startMinute, startSecond));
    int duration = ui->doubleSpinBox_sessionDuration->value()*3600;
    boost::posix_time::ptime end = start + boost::posix_time::seconds(duration);
    std::vector<std::string> station_names;
    for(int i=0; i<selectedStationModel->rowCount(); ++i){
        station_names.push_back(selectedStationModel->item(i)->text().toStdString());
    }
    bool fillinMode = ui->checkBox_fillinMode->isChecked();
    bool subnetting = ui->checkBox_subnetting->isChecked();
    bool fillinModeInfluence = ui->checkBox_fillinModeInfluence->isChecked();
    double minElevation = ui->doubleSpinBox_minElevation->value();
    para.general(start, end, subnetting, fillinMode, fillinModeInfluence, minElevation, station_names);


    std::string experimentName = ui->experimentNameLineEdit->text().toStdString();
    std::string experimentDescription = ui->plainTextEdit_experimentDescription->toPlainText().toStdString();
    std::string scheduler = ui->schedulerLineEdit->text().toStdString();
    std::string correlator = ui->correlatorLineEdit->text().toStdString();
    bool statistics = ui->checkBox_outputStatisticsFile->isChecked();
    bool ngs = ui->checkBox_outputNGSFile->isChecked();
    bool skd = ui->checkBox_outputSkdFile->isChecked();
    bool skyCov = ui->checkBox_outputSkyCoverageFile->isChecked();
    para.output(experimentName, experimentDescription, scheduler, correlator, statistics, ngs, skd, skyCov);

    std::string antenna = ui->lineEdit_pathAntenna->text().toStdString();
    std::string equip = ui->lineEdit_pathEquip->text().toStdString();
    std::string flux = ui->lineEdit_pathFlux->text().toStdString();
    std::string freq = ui->lineEdit_pathFreq->text().toStdString();
    std::string hdpos = ui->lineEdit_pathHdpos->text().toStdString();
    std::string loif = ui->lineEdit_pathLoif->text().toStdString();
    std::string mask = ui->lineEdit_pathMask->text().toStdString();
    std::string modes = ui->lineEdit_pathModes->text().toStdString();
    std::string position = ui->lineEdit_pathPosition->text().toStdString();
    std::string rec = ui->lineEdit_pathRec->text().toStdString();
    std::string rx = ui->lineEdit_pathRx->text().toStdString();
    std::string source = ui->lineEdit_pathSource->text().toStdString();
    std::string tracks = ui->lineEdit_pathTracks->text().toStdString();
    para.catalogs(antenna, equip, flux, freq, hdpos, loif, mask, modes, position, rec, rx, source, tracks);


    para.setup(VieVS::ParameterSettings::Type::station,setupStationTree);
    para.setup(VieVS::ParameterSettings::Type::source,setupSourceTree);
    para.setup(VieVS::ParameterSettings::Type::baseline,setupBaselineTree);

    for(const auto&any:paraSta){
        para.parameters(any.first, any.second);
    }
    for(const auto&any:paraSrc){
        para.parameters(any.first, any.second);
    }
    for(const auto&any:paraBl){
        para.parameters(any.first, any.second);
    }
    for(const auto&any:groupSta){
        para.group(VieVS::ParameterSettings::Type::station,VieVS::ParameterGroup(any.first,any.second));
    }
    for(const auto&any:groupSrc){
        para.group(VieVS::ParameterSettings::Type::source,VieVS::ParameterGroup(any.first,any.second));
    }
    for(const auto&any:groupBl){
        para.group(VieVS::ParameterSettings::Type::baseline,VieVS::ParameterGroup(any.first,any.second));
    }


    for(int i=0; i<ui->treeWidget_setupStationWait->topLevelItemCount(); ++i){
        auto itm = ui->treeWidget_setupStationWait->topLevelItem(i);
        std::string name = itm->text(0).toStdString();
        int setup = QString(itm->text(1).left(itm->text(1).count()-6)).toInt();
        int source = QString(itm->text(2).left(itm->text(2).count()-6)).toInt();
        int tape = QString(itm->text(3).left(itm->text(3).count()-6)).toInt();
        int calibration = QString(itm->text(4).left(itm->text(4).count()-6)).toInt();
        int corrSynch = QString(itm->text(5).left(itm->text(5).count()-6)).toInt();
        para.stationWaitTimes(name,setup,source,tape,calibration,corrSynch);
    }

    for(int i=0; i<ui->treeWidget_setupStationAxis->topLevelItemCount(); ++i){
        auto itm = ui->treeWidget_setupStationAxis->topLevelItem(i);
        std::string name = itm->text(0).toStdString();
        double ax1low = QString(itm->text(1)).toDouble();
        double ax1up = QString(itm->text(2)).toDouble();
        double ax2low = QString(itm->text(3)).toDouble();
        double ax2up = QString(itm->text(4)).toDouble();
        para.stationCableWrapBuffer(name,ax1low,ax1up,ax2low,ax2up);
    }

    double influenceDistance = ui->influenceDistanceDoubleSpinBox->value();
    double influenceTime = ui->influenceTimeSpinBox->value();
    double maxDistanceTwin = ui->maxDistanceForCombiningAntennasDoubleSpinBox->value();
    para.skyCoverage(influenceDistance,influenceTime,maxDistanceTwin);

    double weightSkyCoverage = 0;
    if(ui->checkBox_weightCoverage->isChecked()){
        weightSkyCoverage = ui->doubleSpinBox_weightSkyCoverage->value();
    }
    double weightNumberOfObservations = 0;
    if(ui->checkBox_weightNobs->isChecked()){
        weightNumberOfObservations = ui->doubleSpinBox_weightNumberOfObservations->value();
    }
    double weightDuration = 0;
    if(ui->checkBox_weightDuration->isChecked()){
        weightDuration = ui->doubleSpinBox_weightDuration->value();
    }
    double weightAverageSources = 0;
    if(ui->checkBox_weightAverageSources->isChecked()){
        weightAverageSources = ui->doubleSpinBox_weightAverageSources->value();
    }
    double weightAverageStations = 0;
    if(ui->checkBox_weightAverageStations->isChecked()){
        weightAverageStations = ui->doubleSpinBox_weightAverageStations->value();
    }
    double weightDeclination = 0;
    double weightDeclinationSlopeStart = 0;
    double weightDeclinationSlopeEnd = 0;
    if(ui->checkBox_weightLowDeclination->isChecked()){
        weightDeclination = ui->doubleSpinBox_weightLowDec->value();
        weightDeclinationSlopeStart = ui->doubleSpinBox_weightLowDecStart->value();
        weightDeclinationSlopeEnd = ui->doubleSpinBox_weightLowDecEnd->value();
    }
    double weightElevation = 0;
    double weightElevationSlopeStart = 0;
    double weightElevationSlopeEnd = 0;
    if(ui->checkBox_weightLowElevation->isChecked()){
        weightElevation = ui->doubleSpinBox_weightLowEl->value();
        weightElevationSlopeStart = ui->doubleSpinBox_weightLowElStart->value();
        weightElevationSlopeEnd = ui->doubleSpinBox_weightLowElEnd->value();
    }
    para.weightFactor(weightSkyCoverage, weightNumberOfObservations, weightDuration, weightAverageSources,
                      weightAverageStations, weightDeclination, weightDeclinationSlopeStart, weightDeclinationSlopeEnd,
                      weightElevation, weightElevationSlopeStart, weightElevationSlopeEnd);

    if(ui->groupBox_scanSequence->isChecked()){
        int cadence = ui->spinBox_scanSequenceCadence->value();
        std::vector<unsigned int> modulo;
        std::vector<std::string> member;
        for(int i = 0; i<cadence; ++i){
            QWidget * w = ui->tableWidget_scanSequence->cellWidget(i,0);
            QComboBox* cb = qobject_cast<QComboBox*>(w);
            modulo.push_back(i);
            member.push_back(cb->currentText().toStdString());
        }
        para.ruleScanSequence(cadence,modulo,member);
    }

    // TODO: change elevationAngles in NScanSelections and change their names
    if(ui->groupBox_CalibratorBlock->isChecked()){
        std::string member = ui->comboBox_calibratorBlock_calibratorSources->currentText().toStdString();
        double lowStart = ui->doubleSpinBox_calibratorLowElStart->value();
        double lowEnd = ui->doubleSpinBox_calibratorLowElEnd->value();
        double highStart = ui->doubleSpinBox_calibratorHighElStart->value();
        double highEnd = ui->doubleSpinBox_calibratorHighElEnd->value();
        std::vector<std::pair<double, double> > elevationAngles {{lowStart,lowEnd},{highStart,highEnd}};
        int nmaxScans = ui->spinBox_calibrator_maxScanSequence->value();
        int scanTime = ui->spinBox_calibratorFixedScanLength->value();
        if(ui->radioButton_calibratorTime->isChecked()){
            int cadence = ui->spinBox_calibratorTime->value();
            para.ruleCalibratorBlockTime(cadence,member,elevationAngles,nmaxScans,scanTime);
        }else if(ui->radioButton_calibratorScanSequence->isChecked()){
            int cadence = ui->spinBox_calibratorScanSequence->value();
            para.ruleCalibratorBlockNScanSelections(cadence,member,elevationAngles,nmaxScans,scanTime);
        }
    }

    if(ui->groupBox_modeSked->isChecked()){
        std::string skdMode = ui->comboBox_skedObsModes->currentText().toStdString();
        para.mode(skdMode);
    }else if(ui->groupBox_modeCustom->isChecked()){
        double sampleRate = ui->sampleRateDoubleSpinBox->value();
        int bits = ui->sampleBitsSpinBox->value();
        para.mode(sampleRate, bits);

        for(int i = 0; i<ui->tableWidget_modeCustonBand->rowCount(); ++i){
            std::string name = ui->tableWidget_modeCustonBand->verticalHeaderItem(i)->text().toStdString();
            double wavelength = 1/(qobject_cast<QDoubleSpinBox*>(ui->tableWidget_modeCustonBand->cellWidget(i,0))->value()*1e9)*299792458.;
            int chanels = qobject_cast<QSpinBox*>(ui->tableWidget_modeCustonBand->cellWidget(i,1))->value();
            para.mode_band(name,wavelength,chanels);
        }
    }
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        std::string name = ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text().toStdString();

        double minSNR = qobject_cast<QDoubleSpinBox*>(ui->tableWidget_ModesPolicy->cellWidget(i,0))->value();

        VieVS::ParameterSettings::ObservationModeProperty policySta;
        QString polSta = qobject_cast<QComboBox*>(ui->tableWidget_ModesPolicy->cellWidget(i,1))->currentText();
        if(polSta == "required"){
            policySta = VieVS::ParameterSettings::ObservationModeProperty::required;
        }else if(polSta == "optional"){
            policySta = VieVS::ParameterSettings::ObservationModeProperty::optional;
        }

        VieVS::ParameterSettings::ObservationModeProperty policySrc;
        QString polSrc = qobject_cast<QComboBox*>(ui->tableWidget_ModesPolicy->cellWidget(i,4))->currentText();
        if(polSrc == "required"){
            policySrc = VieVS::ParameterSettings::ObservationModeProperty::required;
        }else if(polSrc == "optional"){
            policySrc = VieVS::ParameterSettings::ObservationModeProperty::optional;
        }

        VieVS::ParameterSettings::ObservationModeBackup backupSta;
        QString bacSta = qobject_cast<QComboBox*>(ui->tableWidget_ModesPolicy->cellWidget(i,2))->currentText();
        if(bacSta == "none"){
            backupSta = VieVS::ParameterSettings::ObservationModeBackup::none;
        }else if(bacSta == "value"){
            backupSta = VieVS::ParameterSettings::ObservationModeBackup::value;
        }else if(bacSta == "min value Times"){
            backupSta = VieVS::ParameterSettings::ObservationModeBackup::minValueTimes;
        }else if(bacSta == "max value Times"){
            backupSta = VieVS::ParameterSettings::ObservationModeBackup::maxValueTimes;
        }

        VieVS::ParameterSettings::ObservationModeBackup backupSrc;
        QString bacSrc = qobject_cast<QComboBox*>(ui->tableWidget_ModesPolicy->cellWidget(i,5))->currentText();
        if(bacSrc == "none"){
            backupSrc = VieVS::ParameterSettings::ObservationModeBackup::none;
        }else if(bacSrc == "value"){
            backupSrc = VieVS::ParameterSettings::ObservationModeBackup::value;
        }else if(bacSrc == "min value Times"){
            backupSrc = VieVS::ParameterSettings::ObservationModeBackup::minValueTimes;
        }else if(bacSrc == "max value Times"){
            backupSrc = VieVS::ParameterSettings::ObservationModeBackup::maxValueTimes;
        }

        double valSta = qobject_cast<QDoubleSpinBox*>(ui->tableWidget_ModesPolicy->cellWidget(i,3))->value();
        double valSrc = qobject_cast<QDoubleSpinBox*>(ui->tableWidget_ModesPolicy->cellWidget(i,6))->value();

        para.mode_bandPolicy(name,minSNR,policySta,backupSta,valSta,policySrc,backupSrc,valSrc);
    }

    if (ui->groupBox_multiScheduling->isChecked() && ui->treeWidget_multiSchedSelected->topLevelItemCount()>0){

        VieVS::MultiScheduling ms;
        QIcon icSta = QIcon(":/icons/icons/station.png");
        QIcon icSrc = QIcon(":/icons/icons/source.png");
        QIcon icBl = QIcon(":/icons/icons/baseline.png");
        QIcon icStaGrp = QIcon(":/icons/icons/station_group_2.png");
        QIcon icSrcGrp = QIcon(":/icons/icons/source_group.png");
        QIcon icBlGrp = QIcon(":/icons/icons/baseline_group.png");

        for(int i = 0; i<ui->treeWidget_multiSchedSelected->topLevelItemCount(); ++i){
            auto itm = ui->treeWidget_multiSchedSelected->topLevelItem(i);
            QString parameter = itm->text(0);
            QIcon parameterIcon = itm->icon(0);
            QString member = itm->text(1);
            QIcon memberIcon = itm->icon(0);
            QComboBox *list = qobject_cast<QComboBox*>(ui->treeWidget_multiSchedSelected->itemWidget(itm,3));

            QStringList parameter2int {"max slew time",
                                      "max wait time",
                                      "max scan time",
                                      "min scan time",
                                      "min number of stations",
                                      "min repeat time"};

            QStringList parameter2double {"sky coverage",
                                          "number of observations",
                                          "duration",
                                          "average stations",
                                          "average sources",
                                          "min flux",
                                          "weight"};

            QStringList parameter2dateTime {"session start"};

            std::vector<double> vecDouble;
            std::vector<unsigned int> vecInt;
            if(parameter2double.indexOf(parameter) != -1 ){
                for(int j = 0; j<list->count(); ++j){
                    vecDouble.push_back( QString(list->itemText(j)).toDouble());
                }

            }else if(parameter2int.indexOf(parameter) != -1){
                for(int j = 0; j<list->count(); ++j){
                    vecInt.push_back( QString(list->itemText(j)).toInt());
                }
            }

            if(parameterIcon.pixmap(16,16).toImage() == icSta.pixmap(16,16).toImage()){
                if(memberIcon.pixmap(16,16).toImage() == icStaGrp.pixmap(16,16).toImage()){
                    auto grpMem = groupSta.at(member.toStdString());
                    VieVS::ParameterGroup grp(member.toStdString(),grpMem);

                    if(parameter == "max slew time"){
                        ms.setStation_maxSlewtime(grp,vecInt);
                    }else if(parameter == "max wait time"){
                        ms.setStation_maxWait(grp,vecInt);
                    }else if(parameter == "max scan time"){
                        ms.setStation_maxScan(grp,vecInt);
                    }else if(parameter == "min scan time"){
                        ms.setStation_minScan(grp,vecInt);
                    }else if(parameter == "weight"){
                        ms.setStation_weight(grp,vecDouble);
                    }
                }else{
                    if(parameter == "max slew time"){
                        ms.setStation_maxSlewtime(member.toStdString(),vecInt);
                    }else if(parameter == "max wait time"){
                        ms.setStation_maxWait(member.toStdString(),vecInt);
                    }else if(parameter == "max scan time"){
                        ms.setStation_maxScan(member.toStdString(),vecInt);
                    }else if(parameter == "min scan time"){
                        ms.setStation_minScan(member.toStdString(),vecInt);
                    }else if(parameter == "weight"){
                        ms.setStation_weight(member.toStdString(),vecDouble);
                    }
                }

            }else if(parameterIcon.pixmap(16,16).toImage() == icSrc.pixmap(16,16).toImage()){
                if(memberIcon.pixmap(16,16).toImage() == icSrcGrp.pixmap(16,16).toImage()){
                    auto grpMem = groupSrc.at(member.toStdString());
                    VieVS::ParameterGroup grp(member.toStdString(),grpMem);

                    if(parameter == "min number of stations"){
                        ms.setSource_minNumberOfStations(grp,vecInt);
                    }else if(parameter == "min flux"){
                        ms.setSource_minFlux(grp,vecDouble);
                    }else if(parameter == "min repeat time"){
                        ms.setSource_minRepeat(grp,vecInt);
                    }else if(parameter == "max scan time"){
                        ms.setSource_minNumberOfStations(grp,vecInt);
                    }else if(parameter == "min scan time"){
                        ms.setSource_minNumberOfStations(grp,vecInt);
                    }else if(parameter == "weight"){
                        ms.setSource_weight(grp,vecDouble);
                    }
                }else{
                    if(parameter == "min number of stations"){
                        ms.setSource_minNumberOfStations(member.toStdString(),vecInt);
                    }else if(parameter == "min flux"){
                        ms.setSource_minFlux(member.toStdString(),vecDouble);
                    }else if(parameter == "min repeat time"){
                        ms.setSource_minRepeat(member.toStdString(),vecInt);
                    }else if(parameter == "max scan time"){
                        ms.setSource_minNumberOfStations(member.toStdString(),vecInt);
                    }else if(parameter == "min scan time"){
                        ms.setSource_minNumberOfStations(member.toStdString(),vecInt);
                    }else if(parameter == "weight"){
                        ms.setSource_weight(member.toStdString(),vecDouble);
                    }
                }

            }else if(parameterIcon.pixmap(16,16).toImage() == icBl.pixmap(16,16).toImage()){
                if(memberIcon.pixmap(16,16).toImage() == icBlGrp.pixmap(16,16).toImage()){
                    auto grpMem = groupBl.at(member.toStdString());
                    VieVS::ParameterGroup grp(member.toStdString(),grpMem);

                    if(parameter == "max scan time"){
                        ms.setBaseline_maxScan(grp,vecInt);
                    }else if(parameter == "min scan time"){
                        ms.setBaseline_minScan(grp,vecInt);
                    }else if(parameter == "weight"){
                        ms.setBaseline_weight(grp,vecDouble);
                    }
                }else{
                    if(parameter == "max scan time"){
                        ms.setBaseline_maxScan(member.toStdString(),vecInt);
                    }else if(parameter == "min scan time"){
                        ms.setBaseline_minScan(member.toStdString(),vecInt);
                    }else if(parameter == "weight"){
                        ms.setBaseline_weight(member.toStdString(),vecDouble);
                    }
                }

            }else{
                if(parameter == "session start"){
                    std::vector<boost::posix_time::ptime> times;
                    for(int j = 0; j<list->count(); ++j){
                        QString txt = list->itemText(j);
                        int year = QString(txt.mid(6,4)).toInt();
                        int month = QString(txt.mid(3,2)).toInt();
                        int day = QString(txt.mid(0,2)).toInt();
                        int hour = QString(txt.mid(11,2)).toInt();
                        int min = QString(txt.mid(14,2)).toInt();
                        int sec = 0;
                        boost::posix_time::ptime t(boost::gregorian::date(year,month,day),boost::posix_time::time_duration(hour,min,sec));
                        times.push_back(t);
                    }
                    ms.setStart(times);
                }else if(parameter == "subnetting"){
                    ms.setMultiSched_subnetting(true);
                }else if(parameter == "fillin mode"){
                    ms.setMultiSched_fillinmode(true);
                }else if(parameter == "sky coverage"){
                    ms.setWeight_skyCoverage(vecDouble);
                }else if(parameter == "number of observations"){
                    ms.setWeight_numberOfObservations(vecDouble);
                }else if(parameter == "duration"){
                    ms.setWeight_duration(vecDouble);
                }else if(parameter == "average stations"){
                    ms.setWeight_averageStations(vecDouble);
                }else if(parameter == "average sources"){
                    ms.setWeight_averageSources(vecDouble);
                }
            }
        }
        para.multisched(ms.createPropertyTree());

        std::string threads = ui->comboBox_nThreads->currentText().toStdString();
        int nThreadsManual = ui->spinBox_nCores->value();
        std::string jobScheduler = ui->comboBox_jobSchedule->currentText().toStdString();
        int chunkSize = ui->spinBox_chunkSize->value();
        std::string threadPlace = ui->comboBox_threadPlace->currentText().toStdString();
        para.multiCore(threads,nThreadsManual,jobScheduler,chunkSize,threadPlace);
    }
    QString path = ui->lineEdit_outputPath->text();
    path = path.simplified();
    path.replace("\\\\","/");
    path.replace("\\","/");
    if(path.right(1) != "/"){
        path.append("/");
    }
    QString ename = QString::fromStdString(experimentName);
    ename.simplified();
    ename.replace(" ","_");
    if(ui->checkBox_outputAddTimestamp->isChecked()){
        QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        path.append(dateTime).append("_").append(ename).append("/");
    }else{
        path.append(ename).append("/");
    }
    QDir mydir(path);
    if(! mydir.exists() ){
        QDir().mkpath(path);
    }
    path.append("parameters.xml");
    para.write(path.toStdString());
    QMessageBox mb;
    QMessageBox::StandardButton reply = mb.information(this,"parameter file created",QString("A new parameter file was created and saved at: \n").append(path),QMessageBox::Open,QMessageBox::Ok);
    if(reply == QMessageBox::Open){
        QDesktopServices::openUrl(QUrl(mydir.path()));
    }
    return path;
}

void MainWindow::readSettings()
{
    std::string name = settings.get<std::string>("settings.general.name","");
    ui->nameLineEdit->setText(QString::fromStdString(name));
    std::string email = settings.get<std::string>("settings.general.email","");
    ui->emailLineEdit->setText(QString::fromStdString(email));
    std::string pathToScheduler = settings.get<std::string>("settings.general.pathToScheduler","");
    ui->pathToSchedulerLineEdit->setText(QString::fromStdString(pathToScheduler));

    std::string cAntenna = settings.get<std::string>("settings.catalog_path.antenna","");
    ui->lineEdit_pathAntenna->setText(QString::fromStdString(cAntenna));
    std::string cEquip = settings.get<std::string>("settings.catalog_path.equip","");
    ui->lineEdit_pathEquip->setText(QString::fromStdString(cEquip));
    std::string cPosition = settings.get<std::string>("settings.catalog_path.position","");
    ui->lineEdit_pathPosition->setText(QString::fromStdString(cPosition));
    std::string cMask = settings.get<std::string>("settings.catalog_path.mask","");
    ui->lineEdit_pathMask->setText(QString::fromStdString(cMask));
    std::string cSource = settings.get<std::string>("settings.catalog_path.source","");
    ui->lineEdit_pathSource->setText(QString::fromStdString(cSource));
    std::string cFlux = settings.get<std::string>("settings.catalog_path.flux","");
    ui->lineEdit_pathFlux->setText(QString::fromStdString(cFlux));
    std::string cModes = settings.get<std::string>("settings.catalog_path.modes","");
    ui->lineEdit_pathModes->setText(QString::fromStdString(cModes));
    std::string cFreq = settings.get<std::string>("settings.catalog_path.freq","");
    ui->lineEdit_pathFreq->setText(QString::fromStdString(cFreq));
    std::string cTracks = settings.get<std::string>("settings.catalog_path.tracks","");
    ui->lineEdit_pathTracks->setText(QString::fromStdString(cTracks));
    std::string cLoif = settings.get<std::string>("settings.catalog_path.loif","");
    ui->lineEdit_pathLoif->setText(QString::fromStdString(cLoif));
    std::string cRec = settings.get<std::string>("settings.catalog_path.rec","");
    ui->lineEdit_pathRec->setText(QString::fromStdString(cRec));
    std::string cRx = settings.get<std::string>("settings.catalog_path.rx","");
    ui->lineEdit_pathRx->setText(QString::fromStdString(cRx));
    std::string cHdpos = settings.get<std::string>("settings.catalog_path.hdpos","");
    ui->lineEdit_pathHdpos->setText(QString::fromStdString(cHdpos));

    std::string outputDirectory = settings.get<std::string>("settings.output.directory","");
    ui->lineEdit_outputPath->setText(QString::fromStdString(outputDirectory));
    std::string outputExpName = settings.get<std::string>("settings.output.experiment_name","");
    ui->experimentNameLineEdit->setText(QString::fromStdString(outputExpName));
    std::string outputScheduler = settings.get<std::string>("settings.output.scheduler","");
    ui->schedulerLineEdit->setText(QString::fromStdString(outputScheduler));
    std::string outputCorrelator = settings.get<std::string>("settings.output.correlator","");
    ui->correlatorLineEdit->setText(QString::fromStdString(outputCorrelator));
    std::string outputExpDesc = settings.get<std::string>("settings.output.experiment_descrtiption","");
    ui->plainTextEdit_experimentDescription->setPlainText(QString::fromStdString(outputExpDesc));


    // bands - mode

    // selected stations

    // selected source

    // station group

    // source group

    // baseline group

    // station parameters

    // source parameters

    // baseline parameters

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
    AddGroupDialog *dial = new AddGroupDialog(settings,AddGroupDialog::Type::station,this);
    dial->addModel(selectedStationModel);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::vector<std::string> stdlist = dial->getSelection();
        std::string stdname = dial->getGroupName();
        VieVS::ParameterGroup newGroup(stdname, stdlist);

        int r = 0;
        for(int i = 0; i<allStationPlusGroupModel->rowCount(); ++i){
            QString txt = allStationPlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++r;
                continue;
            }
            if(groupSta.find(txt.toStdString()) == groupSta.end()){
                break;
            }
            if(txt>QString::fromStdString(stdname)){
                break;
            }else{
                ++r;
            }
        }
        groupSta[stdname] = stdlist;

        allStationPlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/station_group_2.png"),QString::fromStdString(stdname) ));
        if(sender() == ui->pushButton_addGroupStationSetup){
            ui->comboBox_stationSettingMember->setCurrentIndex(r);
        }
    }
    delete(dial);
}

void MainWindow::addGroupSource()
{
    AddGroupDialog *dial = new AddGroupDialog(settings,AddGroupDialog::Type::source,this);
    dial->addModel(selectedSourceModel);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::vector<std::string> stdlist = dial->getSelection();
        std::string stdname = dial->getGroupName();
        VieVS::ParameterGroup newGroup(stdname, stdlist);

        int r = 0;
        for(int i = 0; i<allSourcePlusGroupModel->rowCount(); ++i){
            QString txt = allSourcePlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++r;
                continue;
            }
            if(groupSrc.find(txt.toStdString()) == groupSrc.end()){
                break;
            }
            if(txt>QString::fromStdString(stdname)){
                break;
            }else{
                ++r;
            }
        }

        groupSrc[stdname] = stdlist;

        allSourcePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/source_group.png"),QString::fromStdString(stdname) ));
        if(sender() == ui->pushButton_addGroupSourceSetup){
            ui->comboBox_sourceSettingMember->setCurrentIndex(r);
        }
        if(sender() == ui->pushButton_addSourceGroup_Calibrator){
            ui->comboBox_calibratorBlock_calibratorSources->setCurrentIndex(r);
        }

    }
    delete(dial);
}

void MainWindow::addGroupBaseline()
{
    AddGroupDialog *dial = new AddGroupDialog(settings,AddGroupDialog::Type::baseline,this);
    dial->addModel(selectedBaselineModel);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::vector<std::string> stdlist = dial->getSelection();
        std::string stdname = dial->getGroupName();
        VieVS::ParameterGroup newGroup(stdname, stdlist);

        int r = 0;
        for(int i = 0; i<allBaselinePlusGroupModel->rowCount(); ++i){
            QString txt = allBaselinePlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++r;
                continue;
            }
            if(groupBl.find(txt.toStdString()) == groupBl.end()){
                break;
            }
            if(txt>QString::fromStdString(stdname)){
                break;
            }else{
                ++r;
            }
        }

        groupBl[stdname] = stdlist;

        allBaselinePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),QString::fromStdString(stdname) ));
        if(sender() == ui->pushButton_addGroupBaselineSetup){
            ui->comboBox_baselineSettingMember->setCurrentIndex(r);
        }
    }
    delete(dial);
}

void MainWindow::on_pushButton_stationParameter_clicked()
{
    stationParametersDialog *dial = new stationParametersDialog(settings,this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);
    dial->addSourceNames(allSourcePlusGroupModel);
    dial->addDefaultParameters(paraSta["default"]);

    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersStations> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersStations parameter = res.second;

        paraSta[name] = parameter;

        ui->ComboBox_parameterStation->addItem(QString::fromStdString(name));
        ui->ComboBox_parameterStation->setCurrentIndex(ui->ComboBox_parameterStation->count()-1);

    }
    delete(dial);
}

void MainWindow::on_pushButton_sourceParameter_clicked()
{
    sourceParametersDialog *dial = new sourceParametersDialog(settings,this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);

    dial->addStationModel(allStationPlusGroupModel);
    dial->addBaselineModel(allBaselinePlusGroupModel);
    dial->addDefaultParameters(paraSrc["default"]);

    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersSources> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersSources parameter = res.second;

        paraSrc[name] = parameter;

        ui->ComboBox_parameterSource->addItem(QString::fromStdString(name));
        ui->ComboBox_parameterSource->setCurrentIndex(ui->ComboBox_parameterSource->count()-1);

    }
    delete(dial);
}

void MainWindow::on_pushButton__baselineParameter_clicked()
{
    baselineParametersDialog *dial = new baselineParametersDialog(settings, this);
    dial->addDefaultParameters(paraBl["default"]);
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

        paraBl[name] = parameter;

        ui->ComboBox_parameterBaseline->addItem(QString::fromStdString(name));
        ui->ComboBox_parameterBaseline->setCurrentIndex(ui->ComboBox_parameterBaseline->count()-1);

    }
    delete(dial);
}


void MainWindow::on_dateTimeEdit_sessionStart_dateTimeChanged(const QDateTime &dateTime)
{
    QDateTime dateTimeEnd = dateTime.addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);

    ui->DateTimeEdit_startParameterStation->setDateTime(dateTime);
    ui->DateTimeEdit_endParameterStation->setDateTime(dateTimeEnd);

    ui->DateTimeEdit_startParameterSource->setDateTime(dateTime);
    ui->DateTimeEdit_endParameterSource->setDateTime(dateTimeEnd);

    ui->DateTimeEdit_startParameterBaseline->setDateTime(dateTime);
    ui->DateTimeEdit_endParameterBaseline->setDateTime(dateTimeEnd);

    if(setupChanged){
        QMessageBox::warning(this,"Setup deleted!","Setup was deleted due to session time change!");
    }
    clearSetup(true,true,true);

}

void MainWindow::on_doubleSpinBox_sessionDuration_valueChanged(double arg1)
{
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(arg1*3600);

    ui->DateTimeEdit_endParameterStation->setDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterSource->setDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterBaseline->setDateTime(dateTimeEnd);

    if(setupChanged){
        QMessageBox::warning(this,"Setup deleted!","Setup was deleted due to session time change!");
    }
    clearSetup(true,true,true);
}


void MainWindow::on_DateTimeEdit_startParameterStation_dateTimeChanged(const QDateTime &dateTime_)
{
    QDateTime dateTime = dateTime_;
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    if(dateTime < ui->dateTimeEdit_sessionStart->dateTime()){
        dateTime = ui->dateTimeEdit_sessionStart->dateTime();
    }
    if(dateTime > dateTimeEnd){
        dateTime = dateTimeEnd;
    }
    if(dateTime > ui->DateTimeEdit_endParameterStation->dateTime()){
        ui->DateTimeEdit_endParameterStation->setDateTime(dateTime);
    }
    ui->DateTimeEdit_startParameterStation->setDateTime(dateTime);
}

void MainWindow::on_DateTimeEdit_endParameterStation_dateTimeChanged(const QDateTime &dateTime_)
{
    QDateTime dateTime = dateTime_;
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    if(dateTime < ui->dateTimeEdit_sessionStart->dateTime()){
        dateTime = ui->dateTimeEdit_sessionStart->dateTime();
    }
    if(dateTime > dateTimeEnd){
        dateTime = dateTimeEnd;
    }
    if(dateTime < ui->DateTimeEdit_startParameterStation->dateTime()){
        ui->DateTimeEdit_startParameterStation->setDateTime(dateTime);
    }
    ui->DateTimeEdit_endParameterStation->setDateTime(dateTime);
}

void MainWindow::on_DateTimeEdit_startParameterSource_dateTimeChanged(const QDateTime &dateTime_)
{
    QDateTime dateTime = dateTime_;
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    if(dateTime < ui->dateTimeEdit_sessionStart->dateTime()){
        dateTime = ui->dateTimeEdit_sessionStart->dateTime();
    }
    if(dateTime > dateTimeEnd){
        dateTime = dateTimeEnd;
    }
    if(dateTime > ui->DateTimeEdit_endParameterSource->dateTime()){
        ui->DateTimeEdit_endParameterSource->setDateTime(dateTime);
    }
    ui->DateTimeEdit_startParameterSource->setDateTime(dateTime);
}

void MainWindow::on_DateTimeEdit_endParameterSource_dateTimeChanged(const QDateTime &dateTime_)
{
    QDateTime dateTime = dateTime_;
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    if(dateTime < ui->dateTimeEdit_sessionStart->dateTime()){
        dateTime = ui->dateTimeEdit_sessionStart->dateTime();
    }
    if(dateTime > dateTimeEnd){
        dateTime = dateTimeEnd;
    }
    if(dateTime < ui->DateTimeEdit_startParameterSource->dateTime()){
        ui->DateTimeEdit_startParameterSource->setDateTime(dateTime);
    }
    ui->DateTimeEdit_endParameterSource->setDateTime(dateTime);
}

void MainWindow::on_DateTimeEdit_startParameterBaseline_dateTimeChanged(const QDateTime &dateTime_)
{
    QDateTime dateTime = dateTime_;
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    if(dateTime < ui->dateTimeEdit_sessionStart->dateTime()){
        dateTime = ui->dateTimeEdit_sessionStart->dateTime();
    }
    if(dateTime > dateTimeEnd){
        dateTime = dateTimeEnd;
    }
    if(dateTime > ui->DateTimeEdit_endParameterBaseline->dateTime()){
        ui->DateTimeEdit_endParameterBaseline->setDateTime(dateTime);
    }
    ui->DateTimeEdit_startParameterBaseline->setDateTime(dateTime);
}

void MainWindow::on_DateTimeEdit_endParameterBaseline_dateTimeChanged(const QDateTime &dateTime_)
{
    QDateTime dateTime = dateTime_;
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    if(dateTime < ui->dateTimeEdit_sessionStart->dateTime()){
        dateTime = ui->dateTimeEdit_sessionStart->dateTime();
    }
    if(dateTime > dateTimeEnd){
        dateTime = dateTimeEnd;
    }
    if(dateTime < ui->DateTimeEdit_startParameterBaseline->dateTime()){
        ui->DateTimeEdit_startParameterBaseline->setDateTime(dateTime);
    }
    ui->DateTimeEdit_endParameterBaseline->setDateTime(dateTime);
}


void MainWindow::createBaselineModel()
{
    selectedBaselineModel->clear();

    allBaselinePlusGroupModel->setRowCount(1);
    for(const auto& any:groupBl){
        allBaselinePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),QString::fromStdString(any.first)));
    }

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
        setupChanged = true;
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
            groups = groupSta;
        }else if(targetTreeWidget == ui->treeWidget_setupSource){
            groups = groupSrc;
        }else if(targetTreeWidget == ui->treeWidget_setupBaseline){
            groups = groupBl;
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
            if(groupSta.find(memberName2) != groupSta.end()){
                members2 = groupSta.at(memberName2);
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
        map = groupSta;
    }else if(cv == setupSource){
        map = groupSrc;
    }else if(cv == setupBaseline){
        map = groupBl;
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

void MainWindow::setBackgroundColorOfChildrenWhite(QTreeWidgetItem *item)
{
    for(int i=0; i<item->childCount(); ++i){
        auto itm = item->child(i);
        itm->setBackgroundColor(5,Qt::white);
        setBackgroundColorOfChildrenWhite(itm);
    }
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
            setBackgroundColorOfChildrenWhite(itm);
        }
    }
    return plot;
}


void MainWindow::on_comboBox_stationSettingMember_currentTextChanged(const QString &arg1)
{
    displayStationSetupMember(arg1);
}

void MainWindow::on_comboBox_sourceSettingMember_currentTextChanged(const QString &arg1)
{
    displaySourceSetupMember(arg1);
}

void MainWindow::on_comboBox_baselineSettingMember_currentTextChanged(const QString &arg1)
{
    displayBaselineSetupMember(arg1);
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


void MainWindow::on_ComboBox_parameterSource_currentTextChanged(const QString &arg1)
{
    displaySourceSetupParameter(arg1);
}

void MainWindow::on_ComboBox_parameterBaseline_currentTextChanged(const QString &arg1)
{
    displayBaselineSetupParameter(arg1);
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
    QIcon ic;
    bool inGroup = groupSta.find(name.toStdString()) != groupSta.end();
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

        if(groupSta.find(itmName.toStdString()) != groupSta.end()){
            std::vector<std::string> itmMembers = groupSta.at(itmName.toStdString());
            if(inGroup){
                std::vector<std::string> members = groupSta.at(name.toStdString());
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
                std::vector<std::string> members = groupSta.at(name.toStdString());
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
    QIcon ic;
    bool inGroup = groupSta.find(name.toStdString()) != groupSta.end();
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

        if(groupSta.find(itmName.toStdString()) != groupSta.end()){
            std::vector<std::string> itmMembers = groupSta.at(itmName.toStdString());
            if(inGroup){
                std::vector<std::string> members = groupSta.at(name.toStdString());
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
                std::vector<std::string> members = groupSta.at(name.toStdString());
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
    int i = 0;
    while(i<allSourcePlusGroupModel->rowCount()){
        QString name = allSourcePlusGroupModel->item(i)->text();
        if(name != "__all__" && groupSrc.find(name.toStdString()) == groupSrc.end()){
            allSourcePlusGroupModel->removeRow(i);
        }else{
            ++i;
        }
    }
    ui->treeWidget_multiSchedSelected->clear();
    for(int i=0; i<ui->treeWidget_multiSched->topLevelItemCount(); ++i){
        ui->treeWidget_multiSched->topLevelItem(i)->setDisabled(false);
    }
    ui->label_sourceList_selected->setText("selected: ");
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

void MainWindow::on_pushButton_18_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->pathToGUILineEdit->text());
}

void MainWindow::on_pushButton_19_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->defaultSettingsFileLineEdit->text());
}

void MainWindow::on_actionNew_triggered()
{
    if (QMessageBox::Yes == QMessageBox::question(this, "Restart?", "Do you really want to restart?", QMessageBox::Yes | QMessageBox::No)){
        QApplication::exit(1000);
    }
}



void MainWindow::on_pushButton_25_clicked()
{
    auto list = ui->treeWidget_multiSchedSelected->selectedItems();{
        for(const auto& any:list){
            if(any->text(0) == "session start"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(0)->setDisabled(false);
            }else if(any->text(0) == "subnetting"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(1)->setDisabled(false);
            }else if(any->text(0) == "fillin mode"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(2)->setDisabled(false);
            }else if(any->text(0) == "sky coverage"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(0)->setDisabled(false);
            }else if(any->text(0) == "number of observations"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(1)->setDisabled(false);
            }else if(any->text(0) == "duration"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(2)->setDisabled(false);
            }else if(any->text(0) == "average stations"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(3)->setDisabled(false);
            }else if(any->text(0) == "average sources"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(4)->setDisabled(false);
            }
            delete(any);
        }
    }

    int nsched = 1;
    for(int i = 0; i<ui->treeWidget_multiSchedSelected->topLevelItemCount(); ++i){
        nsched *= ui->treeWidget_multiSchedSelected->topLevelItem(i)->text(2).toInt();
    }
    ui->label_multiSchedulingNsched->setText(QString::number(nsched));

}


void MainWindow::on_pushButton_5_clicked()
{    
    QString path = "settings.general.name";
    QString value = ui->nameLineEdit->text();
    QString name = "user name";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_6_clicked()
{    
    QString path = "settings.general.email";
    QString value = ui->emailLineEdit->text();
    QString name = "email address";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_17_clicked()
{    
    QString path = "settings.general.pathToScheduler";
    QString value = ui->pathToSchedulerLineEdit->text();
    QString name = "path to scheduler executable";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_saveCatalogPathes_clicked()
{
    settings.put("settings.catalog_path.antenna",ui->lineEdit_pathAntenna->text().toStdString());
    settings.put("settings.catalog_path.equip",ui->lineEdit_pathEquip->text().toStdString());
    settings.put("settings.catalog_path.position",ui->lineEdit_pathPosition->text().toStdString());
    settings.put("settings.catalog_path.mask",ui->lineEdit_pathMask->text().toStdString());
    settings.put("settings.catalog_path.source",ui->lineEdit_pathSource->text().toStdString());
    settings.put("settings.catalog_path.flux",ui->lineEdit_pathFlux->text().toStdString());
    settings.put("settings.catalog_path.modes",ui->lineEdit_pathModes->text().toStdString());
    settings.put("settings.catalog_path.freq",ui->lineEdit_pathFreq->text().toStdString());
    settings.put("settings.catalog_path.tracks",ui->lineEdit_pathTracks->text().toStdString());
    settings.put("settings.catalog_path.loif",ui->lineEdit_pathLoif->text().toStdString());
    settings.put("settings.catalog_path.rec",ui->lineEdit_pathRec->text().toStdString());
    settings.put("settings.catalog_path.rx",ui->lineEdit_pathRx->text().toStdString());
    settings.put("settings.catalog_path.hdpos",ui->lineEdit_pathHdpos->text().toStdString());
    std::ofstream os;
    os.open("settings.xml");
    boost::property_tree::xml_parser::write_xml(os, settings,
                                                boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
    os.close();
    QString txt = "Your default catalog pathes have been changed!";
    QMessageBox::information(this,"Default settings changed",txt);
}

void MainWindow::on_pushButton_26_clicked()
{
    QString path = "settings.output.directory";
    QString value = ui->lineEdit_outputPath->text();
    QString name = "output path";
    changeDefaultSettings(path,value,name);
}

void MainWindow::changeDefaultSettings(QString path, QString value, QString name)
{
    settings.put(path.toStdString(),value.toStdString());
    std::ofstream os;
    os.open("settings.xml");
    boost::property_tree::xml_parser::write_xml(os, settings,
                                                boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
    os.close();
    QString txt = "Your default ";
    txt.append(name).append(" is changed to:\n").append(value).append("!");
    QMessageBox::information(this,"Default settings changed",txt);
}

void MainWindow::on_pushButton_24_clicked()
{
    QString path = "settings.output.experiment_descrtiption";
    QString value = ui->plainTextEdit_experimentDescription->toPlainText();
    QString name = "experiment description";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_23_clicked()
{
    QString path = "settings.output.scheduler";
    QString value = ui->schedulerLineEdit->text();
    QString name = "scheduler";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_22_clicked()
{
    QString path = "settings.output.correlator";
    QString value = ui->correlatorLineEdit->text();
    QString name = "correlator";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_27_clicked()
{
    QString path = "settings.output.experiment_name";
    QString value = ui->experimentNameLineEdit->text();
    QString name = "experiment name";
    changeDefaultSettings(path,value,name);
}

void MainWindow::createDefaultParameterSettings()
{
    VieVS::ParameterSettings::ParametersStations sta;
    sta.maxScan = 600;
    sta.minScan = 20;
    sta.maxSlewtime = 600;
    sta.maxWait = 600;
    sta.weight = 1;
    settings.add_child("settings.station.parameters.parameter",VieVS::ParameterSettings::parameterStation2ptree("default",sta).get_child("parameters"));

    VieVS::ParameterSettings::ParametersSources src;
    src.minRepeat = 1800;
    src.minScan = 20;
    src.maxScan = 600;
    src.weight = 1;
    src.minFlux = 0.05;
    src.maxNumberOfScans = 999;
    src.minNumberOfStations = 2;
    settings.add_child("settings.source.parameters.parameter",VieVS::ParameterSettings::parameterSource2ptree("default",src).get_child("parameters"));

    VieVS::ParameterSettings::ParametersBaselines bl;
    bl.maxScan = 600;
    bl.minScan = 20;
    bl.weight = 1;
    settings.add_child("settings.baseline.parameters.parameter",VieVS::ParameterSettings::parameterBaseline2ptree("default",bl).get_child("parameters"));    
    settings.add("settings.station.waitTimes.setup",0);
    settings.add("settings.station.waitTimes.source",5);
    settings.add("settings.station.waitTimes.tape",1);
    settings.add("settings.station.waitTimes.calibration",10);
    settings.add("settings.station.waitTimes.corsynch",3);

    settings.add("settings.station.cableWrapBuffers.axis1LowOffset", 5);
    settings.add("settings.station.cableWrapBuffers.axis1UpOffset", 5);
    settings.add("settings.station.cableWrapBuffers.axis2LowOffset", 0);
    settings.add("settings.station.cableWrapBuffers.axis2UpOffset", 0);

    std::ofstream os;
    os.open("settings.xml");
    boost::property_tree::xml_parser::write_xml(os, settings,
                                                boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
    os.close();

}

void MainWindow::on_pushButton_saveNetwork_clicked()
{
    QVector<QString> selSta;
    for(int i = 0; i<selectedStationModel->rowCount(); ++i){
        selSta.append(selectedStationModel->item(i)->text());
    }
    saveToSettingsDialog *dial = new saveToSettingsDialog(settings,this);
    dial->setType(saveToSettingsDialog::Type::stationNetwork);
    dial->setNetwork(selSta);

    dial->exec();
}

void MainWindow::on_pushButton_loadNetwork_clicked()
{
    auto network= settings.get_child_optional("settings.networks");
    if(!network.is_initialized()){
        QMessageBox::warning(this,"No network found!","There was no network saved in settings.xml file\nCheck settings.networks");
        return;
    }

    QVector<QString> names;
    QVector<QVector<QString> > networks;

    for(const auto &it:*network){
        QString name = QString::fromStdString(it.second.get_child("<xmlattr>.name").data());
        QVector<QString> network;

        for(const auto &it2:it.second){
            if(it2.first == "member"){
                network.push_back(QString::fromStdString(it2.second.data()));
            }
        }

        names.push_back(name);
        networks.push_back(network);
    }
    settingsLoadWindow *dial = new settingsLoadWindow(this);

    dial->setNetwork(names,networks);

    int result = dial->exec();
    if(result == QDialog::Accepted){

        for(int i=0; i<selectedStationModel->rowCount(); ++i){
            QModelIndex idx = selectedStationModel->index(0,0);
            on_listView_allSelectedStations_clicked(idx);
        }

        QString warningTxt;

        QString itm = dial->selectedItem();
        int idx = dial->selectedIdx();
        QVector<QString> members = networks.at(idx);

        for(const auto&any:members){
            auto list = allStationModel->findItems(any);
            if(list.size() == 1){
                for(int i = 0; i<allStationProxyModel->rowCount(); ++i){
                    QModelIndex idx = allStationProxyModel->index(i,0);
                    if(idx.data().toString() == any){
                        ui->treeView_allAvailabeStations->clicked(idx);
                    }
                }
//                selectedStationModel->appendRow(new QStandardItem(QIcon(":icons/icons/station.png"),list.at(0)->text()));
            }else{
                warningTxt.append("    unknown station: ").append(any).append("!\n");
            }
        }
        if(!warningTxt.isEmpty()){
            QString txt = "The following errors occurred while loading the network:\n";
            txt.append(warningTxt).append("These stations were ignored!\nPlease double check stations again!");
            QMessageBox::warning(this,"Unknown network stations!",txt);
        }
    }
}

void MainWindow::on_pushButton_saveSourceList_clicked()
{
    QVector<QString> selSrc;
    for(int i = 0; i<selectedSourceModel->rowCount(); ++i){
        selSrc.append(selectedSourceModel->item(i)->text());
    }
    saveToSettingsDialog *dial = new saveToSettingsDialog(settings,this);
    dial->setType(saveToSettingsDialog::Type::sourceNetwork);
    dial->setNetwork(selSrc);

    dial->exec();
}

void MainWindow::on_pushButton_loadSourceList_clicked()
{
    auto network= settings.get_child_optional("settings.source_lists");
    if(!network.is_initialized()){
        QMessageBox::warning(this,"No source list found!","There was no source list saved in settings.xml file\nCheck settings.source_list");
        return;
    }

    QVector<QString> names;
    QVector<QVector<QString> > source_lists;

    for(const auto &it:*network){
        QString name = QString::fromStdString(it.second.get_child("<xmlattr>.name").data());
        QVector<QString> source_list;

        for(const auto &it2:it.second){
            if(it2.first == "member"){
                source_list.push_back(QString::fromStdString(it2.second.data()));
            }
        }

        names.push_back(name);
        source_lists.push_back(source_list);
    }
    settingsLoadWindow *dial = new settingsLoadWindow(this);

    dial->setSourceList(names,source_lists);

    int result = dial->exec();
    if(result == QDialog::Accepted){
        QString warningTxt;

        QString itm = dial->selectedItem();
        int idx = dial->selectedIdx();
        QVector<QString> members = source_lists.at(idx);

        on_pushButton_15_clicked();
        for(const auto&any:members){
            auto list = allSourceModel->findItems(any);
            if(list.size() == 1){
                for(int i = 0; i<allSourceProxyModel->rowCount(); ++i){
                    QModelIndex idx = allSourceProxyModel->index(i,0);
                    if(idx.data().toString() == any){
                        ui->treeView_allAvailabeSources->clicked(idx);
                    }
                }
//                selectedSourceModel->appendRow(new QStandardItem(QIcon(":icons/icons/source.png"),list.at(0)->text()));
            }else{
                warningTxt.append("    unknown station: ").append(any).append("!\n");
            }
        }
        if(!warningTxt.isEmpty()){
            QString txt = "The following errors occurred while loading the source list:\n";
            txt.append(warningTxt).append("These sources were ignored!\nPlease double check sources again!");
            QMessageBox::warning(this,"Unknown source list source!",txt);
        }
    }
}

void MainWindow::on_pushButton_saveMode_clicked()
{
    int bits = ui->sampleBitsSpinBox->value();
    double srate = ui->sampleRateDoubleSpinBox->value();
    QVector<QString> bands;
    QVector<double> freqs;
    QVector<int> chans;

    for(int i = 0; i<ui->tableWidget_modeCustonBand->rowCount(); ++i){
        QString band = ui->tableWidget_modeCustonBand->verticalHeaderItem(i)->text();
        bands.push_back(band);
        double freq = qobject_cast<QDoubleSpinBox*>(ui->tableWidget_modeCustonBand->cellWidget(i,0))->value();
        freqs.push_back(freq);
        int chan = qobject_cast<QSpinBox*>(ui->tableWidget_modeCustonBand->cellWidget(i,1))->value();
        chans.push_back(chan);
    }
    saveToSettingsDialog *dial = new saveToSettingsDialog(settings,this);
    dial->setType(saveToSettingsDialog::Type::modes);
    dial->setMode(bits,srate,bands,freqs,chans);

    dial->exec();

}

void MainWindow::on_pushButton_loadMode_clicked()
{
    auto modes= settings.get_child_optional("settings.modes");
    if(!modes.is_initialized()){
        QMessageBox::warning(this,"No modes list found!","There were no modes saved in settings.xml file\nCheck settings.modes");
        return;
    }

    QVector<QString> names;
    QVector<int> bits;
    QVector<double> srates;
    QVector<QVector<QString> > bands;
    QVector<QVector<int> > channels;
    QVector<QVector<double> > freqs;

    for(const auto &it:*modes){
        QString name = QString::fromStdString(it.second.get_child("<xmlattr>.name").data());
        int bit;
        double srate;
        QVector<QString> band;
        QVector<int> channel;
        QVector<double> freq;

        for(const auto &it2:it.second){
            if(it2.first == "bits"){
                bit = it2.second.get_value<int>();
            }else if(it2.first == "sampleRate"){
                srate = it2.second.get_value<double>();
            }else if(it2.first == "band"){

                channel.push_back(it2.second.get<int>("channels"));
                freq.push_back(it2.second.get<double>("frequency"));
                band.push_back(QString::fromStdString(it2.second.get<std::string>("<xmlattr>.name")));

            }
        }
        names.push_back(name);
        bits.push_back(bit);
        srates.push_back(srate);
        bands.push_back(band);
        channels.push_back(channel);
        freqs.push_back(freq);
    }

    settingsLoadWindow *dial = new settingsLoadWindow(this);
    dial->setModes(names,bits,srates,bands,channels,freqs);

    int result = dial->exec();
    if(result == QDialog::Accepted){
        QString itm = dial->selectedItem();
        int idx = dial->selectedIdx();

        ui->sampleBitsSpinBox->setValue(bits.at(idx));
        ui->sampleRateDoubleSpinBox->setValue(srates.at(idx));
        ui->tableWidget_modeCustonBand->setRowCount(0);
        ui->tableWidget_ModesPolicy->setRowCount(0);
        for(int i=0; i<bands.at(idx).size(); ++i){
            QString bName = bands.at(idx).at(i);
            double bFreq = freqs.at(idx).at(i);
            int bChannels = channels.at(idx).at(i);
            addModesCustomTable(bName, bFreq, bChannels);
        }
    }

}

void MainWindow::clearGroup(bool sta, bool src, bool bl, QString name)
{
    bool anyMapCleared = false;
    if(sta){
        int i=0;
        bool mapCleared = false;
        while(i<allStationPlusGroupModel->rowCount()){
            QString txt = allStationPlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++i;
                continue;
            }
            if(groupSta.find(txt.toStdString()) != groupSta.end()){
                auto vec = groupSta[txt.toStdString()];
                auto it = std::find(vec.begin(),vec.end(),name.toStdString());
                if(it != vec.end()){
                    vec.erase(it);
                    groupSta[txt.toStdString()] = vec;
                }
                if(vec.empty()){
                    allStationPlusGroupModel->removeRow(i);
                    groupSta.erase(txt.toStdString());
                    mapCleared = true;
                }else{
                    ++i;
                }
            }else{
                break;
            }
        }
        anyMapCleared = anyMapCleared || mapCleared;
        if(mapCleared){
            clearSetup(true,false,false);
        }
    }
    if(src){
        int i=0;
        bool mapCleared = false;
        while(i<allSourcePlusGroupModel->rowCount()){
            QString txt = allSourcePlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++i;
                continue;
            }
            if(groupSrc.find(txt.toStdString()) != groupSrc.end()){
                auto vec = groupSrc[txt.toStdString()];
                auto it = std::find(vec.begin(),vec.end(),name.toStdString());
                if(it != vec.end()){
                    vec.erase(it);
                    groupSrc[txt.toStdString()] = vec;
                }
                if(vec.empty()){
                    if(ui->comboBox_calibratorBlock_calibratorSources->currentText() == txt){
                        QMessageBox::warning(this,"Calibration block error!","A source group was deleted and this group was choosen as calibrator source group!\nCheck calibrator block!");
                        ui->comboBox_calibratorBlock_calibratorSources->setCurrentIndex(0);
                    }
                    for(int i=0; i<ui->tableWidget_scanSequence->rowCount(); ++i){
                        QComboBox* cb = qobject_cast<QComboBox*>(ui->tableWidget_scanSequence->cellWidget(i,0));
                        if(cb->currentText() == txt){
                            QMessageBox::warning(this,"Scan sequence error!","A source group was deleted and this group was choosen in scan sequence!\nCheck scan sequence!");
                            cb->setCurrentIndex(0);
                        }
                    }
//                    int j=0;
//                    while(j<ui->treeWidget_multiSchedSelected->topLevelItemCount()){
//                        if(ui->treeWidget_multiSchedSelected->topLevelItem(j)->text(1) == txt){
//                            QMessageBox::warning(this,"Multi scheduling error!","A source group was deleted and this group was choosen in multi scheduling!\nCheck multi scheduling!");
//                            delete(ui->treeWidget_multiSchedSelected->topLevelItem(j));
//                        }else{
//                            ++j;
//                        }
//                    }
                    allSourcePlusGroupModel->removeRow(i);
                    groupSrc.erase(txt.toStdString());
                    mapCleared = true;
                }else{
                    ++i;
                }
            }else{
                break;
            }
        }
        anyMapCleared = anyMapCleared || mapCleared;
        if(mapCleared){
            clearSetup(false,true,false);
        }
    }
    if(bl){
        int i=0;
        bool mapCleared = false;
        while(i<allBaselinePlusGroupModel->rowCount()){
            QString txt = allBaselinePlusGroupModel->item(i)->text();
            if(txt == "__all__"){
                ++i;
                continue;
            }
            if(groupBl.find(txt.toStdString()) != groupBl.end()){
                auto vec = groupBl[txt.toStdString()];
                int j = 0;
                while(j<vec.size()){
                    QString itm = QString::fromStdString(vec.at(j));
                    auto stations = itm.split("-");
                    if(stations.indexOf(name) != -1){
                        vec.erase(vec.begin()+j);
                    }else{
                        ++j;
                    }
                    groupBl[txt.toStdString()] = vec;
                }
                if(vec.empty()){
                    allBaselinePlusGroupModel->removeRow(i);
                    groupBl.erase(txt.toStdString());
                    mapCleared = true;
                }else{
                    ++i;
                }
            }else{
                break;
            }
        }
        anyMapCleared = anyMapCleared || mapCleared;
        if(mapCleared){
            clearSetup(false,false,true);
        }
    }

    if(anyMapCleared){
        QMessageBox::warning(this,"Group deleted!","At least one group became empty!\nSetup might got removed... please check setup again!");
    }

}

void MainWindow::clearSetup(bool sta, bool src, bool bl)
{
    setupChanged = false;
    std::string parameterName = "default";
    std::string member = "__all__";
    QDateTime sessionStart = ui->dateTimeEdit_sessionStart->dateTime();
    unsigned int startt = 0;
    unsigned int endt = sessionStart.secsTo(ui->DateTimeEdit_endParameterStation->dateTime());

    QDateTime e = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    if(sta){
        QTreeWidgetItem *wsta = new QTreeWidgetItem();
        wsta->setText(0,"__all__");
        wsta->setText(1,"default");
        wsta->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
        wsta->setText(3,e.toString("dd.MM.yyyy hh:mm"));
        wsta->setText(4,"hard");
        wsta->setIcon(0,QIcon(":/icons/icons/station_group_2.png"));
        ui->treeWidget_setupStation->clear();
        ui->treeWidget_setupStation->insertTopLevelItem(0,wsta);
        QHeaderView * hvsta = ui->treeWidget_setupStation->header();
        hvsta->setSectionResizeMode(QHeaderView::ResizeToContents);
        setupStationTree = VieVS::ParameterSetup(parameterName,
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);
        drawSetupPlot(setupStation, ui->comboBox_setupStation, ui->treeWidget_setupStation);
    }

    if(src){
        QTreeWidgetItem *wsrc = new QTreeWidgetItem();
        wsrc->setText(0,"__all__");
        wsrc->setText(1,"default");
        wsrc->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
        wsrc->setText(3,e.toString("dd.MM.yyyy hh:mm"));
        wsrc->setText(4,"hard");
        wsrc->setIcon(0,QIcon(":/icons/icons/source_group.png"));
        ui->treeWidget_setupSource->clear();
        ui->treeWidget_setupSource->insertTopLevelItem(0,wsrc);
        QHeaderView * hvsrc = ui->treeWidget_setupSource->header();
        hvsrc->setSectionResizeMode(QHeaderView::ResizeToContents);
        setupSourceTree = VieVS::ParameterSetup(parameterName,
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);
        drawSetupPlot(setupSource, ui->comboBox_setupSource, ui->treeWidget_setupSource);
    }

    if(bl){
        QTreeWidgetItem *wbl = new QTreeWidgetItem();
        wbl->setText(0,"__all__");
        wbl->setText(1,"default");
        wbl->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
        wbl->setText(3,e.toString("dd.MM.yyyy hh:mm"));
        wbl->setText(4,"hard");
        wbl->setIcon(0,QIcon(":/icons/icons/baseline_group.png"));
        ui->treeWidget_setupBaseline->clear();
        ui->treeWidget_setupBaseline->insertTopLevelItem(0,wbl);
        QHeaderView * hvbl = ui->treeWidget_setupBaseline->header();
        hvbl->setSectionResizeMode(QHeaderView::ResizeToContents);
        setupBaselineTree = VieVS::ParameterSetup(parameterName,
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);
        drawSetupPlot(setupBaseline, ui->comboBox_setupBaseline, ui->treeWidget_setupBaseline);
    }
}

void MainWindow::splitterMoved() {
  QSplitter* senderSplitter = static_cast<QSplitter*>(sender());

  QSplitter* receiverSplitter1;
  QSplitter* receiverSplitter2;
  if(senderSplitter == ui->splitter){
      receiverSplitter1 = ui->splitter_2;
      receiverSplitter2 = ui->splitter_3;
  }else if(senderSplitter == ui->splitter_2){
      receiverSplitter1 = ui->splitter;
      receiverSplitter2 = ui->splitter_3;
  }else if(senderSplitter == ui->splitter_3){
      receiverSplitter1 = ui->splitter;
      receiverSplitter2 = ui->splitter_2;
  }

  receiverSplitter1->blockSignals(true);
  receiverSplitter1->setSizes(senderSplitter->sizes());
  receiverSplitter1->blockSignals(false);
  receiverSplitter2->blockSignals(true);
  receiverSplitter2->setSizes(senderSplitter->sizes());
  receiverSplitter2->blockSignals(false);
}

void MainWindow::on_pushButton_faqSearch_clicked()
{
    QString searchString = ui->lineEdit_faqSearch->text();
    QTextDocument *document = ui->textEdit_faq->document();
    searchString = searchString.simplified();

    bool found = false;

    document->undo();

    if (searchString.isEmpty()) {

    } else {
        QTextCursor highlightCursor(document);
        QTextCursor cursor(document);

        cursor.beginEditBlock();

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setForeground(Qt::red);

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document->find(searchString, highlightCursor);

            if (!highlightCursor.isNull()) {
                found = true;
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }

        cursor.endEditBlock();

        if (found == false) {
            QMessageBox::information(this, tr("Not Found"),
                                     tr("Sorry, the text cannot be found."));
        }
    }

}


void MainWindow::on_actionRun_triggered()
{
    QString path = on_actionSave_triggered();

    QDir mydir(path);
    QString fullPath = mydir.absolutePath();
    if(!path.isEmpty()){
        QDockWidget *dw = new QDockWidget(this);
        dw->setWindowTitle("Scheduling process");

        myTextBrowser *tb = new myTextBrowser(dw);
        dw->setWidget(tb);

        QList<QDockWidget *> dockWidgets = this->findChildren<QDockWidget *>();

        if(dockWidgets.size() == 1){
            addDockWidget(Qt::BottomDockWidgetArea,dw);
        }else{
            tabifyDockWidget(dockWidgets.at(0),dw);
        }
        QString program = ui->pathToSchedulerLineEdit->text();
        QStringList arguments;
        arguments << fullPath;

        QProcess *start = new QProcess(this);
        start->start(program,arguments);
        connect(start,SIGNAL(readyReadStandardOutput()),tb,SLOT(readyReadStandardOutput()));
        connect(start,SIGNAL(readyReadStandardError()),tb,SLOT(readyReadStandardError()));

        if(start->waitForStarted()){
            QMessageBox::information(this,"Scheduling started!","Starting scheduling " + fullPath +"!");
        }else{
            QMessageBox::warning(this,"Scheduling failed to start!","Could not start process:\n" + program +"\nwith arguments:\n" + arguments.at(0));
        }
    }
}



void MainWindow::networkSizeChanged()
{
    int size = selectedStationModel->rowCount();
    ui->label_network_selected->setText(QString("selected: %1").arg(size));
}

void MainWindow::sourceListChanged()
{
    int size = selectedSourceModel->rowCount();
    ui->label_sourceList_selected->setText(QString("selected: %1").arg(size));
}

void MainWindow::on_comboBox_nThreads_currentTextChanged(const QString &arg1)
{
    if(arg1 == "manual"){
        ui->label_nCores->setEnabled(true);
        ui->spinBox_nCores->setEnabled(true);
    }else{
        ui->label_nCores->setEnabled(false);
        ui->spinBox_nCores->setEnabled(false);
    }
}

void MainWindow::on_comboBox_jobSchedule_currentTextChanged(const QString &arg1)
{
    if(arg1 == "auto"){
        ui->label_chunkSize->setEnabled(false);
        ui->spinBox_chunkSize->setEnabled(false);
    }else{
        ui->label_chunkSize->setEnabled(true);
        ui->spinBox_chunkSize->setEnabled(true);
    }
}

void MainWindow::setupStatisticView()
{
    auto hv1 = ui->treeWidget_statisticGeneral->header();
    hv1->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto hv2 = ui->treeWidget_statisticStation->header();
    hv2->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto hv3 = ui->treeWidget_statisticSource->header();
    hv3->setSectionResizeMode(QHeaderView::ResizeToContents);

    statisticsView = new QChartView(this);
    ui->verticalLayout_statisticPlot->insertWidget(0,statisticsView,1);
    ui->horizontalScrollBar_statistics->setRange(0,0);

    for(int i=0; i<ui->treeWidget_statisticGeneral->topLevelItemCount(); ++i){
        auto db = new QDoubleSpinBox(ui->treeWidget_statisticGeneral);
        db->setMinimum(-99);
        ui->treeWidget_statisticGeneral->setItemWidget(ui->treeWidget_statisticGeneral->topLevelItem(i),2,db);
        connect(db,SIGNAL(valueChanged(double)),this,SLOT(plotStatistics()));
    }

    connect(ui->radioButton_statistics_absolute,SIGNAL(toggled(bool)),this,SLOT(plotStatistics()));
    connect(ui->checkBox_statistics_removeMinimum,SIGNAL(toggled(bool)),this,SLOT(plotStatistics()));
}

void MainWindow::on_pushButton_addStatistic_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to statistics.csv file", ui->lineEdit_outputPath->text(),"*.csv");
    if( !path.isEmpty() ){
        QStringList tmp = path.split("/");
        QString folder = tmp.at(tmp.size()-2);
        if(ui->listWidget_statistics->findItems(folder, Qt::MatchExactly).size()>0){
            QMessageBox::warning(this,"already visible","There is already one statistics file from this folder visible!");
            return;
        }

        ui->listWidget_statistics->insertItem(ui->listWidget_statistics->count(),folder);

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this,"could not open file!","Error while opening:\n"+path,QMessageBox::Ok);
            return;
        }

        QTextStream in(&file);
        QString line = in.readLine();
        QStringList names = line.split(",",QString::SplitBehavior::SkipEmptyParts);


        if(statisticsName.isEmpty()){
            statisticsName = names;
        }else{
            int startIdxNew = names.indexOf("n_stations")+1;
            int startIdxOld = statisticsName.indexOf("n_stations")+1;

            for(int inew=startIdxNew; inew<names.count(); ++inew){
                QString thisItem = names.at(inew);
                if(thisItem.left(7) != "n_scans"){
                    break;
                }

                if(statisticsName.indexOf(thisItem) == -1){
                    for(int iold=startIdxOld; iold<statisticsName.count(); ++iold){
                        QString thisItemOld = statisticsName.at(iold);
                        if(thisItemOld.left(7) != "n_scans"){
                            statisticsName.insert(iold,thisItem);
                            addEmptyStatistic(iold);
                            break;
                        }
                        if(thisItemOld>thisItem){
                           statisticsName.insert(iold,thisItem);
                           addEmptyStatistic(iold);
                           break;
                        }
                    }
                }
            }

            startIdxNew = names.indexOf(QRegularExpression("n_baselines.*"),startIdxNew);
            startIdxOld = statisticsName.indexOf(QRegularExpression("n_baselines.*"),startIdxOld);

            for(int inew=startIdxNew; inew<names.count(); ++inew){
                QString thisItem = names.at(inew);
                if(thisItem.left(7) != "n_basel"){
                    break;
                }

                if(statisticsName.indexOf(thisItem) == -1){
                    for(int iold=startIdxOld; iold<statisticsName.count(); ++iold){
                        QString thisItemOld = statisticsName.at(iold);
                        if(thisItemOld>thisItem){
                           statisticsName.insert(iold,thisItem);
                           addEmptyStatistic(iold);
                           break;
                        }
                        if(thisItemOld.left(7) != "n_basel"){
                            statisticsName.insert(iold,thisItem);
                            addEmptyStatistic(iold);
                            break;
                        }
                    }
                }
            }
        }

        QStringList staNames;
        for(int i=8; i<statisticsName.count(); ++i){
            QString thisName = statisticsName.at(i);
            if(thisName.left(7) != "n_scans"){
                break;
            }
            thisName = thisName.right(thisName.count()-8);
            staNames << thisName;
        }

        ui->treeWidget_statisticStation->blockSignals(true);
        ui->treeWidget_statisticStation->clear();
        ui->treeWidget_statisticStation->addTopLevelItem(new QTreeWidgetItem(QStringList() << "scans"));
        ui->treeWidget_statisticStation->addTopLevelItem(new QTreeWidgetItem(QStringList() << "baselines"));
        ui->treeWidget_statisticStation->topLevelItem(0)->setCheckState(0,Qt::Unchecked);
        ui->treeWidget_statisticStation->topLevelItem(1)->setCheckState(0,Qt::Unchecked);
        for(int i=0; i<staNames.count(); ++i){
            ui->treeWidget_statisticStation->topLevelItem(0)->addChild(new QTreeWidgetItem(QStringList() << staNames.at(i)));
            ui->treeWidget_statisticStation->topLevelItem(0)->child(i)->setCheckState(0,Qt::Unchecked);

            auto db1 = new QDoubleSpinBox(ui->treeWidget_statisticStation);
            db1->setMinimum(-99);
            ui->treeWidget_statisticStation->setItemWidget(ui->treeWidget_statisticStation->topLevelItem(0)->child(i),2,db1);
            connect(db1,SIGNAL(valueChanged(double)),this,SLOT(plotStatistics()));

            ui->treeWidget_statisticStation->topLevelItem(1)->addChild(new QTreeWidgetItem(QStringList() << staNames.at(i)));
            ui->treeWidget_statisticStation->topLevelItem(1)->child(i)->setCheckState(0,Qt::Unchecked);

            auto db2 = new QDoubleSpinBox(ui->treeWidget_statisticStation);
            db2->setMinimum(-99);
            ui->treeWidget_statisticStation->setItemWidget(ui->treeWidget_statisticStation->topLevelItem(1)->child(i),2,db2);
            connect(db2,SIGNAL(valueChanged(double)),this,SLOT(plotStatistics()));

        }
        ui->treeWidget_statisticStation->topLevelItem(0)->setExpanded(true);
        ui->treeWidget_statisticStation->topLevelItem(1)->setExpanded(true);
        ui->treeWidget_statisticStation->blockSignals(false);

        while (!in.atEnd()){
            QString line = in.readLine();
            QStringList split = line.split(",",QString::SplitBehavior::SkipEmptyParts);

            QString thisElement = split.at(0);
            int version = thisElement.toInt();
            statistics[folder][version] = QVector<int>(statisticsName.count(),0);
            for(int i = 0; i<split.count();++i){
                QString thisElement = split.at(i);
                int v = thisElement.toInt();
                QString thisName = names.at(i);
                int idx = statisticsName.indexOf(thisName);
                statistics[folder][version][idx] = v;
            }
        }
        plotStatistics();
    }
}

void MainWindow::addEmptyStatistic(int idx)
{
    for(const auto &key1: statistics.keys()){
        for(const auto &key2: statistics[key1].keys()){
            statistics[key1][key2].insert(idx,0);
        }
    }
}


void MainWindow::on_pushButton_removeStatistic_clicked()
{
    if(ui->listWidget_statistics->selectedItems().size()==1){
        QString txt = ui->listWidget_statistics->selectedItems().at(0)->text();
        int row = ui->listWidget_statistics->selectionModel()->selectedRows(0).at(0).row();
        statistics.remove(txt);
        delete ui->listWidget_statistics->item(row);


        plotStatistics();
    }
}

void MainWindow::plotStatistics()
{
    ui->treeWidget_statisticGeneral->blockSignals(true);
    ui->treeWidget_statisticStation->blockSignals(true);
    ui->treeWidget_statisticSource->blockSignals(true);

    int nsta=0;
    for(int i=7; i<statisticsName.count(); ++i){
        QString thisName = statisticsName.at(i);
        if(thisName.left(7) != "n_scans"){
            break;
        }
        ++nsta;
    }


    QMap<QString,int> translateGeneral;
    translateGeneral["# scans"] = 1;
    translateGeneral["# baselines"] = 6;
    translateGeneral["# stations"] = 7;
    translateGeneral["# sources"] = 8+2*nsta;
    translateGeneral["# single source scans"] = 2;
    translateGeneral["# subnetting scans"] = 3;
    translateGeneral["# fillin mode scans"] = 4;
    translateGeneral["# calibration scans"] = 5;

    QVector<QColor> colors{
                QColor(31,120,180),
                QColor(51,160,44),
                QColor(227,26,28),
                QColor(255,127,0),
                QColor(106,61,154),
                QColor(177,89,40),
                QColor(166,206,227),
                QColor(178,223,138),
                QColor(251,154,153),
                QColor(253,191,111),
                QColor(202,178,214),
                QColor(255,255,153),
    };

    QVector<QBrush> brushes;
    for(int i=0; i<colors.size(); ++i){
        brushes.push_back(QBrush(colors.at(i),Qt::SolidPattern));
    }
    for(int i=0; i<colors.size(); ++i){
        brushes.push_back(QBrush(colors.at(i),Qt::BDiagPattern));
    }
    for(int i=0; i<colors.size(); ++i){
        brushes.push_back(QBrush(colors.at(i),Qt::FDiagPattern));
    }
    for(int i=0; i<colors.size(); ++i){
        brushes.push_back(QBrush(colors.at(i),Qt::DiagCrossPattern));
    }
    for(int i=0; i<colors.size(); ++i){
        brushes.push_back(QBrush(colors.at(i),Qt::HorPattern));
    }
    for(int i=0; i<colors.size(); ++i){
        brushes.push_back(QBrush(colors.at(i),Qt::VerPattern));
    }
    for(int i=0; i<colors.size(); ++i){
        brushes.push_back(QBrush(colors.at(i),Qt::CrossPattern));
    }


    QVector<QBarSet*> barSets;
    int counter = 0;

    const auto &general = ui->treeWidget_statisticGeneral;
    for(int i=0; i<general->topLevelItemCount(); ++i){
        if(general->topLevelItem(i)->checkState(0) == Qt::Checked){
            int idx = translateGeneral[general->topLevelItem(i)->text(0)];
            barSets.push_back(statisticsBarSet(idx));
            general->topLevelItem(i)->setBackground(1,brushes.at(counter));

            barSets.at(barSets.count()-1)->setBrush(brushes.at(counter));
            ++counter;
            counter = counter%brushes.count();
        }else{
            general->topLevelItem(i)->setBackground(1,Qt::white);
        }
    }

    const auto &station = ui->treeWidget_statisticStation;
    for(int i=0; i<station->topLevelItemCount(); ++i){
        for(int j=0; j<station->topLevelItem(i)->childCount(); ++j){
            if(station->topLevelItem(i)->child(j)->checkState(0) == Qt::Checked){
                barSets.push_back(statisticsBarSet(8+i*nsta+j));
                station->topLevelItem(i)->child(j)->setBackground(1,brushes.at(counter));

                barSets.at(barSets.count()-1)->setBrush(brushes.at(counter));
                ++counter;
                counter = counter%brushes.count();
            }else{
                station->topLevelItem(i)->child(j)->setBackground(1,Qt::white);
            }
        }
    }

    QStringList categories;
    for(const auto &key1: statistics.keys()){
        for(const auto &key2: statistics[key1].keys()){
            categories.push_back("v"+QString("%1 ").arg(key2)+key1);
        }
    }

    QVector<double>score(categories.size(),0);
    for(int i=0; i<general->topLevelItemCount(); ++i){
        double val = qobject_cast<QDoubleSpinBox*>(general->itemWidget(general->topLevelItem(i),2))->value();
        if(val!=0){
            int idx = translateGeneral[general->topLevelItem(i)->text(0)];
            auto data = statisticsBarSet(idx);
            for(int id = 0; id<data->count(); ++id){
                score[id] += data->at(id)*val;
            }
        }
    }
    for(int i=0; i<station->topLevelItemCount(); ++i){
        for(int j=0; j<station->topLevelItem(i)->childCount(); ++j){
            auto xxx = qobject_cast<QDoubleSpinBox*>(station->itemWidget(station->topLevelItem(i)->child(j),2));
            double val = qobject_cast<QDoubleSpinBox*>(station->itemWidget(station->topLevelItem(i)->child(j),2))->value();
            if(val!=0){
                auto data = statisticsBarSet(8+i*nsta+j);
                for(int id = 0; id<data->count(); ++id){
                    score[id] += data->at(id)*val;
                }
            }
        }
    }

    QVector<int> idx(score.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::stable_sort(idx.begin(), idx.end(),[&score](int i1, int i2) {return score[i1] > score[i2];});

    QStringList sortedCategories;
    for(int i=0; i<idx.count(); ++i){
        sortedCategories << categories.at(idx.at(i));
    }
    QBarSeries* sortedSeries = new QBarSeries();
    for(int i=0; i<barSets.count(); ++i){
        auto thisBarSet = barSets.at(i);
        QBarSet *sortedBarSet = new QBarSet("");
        for(int j = 0; j<thisBarSet->count(); ++j){
            *sortedBarSet << thisBarSet->at(idx.at(j));
        }
        sortedSeries->append(sortedBarSet);
        delete(thisBarSet);
    }

    QChart *chart = new QChart();
    chart->addSeries(sortedSeries);
    chart->setTitle("statistics");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chart->legend()->setVisible(false);

    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(sortedCategories);
    chart->createDefaultAxes();
    chart->setAxisX(axis, sortedSeries);

    int showN = ui->spinBox_statistics_show->value();

    if(!sortedCategories.isEmpty()){
        QString minax = sortedCategories.at(0);
        QString maxax;
        if(sortedCategories.count()>showN){
            maxax = sortedCategories.at(showN-1);
        }else{
            maxax = sortedCategories.at(sortedCategories.size()-1);
        }
        axis->setMin(minax);
        axis->setMax(maxax);
    }

    delete statisticsView->chart();
    statisticsView->setChart(chart);
    statisticsView->setRenderHint(QPainter::Antialiasing);

    if(categories.count()>showN){
        ui->horizontalScrollBar_statistics->setRange(0,categories.size()-showN);
    }else{
        ui->horizontalScrollBar_statistics->setRange(0,0);
    }
    ui->treeWidget_statisticGeneral->blockSignals(false);
    ui->treeWidget_statisticStation->blockSignals(false);
    ui->treeWidget_statisticSource->blockSignals(false);

}

QBarSet *MainWindow::statisticsBarSet(int idx)
{
    QVector<double> v;
    for(const auto &key1: statistics.keys()){
        for(const auto &key2: statistics[key1].keys()){
            v << statistics[key1][key2][idx];
        }
    }

    if(ui->checkBox_statistics_removeMinimum->isChecked()){
        double min = *std::min_element(v.begin(), v.end());
        for(int i=0; i<v.count(); ++i){
            v[i] -= min;
        }
    }

    if(ui->radioButton_statistics_relative->isChecked()){
        double max = *std::max_element(v.begin(), v.end());
        for(int i=0; i<v.count(); ++i){
            v[i] /= max;
        }
    }

    QBarSet *set = new QBarSet("");
    for(int i=0; i<v.count(); ++i){
        *set << v.at(i);
    }

    return set;
}

void MainWindow::on_treeWidget_statisticGeneral_itemChanged(QTreeWidgetItem *item, int column)
{
    plotStatistics();
}

void MainWindow::on_treeWidget_statisticStation_itemChanged(QTreeWidgetItem *item, int column)
{
    ui->treeWidget_statisticStation->blockSignals(true);

    if(item->checkState(0) == Qt::PartiallyChecked){
        return;
    }
    for(int i=0; i<item->childCount(); ++i){
        item->child(i)->setCheckState(0,item->checkState(0));
    }

    auto parent = item->parent();
    bool checked = false;
    bool unchecked = false;
    if(parent){
        for(int i=0; i<parent->childCount();++i){
            if(parent->child(i)->checkState(0) == Qt::Checked){
                checked = true;
            }else{
                unchecked = true;
            }
        }

        if(checked && unchecked){
            parent->setCheckState(0,Qt::PartiallyChecked);
        }else if(checked){
            parent->setCheckState(0,Qt::Checked);
        }else if(unchecked){
            parent->setCheckState(0,Qt::Unchecked);
        }
    }

    ui->treeWidget_statisticStation->blockSignals(false);

    plotStatistics();
}

//void MainWindow::on_horizontalScrollBar_statistics_sliderMoved(int position)
//{
//    auto axis = qobject_cast<QBarCategoryAxis*>(statisticsView->chart()->axisX());
//    auto categories = axis->categories();
//    QString min = categories.at(position);
//    QString max = categories.at(position+3-1);
//    axis->setMin(min);
//    axis->setMax(max);
//}

void MainWindow::on_horizontalScrollBar_statistics_valueChanged(int value)
{

    statisticsView->chart()->setAnimationOptions(QChart::NoAnimation);

    auto axis = qobject_cast<QBarCategoryAxis*>(statisticsView->chart()->axisX());
    auto categories = axis->categories();
    if(!categories.isEmpty()){
        QString min = categories.at(value);
        QString max = categories.at(value+ui->spinBox_statistics_show->value()-1);
        axis->setMin(min);
        axis->setMax(max);
        axis->setMin(min);
    }
}

void MainWindow::on_spinBox_statistics_show_valueChanged(int arg1)
{
    plotStatistics();
}
