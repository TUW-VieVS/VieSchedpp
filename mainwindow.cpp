#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QCoreApplication::setApplicationName("VieSched++ GUI");
    QCoreApplication::setApplicationVersion(GIT_COMMIT_HASH);

    QString schedVersion = GIT_SCHEDULER_COMMIT_HASH;
    if(schedVersion.length() != 40){
        schedVersion = "unknown";
    }
    if( schedVersion !=  "unknown"){
        ui->label_version->setText("VieSched++ GUI version: " + QCoreApplication::applicationVersion() + "\n../VieSched++ version: " + schedVersion);
    }else{
        ui->label_version->setText("VieSched++ GUI version: " + QCoreApplication::applicationVersion());
    }

    ui->label_version->setFont(QFont(QApplication::font().family(),8));
    QCoreApplication::setOrganizationName("TU Wien");
    QCoreApplication::setOrganizationDomain("http://hg.geo.tuwien.ac.at/");

    this->setWindowTitle("VieSched++");

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

    plotSkyCoverageTemplate = false;
    setupChanged = false;
    setupStation = new QChartView;
    setupStation->setToolTip("station setup");
    setupStation->setStatusTip("station setup");
    setupSource = new QChartView;
    setupSource->setToolTip("station setup");
    setupSource->setStatusTip("station setup");
    setupBaseline = new QChartView;
    setupBaseline->setToolTip("station setup");
    setupBaseline->setStatusTip("station setup");
    prepareSetupPlot(setupStation, ui->verticalLayout_28);
    stationSetupCallout = new Callout(setupStation->chart());
    stationSetupCallout->hide();

    prepareSetupPlot(setupSource, ui->verticalLayout_36);
    sourceSetupCallout = new Callout(setupSource->chart());
    sourceSetupCallout->hide();

    prepareSetupPlot(setupBaseline, ui->verticalLayout_40);
    baselineSetupCallout = new Callout(setupBaseline->chart());
    baselineSetupCallout->hide();

    ui->statusBar->addPermanentWidget(new QLabel("no schedules started"));

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
    allStationModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));
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
    allSourceModel = new QStandardItemModel(0,3,this);
    allSourceModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    allSourceModel->setHeaderData(1, Qt::Horizontal, QObject::tr("RA [deg]"));
    allSourceModel->setHeaderData(2, Qt::Horizontal, QObject::tr("DC [deg]"));
    allStationProxyModel = new QSortFilterProxyModel(this);
    allStationProxyModel->setSourceModel(allStationModel);
    allStationProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    allSourceProxyModel = new QSortFilterProxyModel(this);
    allSourceProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    allSourceProxyModel->setSourceModel(allSourceModel);


    selectedStationModel = new QStandardItemModel(0,19,this);
    selectedStationModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));
    selectedStationModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Id"));
    selectedStationModel->setHeaderData(2, Qt::Horizontal, QObject::tr("lat [deg]"));
    selectedStationModel->setHeaderData(3, Qt::Horizontal, QObject::tr("lon [deg]"));
    selectedStationModel->setHeaderData(4, Qt::Horizontal, QObject::tr("diam [m]"));
    selectedStationModel->setHeaderData(5, Qt::Horizontal, QObject::tr("SEFD X [Jy]"));
    selectedStationModel->setHeaderData(6, Qt::Horizontal, QObject::tr("SEFD S [Jy]"));
    selectedStationModel->setHeaderData(7, Qt::Horizontal, QObject::tr("axis offset [m]"));
    selectedStationModel->setHeaderData(8, Qt::Horizontal, QObject::tr("slew rate1 [deg/min]"));
    selectedStationModel->setHeaderData(9, Qt::Horizontal, QObject::tr("constant overhead1 [sec]"));
    selectedStationModel->setHeaderData(10, Qt::Horizontal, QObject::tr("lower axis limit1 [deg]"));
    selectedStationModel->setHeaderData(11, Qt::Horizontal, QObject::tr("upper axis limit1 [deg]"));
    selectedStationModel->setHeaderData(12, Qt::Horizontal, QObject::tr("slew rate2 [deg/min]"));
    selectedStationModel->setHeaderData(13, Qt::Horizontal, QObject::tr("constant overhead2 [sec]"));
    selectedStationModel->setHeaderData(14, Qt::Horizontal, QObject::tr("lower axis limit2 [deg]"));
    selectedStationModel->setHeaderData(15, Qt::Horizontal, QObject::tr("upper axis limit2 [deg]"));
    selectedStationModel->setHeaderData(16, Qt::Horizontal, QObject::tr("x [m]"));
    selectedStationModel->setHeaderData(17, Qt::Horizontal, QObject::tr("y [m]"));
    selectedStationModel->setHeaderData(18, Qt::Horizontal, QObject::tr("z [m]"));
    selectedSourceModel = new QStandardItemModel(0,3,this);
    selectedSourceModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    selectedSourceModel->setHeaderData(1, Qt::Horizontal, QObject::tr("RA [deg]"));
    selectedSourceModel->setHeaderData(2, Qt::Horizontal, QObject::tr("DC [deg]"));
    selectedBaselineModel = new QStandardItemModel(0,2,this);
    selectedBaselineModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));
    selectedBaselineModel->setHeaderData(1, Qt::Horizontal, QObject::tr("distance [km]"));

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

    connect(selectedBaselineModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(baselineListChanged()));
    connect(selectedBaselineModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(baselineListChanged()));

    ui->treeView_allAvailabeStations->setModel(allStationProxyModel);
    ui->treeView_allAvailabeStations->setRootIsDecorated(false);
    ui->treeView_allAvailabeStations->setSortingEnabled(true);
    ui->treeView_allAvailabeStations->sortByColumn(0, Qt::AscendingOrder);
    auto hv1 = ui->treeView_allAvailabeStations->header();
    hv1->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->treeView_allAvailabeSources->setModel(allSourceProxyModel);
    ui->treeView_allAvailabeSources->setRootIsDecorated(false);
    ui->treeView_allAvailabeSources->setSortingEnabled(true);
    ui->treeView_allAvailabeSources->sortByColumn(0, Qt::AscendingOrder);
    auto hv2 = ui->treeView_allAvailabeSources->header();
    hv2->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->treeView_allSelectedBaselines->setModel(selectedBaselineModel);
    ui->treeView_allSelectedBaselines->setRootIsDecorated(false);
    ui->treeView_allSelectedBaselines->setSortingEnabled(true);
    ui->treeView_allSelectedBaselines->sortByColumn(0, Qt::AscendingOrder);
    auto hv3 = ui->treeView_allSelectedBaselines->header();
    hv3->setSectionResizeMode(QHeaderView::ResizeToContents);
    createBaselines = true;

    ui->treeView_allSelectedStations->setModel(selectedStationModel);
    ui->treeView_allSelectedStations->setRootIsDecorated(false);
    ui->treeView_allSelectedStations->setSortingEnabled(true);
    ui->treeView_allSelectedStations->sortByColumn(0, Qt::AscendingOrder);
    auto hv4 = ui->treeView_allSelectedStations->header();
    hv4->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->treeView_allSelectedSources->setModel(selectedSourceModel);
    ui->treeView_allSelectedSources->setRootIsDecorated(false);
    ui->treeView_allSelectedSources->setSortingEnabled(true);
    ui->treeView_allSelectedSources->sortByColumn(0, Qt::AscendingOrder);
    auto hv5 = ui->treeView_allSelectedSources->header();
    hv5->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->comboBox_skedObsModes->setModel(allSkedModesModel);

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

    worldmap = new ChartView(this);
    qtUtil::worldMap(worldmap);
    worldMapCallout = new Callout(worldmap->chart());
    worldMapCallout->hide();
    readStations();
    ui->horizontalLayout_worldmap->insertWidget(0,worldmap,10);


    skymap = new ChartView();
    qtUtil::skyMap(skymap);
    skyMapCallout = new Callout(skymap->chart());
    skyMapCallout->hide();
    readSources();
    ui->horizontalLayout_skymap->insertWidget(0,skymap,10);


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
    ui->splitter_4->setSizes(QList<int>({INT_MAX, INT_MAX}));
    ui->splitter_6->setSizes(QList<int>({INT_MAX, INT_MAX}));

    ui->spinBox_fontSize->setValue(QApplication::font().pointSize());
    ui->iconSizeSpinBox->setValue(ui->fileToolBar->iconSize().width());

    auto hv6 = ui->treeWidget_setupStationWait->header();
    hv6->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto hv7 = ui->treeWidget_setupStationAxis->header();
    hv7->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->pushButton_setupAxisAdd,SIGNAL(clicked(bool)),this,SLOT(setupStationAxisBufferAddRow()));
    connect(ui->pushButton_setupWaitAdd,SIGNAL(clicked(bool)),this,SLOT(setupStationWaitAddRow()));

    setupStationAxisBufferAddRow();
    setupStationWaitAddRow();

    connect(ui->pushButton_addGroupStationSetup_2,SIGNAL(clicked(bool)),this,SLOT(addGroupStation()));
    connect(ui->pushButton_addGroupsourceSetup_2,SIGNAL(clicked(bool)),this,SLOT(addGroupSource()));
    connect(ui->pushButton_addGroupBaselineSetup_2,SIGNAL(clicked(bool)),this,SLOT(addGroupBaseline()));

    connect(ui->lineEdit_faqSearch,SIGNAL(textChanged(QString)),this,SLOT(on_pushButton_faqSearch_clicked()));

    ui->spinBox_scanSequenceCadence->setValue(1);
    ui->spinBox_scanSequenceCadence->setMinimum(1);

    setupStatisticView();
    setupSkyCoverageTemplatePlot();

    ui->comboBox_conditions_members->setModel(allSourcePlusGroupModel);
    connect(ui->pushButton_addSourceGroup_conditions,SIGNAL(clicked(bool)), this, SLOT(addGroupSource()));

    ui->comboBox_highImpactStation->setModel(allStationPlusGroupModel);
    connect(ui->pushButton_addGroupStationHighImpactAzEl,SIGNAL(clicked(bool)), this, SLOT(addGroupStation()));

    connect(ui->lineEdit_ivsMaster,SIGNAL(returnPressed()),this,SLOT(on_pushButton_clicked()));
    connect(ui->horizontalSlider_markerSizeWorldmap,SIGNAL(valueChanged(int)),this,SLOT(markerWorldmap()));
    connect(ui->horizontalSlider_markerSkymap,SIGNAL(valueChanged(int)),this,SLOT(markerSkymap()));

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->plainTextEdit_notes->setFont(fixedFont);

    connect(ui->lineEdit_outputPath,SIGNAL(textChanged(QString)),ui->lineEdit_sessionPath,SLOT(setText(QString)));

    connect(ui->sampleBitsSpinBox,SIGNAL(valueChanged(int)),this,SLOT(gbps()));
    connect(ui->sampleRateDoubleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(gbps()));
    gbps();

    ui->dateTimeEdit_sessionStart->setDisplayFormat("dd.MM.yyyy HH:mm");
    ui->DateTimeEdit_endParameterBaseline->setDisplayFormat("dd.MM.yyyy HH:mm");
    ui->DateTimeEdit_endParameterSource->setDisplayFormat("dd.MM.yyyy HH:mm");
    ui->DateTimeEdit_endParameterStation->setDisplayFormat("dd.MM.yyyy HH:mm");
    ui->DateTimeEdit_startParameterBaseline->setDisplayFormat("dd.MM.yyyy HH:mm");
    ui->DateTimeEdit_startParameterSource->setDisplayFormat("dd.MM.yyyy HH:mm");
    ui->DateTimeEdit_startParameterStation->setDisplayFormat("dd.MM.yyyy HH:mm");
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

// ########################################### DISPLAY LISTS ###########################################

void MainWindow::displayStationSetupMember(QString name)
{
    if(name.isEmpty()){
        return;
    }
    auto t = ui->tableWidget_setupStation;
    t->clear();    
    t->setColumnCount(1);
    t->verticalHeader()->show();
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
    if(name.isEmpty()){
        return;
    }

    auto t = ui->tableWidget_setupSource;
    t->clear();
    t->setColumnCount(1);
    t->verticalHeader()->show();
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
    if(name.isEmpty()){
        return;
    }

    auto t = ui->tableWidget_setupBaseline;
    t->clear();
    t->setColumnCount(1);
    t->verticalHeader()->show();
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
        t->setRowCount(3);
        QStringList list= name.split("-");

        double distance;
        for(int i=0; i<selectedBaselineModel->rowCount(); ++i){
            QString txt = selectedBaselineModel->index(i,0).data().toString();
            if(txt == name){
                distance = selectedBaselineModel->index(i,1).data().toDouble();
            }
        }

        t->setItem(0,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),list.at(0)));
        t->setItem(0,1,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),list.at(1)));
        t->setItem(0,2,new QTableWidgetItem(QString("%1").arg(distance)));

        t->setVerticalHeaderItem(0,new QTableWidgetItem("Station 1"));
        t->setVerticalHeaderItem(1,new QTableWidgetItem("Station 2"));
        t->setVerticalHeaderItem(2,new QTableWidgetItem("distance [km]"));

    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::displayStationSetupParameter(QString name)
{
    if(name.isEmpty()){
        return;
    }

    auto t = ui->tableWidget_setupStation;
    if(name == "multi scheduling"){
        t->clear();
        t->setRowCount(0);
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0, new QTableWidgetItem("multi scheduling"));
        t->verticalHeader()->hide();
        t->insertRow(0);
        t->setItem(0,0,new QTableWidgetItem(QIcon(":/icons/icons/multi_sched.png"),"see multi scheduling setup"));
        return;
    }
    t->verticalHeader()->show();
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
    if(para.availableForFillinmode.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.availableForFillinmode ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("available for fillin mode"));
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
    if(para.maxSlewDistance.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxSlewDistance)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max slew distance [deg]"));
        ++r;
    }
    if(para.minSlewDistance.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minSlewDistance)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min slew distance [deg]"));
        ++r;
    }
    if(para.maxWait.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxWait)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max wait time [s]"));
        ++r;
    }
    if(para.maxNumberOfScans.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.maxNumberOfScans)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("max number of scans"));
        ++r;
    }
    if(para.minElevation.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minElevation)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min elevation [deg]"));
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
    }
    QHeaderView *hv = t->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::displaySourceSetupParameter(QString name){
    if(name.isEmpty()){
        return;
    }

    auto t = ui->tableWidget_setupSource;
    if(name == "multi scheduling"){
        t->clear();
        t->setRowCount(0);
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0, new QTableWidgetItem("multi scheduling"));
        t->verticalHeader()->hide();
        t->insertRow(0);
        t->setItem(0,0,new QTableWidgetItem(QIcon(":/icons/icons/multi_sched.png"),"see multi scheduling setup"));
        return;
    }
    t->verticalHeader()->show();
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
    if(para.availableForFillinmode.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.availableForFillinmode ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("available for fillin mode"));
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
    if(para.minElevation.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minElevation)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("minimum elevation [deg]"));
        ++r;
    }
    if(para.minSunDistance.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minSunDistance)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("minimum sun distance [deg]"));
        ++r;
    }
    if(para.fixedScanDuration.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.fixedScanDuration)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("fixed scan duration"));
        ++r;
    }
    if(para.tryToFocusIfObservedOnce.is_initialized()){
        t->insertRow(r);
        QString boolText = *para.tryToFocusIfObservedOnce ? "true" : "false";
        t->setItem(r,0,new QTableWidgetItem(boolText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("try to focus if observed once"));
        ++r;

        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.tryToFocusFactor)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("increase weight by factor"));
        ++r;

        t->insertRow(r);
        QString occurrencyText = *para.tryToFocusOccurrency == VieVS::ParameterSettings::TryToFocusOccurrency::once ? "once" : "per scan";
        t->setItem(r,0,new QTableWidgetItem(occurrencyText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("increase weight"));
        ++r;

        t->insertRow(r);
        QString typeText = *para.tryToFocusType == VieVS::ParameterSettings::TryToFocusType::additive ? "additive" : "multiplicative";
        t->setItem(r,0,new QTableWidgetItem(typeText));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("increase weight"));
        ++r;
    }
    if(para.tryToObserveXTimesEvenlyDistributed.is_initialized()){
        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.tryToObserveXTimesEvenlyDistributed)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("eavenly distributed scans over time"));
        ++r;

        t->insertRow(r);
        t->setItem(r,0,new QTableWidgetItem(QString::number(*para.tryToObserveXTimesMinRepeat)));
        t->setVerticalHeaderItem(r,new QTableWidgetItem("min time between two scans [s]"));
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
    if(name.isEmpty()){
        return;
    }

    auto t = ui->tableWidget_setupBaseline;
    if(name == "multi scheduling"){
        t->clear();
        t->setRowCount(0);
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0, new QTableWidgetItem("multi scheduling"));
        t->verticalHeader()->hide();
        t->insertRow(0);
        t->setItem(0,0,new QTableWidgetItem(QIcon(":/icons/icons/multi_sched.png"),"see multi scheduling setup"));
        return;
    }
    t->verticalHeader()->show();
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
        t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore"));
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

// ########################################### NAVIGATION AND GUI SETUP ###########################################

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

void MainWindow::on_actionNetwork_triggered()
{
    ui->main_stacked->setCurrentIndex(5);
}

void MainWindow::on_actionSource_List_triggered()
{
    ui->main_stacked->setCurrentIndex(6);
}

void MainWindow::on_actionStation_triggered()
{
    ui->main_stacked->setCurrentIndex(7);
}

void MainWindow::on_actionSource_triggered()
{
    ui->main_stacked->setCurrentIndex(8);
}

void MainWindow::on_actionBaseline_triggered()
{
    ui->main_stacked->setCurrentIndex(9);
}

void MainWindow::on_actionWeight_factors_triggered()
{
    ui->main_stacked->setCurrentIndex(10);
}

void MainWindow::on_actionOutput_triggered()
{
    ui->main_stacked->setCurrentIndex(11);
}

void MainWindow::on_actionRules_triggered()
{
    ui->main_stacked->setCurrentIndex(12);
}

void MainWindow::on_actionMulti_Scheduling_triggered()
{
    ui->main_stacked->setCurrentIndex(13);
}

void MainWindow::on_actionSky_Coverage_triggered()
{
    ui->main_stacked->setCurrentIndex(14);
}

void MainWindow::on_actionConditions_triggered()
{
    ui->main_stacked->setCurrentIndex(15);
}

void MainWindow::on_actionFix_High_Impact_Scans_triggered()
{
    ui->main_stacked->setCurrentIndex(16);
}

void MainWindow::on_actionsummary_triggered()
{
    ui->main_stacked->setCurrentIndex(17);
}

void MainWindow::on_actionSkd_Parser_triggered()
{
    ui->main_stacked->setCurrentIndex(18);
}

void MainWindow::on_actionLog_parser_triggered()
{
    ui->main_stacked->setCurrentIndex(19);
}

void MainWindow::on_actionFAQ_triggered()
{
    ui->main_stacked->setCurrentIndex(20);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
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

void MainWindow::on_actionOpen_triggered()
{
    QString startPath = ui->lineEdit_sessionPath->text();
    QString path = QFileDialog::getOpenFileName(this, "Browse to xml file", startPath, tr("xml files (*.xml)"));
    if( !path.isEmpty() ){
        try{
            loadXML(path);
        }catch(...){
            QMessageBox::warning(this, "Error", "Error while loading "+path);
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
//        dw->setStyleSheet("background-color:gray;");

        dw->setWindowTitle("VieSchedpp log");
        QWidget * base = new QWidget();

        QHBoxLayout *header = new QHBoxLayout(dw);
        QPushButton *d = new QPushButton(dw);
        d->setText("terminate");
        d->setIcon(QIcon(":/icons/icons/edit-delete-6.png"));
        header->insertWidget(0,new QLabel("processing file: "+path,dw),1);
        header->insertWidget(1,d,0);

        QVBoxLayout *l1 = new QVBoxLayout(dw);

        myTextBrowser *tb = new myTextBrowser(dw);

        l1->insertLayout(0,header);
        l1->insertWidget(1,tb);

        base->setLayout(l1);
        dw->setWidget(base);


        QList<QDockWidget *> dockWidgets = this->findChildren<QDockWidget *>();

        if(dockWidgets.size() == 1){
            addDockWidget(Qt::BottomDockWidgetArea,dw);
        }else{
            tabifyDockWidget(dockWidgets.at(0),dw);
        }

        QProcess *start = new QProcess(this);
        #ifdef Q_OS_WIN
            QString program = ui->pathToSchedulerLineEdit->text();
            start->start("cmd.exe",
                         QStringList() << "/c" << program << fullPath);
        #else
            QString program = ui->pathToSchedulerLineEdit->text();
            QStringList arguments;
            arguments << fullPath;
            start->start(program,arguments);
        #endif


        QPushButton *sb = new QPushButton(dw);
        sb->setText("terminate");
        sb->setIcon(QIcon(":/icons/icons/edit-delete-6.png"));
        sb->setToolTip("session: "+path);
        sb->setStatusTip("session: "+path);
        sb->setMinimumSize(30,30);
        ui->statusBar->insertPermanentWidget(1,sb);

        for(auto &any: ui->statusBar->children()){
            QLabel *l = qobject_cast<QLabel *>(any);
            if(l){
                QString txt = l->text();
                double i = 1;
                if(txt == "no schedules started" || txt == "everything finished"){
                    l->setText("1 process running:");
                }else{
                    i = txt.split(" ").at(0).toDouble() +1;
                    l->setText(QString("%1 processes running:").arg(i));
                }

            }
        }
        connect(d,SIGNAL(pressed()),start,SLOT(kill()));
        connect(d,SIGNAL(pressed()),dw,SLOT(close()));
        connect(start,SIGNAL(readyReadStandardOutput()),tb,SLOT(readyReadStandardOutput()));
        connect(start,SIGNAL(readyReadStandardError()),tb,SLOT(readyReadStandardError()));
        connect(start,SIGNAL(finished(int)),d,SLOT(hide()));
        connect(start,SIGNAL(finished(int)),this,SLOT(processFinished()));
        connect(start,SIGNAL(finished(int)),sb,SLOT(deleteLater()));
        connect(sb,SIGNAL(pressed()),d,SLOT(hide()));
        connect(sb,SIGNAL(pressed()),start,SLOT(kill()));
        connect(sb,SIGNAL(pressed()),dw,SLOT(close()));

        if(start->waitForStarted()){
//            QMessageBox::information(this,"Scheduling started!","Starting scheduling " + fullPath +"!");
        }else{
            QMessageBox::warning(this,"Scheduling failed to start!","Could not start process:\n" + program +"\nwith arguments:\n" + fullPath);
        }
    }
}

void MainWindow::processFinished(){
    for(auto &any: ui->statusBar->children()){
        QLabel *l = qobject_cast<QLabel *>(any);
        if(l){
            QString txt = l->text();
            if(txt.isEmpty()){
                l->setText(" ");
            }else{
                double i = txt.split(" ").at(0).toDouble();
                if(i==1){
                    l->setText(QString("everything finished"));
                }else if(i==2){
                    l->setText(QString("1 process running"));
                }else{
                    l->setText(QString("%1 processes running").arg(i-1));
                }
            }
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
    } else if (item->text(0) == "Analysis"){
        if(item->checkState(0) == Qt::Checked){
            ui->analysisToolBar->show();
        }else{
            ui->analysisToolBar->hide();
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
    } else if(item->text(0) == "Network"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Network"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Source List"){
        auto actions = ui->basicToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Source List"){
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
    } else if(item->text(0) == "Conditions"){
        auto actions = ui->advancedToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Conditions"){
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
    } else if(item->text(0) == "FAQ"){
        auto actions = ui->helpToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "FAQ"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Current Reference"){
        auto actions = ui->helpToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Current Reference"){
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
    } else if(item->text(0) == "Statistics"){
        auto actions = ui->analysisToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Statistics"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Log Parser"){
        auto actions = ui->analysisToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Log Parser"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    } else if(item->text(0) == "Sched Analyser"){
        auto actions = ui->analysisToolBar->actions();
        for(const auto &any:actions){
            if(any->text() == "Skd Parser"){
                if(item->checkState(0) == Qt::Checked){
                    any->setVisible(true);
                }else{
                    any->setVisible(false);
                }
            }
        }
    }
}

void MainWindow::on_pushButton_browseExecutable_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to VieSchedpp executable", "");
    if( !path.isEmpty() ){
        ui->pathToSchedulerLineEdit->setText(path);
    }

}


// ########################################### READ AND WRITE XML DOCUMENT ###########################################

void MainWindow::defaultParameters()
{
    VieVS::ParameterSettings::ParametersStations sta;
    sta.available = true;
    sta.availableForFillinmode = true;
    sta.maxScan = 600;
    sta.minScan = 20;
    sta.maxSlewtime = 600;
    sta.maxSlewDistance = 175;
    sta.minSlewDistance = 0;
    sta.maxWait = 600;
    sta.maxNumberOfScans = 9999;
    sta.weight = 1;
    sta.minElevation = 5;

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
                    } else if (paraName == "maxSlewDistance") {
                        sta.maxSlewDistance = it2.second.get_value < double > ();
                    } else if (paraName == "minSlewDistance") {
                        sta.minSlewDistance = it2.second.get_value < double > ();
                    } else if (paraName == "maxWait") {
                        sta.maxWait = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "minElevation") {
                        sta.minElevation = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "maxNumberOfScans") {
                        sta.maxNumberOfScans = it2.second.get_value < unsigned int > ();
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
    src.availableForFillinmode = true;
    src.minRepeat = 1800;
    src.minScan = 20;
    src.maxScan = 600;
    src.weight = 1;
    src.minFlux = 0.05;
    src.maxNumberOfScans = 999;
    src.minNumberOfStations = 2;
    src.minElevation = 0;
    src.minSunDistance = 4;

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
                    } else if (paraName == "minElevation"){
                        src.minElevation = it2.second.get_value<double>();
                    } else if (paraName == "minSunDistance"){
                        src.minSunDistance = it2.second.get_value<double>();
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
                        bl.weight = it2.second.get_value<double>();
                    } else if (paraName == "minScan") {
                        bl.minScan = it2.second.get_value < unsigned int > ();
                    } else if (paraName == "maxScan") {
                        bl.maxScan = it2.second.get_value < unsigned int > ();
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

    boost::optional<int> setup = settings.get_optional<int>("settings.station.waitTimes.fieldSystem");
    if(setup.is_initialized()){
        ui->SpinBox_fieldSystem->setValue(*setup);
    }
    boost::optional<int> source = settings.get_optional<int>("settings.station.waitTimes.preob");
    if(source.is_initialized()){
        ui->SpinBox_preob->setValue(*source);
    }
    boost::optional<int> tape = settings.get_optional<int>("settings.station.waitTimes.midob");
    if(tape.is_initialized()){
        ui->SpinBox_midob->setValue(*tape);
    }
    boost::optional<int> calibration = settings.get_optional<int>("settings.station.waitTimes.postob");
    if(calibration.is_initialized()){
        ui->SpinBox_postob->setValue(*calibration);
    }


    boost::optional<double> ax1low = settings.get_optional<double>("settings.station.cableWrapBuffers.axis1LowOffset");
    if(ax1low.is_initialized()){
        ui->DoubleSpinBox_axis1low->setValue(*ax1low);
    }
    boost::optional<double> ax1up = settings.get_optional<double>("settings.station.cableWrapBuffers.axis1UpOffset");
    if(ax1up.is_initialized()){
        ui->DoubleSpinBox_axis1up->setValue(*ax1up);
    }
    boost::optional<double> ax2low = settings.get_optional<double>("settings.station.cableWrapBuffers.axis2LowOffset");
    if(ax2low.is_initialized()){
        ui->DoubleSpinBox_axis2low->setValue(*ax2low);
    }
    boost::optional<double> ax2up = settings.get_optional<double>("settings.station.cableWrapBuffers.axis2UpOffset");
    if(ax2up.is_initialized()){
        ui->DoubleSpinBox_axis2up->setValue(*ax2up);
    }

    boost::optional<std::string> skdMode = settings.get_optional<std::string>("settings.mode.skdMode");
    if(skdMode.is_initialized()){
        ui->comboBox_skedObsModes->setCurrentText(QString::fromStdString(*skdMode));
    }

    boost::optional<bool> fillinmodeDuringScanSelection = settings.get_optional<bool>("settings.general.fillinmodeDuringScanSelection");
    if(fillinmodeDuringScanSelection.is_initialized()){
        ui->checkBox_fillinmode_duringscan->setChecked(*fillinmodeDuringScanSelection);
    }
    boost::optional<bool> fillinmodeInfluenceOnSchedule = settings.get_optional<bool>("settings.general.fillinmodeInfluenceOnSchedule");
    if(fillinmodeInfluenceOnSchedule.is_initialized()){
        ui->checkBox_fillinModeInfluence->setChecked(*fillinmodeInfluenceOnSchedule);
    }
    boost::optional<bool> fillinmodeAPosteriori = settings.get_optional<bool>("settings.general.fillinmodeAPosteriori");
    if(fillinmodeAPosteriori.is_initialized()){
        ui->checkBox_fillinmode_aposteriori->setChecked(*fillinmodeAPosteriori);
    }

    boost::optional<double> subnettingMinAngle = settings.get_optional<double>("settings.general.subnettingMinAngle");
    if(subnettingMinAngle.is_initialized()){
        ui->doubleSpinBox_subnettingDistance->setValue(*subnettingMinAngle);
    }
    boost::optional<double> subnettingMinNSta = settings.get_optional<double>("settings.general.subnettingMinNStaPercent");
    if(subnettingMinNSta.is_initialized()){
        ui->doubleSpinBox_subnettingMinStations->setValue(*subnettingMinNSta);
    }
    boost::optional<double> subnettingMinNStaAllBut = settings.get_optional<double>("settings.general.subnettingMinNStaAllBut");
    if(subnettingMinNStaAllBut.is_initialized()){
        ui->spinBox_subnetting_min_sta->setValue(*subnettingMinNStaAllBut);
    }
    boost::optional<bool> subnettingFlag = settings.get_optional<bool>("settings.general.subnettingMinNstaPercent_otherwiseAllBut");
    if(subnettingFlag.is_initialized()){
        if(*subnettingFlag){
            ui->radioButton_subnetting_percent->setChecked(true);
        }else{
            ui->radioButton_subnetting_allBut->setChecked(true);
        }
    }
    boost::optional<bool> subnetting = settings.get_optional<bool>("settings.general.subnetting");
    if(subnetting.is_initialized()){
        ui->groupBox_subnetting->setChecked(*subnetting);
    }

    boost::optional<std::string> alignObservingTime = settings.get_optional<std::string>("settings.general.alignObservingTime");
    if(alignObservingTime.is_initialized()){
        if(*alignObservingTime == "start"){
            ui->radioButton_alignStart->setChecked(true);
        }else if(*alignObservingTime == "end"){
            ui->radioButton_alignEnd->setChecked(true);
        }else if(*alignObservingTime == "individual"){
            ui->radioButton_alignIndividual->setChecked(true);
        }
    }

    boost::optional<double> influenceDistance = settings.get_optional<double>("settings.skyCoverage.influenceDistance");
    if(influenceDistance.is_initialized()){
        ui->influenceDistanceDoubleSpinBox->setValue(*influenceDistance);
    }
    boost::optional<int> influenceInterval = settings.get_optional<int>("settings.skyCoverage.influenceInterval");
    if(influenceInterval.is_initialized()){
        ui->influenceTimeSpinBox->setValue(*influenceInterval);
    }
    boost::optional<double> maxTwinTelecopeDistance = settings.get_optional<double>("settings.skyCoverage.maxTwinTelecopeDistance");
    if(maxTwinTelecopeDistance.is_initialized()){
        ui->maxDistanceForCombiningAntennasDoubleSpinBox->setValue(*maxTwinTelecopeDistance);
    }
    boost::optional<std::string> distanceType = settings.get_optional<std::string>("settings.skyCoverage.distanceType");
    if(distanceType.is_initialized()){
        ui->comboBox_skyCoverageDistanceType->setCurrentText(QString::fromStdString(*distanceType));
    }
    boost::optional<std::string> timeType = settings.get_optional<std::string>("settings.skyCoverage.timeType");
    if(timeType.is_initialized()){
        ui->comboBox_skyCoverageTimeType->setCurrentText(QString::fromStdString(*timeType));
    }

    boost::optional<bool> skyCoverageChecked = settings.get_optional<bool>("settings.weightFactor.skyCoverageChecked");
    if(skyCoverageChecked.is_initialized()){
        ui->checkBox_weightCoverage->setChecked(*skyCoverageChecked);
//        ui->doubleSpinBox_weightSkyCoverage->setEnabled(*skyCoverageChecked);
    }
    boost::optional<bool> numberOfObservationsChecked = settings.get_optional<bool>("settings.weightFactor.numberOfObservationsChecked");
    if(numberOfObservationsChecked.is_initialized()){
        ui->checkBox_weightNobs->setChecked(*numberOfObservationsChecked);
//        ui->doubleSpinBox_weightNumberOfObservations->setEnabled(*numberOfObservationsChecked);
    }
    boost::optional<bool> durationChecked = settings.get_optional<bool>("settings.weightFactor.durationChecked");
    if(durationChecked.is_initialized()){
        ui->checkBox_weightDuration->setChecked(*durationChecked);
//        ui->doubleSpinBox_weightDuration->setEnabled(*durationChecked);
    }
    boost::optional<bool> averageSourcesChecked = settings.get_optional<bool>("settings.weightFactor.averageSourcesChecked");
    if(averageSourcesChecked.is_initialized()){
        ui->checkBox_weightAverageSources->setChecked(*averageSourcesChecked);
//        ui->doubleSpinBox_weightAverageSources->setEnabled(*averageSourcesChecked);
    }
    boost::optional<bool> averageStationsChecked = settings.get_optional<bool>("settings.weightFactor.averageStationsChecked");
    if(averageStationsChecked.is_initialized()){
        ui->checkBox_weightAverageStations->setChecked(*averageStationsChecked);
//        ui->doubleSpinBox_weightAverageStations->setEnabled(*averageStationsChecked);
    }
    boost::optional<bool> idleTimeChecked = settings.get_optional<bool>("settings.weightFactor.idleTimeChecked");
    if(idleTimeChecked.is_initialized()){
        ui->checkBox_weightIdleTime->setChecked(*idleTimeChecked);
    }
    boost::optional<bool> weightDeclinationChecked = settings.get_optional<bool>("settings.weightFactor.weightDeclinationChecked");
    if(weightDeclinationChecked.is_initialized()){
        ui->checkBox_weightLowDeclination->setChecked(*weightDeclinationChecked);
//        ui->doubleSpinBox_weightLowDec->setEnabled(*weightDeclinationChecked);
//        ui->doubleSpinBox_weightLowDecStart->setEnabled(*weightDeclinationChecked);
//        ui->doubleSpinBox_weightLowDecEnd->setEnabled(*weightDeclinationChecked);
//        ui->label_weightLowDecEnd->setEnabled(*weightDeclinationChecked);
//        ui->label_weightLowDecStart->setEnabled(*weightDeclinationChecked);
    }
    boost::optional<bool> weightLowElevationChecked = settings.get_optional<bool>("settings.weightFactor.weightLowElevationChecked");
    if(weightLowElevationChecked.is_initialized()){
        ui->checkBox_weightLowElevation->setChecked(*weightLowElevationChecked);
//        ui->doubleSpinBox_weightLowEl->setEnabled(*weightLowElevationChecked);
//        ui->doubleSpinBox_weightLowElStart->setEnabled(*weightLowElevationChecked);
//        ui->doubleSpinBox_weightLowElEnd->setEnabled(*weightLowElevationChecked);
//        ui->label_weightLowElStart->setEnabled(*weightLowElevationChecked);
//        ui->label_weightLowElEnd->setEnabled(*weightLowElevationChecked);
    }

    boost::optional<double> skyCoverage = settings.get_optional<double>("settings.weightFactor.skyCoverage");
    if(skyCoverage.is_initialized()){
        ui->doubleSpinBox_weightSkyCoverage->setValue(*skyCoverage);
    }
    boost::optional<double> numberOfObservations = settings.get_optional<double>("settings.weightFactor.numberOfObservations");
    if(numberOfObservations.is_initialized()){
        ui->doubleSpinBox_weightNumberOfObservations->setValue(*numberOfObservations);
    }
    boost::optional<double> duration = settings.get_optional<double>("settings.weightFactor.duration");
    if(duration.is_initialized()){
        ui->doubleSpinBox_weightDuration->setValue(*duration);
    }
    boost::optional<double> averageSources = settings.get_optional<double>("settings.weightFactor.averageSources");
    if(averageSources.is_initialized()){
        ui->doubleSpinBox_weightAverageSources->setValue(*averageSources);
    }
    boost::optional<double> averageStations = settings.get_optional<double>("settings.weightFactor.averageStations");
    if(averageStations.is_initialized()){
        ui->doubleSpinBox_weightAverageStations->setValue(*averageStations);
    }
    boost::optional<double> weightIdleTime = settings.get_optional<double>("settings.weightFactor.weightIdleTime");
    if(weightIdleTime.is_initialized()){
        ui->doubleSpinBox_weightIdleTime->setValue(*weightIdleTime);
    }
    boost::optional<double> idleTimeInterval = settings.get_optional<double>("settings.weightFactor.idleTimeInterval");
    if(idleTimeInterval.is_initialized()){
        ui->spinBox_idleTimeInterval->setValue(*idleTimeInterval);
    }
    boost::optional<double> weightDeclination = settings.get_optional<double>("settings.weightFactor.weightDeclination");
    if(weightDeclination.is_initialized()){
        ui->doubleSpinBox_weightLowDec->setValue(*weightDeclination);
    }
    boost::optional<double> declinationStartWeight = settings.get_optional<double>("settings.weightFactor.declinationStartWeight");
    if(declinationStartWeight.is_initialized()){
        ui->doubleSpinBox_weightLowDecStart->setValue(*declinationStartWeight);
    }
    boost::optional<double> declinationFullWeight = settings.get_optional<double>("settings.weightFactor.declinationFullWeight");
    if(declinationFullWeight.is_initialized()){
        ui->doubleSpinBox_weightLowDecEnd->setValue(*declinationFullWeight);
    }
    boost::optional<double> weightLowElevation = settings.get_optional<double>("settings.weightFactor.weightLowElevation");
    if(weightLowElevation.is_initialized()){
        ui->doubleSpinBox_weightLowEl->setValue(*weightLowElevation);
    }
    boost::optional<double> lowElevationStartWeight = settings.get_optional<double>("settings.weightFactor.lowElevationStartWeight");
    if(lowElevationStartWeight.is_initialized()){
        ui->doubleSpinBox_weightLowElStart->setValue(*lowElevationStartWeight);
    }
    boost::optional<double> lowElevationFullWeight = settings.get_optional<double>("settings.weightFactor.lowElevationFullWeight");
    if(lowElevationFullWeight.is_initialized()){
        ui->doubleSpinBox_weightLowElEnd->setValue(*lowElevationFullWeight);
    }
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
    std::string cSource2 = settings.get<std::string>("settings.catalog_path.source2","");
    ui->lineEdit_browseSource2->setText(QString::fromStdString(cSource2));
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
    std::string outputScheduler = settings.get<std::string>("settings.output.scheduler","");
    ui->schedulerLineEdit->setText(QString::fromStdString(outputScheduler));
    std::string outputCorrelator = settings.get<std::string>("settings.output.correlator","");
    ui->correlatorLineEdit->setText(QString::fromStdString(outputCorrelator));

    std::string piName = settings.get<std::string>("settings.output.piName","");
    ui->lineEdit_PIName->setText(QString::fromStdString(piName));
    std::string piEmail = settings.get<std::string>("settings.output.piEmail","");
    ui->lineEdit_PIEmail->setText(QString::fromStdString(piEmail));
    std::string contactName = settings.get<std::string>("settings.output.contactName","");
    ui->lineEdit_contactName->setText(QString::fromStdString(contactName));
    std::string contactEmail = settings.get<std::string>("settings.output.contactEmail","");
    ui->lineEdit_contactEmail->setText(QString::fromStdString(contactEmail));

    std::string notes = settings.get<std::string>("settings.output.notes","");
    if(!notes.empty()){
        ui->plainTextEdit_notes->setPlainText(QString::fromStdString(notes).replace("\\n","\n"));
    }
    std::string operationNotes = settings.get<std::string>("settings.output.operationNotes","");
    ui->plainTextEdit_operationNotes->setPlainText(QString::fromStdString(operationNotes).replace("\\n","\n"));


    std::string threads = settings.get<std::string>("settings.multiCore.threads","auto");
    ui->comboBox_nThreads->setCurrentText(QString::fromStdString(threads));
    int nThreadsManual = settings.get<int>("settings.multiCore.nThreads",1);
    ui->spinBox_nCores->setValue(nThreadsManual);
    std::string jobScheduler = settings.get<std::string>("settings.multiCore.jobScheduling","auto");
    ui->comboBox_jobSchedule->setCurrentText(QString::fromStdString(jobScheduler));
    int chunkSize = settings.get<int>("settings.multiCore.chunkSize",0);
    ui->spinBox_chunkSize->setValue(chunkSize);

}

QString MainWindow::writeXML()
{
    para = VieVS::ParameterSettings();
    para.software(QApplication::applicationName().toStdString(), QApplication::applicationVersion().toStdString());

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::string name = ui->nameLineEdit->text().toStdString();
    if(name.empty()){
        name = "unknown";
    }
    std::string email = ui->emailLineEdit->text().toStdString();
    if(email.empty()){
        email = "unknown";
    }
    para.created(now, name, email);

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

    std::vector<std::string> srcNames;
    bool useSourcesFromParameter_otherwiseIgnore;
    for(int i=0; i<selectedSourceModel->rowCount(); ++i){
        srcNames.push_back(selectedSourceModel->item(i)->text().toStdString());
    }

    int selSrc = selectedSourceModel->rowCount();
    int allSrc = allSourceModel->rowCount();
    std::vector<std::string> ignoreSrcNames;
    if(allSrc-selSrc < allSrc/2){
        useSourcesFromParameter_otherwiseIgnore = false;
        for(int i=0; i<allSourceModel->rowCount(); ++i){
            std::string thisSrc = allSourceModel->item(i)->text().toStdString();
            if(std::find(srcNames.begin(),srcNames.end(),thisSrc) == srcNames.end() ){
                ignoreSrcNames.push_back(thisSrc);
            }
        }
    }else{
        useSourcesFromParameter_otherwiseIgnore = true;
    }
    bool fillinModeAPosteriori = ui->checkBox_fillinmode_aposteriori->isChecked();
    bool fillinModeDuringScan = ui->checkBox_fillinmode_duringscan->isChecked();
    bool fillinModeInfluence = ui->checkBox_fillinModeInfluence->isChecked();
    bool idleToObservingTime = ui->checkBox_idleToObservingTime->isChecked();
    bool subnetting = ui->groupBox_subnetting->isChecked();
    double subnettingAngle = ui->doubleSpinBox_subnettingDistance->value();

    double subnettingMinimumPercent = ui->doubleSpinBox_subnettingMinStations->value();
    double subnettingAllBut = ui->spinBox_subnetting_min_sta->value();

    bool useSubnettingPercent_otherwiseAllBut = false;
    double subnettingNumber = subnettingAllBut;

    if(ui->radioButton_subnetting_percent->isChecked()){
        useSubnettingPercent_otherwiseAllBut = true;
        subnettingNumber = subnettingMinimumPercent;
    }
    std::string logConsole = ui->comboBox_log_console->currentText().toStdString();
    std::string logFile = ui->comboBox_log_file->currentText().toStdString();
    std::string scanAlignment = "start";
    if(ui->radioButton_alignEnd->isChecked()){
        scanAlignment = "end";
    }else if(ui->radioButton_alignIndividual->isChecked()){
        scanAlignment = "individual";
    }

    if(useSourcesFromParameter_otherwiseIgnore){
        para.general(start, end, subnetting, subnettingAngle, useSubnettingPercent_otherwiseAllBut, subnettingNumber,
                     fillinModeInfluence, fillinModeDuringScan, fillinModeAPosteriori,
                     idleToObservingTime, station_names, useSourcesFromParameter_otherwiseIgnore,
                     srcNames, scanAlignment, logConsole, logFile);
    }else{
        para.general(start, end, subnetting, subnettingAngle, useSubnettingPercent_otherwiseAllBut, subnettingNumber,
                     fillinModeInfluence, fillinModeDuringScan, fillinModeAPosteriori,
                     idleToObservingTime, station_names, useSourcesFromParameter_otherwiseIgnore,
                     ignoreSrcNames, scanAlignment, logConsole, logFile);
    }


    std::string experimentName = ui->experimentNameLineEdit->text().toStdString();
    std::string experimentDescription = ui->lineEdit_experimentDescription->text().toStdString();
    std::string scheduler = ui->schedulerLineEdit->text().toStdString();
    std::string correlator = ui->correlatorLineEdit->text().toStdString();
    std::string piName = ui->lineEdit_PIName->text().toStdString();
    std::string piEmail = ui->lineEdit_PIEmail->text().toStdString();
    std::string contactName = ui->lineEdit_contactName->text().toStdString();
    std::string contactEmail = ui->lineEdit_contactEmail->text().toStdString();
    std::string notes = ui->plainTextEdit_notes->toPlainText().replace("\n","\\n").toStdString();
    bool statistics = ui->checkBox_outputStatisticsFile->isChecked();
    bool vex = ui->checkBox_outputVex->isChecked();
    bool ngs = ui->checkBox_outputNGSFile->isChecked();
    bool skd = ui->checkBox_outputSkdFile->isChecked();
    bool srcGrp = ui->checkBox_outputSourceGroupStatFile->isChecked();
    bool skyCov = ui->checkBox_outputSkyCoverageFile->isChecked();
    bool operNotes = ui->checkBox_outputOperationsNotes->isChecked();
    std::vector<std::string> srcGroupsForStatistic;
    for(int i=0; i<ui->treeWidget_srcGroupForStatistics->topLevelItemCount(); ++i){
        if(ui->treeWidget_srcGroupForStatistics->topLevelItem(i)->checkState(0) == Qt::Checked){
            srcGroupsForStatistic.push_back(ui->treeWidget_srcGroupForStatistics->topLevelItem(i)->text(0).toStdString());
        }
    }
    std::string operationNotes;
    if(operNotes){
        operationNotes = ui->plainTextEdit_operationNotes->toPlainText().replace("\n","\\n").toStdString();
    }
    para.output(experimentName, experimentDescription, scheduler, correlator, piName, piEmail, contactName,
                contactEmail, notes, statistics, ngs, skd, vex, operNotes, operationNotes, srcGrp, srcGroupsForStatistic, skyCov);

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
    std::string source;
    if(ui->radioButton_browseSource->isChecked()){
        source = ui->lineEdit_pathSource->text().toStdString();
    }else{
        source = ui->lineEdit_browseSource2->text().toStdString();
    }

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
        int fieldSystem = QString(itm->text(1).left(itm->text(1).count()-6)).toInt();
        int preob = QString(itm->text(2).left(itm->text(2).count()-6)).toInt();
        int midob = QString(itm->text(3).left(itm->text(3).count()-6)).toInt();
        int postob = QString(itm->text(4).left(itm->text(4).count()-6)).toInt();
        para.stationWaitTimes(name,fieldSystem,preob,midob,postob);
    }

    for(int i=0; i<ui->treeWidget_setupStationAxis->topLevelItemCount(); ++i){
        auto itm = ui->treeWidget_setupStationAxis->topLevelItem(i);
        std::string name = itm->text(0).toStdString();
        double ax1low = QString(itm->text(1).left(itm->text(1).count()-6)).toDouble();
        double ax1up = QString(itm->text(2).left(itm->text(1).count()-6)).toDouble();
        double ax2low = QString(itm->text(3).left(itm->text(1).count()-6)).toDouble();
        double ax2up = QString(itm->text(4).left(itm->text(1).count()-6)).toDouble();
        para.stationCableWrapBuffer(name,ax1low,ax1up,ax2low,ax2up);
    }

    double influenceDistance = ui->influenceDistanceDoubleSpinBox->value();
    double influenceTime = ui->influenceTimeSpinBox->value();
    double maxDistanceTwin = ui->maxDistanceForCombiningAntennasDoubleSpinBox->value();
    std::string interpolationDistance = ui->comboBox_skyCoverageDistanceType->currentText().toStdString();
    std::string interpolationTime = ui->comboBox_skyCoverageTimeType->currentText().toStdString();
    para.skyCoverage(influenceDistance,influenceTime,maxDistanceTwin, interpolationDistance, interpolationTime);

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
    double weightAverageBaselines = 0;
    if(ui->checkBox_weightAverageBaselines->isChecked()){
        weightAverageBaselines = ui->doubleSpinBox_weightAverageBaselines->value();
    }
    double weightIdleTime = 0;
    unsigned int intervalIdleTime = 0;
    if(ui->checkBox_weightIdleTime->isChecked()){
        weightIdleTime = ui->doubleSpinBox_weightIdleTime->value();
        intervalIdleTime = ui->spinBox_idleTimeInterval->value();
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
                      weightAverageStations, weightAverageBaselines, weightIdleTime, intervalIdleTime, weightDeclination,
                      weightDeclinationSlopeStart, weightDeclinationSlopeEnd,
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

    if(ui->treeWidget_conditions->topLevelItemCount()>0){
        std::vector<std::string> members;
        std::vector<int> minScans;
        std::vector<int> minBaselines;
        for(int i=0; i<ui->treeWidget_conditions->topLevelItemCount(); ++i){
            auto itm = ui->treeWidget_conditions->topLevelItem(i);
            members.push_back(itm->text(1).toStdString());
            minScans.push_back(itm->text(2).toInt());
            minBaselines.push_back(itm->text(3).toInt());
        }
        bool andForCombination;
        if(ui->comboBox_conditions_combinations->currentText() == "and"){
            andForCombination = true;
        }
        int minNumberOfReducedSources = ui->spinBox_minNumberOfReducedSources->value();
        int maxNumberOfIterations = ui->spinBox_maxNumberOfIterations->value();
        int numberOfGentleSourceReductions = ui->spinBox_gentleSourceReduction->value();
        para.conditions(members, minScans, minBaselines, andForCombination, maxNumberOfIterations, numberOfGentleSourceReductions, minNumberOfReducedSources);
    }

    if (ui->groupBox_multiScheduling->isChecked() && ui->treeWidget_multiSchedSelected->topLevelItemCount()>0){

        std::unordered_map<std::string, std::vector<std::string>> gsta;
        std::unordered_map<std::string, std::vector<std::string>> gsrc;
        std::unordered_map<std::string, std::vector<std::string>> gbl;
        for(const auto &any: groupSta){
            gsta[any.first] = any.second;
        }
        for(const auto &any: groupSrc){
            gsrc[any.first] = any.second;
        }
        for(const auto &any: groupBl){
            gbl[any.first] = any.second;
        }

        VieVS::MultiScheduling ms(gsta, gsrc, gbl);
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
            std::string member = itm->text(1).toStdString();
            QIcon memberIcon = itm->icon(0);
            QComboBox *list = qobject_cast<QComboBox*>(ui->treeWidget_multiSchedSelected->itemWidget(itm,3));

            QStringList parameter2dateTime {"session start"};

            QStringList parameter2toggle{"subnetting",
                                         "subnetting min participating stations",
                                         "subnetting min source angle",
                                         "fillinmode during scan selection",
                                         "fillinmode influence on scan selection",
                                         "fillinmode a posteriori"};

            QStringList parameter2double {"max slew time",
                                          "max wait time",
                                          "max scan time",
                                          "min scan time",
                                          "min number of stations",
                                          "min repeat time",
                                          "idle time interval",
                                          "max number of scans",
                                          "subnetting min source angle",
                                          "subnetting min participating stations",
                                          "sky coverage",
                                          "number of observations",
                                          "duration",
                                          "average stations",
                                          "average baselines",
                                          "average sources",
                                          "idle time",
                                          "low declination",
                                          "low declination begin",
                                          "low declination full",
                                          "low elevation",
                                          "low elevation begin",
                                          "low elevation full",
                                          "influence distance",
                                          "influence time",
                                          "weight",
                                          "min slew distance",
                                          "max slew distance",
                                          "min elevation",
                                          "min flux",
                                          "min sun distance"};


            std::vector<double> vecDouble;
            std::vector<unsigned int> vecInt;
            if(parameter2double.indexOf(parameter) != -1 ){
                for(int j = 0; j<list->count(); ++j){
                    vecDouble.push_back( QString(list->itemText(j)).toDouble());
                }
//            }else if(parameter2int.indexOf(parameter) != -1){
//                for(int j = 0; j<list->count(); ++j){
//                    vecInt.push_back( QString(list->itemText(j)).toInt());
//                }
            }

            if(parameterIcon.pixmap(16,16).toImage() == icSta.pixmap(16,16).toImage() || parameterIcon.pixmap(16,16).toImage() == icStaGrp.pixmap(16,16).toImage()){
                if(parameter == "weight"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max slew time"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min slew distance"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max slew distance"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max wait time"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min elevation"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max number of scans"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max scan time"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min scan time"){
                    ms.addParameters(std::string("station_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }

            }else if(parameterIcon.pixmap(16,16).toImage() == icSrc.pixmap(16,16).toImage() || parameterIcon.pixmap(16,16).toImage() == icSrcGrp.pixmap(16,16).toImage()){
                if(parameter == "weight"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min number of stations"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min flux"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max number of scans"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min elevation"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min sun distance"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max scan time"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min scan time"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min repeat time"){
                    ms.addParameters(std::string("source_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }

            }else if(parameterIcon.pixmap(16,16).toImage() == icBl.pixmap(16,16).toImage() || parameterIcon.pixmap(16,16).toImage() == icBlGrp.pixmap(16,16).toImage()){
                if(parameter == "weight"){
                    ms.addParameters(std::string("baseline_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "max scan time"){
                    ms.addParameters(std::string("baseline_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
                }else if(parameter == "min scan time"){
                    ms.addParameters(std::string("baseline_").append(parameter.replace(' ','_').toStdString()), member, vecDouble);
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
                    ms.addParameters(std::string("general_").append(parameter.replace(' ','_').toStdString()));
                }else if(parameter == "fillinmode during scan selection"){
                    ms.addParameters(std::string("general_").append(parameter.replace(' ','_').toStdString()));
                }else if(parameter == "fillinmode influence on scan selection"){
                    ms.addParameters(std::string("general_").append(parameter.replace(' ','_').toStdString()));
                }else if(parameter == "fillinmode a posteriori"){
                    ms.addParameters(std::string("general_").append(parameter.replace(' ','_').toStdString()));

                }else if(parameter == "subnetting min participating stations"){
                    ms.addParameters(std::string("general_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "subnetting min source angle"){
                    ms.addParameters(std::string("general_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "sky coverage"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "number of observations"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "duration"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "average stations"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "average baselines"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "average sources"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "idle time"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "low declination"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "low declination begin"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "low declination full"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "low elevation"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "low elevation begin"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "low elevation full"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "influence distance"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }else if(parameter == "influence time"){
                    ms.addParameters(std::string("weight_factor_").append(parameter.replace(' ','_').toStdString()), vecDouble);
                }
            }
        }

        std::string countNr = ui->comboBox_multiSched_maxNumber->currentText().toStdString();
        int n = ui->spinBox_multiSched_maxNumber->value();
        std::string seedStr = ui->comboBox_multiSched_seed->currentText().toStdString();
        int seed = ui->spinBox_multiSched_seed->value();
        para.multisched(ms.createPropertyTree(),countNr,n,seedStr,seed);

        std::string threads = ui->comboBox_nThreads->currentText().toStdString();
        int nThreadsManual = ui->spinBox_nCores->value();
        std::string jobScheduler = ui->comboBox_jobSchedule->currentText().toStdString();
        int chunkSize = ui->spinBox_chunkSize->value();
        para.multiCore(threads,nThreadsManual,jobScheduler,chunkSize);
    }

    if(ui->groupBox_highImpactAzEl->isChecked() && ui->treeWidget_highImpactAzEl->topLevelItemCount()>0){
        QTreeWidget * tree = ui->treeWidget_highImpactAzEl;
        std::vector<std::string> members;
        std::vector<double> azs;
        std::vector<double> els;
        std::vector<double> margins;
        for(int i=0; i<tree->topLevelItemCount(); ++i){
            QTreeWidgetItem *itm = tree->topLevelItem(i);
            members.push_back(itm->text(0).toStdString());
            azs.push_back(itm->text(1).toDouble());
            els.push_back(itm->text(2).toDouble());
            margins.push_back(itm->text(3).toDouble());
        }
        int interval = ui->spinBox_highImpactInterval->value();
        int repeat = ui->spinBox_highImpactMinRepeat->value();

        para.highImpactAzEl(members,azs,els,margins,interval,repeat);

    }

    QString path = ui->lineEdit_outputPath->text();
    path = path.simplified();
    path.replace("\\\\","/");
    path.replace("\\","/");
    if(!path.isEmpty() && path.right(1) != "/"){
        path.append("/");
    }

    QDir mainDir(path);
    if(!path.isEmpty() && !mainDir.exists() ){
        QDir().mkpath(path);
    }

    QString ename = QString::fromStdString(experimentName).trimmed();
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
        QDesktopServices::openUrl(QUrl(mydir.absolutePath()));
    }
    return path;
}

void MainWindow::loadXML(QString path)
{
    std::ifstream fid(path.toStdString());
    boost::property_tree::ptree xml;
    boost::property_tree::read_xml(fid,xml,boost::property_tree::xml_parser::trim_whitespace);
    QString warning;
    clearSetup(true,true,true);

    // read catalogs
    {
        std::string antenna = xml.get("master.catalogs.antenna","");
        if(!antenna.empty()){
            ui->lineEdit_pathAntenna->setText(QString::fromStdString(antenna));
        }
        std::string equip = xml.get("master.catalogs.equip","");
        if(!equip.empty()){
            ui->lineEdit_pathEquip->setText(QString::fromStdString(equip));
        }
        std::string position = xml.get("master.catalogs.position","");
        if(!position.empty()){
            ui->lineEdit_pathPosition->setText(QString::fromStdString(position));
        }
        std::string mask = xml.get("master.catalogs.mask","");
        if(!mask.empty()){
            ui->lineEdit_pathMask->setText(QString::fromStdString(mask));
        }
        on_pushButton_stations_clicked();

        std::string source = xml.get("master.catalogs.source","");
        if(!source.empty()){
            ui->lineEdit_pathSource->setText(QString::fromStdString(source));
        }
        ui->radioButton_browseSource->setChecked(true);
        std::string flux = xml.get("master.catalogs.flux","");
        if(!flux.empty()){
            ui->lineEdit_pathFlux->setText(QString::fromStdString(flux));
        }
        on_pushButton_reloadsources_clicked();

        std::string freq = xml.get("master.catalogs.freq","");
        if(!freq.empty()){
            ui->lineEdit_pathFreq->setText(QString::fromStdString(freq));
        }
        std::string hdpos = xml.get("master.catalogs.hdpos","");
        if(!hdpos.empty()){
            ui->lineEdit_pathHdpos->setText(QString::fromStdString(hdpos));
        }
        std::string loif = xml.get("master.catalogs.loif","");
        if(!loif.empty()){
            ui->lineEdit_pathLoif->setText(QString::fromStdString(loif));
        }
        std::string modes = xml.get("master.catalogs.modes","");
        if(!modes.empty()){
            ui->lineEdit_pathModes->setText(QString::fromStdString(modes));
        }
        std::string rec = xml.get("master.catalogs.rec","");
        if(!rec.empty()){
            ui->lineEdit_pathRec->setText(QString::fromStdString(rec));
        }
        std::string rx = xml.get("master.catalogs.rx","");
        if(!rx.empty()){
            ui->lineEdit_pathRx->setText(QString::fromStdString(rx));
        }
        std::string tracks = xml.get("master.catalogs.tracks","");
        if(!tracks.empty()){
            ui->lineEdit_pathTracks->setText(QString::fromStdString(tracks));
        }
        on_pushButton_reloadcatalogs_clicked();
    }


    // general
    {
        std::string startTimeStr = xml.get("master.general.startTime","2018.01.01 00:00:00");
        QDateTime startTime = QDateTime::fromString(QString::fromStdString(startTimeStr),"yyyy.MM.dd HH:mm:ss");
        ui->dateTimeEdit_sessionStart->setDateTime(startTime);

        std::string endTimeStr = xml.get("master.general.endTime","2018.01.02 00:00:00");
        QDateTime endTime   = QDateTime::fromString(QString::fromStdString(endTimeStr),"yyyy.MM.dd HH:mm:ss");
        double dur = startTime.secsTo(endTime)/3600.0;
        ui->doubleSpinBox_sessionDuration->setValue(dur);

        double subnettingMinAngle = xml.get("master.general.subnettingMinAngle",150.0);
        ui->doubleSpinBox_subnettingDistance->setValue(subnettingMinAngle);
        bool subnettingFlag = false;
        if(xml.get_optional<double>("master.general.subnettingMinNStaPercent").is_initialized()){
            subnettingFlag = true;
        }
        double subnettingMinNStaPercent = xml.get("master.general.subnettingMinNStaPercent",80);
        ui->doubleSpinBox_subnettingMinStations->setValue(subnettingMinNStaPercent);
        double subnettingMinNStaAllBut = xml.get("master.general.subnettingMinNStaAllBut",1);
        ui->spinBox_subnetting_min_sta->setValue(subnettingMinNStaAllBut);
        if(subnettingFlag){
            ui->radioButton_subnetting_percent->setChecked(true);
        }else{
            ui->radioButton_subnetting_allBut->setChecked(true);
        }
        bool subnetting = xml.get("master.general.subnetting",false);
        ui->groupBox_subnetting->setChecked(subnetting);

        bool fillinmodeAPosteriori = xml.get("master.general.fillinmodeAPosteriori",false);
        ui->checkBox_fillinmode_aposteriori->setChecked(fillinmodeAPosteriori);
        bool fillinmodeDuringScan = xml.get("master.general.fillinmodeDuringScan",false);
        ui->checkBox_fillinmode_duringscan->setChecked(fillinmodeDuringScan);
        bool fillinmodeInfluenceOnSchedule = xml.get("master.general.fillinmodeInfluenceOnSchedule",false);
        ui->checkBox_fillinmode_duringscan->setChecked(fillinmodeInfluenceOnSchedule);

        bool idleToObservingTime = xml.get("master.general.idleToObservingTime",false);
        ui->checkBox_idleToObservingTime->setChecked(idleToObservingTime);

        std::vector<std::string> sel_stations;
        const auto &stations = xml.get_child_optional("master.general.stations");
        if(stations.is_initialized()){
            auto it = stations->begin();
            while (it != stations->end()) {
                auto item = it->second.data();
                sel_stations.push_back(item);
                ++it;
            }
            for(const auto &station : sel_stations){
                bool found = false;
                for(int i=0; i<allStationProxyModel->rowCount(); ++i){
                    if(allStationProxyModel->index(i,0).data().toString() == QString::fromStdString(station)){
                        on_treeView_allAvailabeStations_clicked(allStationProxyModel->index(i,0));
                        found = true;
                        break;
                    }
                }
                if(!found){
                    warning.append("Station "+QString::fromStdString(station)+" not found!");
                }
            }
        }


        std::vector<std::string> sel_sources;
        const auto &ptree_useSources = xml.get_child_optional("master.general.onlyUseListedSources");
        if(ptree_useSources.is_initialized()){
            auto it = ptree_useSources->begin();
            while (it != ptree_useSources->end()) {
                auto item = it->second.data();
                sel_sources.push_back(item);
                ++it;
            }
        }
        for(const auto &source : sel_sources){
            bool found = false;
            for(int i=0; i<allSourceProxyModel->rowCount(); ++i){
                if(allSourceProxyModel->index(i,0).data().toString() == QString::fromStdString(source)){
                    on_treeView_allAvailabeSources_clicked(allSourceProxyModel->index(i,0));
                    found = true;
                    break;
                }
            }
            if(!found){
                warning.append("Source "+QString::fromStdString(source)+" not found!");
            }
        }

        std::vector<std::string> ignore_sources;
        const auto &ptree_ignoreSources = xml.get_child_optional("master.general.ignoreListedSources");
        if(ptree_ignoreSources.is_initialized()){
            auto it = ptree_ignoreSources->begin();
            while (it != ptree_ignoreSources->end()) {
                auto item = it->second.data();
                ignore_sources.push_back(item);
                ++it;
            }
        }

        std::string scanAlignment = xml.get("master.general.scanAlignment","start");
        if(scanAlignment == "start"){
            ui->radioButton_alignStart->setChecked(true);
        }else if(scanAlignment == "end"){
            ui->radioButton_alignEnd->setChecked(true);
        }else if(scanAlignment == "individual"){
            ui->radioButton_alignIndividual->setChecked(true);
        }

        std::string logSeverityConsole = xml.get("master.general.logSeverityConsole","info");
        if(logSeverityConsole == "trace"){
            ui->comboBox_log_console->setCurrentIndex(0);
        }else if(logSeverityConsole == "debug"){
            ui->comboBox_log_console->setCurrentIndex(1);
        }else if(logSeverityConsole == "info"){
            ui->comboBox_log_console->setCurrentIndex(2);
        }else if(logSeverityConsole == "warning"){
            ui->comboBox_log_console->setCurrentIndex(3);
        }else if(logSeverityConsole == "error"){
            ui->comboBox_log_console->setCurrentIndex(4);
        }else if(logSeverityConsole == "fatal"){
            ui->comboBox_log_console->setCurrentIndex(5);
        }

        std::string logSeverityFile = xml.get("master.general.logSeverityFile","info");
        if(logSeverityFile == "trace"){
            ui->comboBox_log_file->setCurrentIndex(0);
        }else if(logSeverityFile == "debug"){
            ui->comboBox_log_file->setCurrentIndex(1);
        }else if(logSeverityFile == "info"){
            ui->comboBox_log_file->setCurrentIndex(2);
        }else if(logSeverityFile == "warning"){
            ui->comboBox_log_file->setCurrentIndex(3);
        }else if(logSeverityFile == "error"){
            ui->comboBox_log_file->setCurrentIndex(4);
        }else if(logSeverityFile == "fatal"){
            ui->comboBox_log_file->setCurrentIndex(5);
        }
    }

    // groups
    {
        groupSta.clear();
        auto groupTree = xml.get_child_optional("master.station.groups");
        if(groupTree.is_initialized()){
            for (auto &it: *groupTree) {
                std::string name = it.first;
                if (name == "group") {
                    std::string groupName = it.second.get_child("<xmlattr>.name").data();
                    std::vector<std::string> members;
                    for (auto &it2: it.second) {
                        if (it2.first == "member") {
                            members.push_back(it2.second.data());
                        }
                    }

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
                        if(txt>QString::fromStdString(groupName)){
                            break;
                        }else{
                            ++r;
                        }
                    }

                    groupSta[groupName] = members;
                    allStationPlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/station_group_2.png"),
                                                                            QString::fromStdString(groupName) ));
                }
            }
        }
    }
    {
        groupSrc.clear();
        ui->treeWidget_srcGroupForStatistics->clear();
        auto groupTree = xml.get_child_optional("master.source.groups");
        if(groupTree.is_initialized()){
            for (auto &it: *groupTree) {
                std::string name = it.first;
                if (name == "group") {
                    std::string groupName = it.second.get_child("<xmlattr>.name").data();
                    std::vector<std::string> members;
                    for (auto &it2: it.second) {
                        if (it2.first == "member") {
                            members.push_back(it2.second.data());
                        }
                    }

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
                        if(txt>QString::fromStdString(groupName)){
                            break;
                        }else{
                            ++r;
                        }
                    }

                    groupSrc[groupName] = members;
                    allSourcePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/source_group.png"),
                                                                            QString::fromStdString(groupName) ));
                    QTreeWidgetItem *itm = new QTreeWidgetItem();
                    itm->setText(0,QString::fromStdString(groupName));
                    itm->setCheckState(0,Qt::Unchecked);
                    ui->treeWidget_srcGroupForStatistics->addTopLevelItem(itm);

                }
            }
        }
    }
    {
        groupBl.clear();
        auto groupTree = xml.get_child_optional("master.baseline.groups");
        if(groupTree.is_initialized()){
            for (auto &it: *groupTree) {
                std::string name = it.first;
                if (name == "group") {
                    std::string groupName = it.second.get_child("<xmlattr>.name").data();
                    std::vector<std::string> members;
                    for (auto &it2: it.second) {
                        if (it2.first == "member") {
                            members.push_back(it2.second.data());
                        }
                    }

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
                        if(txt>QString::fromStdString(groupName)){
                            break;
                        }else{
                            ++r;
                        }
                    }

                    groupBl[groupName] = members;
                    allBaselinePlusGroupModel->insertRow(r,new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),
                                                                            QString::fromStdString(groupName) ));
                }
            }
        }
    }
    
    //parameters
    {
        paraSta.clear();
        ui->ComboBox_parameterStation->clear();
        const auto &para_tree = xml.get_child("master.station.parameters");
        for (auto &it: para_tree) {
            std::string name = it.first;
            if (name == "parameter") {

                VieVS::ParameterSettings::ParametersStations PARA;
                auto PARA_ = VieVS::ParameterSettings::ptree2parameterStation(it.second);
                PARA = PARA_.second;
                paraSta[PARA_.first] = PARA;
                ui->ComboBox_parameterStation->addItem(QString::fromStdString(PARA_.first));

            }
        }
    }
    {   
        paraSrc.clear();
        ui->ComboBox_parameterSource->clear();
        const auto &para_tree = xml.get_child("master.source.parameters");
        for (auto &it: para_tree) {
            std::string name = it.first;
            if (name == "parameter") {

                VieVS::ParameterSettings::ParametersSources PARA;
                auto PARA_ = VieVS::ParameterSettings::ptree2parameterSource(it.second);
                PARA = PARA_.second;
                paraSrc[PARA_.first] = PARA;
                ui->ComboBox_parameterSource->addItem(QString::fromStdString(PARA_.first));

            }
        }
    }
    {   
        paraBl.clear();
        ui->ComboBox_parameterBaseline->clear();
        const auto &para_tree = xml.get_child("master.baseline.parameters");
        for (auto &it: para_tree) {
            std::string name = it.first;
            if (name == "parameter") {

                VieVS::ParameterSettings::ParametersBaselines PARA;
                auto PARA_ = VieVS::ParameterSettings::ptree2parameterBaseline(it.second);
                PARA = PARA_.second;
                paraBl[PARA_.first] = PARA;
                ui->ComboBox_parameterBaseline->addItem(QString::fromStdString(PARA_.first));

            }
        }
    }
    
    //setup
    {
        auto ctree = xml.get_child("master.station.setup");
        for(const auto &any: ctree){
            if(any.first == "setup"){
                ui->treeWidget_setupStation->topLevelItem(0)->child(0)->setSelected(true);
                addSetup(ui->treeWidget_setupStation, any.second, ui->comboBox_stationSettingMember,
                         ui->ComboBox_parameterStation, ui->DateTimeEdit_startParameterStation,
                         ui->DateTimeEdit_endParameterStation, ui->comboBox_parameterStationTransition,
                         ui->pushButton_3);
            }
        }
    }
    {
        auto ctree = xml.get_child("master.source.setup");
        for(const auto &any: ctree){
            if(any.first == "setup"){
                ui->treeWidget_setupSource->topLevelItem(0)->child(0)->setSelected(true);
                addSetup(ui->treeWidget_setupSource, any.second, ui->comboBox_sourceSettingMember,
                         ui->ComboBox_parameterSource, ui->DateTimeEdit_startParameterSource,
                         ui->DateTimeEdit_endParameterSource, ui->comboBox_parameterSourceTransition,
                         ui->pushButton_addSetupSource);
            }
        }
    }
    {
        auto ctree = xml.get_child("master.baseline.setup");
        for(const auto &any: ctree){
            if(any.first == "setup"){
                ui->treeWidget_setupBaseline->topLevelItem(0)->child(0)->setSelected(true);
                addSetup(ui->treeWidget_setupBaseline, any.second, ui->comboBox_baselineSettingMember,
                         ui->ComboBox_parameterBaseline, ui->DateTimeEdit_startParameterBaseline,
                         ui->DateTimeEdit_endParameterBaseline, ui->comboBox_parameterBaselineTransition,
                         ui->pushButton_addSetupBaseline);
            }
        }
    }

    //wait times
    {
        auto waitTime_tree = xml.get_child("master.station.waitTimes");
        ui->treeWidget_setupStationWait->clear();
        for (auto &it: waitTime_tree) {
            std::string name = it.first;
            if (name == "waitTime") {
                std::string memberName = it.second.get_child("<xmlattr>.member").data();

                int fieldSystem = it.second.get<int>("fieldSystem");
                int preob = it.second.get<int>("preob");
                int midob = it.second.get<int>("midob");
                int postob = it.second.get<int>("postob");
                
                ui->comboBox_stationSettingMember_wait->setCurrentText(QString::fromStdString(memberName));
                ui->SpinBox_fieldSystem->setValue(fieldSystem);
                ui->SpinBox_preob->setValue(preob);
                ui->SpinBox_midob->setValue(midob);
                ui->SpinBox_postob->setValue(postob);
                
                ui->pushButton_setupWaitAdd->click();
            }
        }
    }
    // cable wrap buffer
    {
        auto waitTime_tree = xml.get_child("master.station.cableWrapBuffers");
        ui->treeWidget_setupStationAxis->clear();
        for (auto &it: waitTime_tree) {
            std::string name = it.first;
            if (name == "cableWrapBuffer") {
                std::string memberName = it.second.get_child("<xmlattr>.member").data();

                auto axis1Low = it.second.get<double>("axis1LowOffset");
                auto axis1Up = it.second.get<double>("axis1UpOffset");
                auto axis2Low = it.second.get<double>("axis2LowOffset");
                auto axis2Up = it.second.get<double>("axis2UpOffset");
                
                ui->comboBox_stationSettingMember_axis->setCurrentText(QString::fromStdString(memberName));
                ui->DoubleSpinBox_axis1low->setValue(axis1Low);
                ui->DoubleSpinBox_axis1up->setValue(axis1Up);
                ui->DoubleSpinBox_axis2low->setValue(axis2Low);
                ui->DoubleSpinBox_axis2up->setValue(axis2Up);
                
                ui->pushButton_setupAxisAdd->click();
            }
        }
    }

    // sky coverage
    {
        double influenceDistance = xml.get("master.skyCoverage.influenceDistance",30.0);
        ui->influenceDistanceDoubleSpinBox->setValue(influenceDistance);
        int influenceInterval = xml.get("master.skyCoverage.influenceInterval",3600);
        ui->influenceTimeSpinBox->setValue(influenceInterval);
        double maxTwinTelecopeDistance = xml.get("master.skyCoverage.maxTwinTelecopeDistance",0.0);
        ui->maxDistanceForCombiningAntennasDoubleSpinBox->setValue(maxTwinTelecopeDistance);
        std::string interpolationDistance = xml.get("master.skyCoverage.interpolationDistance","cosine");
        if(interpolationDistance == "cosine"){
            ui->comboBox_skyCoverageDistanceType->setCurrentIndex(0);
        }else if(interpolationDistance == "linear"){
            ui->comboBox_skyCoverageDistanceType->setCurrentIndex(1);
        }else if(interpolationDistance == "constant"){
            ui->comboBox_skyCoverageDistanceType->setCurrentIndex(2);
        }
        std::string interpolationTime = xml.get("master.skyCoverage.interpolationTime","cosine");
        if(interpolationTime == "cosine"){
            ui->comboBox_skyCoverageTimeType->setCurrentIndex(0);
        }else if(interpolationTime == "linear"){
            ui->comboBox_skyCoverageTimeType->setCurrentIndex(1);
        }else if(interpolationTime == "constant"){
            ui->comboBox_skyCoverageTimeType->setCurrentIndex(2);
        }
    }

    //weight factors
    {
        double weightFactor_skyCoverage = xml.get("master.weightFactor.skyCoverage",0.0);
        if(weightFactor_skyCoverage == 0){
            ui->checkBox_weightCoverage->setChecked(false);
        }else{
            ui->checkBox_weightCoverage->setChecked(true);
            ui->doubleSpinBox_weightSkyCoverage->setValue(weightFactor_skyCoverage);
        }
        double weightFactor_numberOfObservations = xml.get("master.weightFactor.numberOfObservations",0.0);
        if(weightFactor_numberOfObservations == 0){
            ui->checkBox_weightNobs->setChecked(false);
        }else{
            ui->checkBox_weightNobs->setChecked(true);
            ui->doubleSpinBox_weightNumberOfObservations->setValue(weightFactor_numberOfObservations);
        }
        double weightFactor_duration = xml.get("master.weightFactor.duration",0.0);
        if(weightFactor_duration == 0){
            ui->checkBox_weightDuration->setChecked(false);
        }else{
            ui->checkBox_weightDuration->setChecked(true);
            ui->doubleSpinBox_weightDuration->setValue(weightFactor_duration);
        }
        double weightFactor_averageSources = xml.get("master.weightFactor.averageSources",0.0);
        if(weightFactor_averageSources == 0){
            ui->checkBox_weightAverageSources->setChecked(false);
        }else{
            ui->checkBox_weightAverageSources->setChecked(true);
            ui->doubleSpinBox_weightAverageSources->setValue(weightFactor_averageSources);
        }
        double weightFactor_averageStations = xml.get("master.weightFactor.averageStations",0.0);
        if(weightFactor_averageStations == 0){
            ui->checkBox_weightAverageStations->setChecked(false);
        }else{
            ui->checkBox_weightAverageStations->setChecked(true);
            ui->doubleSpinBox_weightAverageStations->setValue(weightFactor_averageStations);
        }
        double weightFactor_averageBaselines = xml.get("master.weightFactor.averageBaselines",0.0);
        if(weightFactor_averageBaselines == 0){
            ui->checkBox_weightAverageBaselines->setChecked(false);
        }else{
            ui->checkBox_weightAverageBaselines->setChecked(true);
            ui->doubleSpinBox_weightAverageBaselines->setValue(weightFactor_averageBaselines);
        }
        double weightFactor_idleTime = xml.get("master.weightFactor.idleTime",0.0);
        int weightFactor_idleTimeInterval = xml.get("master.weightFactor.idleTimeInterval",600);
        if(weightFactor_idleTime == 0){
            ui->checkBox_weightIdleTime->setChecked(false);
        }else{
            ui->checkBox_weightIdleTime->setChecked(true);
            ui->doubleSpinBox_weightIdleTime->setValue(weightFactor_idleTime);
            ui->spinBox_idleTimeInterval->setValue(weightFactor_idleTimeInterval);
        }
        double weightFactor_weightDeclination = xml.get("master.weightFactor.weightDeclination",0.0);
        double weightFactor_declinationStartWeight = xml.get("master.weightFactor.declinationStartWeight",0.0);
        double weightFactor_declinationFullWeight = xml.get("master.weightFactor.declinationFullWeight",0.0);
        if(weightFactor_weightDeclination == 0){
            ui->checkBox_weightLowDeclination->setChecked(false);
        }else{
            ui->checkBox_weightLowDeclination->setChecked(true);
            ui->doubleSpinBox_weightLowDec->setValue(weightFactor_weightDeclination);
            ui->doubleSpinBox_weightLowDecStart->setValue(weightFactor_declinationStartWeight);
            ui->doubleSpinBox_weightLowDecEnd->setValue(weightFactor_declinationFullWeight);
        }
        double weightFactor_weightLowElevation = xml.get("master.weightFactor.weightLowElevation",0.0);
        double weightFactor_lowElevationStartWeight = xml.get("master.weightFactor.lowElevationStartWeight",0.0);
        double weightFactor_lowElevationFullWeight = xml.get("master.weightFactor.lowElevationFullWeight",0.0);
        if(weightFactor_weightLowElevation == 0){
            ui->checkBox_weightLowElevation->setChecked(false);
        }else{
            ui->checkBox_weightLowElevation->setChecked(true);
            ui->doubleSpinBox_weightLowEl->setValue(weightFactor_weightLowElevation);
            ui->doubleSpinBox_weightLowElStart->setValue(weightFactor_lowElevationStartWeight);
            ui->doubleSpinBox_weightLowElEnd->setValue(weightFactor_lowElevationFullWeight);
        }
    }
    
    //conditions
    boost::optional<boost::property_tree::ptree &> ctree = xml.get_child_optional("master.optimization");
    if (ctree.is_initialized()) {

        boost::property_tree::ptree PARA_source = xml.get_child("master.source");
        ui->checkBox_gentleSourceReduction->setChecked(false);

        for(const auto &any: *ctree){
            if(any.first == "combination"){
                if(any.second.get_value<std::string>() == "and"){
                    ui->comboBox_conditions_combinations->setCurrentText("and");
                }else{
                    ui->comboBox_conditions_combinations->setCurrentText("or");
                }
            }else if(any.first == "maxNumberOfIterations"){
                ui->spinBox_maxNumberOfIterations->setValue(any.second.get_value<int>());
            }else if(any.first == "numberOfGentleSourceReductions"){
                ui->spinBox_gentleSourceReduction->setValue(any.second.get_value<unsigned int>());
                ui->checkBox_gentleSourceReduction->setChecked(true);
            }else if(any.first == "minNumberOfSourcesToReduce"){
                ui->spinBox_minNumberOfReducedSources->setValue(any.second.get_value<unsigned int>());
            }else if(any.first == "condition"){
                std::string member = any.second.get<std::string>("members");
                auto scans = any.second.get<unsigned int>("minScans");
                auto bls = any.second.get<unsigned int>("minBaselines");

                ui->comboBox_conditions_members->setCurrentText(QString::fromStdString(member));
                ui->spinBox_condtionsMinNumScans->setValue(scans);
                ui->spinBox_conditionsMinNumBaselines->setValue(bls);
                
                ui->pushButton_addCondition->click();
            }
        }
    }
    
    //mode
    {
        ui->groupBox_modeSked->setChecked(false);
        ui->groupBox_modeCustom->setChecked(false);
        while(ui->tableWidget_modeCustonBand->rowCount() >0){
            ui->tableWidget_modeCustonBand->removeRow(0);
        }
        ui->tableWidget_modeCustonBand->setRowCount(0);

        while(ui->tableWidget_ModesPolicy->rowCount() >0){
            ui->tableWidget_ModesPolicy->removeRow(0);
        }
        ui->tableWidget_ModesPolicy->setRowCount(0);

        if(xml.get_optional<std::string>("master.mode.skdMode").is_initialized()){
            QString mode = QString::fromStdString(xml.get<std::string>("master.mode.skdMode"));
            ui->groupBox_modeSked->setChecked(true);
            ui->comboBox_skedObsModes->setCurrentText(mode);
            addModesCustomTable("X",8.590,10);
            addModesCustomTable("S",2.260,6);
        }
        if(xml.get_optional<double>("master.mode.sampleRate").is_initialized()){
            ui->sampleRateDoubleSpinBox->setValue(xml.get<double>("master.mode.sampleRate"));
            ui->sampleBitsSpinBox->setValue(xml.get<double>("master.mode.bits"));
            ui->groupBox_modeCustom->setChecked(true);
            boost::property_tree::ptree & bands = xml.get_child("master.mode.bands");
            for(const auto &band:bands){
                if(band.first == "band"){
                    double wavelength = band.second.get<double>("wavelength");
                    int channels = band.second.get<int>("chanels");
                    QString name = QString::fromStdString(band.second.get<std::string>("<xmlattr>.name"));

                    double freq = 1/(wavelength*1e9/299792458.);

                    addModesCustomTable(name,freq,channels);
                }
            }
        }
    }

    //mode_bandPolicy
    {
        boost::optional<boost::property_tree::ptree &> ctree = xml.get_child_optional("master.mode.bandPolicies");
        if(ctree.is_initialized()){
            for(const auto & any:*ctree){
                if(any.first == "bandPolicy"){
                    std::string name = any.second.get<std::string>("<xmlattr>.name");
                    std::string staReq = any.second.get("station.tag","required");
                    std::string srcReq = any.second.get("source.tag","required");
                    double minSNR = any.second.get<double>("minSNR");
                    std::string staBackup;
                    std::string srcBackup;
                    double stationBackupValue;
                    double sourceBackupValue;
                    if(any.second.get_optional<double>("station.backup_maxValueTimes").is_initialized()){
                        staBackup = "max value Times";
                        stationBackupValue = any.second.get<double>("station.backup_maxValueTimes");
                    } else if(any.second.get_optional<double>("station.backup_minValueTimes").is_initialized()){
                        staBackup = "min value Times";
                        stationBackupValue = any.second.get<double>("station.backup_minValueTimes");
                    } else if(any.second.get_optional<double>("station.backup_value").is_initialized()){
                        staBackup = "value";
                        stationBackupValue = any.second.get<double>("station.backup_value");
                    } else {
                        staBackup = "none";
                        stationBackupValue = 0;
                    }
                    if(any.second.get_optional<double>("source.backup_maxValueTimes").is_initialized()){
                        srcBackup = "max value Times";
                        sourceBackupValue = any.second.get<double>("source.backup_maxValueTimes");
                    } else if(any.second.get_optional<double>("source.backup_minValueTimes").is_initialized()){
                        srcBackup = "min value Times";
                        sourceBackupValue = any.second.get<double>("source.backup_minValueTimes");
                    } else if(any.second.get_optional<double>("source.backup_value").is_initialized()){
                        srcBackup = "value";
                        sourceBackupValue = any.second.get<double>("source.backup_value");
                    } else {
                        srcBackup = "none";
                        sourceBackupValue = 0;
                    }


                    auto t = ui->tableWidget_ModesPolicy;
                    for(int i=0; i<t->rowCount(); ++i){

                        std::string tname = t->verticalHeaderItem(i)->text().toStdString();
                        if(tname != name){
                            continue;
                        }

                        qobject_cast<QDoubleSpinBox*>(t->cellWidget(i,0))->setValue(minSNR);
                        qobject_cast<QComboBox*>(t->cellWidget(i,1))->setCurrentText(QString::fromStdString(staReq));
                        qobject_cast<QComboBox*>(t->cellWidget(i,4))->setCurrentText(QString::fromStdString(srcReq));
                        qobject_cast<QComboBox*>(t->cellWidget(i,2))->setCurrentText(QString::fromStdString(staBackup));
                        qobject_cast<QComboBox*>(t->cellWidget(i,5))->setCurrentText(QString::fromStdString(srcBackup));
                        qobject_cast<QDoubleSpinBox*>(t->cellWidget(i,3))->setValue(stationBackupValue);
                        qobject_cast<QDoubleSpinBox*>(t->cellWidget(i,6))->setValue(sourceBackupValue);
                        break;
                    }

                }
            }
        }

    }

    //multisched
    {
        auto twmss = ui->treeWidget_multiSchedSelected;
        auto twms = ui->treeWidget_multiSched;
        ui->groupBox_multiScheduling->setChecked(false);
        twmss->clear();
        for(int i=0; i<3; ++i){
            for(int j=0; j<twms->topLevelItem(i)->childCount(); ++j){
                twms->topLevelItem(i)->child(j)->setDisabled(false);
            }
        }


        boost::optional<boost::property_tree::ptree &> ctree_o = xml.get_child_optional("master.multisched");
        if(ctree_o.is_initialized()){
            const boost::property_tree::ptree &ctree = ctree_o.get();
            ui->groupBox_multiScheduling->setChecked(true);
            ui->comboBox_multiSched_maxNumber->setCurrentText("all");
            ui->comboBox_multiSched_seed->setCurrentText("random");

            if(ctree.get_optional<int>("maxNumber").is_initialized()){
                ui->comboBox_multiSched_maxNumber->setCurrentText("select");
                ui->spinBox_multiSched_maxNumber->setValue(ctree.get<int>("maxNumber"));
            }
            if(ctree.get_optional<int>("seed").is_initialized()){
                ui->comboBox_multiSched_seed->setCurrentText("select");
                ui->spinBox_multiSched_seed->setValue(ctree.get<int>("seed"));
            }


            for(const auto &any: ctree){
                QString name = QString::fromStdString(any.first);
                if(name == "maxNumber" || name == "seed"){
                    continue;
                }
                QString parameterName;
                bool hasMember = false;
                QString member = "global";

                if(name.left(8) == "station_"){
                    name = name.mid(8);
                    hasMember = true;
                    parameterName = "Station";
                }else if(name.left(7) == "source_"){
                    name = name.mid(7);
                    hasMember = true;
                    parameterName = "Source";
                }else if(name.left(9) == "baseline_"){
                    name = name.mid(9);
                    hasMember = true;
                    parameterName = "Baseline";
                }else if(name.left(14) == "weight_factor_"){
                    name = name.mid(14);
                    hasMember = false;
                    parameterName = "Weight factor";
                }else if(name.left(8) == "general_"){
                    name = name.mid(8);
                    hasMember = false;
                    parameterName = "General";
                }else if(name.left(13) == "Sky_Coverage_"){
                    name = name.mid(13);
                    hasMember = false;
                    parameterName = "Sky Coverage";
                }
                name.replace("_"," ");
                for(int i=0; i<3; ++i){
                    for(int j=0; j<twms->topLevelItem(i)->childCount(); ++j){
                        if(twms->topLevelItem(i)->child(j)->text(0) == name){
                            parameterName = twms->topLevelItem(i)->text(0);
                            break;
                        }
                    }
                }



                if(hasMember){
                    member = QString::fromStdString(any.second.get<std::string>("<xmlattr>.member"));
                }
                QVector<double> values;
                if(name != "general subnetting" && name != "general fillinmode during scan selection" &&
                        name != "general fillinmode influence on scan selection" && name != "general fillinmode a posteriori" ){
                    for(const auto &any2 : any.second){
                        if(any2.first == "value"){
                            values.push_back(any2.second.get_value<double>());
                        }
                    }
                }

                if(parameterName == "General"){
                    for(int i=0; i<twms->topLevelItem(0)->childCount(); ++i){
                        if(name == twms->topLevelItem(0)->child(i)->text(0)){
                            twms->topLevelItem(0)->child(i)->setDisabled(true);
                            break;
                        }
                    }
                }else if(parameterName == "Weight factor"){
                    for(int i=0; i<twms->topLevelItem(1)->childCount(); ++i){
                        if(name == twms->topLevelItem(1)->child(i)->text(0)){
                            twms->topLevelItem(1)->child(i)->setDisabled(true);
                            break;
                        }
                    }
                }else if(parameterName == "Sky Coverage"){
                    for(int i=0; i<twms->topLevelItem(2)->childCount(); ++i){
                        if(name == twms->topLevelItem(2)->child(i)->text(0)){
                            twms->topLevelItem(2)->child(i)->setDisabled(true);
                            break;
                        }
                    }
                }

                QIcon ic1;
                QIcon ic2;
                if(parameterName == "General"){
                    ic1 = QIcon(":/icons/icons/applications-internet-2.png");
                    ic2 = QIcon(":/icons/icons/applications-internet-2.png");
                }else if(parameterName == "Weight factor"){
                    ic1 = QIcon(":/icons/icons/weight.png");
                    ic2 = QIcon(":/icons/icons/applications-internet-2.png");
                }else if(parameterName == "Sky Coverage"){
                    ic1 = QIcon(":/icons/icons/sky_coverage.png");
                    ic2 = QIcon(":/icons/icons/sky_coverage.png");
                }else if(parameterName == "Station"){
                    ic1 = QIcon(":/icons/icons/station.png");
                    if(member == "__all__" || groupSta.find(member.toStdString()) != groupSta.end()){
                        ic2 = QIcon(":/icons/icons/station_group_2.png");
                    }else{
                        ic2 = QIcon(":/icons/icons/station.png");
                    }
                }else if(parameterName == "Source"){
                    ic1 = QIcon(":/icons/icons/source.png");
                    if(member == "__all__" || groupSrc.find(member.toStdString()) == groupSrc.end()){
                        ic2 = QIcon(":/icons/icons/source_group.png");
                    }else{
                        ic2 = QIcon(":/icons/icons/source.png");
                    }

                }else if(parameterName == "Baseline"){
                    ic1 = QIcon(":/icons/icons/baseline.png");
                    if(member == "__all__" || groupBl.find(member.toStdString()) == groupBl.end()){
                        ic2 = QIcon(":/icons/icons/baseline.png");
                    }else{
                        ic2 = QIcon(":/icons/icons/baseline_group.png");
                    }

                }

                QTreeWidgetItem *itm = new QTreeWidgetItem();
                itm->setText(0,name);
                itm->setText(1,member);
                itm->setIcon(0,ic1);
                itm->setIcon(1,ic2);
                QComboBox *cb = new QComboBox(this);
                if(!values.empty()){
                    itm->setText(2,QString::number(values.count()));
                    for(const auto& any:values){
                        cb->addItem(QString::number(any));
                    }
                }else{
                    itm->setText(2,QString::number(2));
                    cb->addItem("True");
                    cb->addItem("False");
                }

                twmss->addTopLevelItem(itm);
                twmss->setItemWidget(itm,3,cb);

            }


        }
    }

    //output
    {
        ui->experimentNameLineEdit->setText(QString::fromStdString(xml.get("master.output.experimentName","dummy")));
        ui->lineEdit_experimentDescription->setText(QString::fromStdString(xml.get("master.output.experimentDescription","dummy")));

        ui->schedulerLineEdit->setText(QString::fromStdString(xml.get("master.output.scheduler","unknown")));
        ui->correlatorLineEdit->setText(QString::fromStdString(xml.get("master.output.correlator","unknown")));

        ui->lineEdit_PIName->setText(QString::fromStdString(xml.get("master.output.piName","")));
        ui->lineEdit_PIEmail->setText(QString::fromStdString(xml.get("master.output.piEmail","")));
        ui->lineEdit_contactName->setText(QString::fromStdString(xml.get("master.output.contactName","")));
        ui->lineEdit_contactEmail->setText(QString::fromStdString(xml.get("master.output.contactEmail","")));
        ui->plainTextEdit_notes->setPlainText(QString::fromStdString(xml.get("master.output.notes","")));
        ui->plainTextEdit_operationNotes->setPlainText(QString::fromStdString(xml.get("master.output.operationNotes","")));

        if(xml.get("master.output.createSummary",false)){
            ui->checkBox_outputStatisticsFile->setChecked(true);
        }else{
            ui->checkBox_outputStatisticsFile->setChecked(false);
        }
        if(xml.get("master.output.createNGS",false)){
            ui->checkBox_outputNGSFile->setChecked(true);
        }else{
            ui->checkBox_outputNGSFile->setChecked(false);
        }
        if(xml.get("master.output.createSKD",false)){
            ui->checkBox_outputSkdFile->setChecked(true);
        }else{
            ui->checkBox_outputSkdFile->setChecked(false);
        }
        if(xml.get("master.output.createVEX",false)){
            ui->checkBox_outputVex->setChecked(true);
        }else{
            ui->checkBox_outputVex->setChecked(false);
        }
        if(xml.get("master.output.createOperationsNotes",false)){
            ui->checkBox_outputOperationsNotes->setChecked(true);
        }else{
            ui->checkBox_outputOperationsNotes->setChecked(false);
        }
        if(xml.get("master.output.createSourceGroupStatistics",false)){
            ui->checkBox_outputSourceGroupStatFile->setChecked(true);
            // TODO check statistics
        }else{
            ui->checkBox_outputSourceGroupStatFile->setChecked(false);
        }
    }

    //ruleScanSequence
    {
        ui->groupBox_scanSequence->setChecked(false);
        boost::optional<boost::property_tree::ptree &> ctree = xml.get_child_optional("master.rules.sourceSequence");
        if (ctree.is_initialized()) {
            ui->groupBox_scanSequence->setChecked(true);
            ui->spinBox_scanSequenceCadence->setValue(xml.get<int>("master.rules.sourceSequence.cadence"));
            for(const auto &any: *ctree){
                if(any.first == "sequence"){
                    int modulo = any.second.get<int>("modulo");
                    QString member = QString::fromStdString(any.second.get<std::string>("member"));
                    QComboBox* cb = qobject_cast<QComboBox*>(ui->tableWidget_scanSequence->cellWidget(modulo,0));
                    cb->setCurrentText(member);
                }
            }
        }
    }

    //ruleCalibratorBlock
    {
        ui->groupBox_CalibratorBlock->setChecked(false);
        boost::optional<boost::property_tree::ptree &> ctree = xml.get_child_optional("master.rules.calibratorBlock");
        if (ctree.is_initialized()) {
            ui->groupBox_CalibratorBlock->setChecked(true);
            if(xml.get("master.rules.calibratorBlock.cadence_nScanSelections", -1) != -1){
                ui->radioButton_calibratorScanSequence->setChecked(true);
                ui->spinBox_calibratorScanSequence->setValue(xml.get("master.rules.calibratorBlock.cadence_nScanSelections",5));
            }
            if(xml.get("master.rules.calibratorBlock.cadence_seconds", -1) != -1){
                ui->radioButton_calibratorTime->setChecked(true);
                ui->spinBox_calibratorTime->setValue(xml.get("master.rules.calibratorBlock.cadence_seconds",3600));
            }
            std::string members = xml.get("master.rules.calibratorBlock.member","__all__");
            ui->comboBox_calibratorBlock_calibratorSources->setCurrentText(QString::fromStdString(members));
            ui->spinBox_calibrator_maxScanSequence->setValue(xml.get("master.rules.calibratorBlock.nMaxScans",4));
            ui->spinBox_calibratorFixedScanLength->setValue(xml.get("master.rules.calibratorBlock.fixedScanTime",120));
            ui->radioButton->setChecked(true);

            ui->doubleSpinBox_calibratorHighElEnd->setValue(xml.get("master.rules.calibratorBlock.highElevation.fullWeight",70));
            ui->doubleSpinBox_calibratorHighElStart->setValue(xml.get("master.rules.calibratorBlock.highElevation.startWeight",50));
            ui->doubleSpinBox_calibratorLowElEnd->setValue(xml.get("master.rules.calibratorBlock.lowElevation.fullWeight",20));
            ui->doubleSpinBox_calibratorLowElStart->setValue(xml.get("master.rules.calibratorBlock.lowElevation.startWeight",40));
        }
    }

    //highImpactAzEl
    {
        ui->groupBox_highImpactAzEl->setChecked(false);
        boost::optional<boost::property_tree::ptree &> ctree = xml.get_child_optional("master.highImpact");
        if (ctree.is_initialized()) {
            ui->groupBox_highImpactAzEl->setChecked(true);

            ui->spinBox_highImpactInterval->setValue(xml.get("master.highImpact.interval",60));
            ui->spinBox_highImpactMinRepeat->setValue(xml.get("master.highImpact.repeat",300));

            for(const auto &any: *ctree){
                if(any.first == "targetAzEl"){
                    std::string member = any.second.get<std::string>("member");
                    ui->comboBox_highImpactStation->setCurrentText(QString::fromStdString(member));
                    ui->doubleSpinBox_highImpactAzimuth->setValue(any.second.get<double>("az"));
                    ui->doubleSpinBox_highImpactElevation->setValue(any.second.get<double>("el"));
                    ui->doubleSpinBox_highImpactMargin->setValue(any.second.get<double>("margin"));

                    ui->pushButton_addHighImpactAzEl->click();
                }
            }
        }
    }
}

void MainWindow::addSetup(QTreeWidget *tree, const boost::property_tree::ptree &ptree,
                          QComboBox *cmember, QComboBox *cpara, QDateTimeEdit *dte_start,
                          QDateTimeEdit *dte_end, QComboBox *trans, QPushButton *add){

    QDateTime start_time = ui->dateTimeEdit_sessionStart->dateTime();
    double dur = ui->doubleSpinBox_sessionDuration->value();
    int sec = dur*3600;
    QDateTime end_time = start_time.addSecs(sec);
    QString parameter;
    QString member;
    QString transition = "soft";
    QTreeWidgetItem *selected = tree->selectedItems().at(0);


    for(const auto & any: ptree){
        if(any.first == "group" || any.first == "member"){
            member = QString::fromStdString(any.second.get_value<std::string>());
        }else if(any.first == "parameter"){
            parameter = QString::fromStdString(any.second.get_value<std::string>());
        }else if(any.first == "start"){
            QString starTimeStr = QString::fromStdString(any.second.get_value<std::string>());
            start_time = QDateTime::fromString(starTimeStr,"yyyy.MM.dd HH:mm:ss");
        }else if(any.first == "end"){
            QString endTimeStr = QString::fromStdString(any.second.get_value<std::string>());
            end_time   = QDateTime::fromString(endTimeStr,"yyyy.MM.dd HH:mm:ss");
        }else if(any.first == "transition"){
            transition = QString::fromStdString(any.second.get_value<std::string>());
        }
    }

    cmember->setCurrentText(member);
    cpara->setCurrentText(parameter);
    dte_start->setDateTime(start_time);
    dte_end->setDateTime(end_time);
    trans->setCurrentText(transition);
    int ns = selected->childCount();

    add->click();
    selected->setSelected(false);

    for(const auto & any: ptree){
        if(any.first == "setup"){
            selected->child(selected->childCount()-1)->setSelected(true);
            int n = selected->childCount();
            addSetup(tree, any.second, cmember, cpara, dte_start, dte_end, trans, add);
        }
    }


}

void MainWindow::createDefaultParameterSettings()
{
    VieVS::ParameterSettings::ParametersStations sta;
    sta.maxScan = 600;
    sta.minScan = 30;
    sta.maxSlewtime = 600;
    sta.maxSlewDistance = 175;
    sta.minSlewDistance = 0;
    sta.maxWait = 600;
    sta.maxNumberOfScans = 9999;
    sta.weight = 1;
    sta.minElevation = 5;
    settings.add_child("settings.station.parameters.parameter",VieVS::ParameterSettings::parameterStation2ptree("default",sta).get_child("parameters"));

    VieVS::ParameterSettings::ParametersSources src;
    src.minRepeat = 1800;
    src.minScan = 20;
    src.maxScan = 600;
    src.weight = 1;
    src.minFlux = 0.05;
    src.maxNumberOfScans = 999;
    src.minNumberOfStations = 2;
    src.minElevation = 0;
    src.minSunDistance = 4;
    settings.add_child("settings.source.parameters.parameter",VieVS::ParameterSettings::parameterSource2ptree("default",src).get_child("parameters"));

    VieVS::ParameterSettings::ParametersBaselines bl;
    bl.maxScan = 600;
    bl.minScan = 30;
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

    settings.add("settings.output.directory", "out/");

    std::ofstream os;
    os.open("settings.xml");
    boost::property_tree::xml_parser::write_xml(os, settings,
                                                boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
    os.close();

}

// ########################################### MODE ###########################################

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

void MainWindow::gbps()
{
    int bits = ui->sampleBitsSpinBox->value();
    double mhz = ui->sampleRateDoubleSpinBox->value();
    int channels = 0;
    for(int i=0; i<ui->tableWidget_modeCustonBand->rowCount(); ++i){
        channels += qobject_cast<QSpinBox *>(ui->tableWidget_modeCustonBand->cellWidget(i,1))->value();
    }
    double mb = bits * mhz * channels;
    ui->label_gbps->setText(QString("%1 [Mbps]").arg(mb));
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
    connect(nChannelSB,SIGNAL(valueChanged(int)),this,SLOT(gbps()));

    QPushButton *d = new QPushButton("delete",this);
    d->setIcon(QIcon(":/icons/icons/edit-delete-6.png"));
    connect(d,SIGNAL(clicked(bool)),deleteModeMapper,SLOT(map()));
    deleteModeMapper->setMapping(d,name);

    ui->tableWidget_modeCustonBand->setCellWidget(ui->tableWidget_modeCustonBand->rowCount()-1,0,freqSB);
    ui->tableWidget_modeCustonBand->setCellWidget(ui->tableWidget_modeCustonBand->rowCount()-1,1,nChannelSB);
    ui->tableWidget_modeCustonBand->setCellWidget(ui->tableWidget_modeCustonBand->rowCount()-1,2,d);
    addModesPolicyTable(name);

    gbps();
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
    gbps();
}

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


void MainWindow::on_pushButton_modeCustomAddBand_clicked()
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

void MainWindow::on_groupBox_modeSked_toggled(bool arg1)
{
    ui->groupBox_modeCustom->setChecked(!arg1);
}

void MainWindow::on_groupBox_modeCustom_toggled(bool arg1)
{
    ui->groupBox_modeSked->setChecked(!arg1);
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

// ########################################### CATALOGS ###########################################

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

void MainWindow::on_pushButton_browseAntenna_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathAntenna->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathAntenna->setText(path);
    }
}

void MainWindow::on_pushButton_browseEquip_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathEquip->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathEquip->setText(path);
    }
}

void MainWindow::on_pushButton_browsePosition_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathPosition->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathPosition->setText(path);
    }
}

void MainWindow::on_pushButton_browseMask_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathMask->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathMask->setText(path);
    }
}

void MainWindow::on_pushButton_browseSource_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathSource->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathSource->setText(path);
    }
}


void MainWindow::on_pushButton_browseSource2_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_browseSource2->text());
    if( !path.isEmpty() ){
        ui->lineEdit_browseSource2->setText(path);
    }

}


void MainWindow::on_pushButton_browseFlux_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathFlux->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathFlux->setText(path);
    }
}

void MainWindow::on_pushButton_browsModes_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathModes->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathModes->setText(path);
    }
}

void MainWindow::on_pushButton_browseFreq_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathModes->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathFreq->setText(path);
    }
}

void MainWindow::on_pushButton_browseTracks_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathTracks->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathTracks->setText(path);
    }
}

void MainWindow::on_pushButton_browseLoif_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathLoif->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathLoif->setText(path);
    }
}

void MainWindow::on_pushButton_browseRec_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathRec->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathRec->setText(path);
    }
}

void MainWindow::on_pushButton_browseRx_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathRx->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathRx->setText(path);
    }
}

void MainWindow::on_pushButton_browseHdpos_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Browse to catalog", ui->lineEdit_pathHdpos->text());
    if( !path.isEmpty() ){
        ui->lineEdit_pathHdpos->setText(path);
    }
}

void MainWindow::on_pushButton_stations_clicked()
{
    allStationModel->removeRows(0,allStationModel->rowCount());

    selectedStationModel->removeRows(0,selectedStationModel->rowCount());
    selectedBaselineModel->removeRows(0,selectedBaselineModel->rowCount());

    allStationPlusGroupModel->removeRows(0,allStationPlusGroupModel->rowCount());
    allStationPlusGroupModel->insertRow(0,new QStandardItem(QIcon(":/icons/icons/station_group_2.png"),"__all__"));

    allBaselinePlusGroupModel->removeRows(0,allStationPlusGroupModel->rowCount());
    allBaselinePlusGroupModel->insertRow(0,new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),"__all__"));


    availableStations->clear();
    selectedStations->clear();
    auto series = worldmap->chart()->series();
    for(auto &s : series){
        if(s->name().right(4) == "[km]"){
            worldmap->chart()->removeSeries(s);
            delete(s);
        }
    }

    readStations();

    QString warnings;
    if(ui->treeWidget_setupStation->topLevelItem(0)->child(0)->childCount() != 0){
        warnings.append("station setup cleared\n");
        clearSetup(true,false,false);
    }
    if(ui->treeWidget_setupBaseline->topLevelItem(0)->child(0)->childCount() != 0){
        warnings.append("baseline setup cleared\n");
        clearSetup(false,false,true);
    }
    if(!groupSta.empty()){
        groupSta.clear();
        warnings.append("station groups cleared\n");
    }
    if(!groupBl.empty()){
        groupBl.clear();
        warnings.append("baseline groups cleared\n");
    }
    if(ui->treeWidget_highImpactAzEl->topLevelItemCount() != 0){
        ui->treeWidget_highImpactAzEl->clear();
        warnings.append("high impact scans setup cleared\n");
    }

    QIcon icSta = QIcon(":/icons/icons/station.png");
    QIcon icBl = QIcon(":/icons/icons/baseline.png");
    QIcon icStaGrp = QIcon(":/icons/icons/station_group_2.png");
    QIcon icBlGrp = QIcon(":/icons/icons/baseline_group.png");
    bool mssta = false;
    bool msbl = false;
    int i=0;
    while(i < ui->treeWidget_multiSchedSelected->topLevelItemCount()){
        QIcon parameterIcon = ui->treeWidget_multiSchedSelected->topLevelItem(i)->icon(0);
        if(parameterIcon.pixmap(16,16).toImage() == icSta.pixmap(16,16).toImage() || parameterIcon.pixmap(16,16).toImage() == icStaGrp.pixmap(16,16).toImage()){
            auto itm = ui->treeWidget_multiSchedSelected->takeTopLevelItem(i);
            delete(itm);
            mssta = true;
            continue;
        }else if(parameterIcon.pixmap(16,16).toImage() == icBl.pixmap(16,16).toImage() || parameterIcon.pixmap(16,16).toImage() == icBlGrp.pixmap(16,16).toImage()){
            auto itm = ui->treeWidget_multiSchedSelected->takeTopLevelItem(i);
            delete(itm);
            msbl = true;
            continue;
        }else{
            ++i;
        }
    }
    if(mssta){
        warnings.append("station multi scheduling parameters cleared\n");
    }
    if(msbl){
        warnings.append("baseline multi scheduling parameters cleared\n");
    }

    if(!warnings.isEmpty()){
        QMessageBox::warning(this,"reload stations",warnings);
    }else{
        QMessageBox::information(this, "reload stations", "stations successfully reloaded");
    }

}

void MainWindow::on_pushButton_reloadsources_clicked()
{
    allSourceModel->removeRows(0,allSourceModel->rowCount());

    selectedSourceModel->removeRows(0,selectedSourceModel->rowCount());

    allSourcePlusGroupModel->removeRows(0,allSourcePlusGroupModel->rowCount());
    allSourcePlusGroupModel->insertRow(0,new QStandardItem(QIcon(":/icons/icons/source_group.png"),"__all__"));

    availableSources->clear();
    selectedSources->clear();

    readSources();

    QString warnings;
    if(ui->treeWidget_setupSource->topLevelItem(0)->child(0)->childCount() != 0){
        warnings.append("source setup cleared\n");
        clearSetup(false,true,false);
    }
    if(!groupSrc.empty()){
        groupSrc.clear();
        warnings.append("source groups cleared\n");
    }

    QIcon icSrc = QIcon(":/icons/icons/source.png");
    QIcon icSrcGrp = QIcon(":/icons/icons/source_group.png");
    bool mssrc = false;
    int i=0;
    while(i < ui->treeWidget_multiSchedSelected->topLevelItemCount()){
        QIcon parameterIcon = ui->treeWidget_multiSchedSelected->topLevelItem(i)->icon(0);
        if(parameterIcon.pixmap(16,16).toImage() == icSrc.pixmap(16,16).toImage() || parameterIcon.pixmap(16,16).toImage() == icSrcGrp.pixmap(16,16).toImage()){
            auto itm = ui->treeWidget_multiSchedSelected->takeTopLevelItem(i);
            delete(itm);
            mssrc = true;
            continue;
        }else{
            ++i;
        }
    }
    if(mssrc){
        warnings.append("source multi scheduling parameters cleared\n");
    }

    ui->treeWidget_srcGroupForStatistics->clear();
    QTreeWidgetItem *itm = new QTreeWidgetItem();
    itm->setText(0,"__all__");
    itm->setCheckState(0,Qt::Unchecked);
    ui->treeWidget_srcGroupForStatistics->addTopLevelItem(itm);

    if(ui->spinBox_scanSequenceCadence->value() != 1){
        ui->spinBox_scanSequenceCadence->setValue(0);
        warnings.append("scan sequence cleared\n");
    }

    if(ui->comboBox_calibratorBlock_calibratorSources->currentText() != "__all__"){
        ui->comboBox_calibratorBlock_calibratorSources->setCurrentIndex(0);
        warnings.append("calibrator sources selection cleared\n");
    }

    if(ui->treeWidget_conditions->topLevelItemCount() != 0){
        ui->treeWidget_conditions->clear();
        warnings.append("optimisation conditions cleared\n");
    }

    if(!warnings.isEmpty()){
        QMessageBox::warning(this,"reload sources",warnings);
    }else{
        QMessageBox::information(this, "reload sources", "sources successfully reloaded");
    }

}

void MainWindow::on_pushButton_reloadcatalogs_clicked()
{
    readAllSkedObsModes();
    QMessageBox::information(this, "reload modes", "modes successfully reloaded\ncheck current selection");
}


void MainWindow::on_pushButton_howAreSkedCatalogsLinked_clicked()
{
    SkedCatalogInfo dial(this);
    dial.setFonts();
    dial.exec();
}

// ########################################### GENERAL ###########################################

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

void MainWindow::on_dateTimeEdit_sessionStart_dateChanged(const QDate &date)
{
    int doy = date.dayOfYear();
    ui->spinBox_doy->setValue(doy);
}

void MainWindow::on_spinBox_doy_valueChanged(int arg1)
{
    QDate x = ui->dateTimeEdit_sessionStart->date();
    int y = x.year();
    x.setDate(y,1,1);
    x = x.addDays(arg1-1);
    ui->dateTimeEdit_sessionStart->setDate(x);
}

void MainWindow::on_pushButton_clicked()
{
    QString txt = ui->lineEdit_ivsMaster->text();
    QString errorText = "";
    QStringList t = txt.split("\t");
    if(t.size()>=0){
        QString sessionName = t.at(0);
        ui->lineEdit_experimentDescription->setText(sessionName);
    }
    if(t.size()>=1){
        QString sessionName = t.at(1);
        ui->experimentNameLineEdit->setText(sessionName);
    }
    if(t.size()>=2){
        QString time = t.at(2);
        time = time.split(" ",QString::SkipEmptyParts).at(1);

        QStringList ts = time.split(":");
        int hour, min;

        if(ts.size()==2){
            bool okh, okm;
            hour = ts.at(0).toInt(&okh);
            min = ts.at(1).toInt(&okm);
            if(okh && okm){
                ui->dateTimeEdit_sessionStart->setTime(QTime(hour,min,0,0));
            }else{
                errorText.append("cannot convert TIME\n");
            }
        }else{
            errorText.append("cannot convert TIME\n");
        }
    }
    if(t.size()>=3){
        QString doys = t.at(3);
        bool ok;
        int doy = doys.toInt(&ok);
        if(ok){
            ui->spinBox_doy->setValue(doy);
        }else{
            errorText.append("cannot convert DOY\n");
        }
    }
    if(t.size()>=4){
        QString durs = t.at(4);
        QStringList ts = durs.split(":");
        int hour, min;

        if(ts.size()==2){
            bool okh, okm;
            hour = ts.at(0).toInt(&okh);
            min = ts.at(1).toInt(&okm);
            if(okh && okm){
                ui->doubleSpinBox_sessionDuration->setValue(hour+min/60);
            }else{
                errorText.append("cannot convert Duration\n");
            }
        }else{
            errorText.append("cannot convert Duration\n");
        }
    }
    if(t.size()>=5){
        createBaselines = false;

        QString tmp = t.at(5);
        tmp = tmp.trimmed();
        QStringList stas = tmp.split(" ", QString::SkipEmptyParts);
        if(stas.size() >= 2){
            int n = selectedStationModel->rowCount();
            for(int i=0; i<n; ++i){
                QModelIndex index = selectedStationModel->index(0,0);
                on_treeView_allSelectedStations_clicked(index);
            }

            allStationProxyModel->setFilterRegExp("");
            for(int i=0; i<stas.size(); ++i){
                QString sta = stas.at(i).toUpper();
                bool found = false;
                for(int j=0; j<allStationProxyModel->rowCount(); ++j){
                    QString itsta = allStationProxyModel->index(j,1).data().toString().toUpper();
                    if(itsta == sta){
                        QModelIndex index = allStationProxyModel->index(j,0);
                        on_treeView_allAvailabeStations_clicked(index);
                        found = true;
                        break;
                    }
                }
                if(!found){
                    errorText.append(QString("unknown station %1\n").arg(sta));
                }
            }
        }else{
            errorText.append("error while reading stations\n");
        }
        createBaselines = true;
        createBaselineModel();
    }
    if(t.size()>=7){
        QString sked = t.at(7);
        ui->schedulerLineEdit->setText(sked);
    }
    if(t.size()>=8){
        QString corr = t.at(8);
        ui->correlatorLineEdit->setText(corr);
    }

    if(errorText.size() != 0){
        QMessageBox::warning(this,"errors while reading session master line",errorText);
    }
}

void MainWindow::on_experimentNameLineEdit_textChanged(const QString &arg1)
{
    if(arg1.length() >6){
        QPalette p = ui->experimentNameLineEdit->palette();
        p.setColor(QPalette::Base, Qt::red);
        ui->experimentNameLineEdit->setPalette(p);
    }else{
        QPalette p = ui->experimentNameLineEdit->palette();
        p.setColor(QPalette::Base, Qt::white);
        ui->experimentNameLineEdit->setPalette(p);
    }
}

void MainWindow::on_comboBox_log_file_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "trace" || arg1 == "debug"){
        ui->label_log->setText("high log level can slow down application");
    }else{
        if(ui->comboBox_log_console->currentText() == "trace" || ui->comboBox_log_console->currentText() == "debug"){
            ui->label_log->setText("heavy logging can slow down application");
        }else{
            ui->label_log->setText("");
        }
    }
}

void MainWindow::on_comboBox_log_console_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "trace" || arg1 == "debug"){
        ui->label_log->setText("high log level can slow down application");
    }else{
        if(ui->comboBox_log_file->currentText() == "trace" || ui->comboBox_log_file->currentText() == "debug"){
            ui->label_log->setText("heavy logging can slow down application");
        }else{
            ui->label_log->setText("");
        }
    }
}

// ########################################### SETUP ###########################################

void MainWindow::prepareSetupPlot(QChartView *figure, QVBoxLayout *container)
{
    QChart *chart = new QChart();
    figure->setChart(chart);

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
    axisX->setRange(start,end);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Parameters");
    axisY->setTickCount(1);
    axisY->setRange(-10,10);
    chart->addAxis(axisY,Qt::AlignLeft);
    series->attachAxis(axisY);

    figure->setRenderHint(QPainter::Antialiasing);
    container->insertWidget(1,figure,1);
    axisY->hide();
    axisX->show();
    chart->legend()->hide();

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

void MainWindow::deleteSetupSelection(VieVS::ParameterSetup &setup, QChartView *setupChartView, QComboBox *setupCB,
                                      QTreeWidget *setupTW){
    QList<QTreeWidgetItem *> sel = setupTW->selectedItems();
    for(int i = 0; i<sel.size(); ++i){
        if(sel.at(0)->text(1) == "multi scheduling" || !sel.at(0)->parent()){
            QMessageBox *ms = new QMessageBox;
            ms->warning(this,"Wrong selection","You can not delete top level parameters!");
        }else{
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
        }
    }
}

void MainWindow::setBackgroundColorOfChildrenWhite(QTreeWidgetItem *item)
{
    for(int i=0; i<item->childCount(); ++i){
        auto itm = item->child(i);
        itm->setBackgroundColor(5,Qt::white);
        setBackgroundColorOfChildrenWhite(itm);
    }
}

int MainWindow::plotParameter(QChart* chart, QTreeWidgetItem *root, int level, int plot, QString target,
                              const std::map<std::string, std::vector<std::string> > &map){
    QDateTime start = QDateTime::fromString(root->text(2),"dd.MM.yyyy hh:mm");
    QDateTime end = QDateTime::fromString(root->text(3),"dd.MM.yyyy hh:mm");

    QLineSeries *series = new QLineSeries();

//    connect(series,SIGNAL(clicked(QPointF)),this,SLOT(worldmap_clicked(QPointF)));
    series->setName(root->text(1));

    QColor c;
    switch (plot%9) {
    case 0: c = QColor(228,26,28); break;
    case 1: c = QColor(200,200,200); break;
    case 2: c = QColor(55,126,184); break;
    case 3: c = QColor(77,175,74); break;
    case 4: c = QColor(152,78,163); break;
    case 5: c = QColor(255,127,0); break;
    case 6: c = QColor(255,255,51); break;
    case 7: c = QColor(166,86,40); break;
    case 8: c = QColor(247,129,191); break;
    case 9: c = QColor(153,153,153); break;
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
        connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(displayStationSetupParameterFromPlot(QPointF,bool)));
    }else if(chart == setupSource->chart()){
        connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(displaySourceSetupParameterFromPlot(QPointF,bool)));
    }else if(chart == setupBaseline->chart()){
        connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(displayBaselineSetupParameterFromPlot(QPointF,bool)));
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

        QTreeWidgetItem *ms = new QTreeWidgetItem();
        ms->setText(0,"__all__");
        ms->setText(1,"multi scheduling");
        ms->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
        ms->setText(3,e.toString("dd.MM.yyyy hh:mm"));
        ms->setText(4,"hard");
        ms->setIcon(0,QIcon(":/icons/icons/station_group_2.png"));
        wsta->addChild(ms);

        ui->treeWidget_setupStation->clear();
        ui->treeWidget_setupStation->insertTopLevelItem(0,wsta);
        ui->treeWidget_setupStation->expandAll();
        QHeaderView * hvsta = ui->treeWidget_setupStation->header();
        hvsta->setSectionResizeMode(QHeaderView::ResizeToContents);
        setupStationTree = VieVS::ParameterSetup(parameterName,
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);

        VieVS::ParameterSetup mss = VieVS::ParameterSetup("multi scheduling",
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);
        setupStationTree.addChild(mss);

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

        QTreeWidgetItem *ms = new QTreeWidgetItem();
        ms->setText(0,"__all__");
        ms->setText(1,"multi scheduling");
        ms->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
        ms->setText(3,e.toString("dd.MM.yyyy hh:mm"));
        ms->setText(4,"hard");
        ms->setIcon(0,QIcon(":/icons/icons/source_group.png"));
        wsrc->addChild(ms);

        ui->treeWidget_setupSource->clear();
        ui->treeWidget_setupSource->insertTopLevelItem(0,wsrc);
        ui->treeWidget_setupSource->expandAll();

        QHeaderView * hvsrc = ui->treeWidget_setupSource->header();
        hvsrc->setSectionResizeMode(QHeaderView::ResizeToContents);
        setupSourceTree = VieVS::ParameterSetup(parameterName,
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);

        VieVS::ParameterSetup mss = VieVS::ParameterSetup("multi scheduling",
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);
        setupSourceTree.addChild(mss);

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

        QTreeWidgetItem *ms = new QTreeWidgetItem();
        ms->setText(0,"__all__");
        ms->setText(1,"multi scheduling");
        ms->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yyyy hh:mm"));
        ms->setText(3,e.toString("dd.MM.yyyy hh:mm"));
        ms->setText(4,"hard");
        ms->setIcon(0,QIcon(":/icons/icons/baseline_group.png"));
        wbl->addChild(ms);

        ui->treeWidget_setupBaseline->clear();
        ui->treeWidget_setupBaseline->insertTopLevelItem(0,wbl);
        ui->treeWidget_setupBaseline->expandAll();

        QHeaderView * hvbl = ui->treeWidget_setupBaseline->header();
        hvbl->setSectionResizeMode(QHeaderView::ResizeToContents);
        setupBaselineTree = VieVS::ParameterSetup(parameterName,
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);

        VieVS::ParameterSetup mss = VieVS::ParameterSetup("multi scheduling",
                                      member,
                                      startt,
                                      endt,
                                      VieVS::ParameterSetup::Transition::hard);
        setupBaselineTree.addChild(mss);

        drawSetupPlot(setupBaseline, ui->comboBox_setupBaseline, ui->treeWidget_setupBaseline);
    }
}

// ########################################### STATION AND BASELINE ###########################################

void MainWindow::readStations()
{
    QString antennaPath = ui->lineEdit_pathAntenna->text();
    QString equipPath = ui->lineEdit_pathEquip->text();
    QString positionPath = ui->lineEdit_pathPosition->text();
    QMap<QString,QStringList > antennaMap;
    QMap<QString,QStringList > equipMap;
    QMap<QString,QStringList > positionMap;

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

}

void MainWindow::on_treeView_allSelectedStations_clicked(const QModelIndex &index)
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


    if(createBaselines){
        createBaselineModel();
    }

    ui->treeWidget_multiSchedSelected->clear();
    for(int i=0; i<ui->treeWidget_multiSched->topLevelItemCount(); ++i){
        ui->treeWidget_multiSched->topLevelItem(i)->setDisabled(false);
    }
}

void MainWindow::on_treeView_allAvailabeStations_clicked(const QModelIndex &index)
{
    int row = index.row();
    QString name = allStationProxyModel->index(row,0).data().toString();

    if(selectedStationModel->findItems(name).isEmpty()){
        selectedStationModel->insertRow(0);

        int nrow = allStationModel->findItems(name).at(0)->row();
        for(int i=0; i<allStationModel->columnCount(); ++i){
            selectedStationModel->setItem(0, i, allStationModel->item(nrow,i)->clone() );
        }

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
        if(createBaselines){
            createBaselineModel();
        }
    }
    ui->lineEdit_allStationsFilter->setFocus();
    ui->lineEdit_allStationsFilter->selectAll();
}

void MainWindow::on_lineEdit_allStationsFilter_textChanged(const QString &arg1)
{
    allStationProxyModel->setFilterRegExp(arg1);
}

void MainWindow::on_treeView_allAvailabeStations_entered(const QModelIndex &index)
{
    int row = index.row();
    QString name = allStationProxyModel->index(row,0).data().toString();
    QString id = allStationProxyModel->index(row,1).data().toString();

    double x = allStationProxyModel->index(row,3).data().toDouble();
    double y = allStationProxyModel->index(row,2).data().toDouble();

    QString text = QString("%1 (%2) \nlat: %3 [deg] \nlon: %4 [deg] ").arg(name).arg(id).arg(x).arg(y);
    worldMapCallout->setText(text);
    worldMapCallout->setAnchor(QPointF(x,y));
    worldMapCallout->setZValue(11);
    worldMapCallout->updateGeometry();
    worldMapCallout->show();
}

void MainWindow::on_treeView_allSelectedBaselines_entered(const QModelIndex &index)
{
    int row = index.row();
    QString txt = selectedBaselineModel->index(row,0).data().toString();
    QString txt2 = selectedBaselineModel->index(row,1).data().toString();
    txt.append("\n").append(txt2).append(" [km]");

    double x,y;
    for(int i=0; i<worldmap->chart()->series().count(); ++i){
        if(worldmap->chart()->series().at(i)->name() == txt){
            auto s = qobject_cast<QLineSeries *>(worldmap->chart()->series().at(i));
            x = (s->at(0).x()+s->at(1).x())/2;
            y = (s->at(0).y()+s->at(1).y())/2;
            break;
        }
    }


    worldMapCallout->setText(txt);
    worldMapCallout->setAnchor(QPointF(x,y));
    worldMapCallout->setZValue(11);
    worldMapCallout->updateGeometry();
    worldMapCallout->show();
}

void MainWindow::on_treeView_allSelectedStations_entered(const QModelIndex &index)
{
    int row = index.row();
    QString name = selectedStationModel->index(row,0).data().toString();
    QString id = selectedStationModel->index(row,1).data().toString();

    for(int i = 0; i < allStationModel->rowCount(); ++i){
        QString newName = allStationModel->index(i,0).data().toString();
        if (newName == name){
            double x = allStationModel->index(i,3).data().toDouble();;
            double y = allStationModel->index(i,2).data().toDouble();;
            QString text = QString("%1 (%2) \nlat: %3 [deg] \nlon: %4 [deg] ").arg(name).arg(id).arg(x).arg(y);
            worldMapCallout->setText(text);
            worldMapCallout->setAnchor(QPointF(x,y));
            worldMapCallout->setZValue(11);
            worldMapCallout->updateGeometry();
            worldMapCallout->show();
            break;
        }
    }
}

void MainWindow::plotWorldMap()
{
    QChart *worldChart = worldmap->chart();

    availableStations = new QScatterSeries(worldChart);
    availableStations->setColor(Qt::red);
    availableStations->setMarkerSize(10);
    availableStations->setName("availableStations");

    selectedStations = new QScatterSeries(worldChart);
    selectedStations->setName("selectedStations");
    markerWorldmap();

    worldChart->addSeries(availableStations);
    worldChart->addSeries(selectedStations);

    connect(availableStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));
    connect(selectedStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));


    for(int row = 0; row<allStationModel->rowCount(); ++row){
        double lat = allStationModel->index(row,2).data().toDouble();
        double lon = allStationModel->index(row,3).data().toDouble();
        availableStations->append(lon,lat);
    }

    availableStations->attachAxis(worldChart->axisX());
    availableStations->attachAxis(worldChart->axisY());
    selectedStations->attachAxis(worldChart->axisX());
    selectedStations->attachAxis(worldChart->axisY());

}

void MainWindow::worldmap_hovered(QPointF point, bool state)
{
    if (state) {
        QString sta;
        int scans;
        int obs;
        for(int i = 0; i<allStationModel->rowCount();++i){
            double x = allStationModel->index(i,3).data().toDouble();
            double y = allStationModel->index(i,2).data().toDouble();
            QString name = allStationModel->index(i,0).data().toString();
            QString id = allStationModel->index(i,1).data().toString();

            auto dx = x-point.x();
            auto dy = y-point.y();
            if(dx*dx+dy*dy < 1e-3){
                if(sta.size()==0){
                    sta.append(QString("%1 (%2)").arg(name).arg(id));
                }else{
                    sta.append(",").append(QString("%1 (%2)").arg(name).arg(id));
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

void MainWindow::on_pushButton_parameterStation_edit_clicked()
{
    stationParametersDialog *dial = new stationParametersDialog(settings,this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);
    dial->addSourceNames(allSourcePlusGroupModel);
    dial->addDefaultParameters(paraSta["default"]);
    dial->addSelectedParameters(paraSta[ui->ComboBox_parameterStation->currentText().toStdString()],ui->ComboBox_parameterStation->currentText());

    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersStations> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersStations parameter = res.second;

        paraSta[name] = parameter;

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

void MainWindow::on_pushButton_parameterBaseline_edit_clicked()
{
    baselineParametersDialog *dial = new baselineParametersDialog(settings, this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);

    dial->addDefaultParameters(paraBl["default"]);
    dial->addSelectedParameters(paraBl[ui->ComboBox_parameterBaseline->currentText().toStdString()],ui->ComboBox_parameterBaseline->currentText());


    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersBaselines> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersBaselines parameter = res.second;

        paraBl[name] = parameter;

    }
    delete(dial);

}

void MainWindow::createBaselineModel()
{
    selectedBaselineModel->removeRows(0,selectedBaselineModel->rowCount());

    allBaselinePlusGroupModel->setRowCount(1);
    for(const auto& any:groupBl){
        allBaselinePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/baseline_group.png"),QString::fromStdString(any.first)));
    }

    int n = selectedStationModel->rowCount();
    for(int i = 0; i<n; ++i){
        for(int j = i+1; j<n; ++j){
            QString bl = selectedStationModel->index(i,1).data().toString();
            bl.append("-").append(selectedStationModel->index(j,1).data().toString());
            allBaselinePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/baseline.png"),bl));
            int row = selectedBaselineModel->rowCount();

            selectedBaselineModel->insertRow(row);
            selectedBaselineModel->setItem(row,new QStandardItem(QIcon(":/icons/icons/baseline.png"),bl));
        }
    }

    auto series = worldmap->chart()->series();
    QAbstractSeries *tmpSel;
    QAbstractSeries *tmpAva;
    int nn = series.count();
    for(int i=0; i<nn; ++i){
        QString name = series.at(i)->name();
        if(name.size() >=5 && name.left(5) == "coast"){
            continue;
        }
        if(name == "selectedStations"){
            tmpSel = series.at(i);
            worldmap->chart()->removeSeries(series.at(i));
            continue;
        }
        if(name == "availableStations"){
            tmpAva = series.at(i);
            worldmap->chart()->removeSeries(series.at(i));
            continue;
        }
        worldmap->chart()->removeSeries(series.at(i));
        delete(series.at(i));
    }

    for(int i=0; i<selectedBaselineModel->rowCount(); ++i){
        QString txt = selectedBaselineModel->item(i,0)->text();
        QStringList stas = txt.split("-");
        double lat1, lat2, lon1, lon2, x1,y1,z1,x2,y2,z2;

        bool found1 = false;
        bool found2 = false;
        for(int j=0; j<selectedStationModel->rowCount(); ++j){
            auto thisSta = selectedStationModel->item(j,1)->text();
            if(thisSta == stas.at(0)){
                lon1 = selectedStationModel->index(j,3).data().toDouble();
                lat1 = selectedStationModel->index(j,2).data().toDouble();
                x1 = selectedStationModel->index(j, 16).data().toDouble();
                y1 = selectedStationModel->index(j, 17).data().toDouble();
                z1 = selectedStationModel->index(j, 18).data().toDouble();

                found1 = true;
            }else if(thisSta == stas.at(1)){
                lon2 = selectedStationModel->index(j,3).data().toDouble();
                lat2 = selectedStationModel->index(j,2).data().toDouble();
                x2 = selectedStationModel->index(j, 16).data().toDouble();
                y2 = selectedStationModel->index(j, 17).data().toDouble();
                z2 = selectedStationModel->index(j, 18).data().toDouble();

                found2 = true;
            }

            if(found1 && found2){
                break;
            }
        }
        double dist = qRound(qSqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)+(z2-z1)*(z2-z1))/1000);
//        selectedBaselineModel->setItem(i,1,new QStandardItem());
        selectedBaselineModel->setData(selectedBaselineModel->index(i, 1), dist);

        if(lon1>lon2){
            auto tmp1 = lon1;
            lon1 = lon2;
            lon2 = tmp1;
            auto tmp2 = lat1;
            lat1 = lat2;
            lat2 = tmp2;
        }

        if(qAbs(lon2-lon1)<180){
            QLineSeries *bl = new QLineSeries(worldmap->chart());
            bl->setPen(QPen(QBrush(Qt::darkGreen),1.5,Qt::DashLine));
            bl->append(lon1,lat1);
            bl->append(lon2,lat2);
            bl->setName(txt.append(QString("\n%1 [km]").arg(dist)));
            connect(bl,SIGNAL(hovered(QPointF,bool)),this,SLOT(baselineHovered(QPointF,bool)));
            worldmap->chart()->addSeries(bl);
        }else{

            double dx = 180-qAbs(lon1)+180-qAbs(lon2);
            double dy = lat2-lat1;

            QLineSeries *bl1 = new QLineSeries(worldmap->chart());
            bl1->setPen(QPen(QBrush(Qt::darkGreen),1.5,Qt::DashLine));
            bl1->append(lon1,lat1);
            double fracx = (180-qAbs(lon1))/dx;
            double fracy = dy*fracx;
            bl1->append(-180,lat1+fracy);
            bl1->setName(txt.append(QString("\n%1 [km]").arg(dist)));
            connect(bl1,SIGNAL(hovered(QPointF,bool)),this,SLOT(baselineHovered(QPointF,bool)));

            QLineSeries *bl2 = new QLineSeries(worldmap->chart());
            bl2->setPen(QPen(QBrush(Qt::darkGreen),1.5,Qt::DashLine));
            bl2->append(lon2,lat2);
            bl2->append(180,lat2-(dy-fracy));
            bl2->setName(txt.append(QString("\n%1 [km]").arg(dist)));
            connect(bl2,SIGNAL(hovered(QPointF,bool)),this,SLOT(baselineHovered(QPointF,bool)));

            if(qAbs(lon1)>qAbs(lon2)){
                worldmap->chart()->addSeries(bl2);
                worldmap->chart()->addSeries(bl1);
            }else{
                worldmap->chart()->addSeries(bl1);
                worldmap->chart()->addSeries(bl2);
            }
        }
    }
    worldmap->chart()->addSeries(tmpAva);
    worldmap->chart()->addSeries(tmpSel);

    worldmap->chart()->createDefaultAxes();
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

void MainWindow::on_pushButton_addSetupBaseline_clicked()
{
    addSetup(ui->treeWidget_setupBaseline, ui->DateTimeEdit_startParameterBaseline, ui->DateTimeEdit_endParameterBaseline,
             ui->comboBox_parameterBaselineTransition, ui->comboBox_baselineSettingMember, ui->ComboBox_parameterBaseline,
             setupBaselineTree, setupBaseline, ui->comboBox_setupBaseline);
}

void MainWindow::on_pushButton_3_clicked()
{

    addSetup(ui->treeWidget_setupStation, ui->DateTimeEdit_startParameterStation, ui->DateTimeEdit_endParameterStation,
             ui->comboBox_parameterStationTransition, ui->comboBox_stationSettingMember, ui->ComboBox_parameterStation,
             setupStationTree, setupStation, ui->comboBox_setupStation);
}

void MainWindow::on_pushButton_4_clicked()
{
    deleteSetupSelection(setupStationTree, setupStation, ui->comboBox_setupStation, ui->treeWidget_setupStation);
}

void MainWindow::on_pushButton_removeSetupBaseline_clicked()
{
    deleteSetupSelection(setupBaselineTree, setupBaseline, ui->comboBox_setupBaseline, ui->treeWidget_setupBaseline);
}

void MainWindow::on_treeWidget_setupStation_itemEntered(QTreeWidgetItem *item, int column)
{
    if(column == 0){
        displayStationSetupMember(item->text(column));
    }else if(column == 1){
        displayStationSetupParameter(item->text(column));
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

void MainWindow::on_comboBox_stationSettingMember_currentTextChanged(const QString &arg1)
{
    displayStationSetupMember(arg1);
}

void MainWindow::on_comboBox_baselineSettingMember_currentTextChanged(const QString &arg1)
{
    displayBaselineSetupMember(arg1);
}

void MainWindow::displayStationSetupParameterFromPlot(QPointF point, bool flag){
    QLineSeries* series = qobject_cast<QLineSeries*>(sender());
    QString name = series->name();
    displayStationSetupParameter(name);

    if(flag){
        stationSetupCallout->setAnchor(point);
        QDateTime st = QDateTime::fromMSecsSinceEpoch(series->at(0).x());
        QDateTime et = QDateTime::fromMSecsSinceEpoch(series->at(series->count()-1).x());
        QString txt = QString("Parameter: ").append(name);
        txt.append("\nfrom: ").append(st.toString("dd.MM.yyyy hh:mm"));
        txt.append("\nuntil: ").append(et.toString("dd.MM.yyyy hh:mm"));
        stationSetupCallout->setText(txt);
        stationSetupCallout->setZValue(11);
        stationSetupCallout->updateGeometry();
        stationSetupCallout->show();
    }else{
        stationSetupCallout->hide();
    }

}

void MainWindow::displayBaselineSetupParameterFromPlot(QPointF point, bool flag){
    QLineSeries* series = qobject_cast<QLineSeries*>(sender());
    QString name = series->name();
    displayBaselineSetupParameter(name);
    if(flag){
        baselineSetupCallout->setAnchor(point);
        QDateTime st = QDateTime::fromMSecsSinceEpoch(series->at(0).x());
        QDateTime et = QDateTime::fromMSecsSinceEpoch(series->at(series->count()-1).x());
        QString txt = QString("Parameter: ").append(name);
        txt.append("\nfrom: ").append(st.toString("dd.MM.yyyy hh:mm"));
        txt.append("\nuntil: ").append(et.toString("dd.MM.yyyy hh:mm"));
        baselineSetupCallout->setText(txt);
        baselineSetupCallout->setZValue(11);
        baselineSetupCallout->updateGeometry();
        baselineSetupCallout->show();
    }else{
        baselineSetupCallout->hide();
    }


}

void MainWindow::on_ComboBox_parameterStation_currentTextChanged(const QString &arg1)
{
    displayStationSetupParameter(arg1);
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
        t->topLevelItem(row)->setText(1,QString::number(ui->SpinBox_fieldSystem->value()).append(" [sec]"));
        t->topLevelItem(row)->setText(2,QString::number(ui->SpinBox_preob->value()).append(" [sec]"));
        t->topLevelItem(row)->setText(3,QString::number(ui->SpinBox_midob->value()).append(" [sec]"));
        t->topLevelItem(row)->setText(4,QString::number(ui->SpinBox_postob->value()).append(" [sec]"));
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
            on_treeView_allSelectedStations_clicked(idx);
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

void MainWindow::networkSizeChanged()
{
    int size = selectedStationModel->rowCount();
    ui->label_network_selected->setText(QString("selected: %1").arg(size));
}

void MainWindow::baselineListChanged()
{
    int size = selectedBaselineModel->rowCount();
    ui->label_network_baselines->setText(QString("baselines: %1").arg(size));
}

void MainWindow::markerWorldmap()
{
    if(ui->radioButton_marker_worldmap->isChecked()){
        selectedStations->setMarkerSize(ui->horizontalSlider_markerSizeWorldmap->value());
        selectedStations->setBrush(QBrush(Qt::darkGreen,Qt::SolidPattern));
        selectedStations->setPen(QColor(Qt::white));
    }else{
        QImage img(":/icons/icons/station_white.png");
        img = img.scaled(ui->horizontalSlider_markerSizeWorldmap->value(),ui->horizontalSlider_markerSizeWorldmap->value());
        selectedStations->setBrush(QBrush(img));
        selectedStations->setMarkerSize(ui->horizontalSlider_markerSizeWorldmap->value());
        selectedStations->setPen(QColor(Qt::transparent));
    }
}

void MainWindow::on_radioButton_imageWorldmap_toggled(bool checked)
{
    if(checked){
        ui->horizontalSlider_markerSizeWorldmap->setValue(30);
    }else{
        ui->horizontalSlider_markerSizeWorldmap->setValue(15);
    }
    markerWorldmap();
}

void MainWindow::baselineHovered(QPointF point, bool flag)
{
    if (flag) {
        auto tmp = sender();
        auto x = qobject_cast<QLineSeries *>(tmp);
        QString name = x->name();
        QString text = QString("%1").arg(name);
        worldMapCallout->setText(text);
        worldMapCallout->setAnchor(point);
        worldMapCallout->setZValue(11);
        worldMapCallout->updateGeometry();
        worldMapCallout->show();
    } else {
        worldMapCallout->hide();
    }
}

void MainWindow::on_checkBox_showBaselines_clicked(bool checked)
{
    auto series = worldmap->chart()->series();
    for(int i=0; i<series.count(); ++i){
        QString name = series.at(i)->name();
        if(name.size() >=5 && name.left(5) == "coast"){
            continue;
        }
        if(name == "selectedStations"){
            continue;
        }
        if(name == "availableStations"){
            continue;
        }
        if(checked){
            series.at(i)->setVisible(true);
        }else{
            series.at(i)->setVisible(false);
        }
    }
}

// ########################################### SOURCE ###########################################

void MainWindow::readSources()
{

    QString sourcePath;
    if(ui->radioButton_browseSource->isChecked()){
        sourcePath = ui->lineEdit_pathSource->text();
    }else{
        sourcePath = ui->lineEdit_browseSource2->text();
    }

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

            ui->comboBox_setupSource->blockSignals(true);
            allSourceModel->insertRow(allSourceModel->rowCount());
            allSourceModel->setData(allSourceModel->index(allSourceModel->rowCount()-1,0), sourceName);
            allSourceModel->item(allSourceModel->rowCount()-1,0)->setIcon(QIcon(":/icons/icons/source.png"));
            allSourceModel->setData(allSourceModel->index(allSourceModel->rowCount()-1, 1), (double)((int)(ra*100 +0.5))/100.0);
            allSourceModel->setData(allSourceModel->index(allSourceModel->rowCount()-1, 2), (double)((int)(de*100 +0.5))/100.0);

            selectedSourceModel->insertRow(selectedSourceModel->rowCount());
            selectedSourceModel->setData(selectedSourceModel->index(selectedSourceModel->rowCount()-1,0), sourceName);
            selectedSourceModel->item(selectedSourceModel->rowCount()-1,0)->setIcon(QIcon(":/icons/icons/source.png"));
            selectedSourceModel->setData(selectedSourceModel->index(selectedSourceModel->rowCount()-1, 1), (double)((int)(ra*100 +0.5))/100.0);
            selectedSourceModel->setData(selectedSourceModel->index(selectedSourceModel->rowCount()-1, 2), (double)((int)(de*100 +0.5))/100.0);
            ui->comboBox_setupSource->blockSignals(false);
            ui->comboBox_setupSource->setCurrentIndex(0);


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
}

void MainWindow::on_treeView_allSelectedSources_clicked(const QModelIndex &index)
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

void MainWindow::on_treeView_allAvailabeSources_clicked(const QModelIndex &index)
{
    int row = index.row();
    QString name = allSourceProxyModel->index(row,0).data().toString();

    if(selectedSourceModel->findItems(name).isEmpty()){


        selectedSourceModel->insertRow(0);

        int nrow = allSourceModel->findItems(name).at(0)->row();
        for(int i=0; i<allSourceModel->columnCount(); ++i){
            selectedSourceModel->setItem(0, i, allSourceModel->item(nrow,i)->clone() );
        }

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
    ui->lineEdit_allStationsFilter_3->setFocus();
    ui->lineEdit_allStationsFilter_3->selectAll();
}

void MainWindow::on_lineEdit_allStationsFilter_3_textChanged(const QString &arg1)
{
    allSourceProxyModel->setFilterRegExp(arg1);
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

    QString text = QString("%1 \nra: %2 [deg] \ndec: %3 [deg] ").arg(name).arg(ra+180).arg(dc);
    skyMapCallout->setText(text);
    skyMapCallout->setAnchor(QPointF(x,y));
    skyMapCallout->setZValue(11);
    skyMapCallout->updateGeometry();
    skyMapCallout->show();
}

void MainWindow::on_treeView_allSelectedSources_entered(const QModelIndex &index)
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


            QString text = QString("%1 \nra: %2 [deg] \ndec: %3 [deg] ").arg(name).arg(ra+180).arg(dc);
            skyMapCallout->setText(text);
            skyMapCallout->setAnchor(QPointF(x,y));
            skyMapCallout->setZValue(11);
            skyMapCallout->updateGeometry();
            skyMapCallout->show();
            break;
        }
    }
}

void MainWindow::plotSkyMap(){

    QChart *skyChart = skymap->chart();


    availableSources = new QScatterSeries(skyChart);
    availableSources->setName("available");
    availableSources->setColor(Qt::red);
    availableSources->setMarkerSize(10);

    selectedSources = new QScatterSeries(skyChart);
    selectedSources->setName("selected");
    markerSkymap();

    skyChart->addSeries(availableSources);
    skyChart->addSeries(selectedSources);

    connect(availableSources,SIGNAL(hovered(QPointF,bool)),this,SLOT(skymap_hovered(QPointF,bool)));
    connect(selectedSources,SIGNAL(hovered(QPointF,bool)),this,SLOT(skymap_hovered(QPointF,bool)));

    for(int i = 0; i< allSourceModel->rowCount(); ++i){
        double ra = allSourceModel->item(i,1)->text().toDouble();
        double lambda = qDegreesToRadians(ra);

        double dc = allSourceModel->item(i,2)->text().toDouble();
        double phi = qDegreesToRadians(dc);

        auto xy = qtUtil::radec2xy(lambda, phi);

        availableSources->append(xy.first, xy.second);
        selectedSources->append(xy.first, xy.second);
    }

    availableSources->attachAxis(skyChart->axisX());
    availableSources->attachAxis(skyChart->axisY());
    selectedSources->attachAxis(skyChart->axisX());
    selectedSources->attachAxis(skyChart->axisY());

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
        QTreeWidgetItem *itm = new QTreeWidgetItem();
        itm->setText(0,QString::fromStdString(stdname));
        itm->setCheckState(0,Qt::Unchecked);
        ui->treeWidget_srcGroupForStatistics->addTopLevelItem(itm);
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

void MainWindow::on_pushButton_parameterSource_edit_clicked()
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
    dial->addSelectedParameters(paraSrc[ui->ComboBox_parameterSource->currentText().toStdString()],ui->ComboBox_parameterSource->currentText());

    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::pair<std::string, VieVS::ParameterSettings::ParametersSources> res = dial->getParameters();
        std::string name = res.first;
        VieVS::ParameterSettings::ParametersSources parameter = res.second;

        paraSrc[name] = parameter;

    }
    delete(dial);
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

void MainWindow::on_pushButton_addSetupSource_clicked()
{
    addSetup(ui->treeWidget_setupSource, ui->DateTimeEdit_startParameterSource, ui->DateTimeEdit_endParameterSource,
             ui->comboBox_parameterSourceTransition, ui->comboBox_sourceSettingMember, ui->ComboBox_parameterSource,
             setupSourceTree, setupSource, ui->comboBox_setupSource);
}

void MainWindow::on_pushButton_removeSetupSource_clicked()
{
    deleteSetupSelection(setupSourceTree, setupSource, ui->comboBox_setupSource, ui->treeWidget_setupSource);
}

void MainWindow::on_treeWidget_setupSource_itemEntered(QTreeWidgetItem *item, int column)
{
    if(column == 0){
        displaySourceSetupMember(item->text(column));
    }else if(column == 1){
        displaySourceSetupParameter(item->text(column));
    }
}

void MainWindow::on_comboBox_sourceSettingMember_currentTextChanged(const QString &arg1)
{
    displaySourceSetupMember(arg1);
}

void MainWindow::displaySourceSetupParameterFromPlot(QPointF point, bool flag){
    QLineSeries* series = qobject_cast<QLineSeries*>(sender());
    QString name = series->name();
    displaySourceSetupParameter(name);
    if(flag){
        sourceSetupCallout->setAnchor(point);
        QDateTime st = QDateTime::fromMSecsSinceEpoch(series->at(0).x());
        QDateTime et = QDateTime::fromMSecsSinceEpoch(series->at(series->count()-1).x());
        QString txt = QString("Parameter: ").append(name);
        txt.append("\nfrom: ").append(st.toString("dd.MM.yyyy hh:mm"));
        txt.append("\nuntil: ").append(et.toString("dd.MM.yyyy hh:mm"));
        sourceSetupCallout->setText(txt);
        sourceSetupCallout->setZValue(11);
        sourceSetupCallout->updateGeometry();
        sourceSetupCallout->show();
    }else{
        sourceSetupCallout->hide();
    }


}

void MainWindow::on_ComboBox_parameterSource_currentTextChanged(const QString &arg1)
{
    displaySourceSetupParameter(arg1);
}

void MainWindow::on_comboBox_setupSource_currentTextChanged(const QString &arg1)
{
    drawSetupPlot(setupSource, ui->comboBox_setupSource, ui->treeWidget_setupSource);
    if(!arg1.isEmpty()){
        displaySourceSetupMember(arg1);
    }
}

void MainWindow::on_pushButton_13_clicked()
{
    ui->comboBox_setupSource->blockSignals(true);

    for(int i=0; i<allSourceModel->rowCount(); ++i){
        on_treeView_allAvailabeSources_clicked(allSourceModel->index(i,0));
    }
    ui->comboBox_setupSource->blockSignals(false);
    ui->comboBox_setupSource->setCurrentIndex(0);
}

void MainWindow::on_pushButton_15_clicked()
{
    selectedSourceModel->removeRows(0, selectedSourceModel->rowCount());
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

void MainWindow::sourceListChanged()
{
    int size = selectedSourceModel->rowCount();
    ui->label_sourceList_selected->setText(QString("selected: %1").arg(size));
}

void MainWindow::markerSkymap()
{
    if(ui->radioButton_markerSkymap->isChecked()){
        selectedSources->setMarkerSize(ui->horizontalSlider_markerSkymap->value());
        selectedSources->setBrush(QBrush(Qt::darkGreen,Qt::SolidPattern));
        selectedSources->setPen(QColor(Qt::white));
    }else{
        QImage img(":/icons/icons/source_white.png");
        img = img.scaled(ui->horizontalSlider_markerSkymap->value(),ui->horizontalSlider_markerSkymap->value());
        selectedSources->setBrush(QBrush(img));
        selectedSources->setMarkerSize(ui->horizontalSlider_markerSkymap->value());
        selectedSources->setPen(QColor(Qt::transparent));
    }

}

void MainWindow::on_radioButton_imageSkymap_toggled(bool checked)
{
    if(checked){
        ui->horizontalSlider_markerSkymap->setValue(30);
        auto series = skymap->chart()->series();
        QScatterSeries *tmp;
        for(const auto&serie : series){
            if(serie->name() == "selected"){
                tmp = qobject_cast<QScatterSeries *>(serie);
            }
        }
        tmp->setPen(QPen(QBrush(Qt::darkGreen),3,Qt::DashLine));

    }else{
        ui->horizontalSlider_markerSkymap->setValue(15);
        auto series = skymap->chart()->series();
        QScatterSeries *tmp;
        for(const auto&serie : series){
            if(serie->name() == "selected"){
                tmp = qobject_cast<QScatterSeries *>(serie);
            }
        }
        tmp->setPen(QPen(QBrush(Qt::blue),3,Qt::DashLine));
        selectedSources->setPen(QColor(Qt::transparent));
    }
    markerSkymap();
}

void MainWindow::on_checkBox_showEcliptic_clicked(bool checked)
{
    if(checked){
        auto series = skymap->chart()->series();
        series.back()->setVisible(true);
    }else{
        auto series = skymap->chart()->series();
        series.back()->setVisible(false);
    }
}


// ########################################### WEIGHT FACTORS ###########################################

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

// ########################################### RULES ###########################################

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

// ########################################### MULTI SCHED ###########################################

void MainWindow::createMultiSchedTable()
{

    QTreeWidget *t = ui->treeWidget_multiSched;

    t->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    t->expandAll();
    ui->treeWidget_multiSchedSelected->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    for(int i=0; i<ui->treeWidget_multiSched->topLevelItemCount(); ++i){
        for(int j=0; j<ui->treeWidget_multiSched->topLevelItem(i)->childCount(); ++j){
            ui->treeWidget_multiSched->topLevelItem(i)->child(j)->setDisabled(false);
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

            QStringList row2dateTimeDialog {"session start"};

            QStringList row2toggle{"subnetting",
                                   "fillinmode during scan selection",
                                   "fillinmode influence on scan selection",
                                   "fillinmode a posteriori"};

            QStringList row2intDialog {"max slew time",
                                       "max wait time",
                                       "max scan time",
                                       "min scan time",
                                       "min number of stations",
                                       "min repeat time",
                                       "idle time interval",
                                       "max number of scans"};

            QStringList row2doubleDialog {"subnetting min source angle",
                                          "subnetting min participating stations",
                                          "sky coverage",
                                          "number of observations",
                                          "duration",
                                          "average stations",
                                          "average sources",
                                          "average baselines",
                                          "idle time",
                                          "low declination",
                                          "low declination begin",
                                          "low declination full",
                                          "low elevation",
                                          "low elevation begin",
                                          "low elevation full",
                                          "influence distance",
                                          "influence time",
                                          "weight",
                                          "min slew distance",
                                          "max slew distance",
                                          "min elevation",
                                          "min flux",
                                          "min sun distance"};

            QIcon ic;
            if(parameterType == "General"){
                ic = QIcon(":/icons/icons/applications-internet-2.png");
            }else if(parameterType == "Weight factor"){
                ic = QIcon(":/icons/icons/weight.png");
            }else if(parameterType == "Sky Coverage"){
                ic = QIcon(":/icons/icons/sky_coverage.png");
            }else if(parameterType == "Station"){
                ic = QIcon(":/icons/icons/station.png");
            }else if(parameterType == "Source"){
                ic = QIcon(":/icons/icons/source.png");
            }else if(parameterType == "Baseline"){
                ic = QIcon(":/icons/icons/baseline.png");
            }

            auto t = ui->treeWidget_multiSchedSelected;

            QTreeWidgetItem *itm = new QTreeWidgetItem();

            if(row2toggle.indexOf(name) != -1){
                if(parameterType == "General" || parameterType == "Weight factor" || parameterType == "Sky Coverage"){
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
                    if(parameterType == "General" || parameterType == "Weight factor" || parameterType == "Sky Coverage"){
                        any->setDisabled(true);
                    }
                    QVector<int> val = dialog->getValues();
                    int n = val.size();
                    if(parameterType == "Station" || parameterType == "Source" || parameterType == "Baseline"){
                        QStandardItem* member = dialog->getMember();
                        itm->setText(1,member->text());
                        itm->setIcon(1,member->icon());
                    }else if(parameterType == "Weight factor"){
                        itm->setText(1,"global");
                        itm->setIcon(1,QIcon(":/icons/icons/weight.png"));
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
                }else if(parameterType == "Weight factor"){
                    itm->setText(1,"global");
                    itm->setIcon(1,QIcon(":/icons/icons/weight.png"));
                }
                int result = dialog->exec();
                if(result == QDialog::Accepted){
                    if(parameterType == "General" || parameterType == "Weight factor" || parameterType == "Sky Coverage"){
                        any->setDisabled(true);
                    }
                    QVector<double> val = dialog->getValues();
                    int n = val.size();

                    if(parameterType == "Station" || parameterType == "Source" || parameterType == "Baseline"){
                        QStandardItem* member = dialog->getMember();
                        itm->setText(1,member->text());
                        itm->setIcon(1,member->icon());
                    }else if(parameterType == "Sky Coverage"){
                        itm->setText(1,"global");
                        itm->setIcon(1,QIcon(":/icons/icons/sky_coverage.png"));
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
                    if(parameterType == "General" || parameterType == "Weight factor" || parameterType == "Sky Coverage"){
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

            multi_sched_count_nsched();

        }
    }
}

void MainWindow::on_pushButton_25_clicked()
{
    auto list = ui->treeWidget_multiSchedSelected->selectedItems();{
        for(const auto& any:list){
            if(any->text(0) == "session start"){
//                ui->treeWidget_multiSched->topLevelItem(0)->child(0)->setDisabled(false);
            }else if(any->text(0) == "subnetting"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(0)->setDisabled(false);
            }else if(any->text(0) == "subnetting min source angle"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(1)->setDisabled(false);
            }else if(any->text(0) == "subnetting min participating stations"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(2)->setDisabled(false);
            }else if(any->text(0) == "fillin mode during scan selection"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(3)->setDisabled(false);
            }else if(any->text(0) == "fillin mode influence on scan selection"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(4)->setDisabled(false);
            }else if(any->text(0) == "fillin mode a posteriori"){
                ui->treeWidget_multiSched->topLevelItem(0)->child(5)->setDisabled(false);

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
            }else if(any->text(0) == "average baselines"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(5)->setDisabled(false);
            }else if(any->text(0) == "idle time"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(6)->setDisabled(false);
            }else if(any->text(0) == "idle time interval"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(7)->setDisabled(false);
            }else if(any->text(0) == "low declination"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(8)->setDisabled(false);
            }else if(any->text(0) == "low declination begin"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(9)->setDisabled(false);
            }else if(any->text(0) == "low declination full"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(10)->setDisabled(false);
            }else if(any->text(0) == "low elevation"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(11)->setDisabled(false);
            }else if(any->text(0) == "low elevation begin"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(12)->setDisabled(false);
            }else if(any->text(0) == "low elevation full"){
                ui->treeWidget_multiSched->topLevelItem(1)->child(13)->setDisabled(false);

            }else if(any->text(0) == "influence distance"){
                ui->treeWidget_multiSched->topLevelItem(2)->child(0)->setDisabled(false);
            }else if(any->text(0) == "influence time"){
                ui->treeWidget_multiSched->topLevelItem(2)->child(1)->setDisabled(false);
            }
            delete(any);
        }
    }

    multi_sched_count_nsched();

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

void MainWindow::on_comboBox_multiSched_maxNumber_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "all"){
        ui->spinBox_multiSched_maxNumber->setEnabled(false);
        ui->comboBox_multiSched_seed->setEnabled(false);
        ui->label_multiSched_seed->setEnabled(false);
        ui->spinBox_multiSched_seed->setEnabled(false);
    } else {
        ui->spinBox_multiSched_maxNumber->setEnabled(true);
        ui->comboBox_multiSched_seed->setEnabled(true);
        ui->label_multiSched_seed->setEnabled(true);
        on_comboBox_multiSched_seed_currentIndexChanged(ui->comboBox_multiSched_seed->currentText());
    }
}

void MainWindow::on_comboBox_multiSched_seed_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "random"){
        ui->spinBox_multiSched_seed->setEnabled(false);
    } else {
        ui->spinBox_multiSched_seed->setEnabled(true);
    }
}

void MainWindow::multi_sched_count_nsched()
{

    auto t = ui->treeWidget_multiSchedSelected;

    int nsched = 1;
    double wsky_ = 0;
    if(ui->checkBox_weightCoverage->isChecked()){
        wsky_ = ui->doubleSpinBox_weightSkyCoverage->value();
    }
    double wobs_ = 0;
    if(ui->checkBox_weightNobs->isChecked()){
        wobs_ = ui->doubleSpinBox_weightNumberOfObservations->value();
    }
    double wdur_ = 0;
    if(ui->checkBox_weightDuration->isChecked()){
        wdur_ = ui->doubleSpinBox_weightDuration->value();
    }
    double wasrc_ = 0;
    if(ui->checkBox_weightAverageSources->isChecked()){
        wasrc_ = ui->doubleSpinBox_weightAverageSources->value();
    }
    double wasta_ = 0;
    if(ui->checkBox_weightAverageStations->isChecked()){
        wasta_ = ui->doubleSpinBox_weightAverageStations->value();
    }
    double wabls_ = 0;
    if(ui->checkBox_weightAverageBaselines->isChecked()){
        wabls_ = ui->doubleSpinBox_weightAverageBaselines->value();
    }
    double widle_ = 0;
    if(ui->checkBox_weightIdleTime->isChecked()){
        widle_ = ui->doubleSpinBox_weightIdleTime->value();
    }
    double wdec_ = 0;
    if(ui->checkBox_weightLowDeclination->isChecked()){
        wdec_ = ui->doubleSpinBox_weightLowDec->value();
    }
    double wel_ = 0;
    if(ui->checkBox_weightLowElevation->isChecked()){
        wel_ = ui->doubleSpinBox_weightLowEl->value();
    }


    std::map<std::string,std::vector<double>> weightFactors = {{"weight_factor_sky_coverage",std::vector<double>{wsky_}},
                                                    {"weight_factor_number_of_observations",std::vector<double>{wobs_}},
                                                    {"weight_factor_duration",std::vector<double>{wdur_}},
                                                    {"weight_factor_average_sources",std::vector<double>{wasrc_}},
                                                    {"weight_factor_average_stations",std::vector<double>{wasta_}},
                                                    {"weight_factor_average_baselines",std::vector<double>{wabls_}},
                                                    {"weight_factor_idle_time",std::vector<double>{widle_}},
                                                    {"weight_factor_low_declination",std::vector<double>{wdec_}},
                                                    {"weight_factor_low_elevation",std::vector<double>{wel_}}};

    bool weigthFactorFound = false;
    for(int i = 0; i<t->topLevelItemCount(); ++i){
        if(t->topLevelItem(i)->text(0) == "sky coverage"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_sky_coverage"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "number of observations"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_number_of_observations"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "duration"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_duration"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "average stations"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_average_stations"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "average baselines"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_average_baselines"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "average sources"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_average_sources"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "idle time"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_idle_time"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "low declination"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_low_declination"] = values;
            weigthFactorFound = true;
        }else if(t->topLevelItem(i)->text(0) == "low elevation"){
            QComboBox *list = qobject_cast<QComboBox*>(t->itemWidget(t->topLevelItem(i),3));
            std::vector<double> values;
            for(int ilist = 0; ilist<list->count(); ++ilist){
                values.push_back( QString(list->itemText(ilist)).toDouble());
            }
            weightFactors["weight_factor_low_elevation"] = values;
            weigthFactorFound = true;
        }
    }

    std::vector<std::vector<double> > weightFactorValues;
    if(weigthFactorFound){
        for (double wsky: weightFactors["weight_factor_sky_coverage"]) {
            for (double wobs: weightFactors["weight_factor_number_of_observations"]) {
                for (double wdur: weightFactors["weight_factor_duration"]) {
                    for (double wasrc: weightFactors["weight_factor_average_sources"]) {
                        for (double wasta: weightFactors["weight_factor_average_stations"]) {
                            for (double wabls: weightFactors["weight_factor_average_baselines"]) {
                                for (double widle: weightFactors["weight_factor_idle_time"]) {
                                    for (double wdec: weightFactors["weight_factor_low_declination"]) {
                                        for (double wel: weightFactors["weight_factor_low_elevation"]) {

                                            double sum = wsky + wobs + wdur + wasrc + wasta + wabls + widle + wdec + wel;

                                            if (sum == 0) {
                                                continue;
                                            }

                                            std::vector<double> wf{wsky/sum, wobs/sum, wdur/sum, wasrc/sum, wasta/sum,
                                                                   wabls/sum, widle/sum, wdec/sum, wel/sum};
                                            weightFactorValues.push_back(std::move(wf));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // remove duplicated weight factors
    int i1 = 0;
    while (i1 < weightFactorValues.size()) {
        const std::vector<double> &v1 = weightFactorValues[i1];
        int i2 = i1 + 1;

        while (i2 < weightFactorValues.size()) {
            const std::vector<double> &v2 = weightFactorValues[i2];
            int equal = 0;
            for (int i3 = 0; i3 < v1.size(); ++i3) {
                if (abs(v1[i3] - v2[i3]) < 1e-10) {
                    ++equal;
                }
            }
            if (equal == v1.size()) {
                weightFactorValues.erase(next(weightFactorValues.begin(), i2));
            } else {
                ++i2;
            }
        }
        ++i1;
    }

    if (!weightFactorValues.empty()) {
        nsched = weightFactorValues.size();
    }

    QStringList weightFactorsStr {"sky coverage",
                                  "number of observations",
                                  "duration",
                                  "average stations",
                                  "average sources",
                                  "average baselines",
                                  "idle time",
                                  "low declination",
                                  "low elevation"};

    for(int i = 0; i<t->topLevelItemCount(); ++i){
        if(weightFactorsStr.indexOf(t->topLevelItem(i)->text(0)) != -1){
            continue;
        }
        nsched *= t->topLevelItem(i)->text(2).toInt();
    }
    ui->label_multiSchedulingNsched->setText(QString::number(nsched));
    if(nsched>999){
        ui->spinBox_multiSched_maxNumber->setValue(999);
        ui->comboBox_multiSched_maxNumber->setCurrentIndex(1);
        ui->comboBox_multiSched_maxNumber->setEnabled(false);
    }else{
        ui->spinBox_multiSched_maxNumber->setValue(nsched);
        ui->comboBox_multiSched_maxNumber->setEnabled(true);
    }

    if(nsched >9999){
        QMessageBox::warning(this,"ignoring multi scheduling","Too many possible multi scheduling parameters!\nMulti scheduling will be ignored");
    }
}

// ########################################### SAVE DEFAULT PARAMETERS ###########################################

void MainWindow::changeDefaultSettings(QStringList path, QStringList value, QString name)
{
    for(int i=0; i<path.count(); ++i){
        settings.put(path.at(i).toStdString(),value.at(i).toStdString());
    }
    std::ofstream os;
    os.open("settings.xml");
    boost::property_tree::xml_parser::write_xml(os, settings,
                                                boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
    os.close();
    QMessageBox::information(this,"Default settings changed",name);
}

void MainWindow::on_pushButton_5_clicked()
{    
    QStringList path {"settings.general.name"};
    QStringList value {ui->nameLineEdit->text()};
    QString name = "Default user name changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_6_clicked()
{    
    QStringList path {"settings.general.email"};
    QStringList value {ui->emailLineEdit->text()};
    QString name = "Default user email address changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_17_clicked()
{    
    QStringList path {"settings.general.pathToScheduler"};
    QStringList value {ui->pathToSchedulerLineEdit->text()};
    QString name = "Default path to scheduler executable changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_saveCatalogPathes_clicked()
{
    settings.put("settings.catalog_path.antenna",ui->lineEdit_pathAntenna->text().toStdString());
    settings.put("settings.catalog_path.equip",ui->lineEdit_pathEquip->text().toStdString());
    settings.put("settings.catalog_path.position",ui->lineEdit_pathPosition->text().toStdString());
    settings.put("settings.catalog_path.mask",ui->lineEdit_pathMask->text().toStdString());
    settings.put("settings.catalog_path.source",ui->lineEdit_pathSource->text().toStdString());
    settings.put("settings.catalog_path.source2",ui->lineEdit_browseSource2->text().toStdString());
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
    QStringList path {"settings.output.directory"};
    QStringList value {ui->lineEdit_outputPath->text()};
    QString name = "Default output path changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_23_clicked()
{
    QStringList path {"settings.output.scheduler"};
    QStringList value {ui->schedulerLineEdit->text()};
    QString name = "Default scheduler changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_22_clicked()
{
    QStringList path {"settings.output.correlator"};
    QStringList value {ui->correlatorLineEdit->text()};
    QString name = "Default correlator changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_7_clicked()
{
    QStringList path {"settings.mode.skdMode"};
    QStringList value {ui->comboBox_skedObsModes->currentText()};
    QString name = "Default skd observing mode changed";
    changeDefaultSettings(path,value,name);

}

void MainWindow::on_pushButton_8_clicked()
{
    QStringList path {"settings.general.subnetting"};
    QStringList value;
    if(ui->groupBox_subnetting->isChecked()){
        value << "true";
    }else{
        value << "false";
    }

    path << "settings.general.subnettingMinAngle";
    value << QString::number(ui->doubleSpinBox_subnettingDistance->value());

    path << "settings.general.subnettingMinNSta";
    value << QString::number(ui->doubleSpinBox_subnettingMinStations->value());
    path << "settings.general.subnettingMinNStaPercent";
    value << QString::number(ui->doubleSpinBox_subnettingMinStations->value());
    path << "settings.general.subnettingMinNStaAllBut";
    value << QString::number(ui->spinBox_subnetting_min_sta->value());
    path << "settings.general.subnettingMinNstaPercent_otherwiseAllBut";
    if(ui->radioButton_subnetting_percent->isChecked()){
        value << "true";
    }else{
        value << "false";
    }

    path << "settings.general.fillinmodeInfluenceOnSchedule";
    if(ui->checkBox_fillinModeInfluence->isChecked()){
        value << "true";
    }else{
        value << "false";
    }

    path << "settings.general.fillinmodeAPosteriori";
    if(ui->checkBox_fillinmode_aposteriori->isChecked()){
        value << "true";
    }else{
        value << "false";
    }

    path << "settings.general.fillinmodeDuringScanSelection";
    if(ui->checkBox_fillinmode_duringscan->isChecked()){
        value << "true";
    }else{
        value << "false";
    }

    path << "settings.general.alignObservingTime";
    if(ui->radioButton_alignStart->isChecked()){
        value << "start";
    }else if(ui->radioButton_alignEnd->isChecked()){
        value << "end";
    }else{
        value << "individual";
    }

    QString name = "Default general parameters changed!";
    changeDefaultSettings(path,value,name);

}

void MainWindow::on_pushButton_9_clicked()
{
    QStringList path;
    QStringList value;

    path << "settings.weightFactor.skyCoverageChecked";
    if(ui->checkBox_weightCoverage->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.skyCoverage";
    value << QString("%1").arg(ui->doubleSpinBox_weightSkyCoverage->value());

    path << "settings.weightFactor.numberOfObservationsChecked";
    if(ui->checkBox_weightNobs->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.numberOfObservations";
    value << QString("%1").arg(ui->doubleSpinBox_weightNumberOfObservations->value());

    path << "settings.weightFactor.durationChecked";
    if(ui->checkBox_weightDuration->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.duration";
    value << QString("%1").arg(ui->doubleSpinBox_weightDuration->value());

    path << "settings.weightFactor.averageSourcesChecked";
    if(ui->checkBox_weightAverageSources->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.averageSources";
    value << QString("%1").arg(ui->doubleSpinBox_weightAverageSources->value());

    path << "settings.weightFactor.averageStationsChecked";
    if(ui->checkBox_weightAverageStations->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.averageStations";
    value << QString("%1").arg(ui->doubleSpinBox_weightAverageStations->value());

    path << "settings.weightFactor.idleTimeChecked";
    if(ui->checkBox_weightIdleTime->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.weightIdleTime";
    value << QString("%1").arg(ui->doubleSpinBox_weightIdleTime->value());
    path << "settings.weightFactor.idleTimeInterval";
    value << QString("%1").arg(ui->spinBox_idleTimeInterval->value());


    path << "settings.weightFactor.weightDeclinationChecked";
    if(ui->checkBox_weightLowDeclination->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.weightDeclination";
    value << QString("%1").arg(ui->doubleSpinBox_weightLowDec->value());
    path << "settings.weightFactor.declinationStartWeight";
    value << QString("%1").arg(ui->doubleSpinBox_weightLowDecStart->value());
    path << "settings.weightFactor.declinationFullWeight";
    value << QString("%1").arg(ui->doubleSpinBox_weightLowDecEnd->value());

    path << "settings.weightFactor.weightLowElevationChecked";
    if(ui->checkBox_weightLowElevation->isChecked()){
        value << "true";
    }else{
        value << "false";
    }
    path << "settings.weightFactor.weightLowElevation";
    value << QString("%1").arg(ui->doubleSpinBox_weightLowEl->value());
    path << "settings.weightFactor.lowElevationStartWeight";
    value << QString("%1").arg(ui->doubleSpinBox_weightLowElStart->value());
    path << "settings.weightFactor.lowElevationFullWeight";
    value << QString("%1").arg(ui->doubleSpinBox_weightLowElEnd->value());

    QString name = "Default weight factors changed!";
    changeDefaultSettings(path,value,name);

}

void MainWindow::on_pushButton_10_clicked()
{
    QStringList path;
    QStringList value;

    path << "settings.skyCoverage.influenceDistance";
    value << QString("%1").arg(ui->influenceDistanceDoubleSpinBox->value());
    path << "settings.skyCoverage.influenceInterval";
    value << QString("%1").arg(ui->influenceTimeSpinBox->value());
    path << "settings.skyCoverage.distanceType";
    value << ui->comboBox_skyCoverageDistanceType->currentText();
    path << "settings.skyCoverage.timeType";
    value << ui->comboBox_skyCoverageTimeType->currentText();
    path << "settings.skyCoverage.maxTwinTelecopeDistance";
    value << QString("%1").arg(ui->maxDistanceForCombiningAntennasDoubleSpinBox->value());

    QString name = "Default sky coverage parametrization changed!";
    changeDefaultSettings(path,value,name);

}

void MainWindow::on_pushButton_11_clicked()
{
    QStringList path;
    QStringList value;

    path << "settings.station.waitTimes.fieldSystem";
    value << QString("%1").arg(ui->SpinBox_fieldSystem->value());
    path << "settings.station.waitTimes.preob";
    value << QString("%1").arg(ui->SpinBox_preob->value());
    path << "settings.station.waitTimes.midob";
    value << QString("%1").arg(ui->SpinBox_midob->value());
    path << "settings.station.waitTimes.postob";
    value << QString("%1").arg(ui->SpinBox_postob->value());

    QString name = "Default wait times changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_12_clicked()
{
    QStringList path;
    QStringList value;

    path << "settings.station.cableWrapBuffers.axis1LowOffset";
    value << QString("%1").arg(ui->DoubleSpinBox_axis1low->value());
    path << "settings.station.cableWrapBuffers.axis1UpOffset";
    value << QString("%1").arg(ui->DoubleSpinBox_axis1up->value());
    path << "settings.station.cableWrapBuffers.axis2LowOffset";
    value << QString("%1").arg(ui->DoubleSpinBox_axis2low->value());
    path << "settings.station.cableWrapBuffers.axis2UpOffset";
    value << QString("%1").arg(ui->DoubleSpinBox_axis2up->value());

    QString name = "Default cable wrap buffers changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_31_clicked()
{
    QStringList path {"settings.output.piName"};
    QStringList value {ui->lineEdit_PIName->text()};
    QString name = "Default pi name changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_29_clicked()
{
    QStringList path {"settings.output.piEmail"};
    QStringList value {ui->lineEdit_PIEmail->text()};
    QString name = "Default pi email changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_28_clicked()
{
    QStringList path {"settings.output.contactName"};
    QStringList value {ui->lineEdit_contactName->text()};
    QString name = "Default contact name changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_30_clicked()
{
    QStringList path {"settings.output.contactEmail"};
    QStringList value {ui->lineEdit_contactEmail->text()};
    QString name = "Default contact email changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_41_clicked()
{
    QStringList path {"settings.output.notes"};
    QStringList value {ui->plainTextEdit_notes->toPlainText().replace("\n","\\n")};
    QString name = "Default notes changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_40_clicked()
{
    QStringList path {"settings.output.operationNotes"};
    QStringList value {ui->plainTextEdit_operationNotes->toPlainText().replace("\n","\\n")};
    QString name = "Default operation notes changed!";
    changeDefaultSettings(path,value,name);
}

void MainWindow::on_pushButton_save_multiCore_clicked()
{
    QString threads = ui->comboBox_nThreads->currentText();
    QString nThreadsManual = QString::number(ui->spinBox_nCores->value());
    QString jobScheduler = ui->comboBox_jobSchedule->currentText();
    QString chunkSize = QString::number(ui->spinBox_chunkSize->value());

    QStringList path {"settings.multiCore.threads", "settings.multiCore.nThreads", "settings.multiCore.jobScheduling", "settings.multiCore.chunkSize"};
    QStringList value {threads, nThreadsManual, jobScheduler, chunkSize};
    QString name = "Default multi core settings changed!";
    changeDefaultSettings(path,value,name);

}

// ########################################### GUI UTILITY ###########################################

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

// ########################################### STATISTICS ###########################################

void MainWindow::setupStatisticView()
{
    auto hv1 = ui->treeWidget_statisticGeneral->header();
    hv1->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto hv2 = ui->treeWidget_statisticStation->header();
    hv2->setSectionResizeMode(QHeaderView::ResizeToContents);
    auto hv3 = ui->treeWidget_statisticHovered->header();
    hv3->setSectionResizeMode(QHeaderView::ResizeToContents);

    statisticsView = new QChartView(this);
    statisticsView->setToolTip("version comparison");
    statisticsView->setStatusTip("version comparison");
    statisticsView->setChart(new QChart());
    ui->verticalLayout_statisticPlot->insertWidget(0,statisticsView,1);
    ui->horizontalScrollBar_statistics->setRange(0,0);
    ui->horizontalScrollBar_statistics->setSingleStep(1);

    for(int i=0; i<ui->treeWidget_statisticGeneral->topLevelItemCount(); ++i){
        auto db = new QDoubleSpinBox(ui->treeWidget_statisticGeneral);
        db->setMinimum(-99);
        ui->treeWidget_statisticGeneral->setItemWidget(ui->treeWidget_statisticGeneral->topLevelItem(i),2,db);
        connect(db,SIGNAL(valueChanged(double)),this,SLOT(plotStatistics()));
    }


    connect(ui->radioButton_statistics_absolute,SIGNAL(toggled(bool)),this,SLOT(plotStatistics()));
    connect(ui->checkBox_statistics_removeMinimum,SIGNAL(toggled(bool)),this,SLOT(plotStatistics()));

    ui->label_statistics_hoveredItem_title->setStyleSheet("font-weight: bold");

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

            startIdxNew = names.indexOf(QRegularExpression("n_obs.*"),startIdxNew);
            startIdxOld = statisticsName.indexOf(QRegularExpression("n_obs.*"),startIdxOld);

            for(int inew=startIdxNew; inew<names.count(); ++inew){
                QString thisItem = names.at(inew);
                if(thisItem.left(5) != "n_obs"){
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
                        if(thisItemOld.left(5) != "n_obs"){
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
        ui->treeWidget_statisticStation->addTopLevelItem(new QTreeWidgetItem(QStringList() << "# scans"));
        ui->treeWidget_statisticStation->addTopLevelItem(new QTreeWidgetItem(QStringList() << "# obs"));
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
        plotStatistics(true);
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


        plotStatistics(true);
    }
}

void MainWindow::plotStatistics(bool animation)
{

    try{

    ui->treeWidget_statisticGeneral->blockSignals(true);
    ui->treeWidget_statisticStation->blockSignals(true);

    int nsta=0;
    for(int i=8; i<statisticsName.count(); ++i){
        QString thisName = statisticsName.at(i);
        if(thisName.left(7) != "n_scans"){
            break;
        }
        ++nsta;
    }


    QMap<QString,int> translateGeneral;
    translateGeneral["# scans"] = 1;
    translateGeneral["# obs"] = 6;
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
            barSets.push_back(statisticsBarSet(idx,general->topLevelItem(i)->text(0)));
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
        QString prefix = station->topLevelItem(i)->text(0).append(" ");
        for(int j=0; j<station->topLevelItem(i)->childCount(); ++j){
            if(station->topLevelItem(i)->child(j)->checkState(0) == Qt::Checked){
                barSets.push_back(statisticsBarSet(8+i*nsta+j,prefix + station->topLevelItem(i)->child(j)->text(0)));
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
        QBarSet *sortedBarSet = new QBarSet(thisBarSet->label());
        sortedBarSet->setBrush(thisBarSet->brush());
        for(int j = 0; j<thisBarSet->count(); ++j){
            *sortedBarSet << thisBarSet->at(idx.at(j));
        }
        sortedSeries->append(sortedBarSet);
        delete(thisBarSet);
    }

    QChart *chart = statisticsView->chart();
    chart->removeAllSeries();
    chart->addSeries(sortedSeries);
    chart->setTitle("statistics");
    if(animation){
        chart->setAnimationOptions(QChart::SeriesAnimations);
    }else{
        chart->setAnimationOptions(QChart::NoAnimation);
    }

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

    statisticsView->setRenderHint(QPainter::Antialiasing);

    if(categories.count()>showN){
        ui->horizontalScrollBar_statistics->setRange(0,categories.size()-showN);
        ui->horizontalScrollBar_statistics->setSingleStep(1);
    }else{
        ui->horizontalScrollBar_statistics->setRange(0,0);
        ui->horizontalScrollBar_statistics->setSingleStep(1);
    }

    connect(sortedSeries,SIGNAL(hovered(bool,int,QBarSet*)),this,SLOT(statisticsHovered(bool,int,QBarSet*)));
    ui->treeWidget_statisticGeneral->blockSignals(false);
    ui->treeWidget_statisticStation->blockSignals(false);

    }catch(...){
        QMessageBox::warning(this,"keep it slow!","A Error occured! Maybe because you canged too many values too fast!");
        ui->treeWidget_statisticGeneral->blockSignals(false);
        ui->treeWidget_statisticStation->blockSignals(false);
    }

}

void MainWindow::statisticsHovered(bool status, int index, QBarSet *barset)
{
    if (status) {

        int nsta=0;
        for(int i=8; i<statisticsName.count(); ++i){
            QString thisName = statisticsName.at(i);
            if(thisName.left(7) != "n_scans"){
                break;
            }
            ++nsta;
        }
        QMap<QString,int> translateGeneral;
        translateGeneral["# scans"] = 1;
        translateGeneral["# obs"] = 6;
        translateGeneral["# stations"] = 7;
        translateGeneral["# sources"] = 8+2*nsta;
        translateGeneral["# single source scans"] = 2;
        translateGeneral["# subnetting scans"] = 3;
        translateGeneral["# fillin mode scans"] = 4;
        translateGeneral["# calibration scans"] = 5;


        auto axis = qobject_cast<QBarCategoryAxis*>(statisticsView->chart()->axisX());
        auto categories = axis->categories();
        QString catName = categories.at(index);
        QStringList splitCatName = catName.split(" ");
        int version = splitCatName.at(0).right(splitCatName.at(0).count()-1).toInt();
        QString name = splitCatName.at(1);
        int value = 0;
        if(translateGeneral.keys().indexOf(barset->label()) != -1){
            value = statistics[name][version][translateGeneral[barset->label()]];
        }else{
            int li = barset->label().lastIndexOf(" ");
            QString staName = barset->label().mid(li+1);
            if(barset->label().left(7) == "# scans"){
                int idx = statisticsName.indexOf("n_scans_"+staName);
                value = statistics[name][version][idx];
            }else{
                int idx = statisticsName.indexOf("n_obs_"+staName);
                value = statistics[name][version][idx];
            }
        }

        ui->label_statistics_hoveredItem_title->setText(barset->label());
        ui->treeWidget_statisticHovered->clear();
        ui->treeWidget_statisticHovered->addTopLevelItem(new QTreeWidgetItem(QStringList() << "session" << name));
        ui->treeWidget_statisticHovered->addTopLevelItem(new QTreeWidgetItem(QStringList() << "version" << QString("%1").arg(version)));
        ui->treeWidget_statisticHovered->addTopLevelItem(new QTreeWidgetItem(QStringList() << "value" << QString("%1").arg(value)));
    } else {
        ui->label_statistics_hoveredItem_title->setText("hovered item");
        ui->treeWidget_statisticHovered->clear();
    }
}

QBarSet *MainWindow::statisticsBarSet(int idx, QString name)
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

    QBarSet *set = new QBarSet(name);
    for(int i=0; i<v.count(); ++i){
        *set << v.at(i);
    }

    return set;
}

void MainWindow::on_treeWidget_statisticGeneral_itemChanged(QTreeWidgetItem *item, int column)
{
    plotStatistics(false);
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

    plotStatistics(false);
}

void MainWindow::on_horizontalScrollBar_statistics_valueChanged(int value)
{

    statisticsView->chart()->setAnimationOptions(QChart::NoAnimation);
    ui->label_statistics_hoveredItem_title->setText("hovered item");
    ui->treeWidget_statisticHovered->clear();

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
    ui->label_statistics_hoveredItem_title->setText("hovered item");
    ui->treeWidget_statisticHovered->clear();

    plotStatistics(false);
}

// ########################################### SKY COVERAGE ###########################################

void MainWindow::setupSkyCoverageTemplatePlot()
{
    plotSkyCoverageTemplate = true;
    skyCoverageTemplateView = new QChartView(this);
    skyCoverageTemplateView->setStatusTip("sky coverage example");
    skyCoverageTemplateView->setToolTip("sky coverage example");

    QPolarChart *chart = new QPolarChart();

    QValueAxis *angularAxis = new QValueAxis();
    angularAxis->setTickCount(13);
    angularAxis->setLabelFormat("%d");
    angularAxis->setShadesVisible(true);
    angularAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
    chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
    angularAxis->setRange(0, 361);


    QValueAxis *radialAxis = new QValueAxis();
    radialAxis->setTickCount(10);
    radialAxis->setLabelFormat("");
    chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);
    radialAxis->setRange(0, 90);


    chart->legend()->setVisible(false);
    chart->setTitle("possible sky coverage example");
    skyCoverageTemplateView->setChart(chart);
    skyCoverageTemplateView->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayout_54->insertWidget(0,skyCoverageTemplateView,2);

    connect(ui->spinBox_skyCoverageTemplateRandomObservations,SIGNAL(valueChanged(int)),this,SLOT(on_pushButton_skyCoverageTemplateRandom_clicked()));

    connect(ui->influenceDistanceDoubleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(skyCoverageTemplate()));
    connect(ui->horizontalSlider_skyCoverageMarkerSize,SIGNAL(valueChanged(int)),this,SLOT(skyCoverageTemplate()));
    connect(ui->horizontalSlider_skyCoverageMarkerDistance,SIGNAL(valueChanged(int)),this,SLOT(skyCoverageTemplate()));
    connect(ui->horizontalSlider_skyCoverageColorResultion,SIGNAL(valueChanged(int)),this,SLOT(skyCoverageTemplate()));

    connect(ui->comboBox_skyCoverageDistanceType,SIGNAL(currentIndexChanged(int)),this,SLOT(skyCoverageTemplate()));
    connect(ui->comboBox_skyCoverageTimeType,SIGNAL(currentIndexChanged(int)),this,SLOT(skyCoverageTemplate()));
    on_pushButton_skyCoverageTemplateRandom_clicked();

}

void MainWindow::skyCoverageTemplate()
{
    if(plotSkyCoverageTemplate){
        auto chart = skyCoverageTemplateView->chart();
        chart->removeAllSeries();

        QLineSeries *upper = new QLineSeries();
        QLineSeries *lower = new QLineSeries();
        double minElevation = 5;
        for(int az=0; az<=365; az+=5){
            upper->append(az,90);
            lower->append(az,90-minElevation);
        }

        QAreaSeries *area = new QAreaSeries();
        area->setName("minimum elevation");
        area->setUpperSeries(upper);
        area->setLowerSeries(lower);
        area->setBrush(QBrush(Qt::gray));
        area->setOpacity(0.7);

        chart->addSeries(area);

        area->attachAxis(chart->axisX());
        area->attachAxis(chart->axisY());

        QVector<double> V{0.00,0.05,0.10,0.15,0.20,0.25,0.30,0.35,0.40,0.45,0.50,0.55,0.60,0.65,0.70,0.75,0.80,0.85,0.90,0.95,1.00};
        QVector<double> R{62,68,71,72,67,52,45,37,28,4,18,48,72,113,159,200,234,254,250,245,249};
        QVector<double> G{38,51,67,85,103,122,140,156,170,182,190,197,203,205,201,193,186,193,212,232,251};
        QVector<double> B{168,204,231,246,253,253,243,231,223,206,185,162,134,100,66,41,48,58,46,37,21};
        int nColor = ui->horizontalSlider_skyCoverageColorResultion->value();
        double dist = 1.0/(nColor-1);

        QVector<double> Rq;
        QVector<double> Gq;
        QVector<double> Bq;
        for(int i=0; i<nColor; ++i){
            double vq = i*dist;
            Rq.append(interpolate(V,R,vq,false));
            Gq.append(interpolate(V,G,vq,false));
            Bq.append(interpolate(V,B,vq,false));
        }


        QVector<QScatterSeries*> ss;
        for(int i=0;i<nColor;++i){
            QScatterSeries *tss = new QScatterSeries();
            tss->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            tss->setMarkerSize(ui->horizontalSlider_skyCoverageMarkerSize->value());
            tss->setBrush(QBrush(QColor(Rq.at(i),Gq.at(i),Bq.at(i))));
            tss->setBorderColor(QColor(Rq.at(i),Gq.at(i),Bq.at(i)));
            ss.append(tss);
        }

        double d = (double)ui->horizontalSlider_skyCoverageMarkerDistance->value()/10;
        for (double el = 0; el <= 90; el+=d) {
            if(el<= minElevation){
                continue;
            }
            double zd = 90-el;

            double deltaAz;
            if(el!=90){
                deltaAz = d/qCos(qDegreesToRadians(el));
            }else{
                deltaAz = 360;
            }

            for (double az = 0; az < 360; az+=deltaAz) {

                double score = 1;
                for(int i=0; i<obsTime.count(); ++i){
                    int deltaTime = obsTime.at(0)-obsTime.at(i);

                    double el1 = qDegreesToRadians(el);
                    double el2 = qDegreesToRadians(obsEl.at(i));
                    double az1 = qDegreesToRadians(az);
                    double az2 = qDegreesToRadians(obsAz.at(i));

                    double tmp = (qSin(el1) * qSin(el2) + qCos(el1) * qCos(el2) * qCos(az2-az1));
                    double deltaDistance = qRadiansToDegrees(qAcos(tmp));
                    double scoreDistance;
                    double scoreTime;

                    if(deltaDistance >= ui->influenceDistanceDoubleSpinBox->value()){
                        scoreDistance = 0;
                    }else if(ui->comboBox_skyCoverageDistanceType->currentText() == "cosine"){
                        scoreDistance = .5+.5*qCos(deltaDistance * M_PI / ui->influenceDistanceDoubleSpinBox->value());
                    }else if(ui->comboBox_skyCoverageDistanceType->currentText() == "linear"){
                        scoreDistance = 1-deltaDistance/ui->influenceDistanceDoubleSpinBox->value();
                    }else{
                        scoreDistance = 1;
                    }

                    if(deltaTime >= ui->influenceTimeSpinBox->value()){
                        scoreTime = 0;
                    }else if(ui->comboBox_skyCoverageTimeType->currentText() == "cosine"){
                        scoreTime = .5+.5*qCos(deltaTime * M_PI / ui->influenceTimeSpinBox->value());
                    }else if(ui->comboBox_skyCoverageTimeType->currentText() == "linear"){
                        scoreTime = 1-(double)deltaTime/(double)ui->influenceTimeSpinBox->value();
                    }else{
                        scoreTime = 1;
                    }

                    double thisScore = 1-(scoreDistance*scoreTime);
                    if(thisScore<score){
                        score=thisScore;
                    }
                }

                int idx = score*(nColor-1);
                ss.at(idx)->append(az,zd);
            }
        }

        for(int i=nColor-1; i>=0; --i){
            chart->addSeries(ss.at(i));
            ss.at(i)->attachAxis(chart->axisX());
            ss.at(i)->attachAxis(chart->axisY());
        }

        QScatterSeries *obs = new QScatterSeries();
        obs->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        obs->setMarkerSize(12);
        obs->setBrush(QBrush(Qt::red));
        obs->setBorderColor(Qt::red);
        for(int i=0;i<obsTime.count();++i){
            obs->append(obsAz.at(i),90-obsEl.at(i));
        }

        chart->addSeries(obs);
        obs->attachAxis(chart->axisX());
        obs->attachAxis(chart->axisY());
    }
}

void MainWindow::on_pushButton_skyCoverageTemplateRandom_clicked()
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    obsAz.clear();
    obsEl.clear();
    obsTime.clear();
    double minElevation = 5;
    int nobs = ui->spinBox_skyCoverageTemplateRandomObservations->value() * (double)ui->influenceTimeSpinBox->value()/3600.;

    for(int i=0; i<nobs; ++i){
        obsAz.append(qrand() % ((360 + 1) - 0) + 0);

        double thisEl;
        double rn = (double)(qrand() % ((100 + 1) - 0) + 0);

        if(rn<58){
            thisEl = qrand() % ((40 + 1) - (int)minElevation) + (int)minElevation;
        }else{
            double u = (double)(qrand() % ((1000 + 1) - 0) + 0);
            u = u/1000;
            thisEl = 90-qSqrt((1-u)*(90-40)*(90-40));
        }
        obsEl.append(thisEl);
        obsTime.append((nobs-i)*ui->influenceTimeSpinBox->value()/nobs);
    }
    skyCoverageTemplate();
}

void MainWindow::on_influenceTimeSpinBox_valueChanged(int arg1)
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    obsTime.clear();
    double minElevation = 5;
    int nobs = ui->spinBox_skyCoverageTemplateRandomObservations->value() * (double)ui->influenceTimeSpinBox->value()/3600.;

    if(nobs>obsAz.count()){
        for(int i=obsAz.count(); i<nobs; ++i){
            obsAz.append(qrand() % ((360 + 1) - 0) + 0);
            obsEl.append(qrand() % ((90 + 1) - (int)minElevation) + (int)minElevation);
        }
    }else{
        for(int i=nobs; i<obsAz.count(); ++i){
            obsAz.remove(obsAz.count()-1);
            obsEl.remove(obsEl.count()-1);
        }
    }

    for(int i=0; i<nobs; ++i){
        obsTime.append((nobs-i)*ui->influenceTimeSpinBox->value()/nobs);
    }
    skyCoverageTemplate();

}

double MainWindow::interpolate( QVector<double> &xData, QVector<double> &yData, double x, bool extrapolate )
{
   int size = xData.size();

   int i = 0;
   if ( x >= xData[size - 2] )
   {
      i = size - 2;
   }
   else
   {
      while ( x > xData[i+1] ) i++;
   }
   double xL = xData[i], yL = yData[i], xR = xData[i+1], yR = yData[i+1];
   if ( !extrapolate )
   {
      if ( x < xL ) yR = yL;
      if ( x > xR ) yL = yR;
   }

   double dydx = ( yR - yL ) / ( xR - xL );
   return yL + dydx * ( x - xL );
}

// ########################################### CONDITIONS ###########################################

void MainWindow::on_pushButton_addCondition_clicked()
{
    QString members = ui->comboBox_conditions_members->currentText();
    bool isGroup = groupSrc.find(members.toStdString() ) != groupSrc.end();

    int scans = ui->spinBox_condtionsMinNumScans->value();
    int bls = ui->spinBox_conditionsMinNumBaselines->value();

    QIcon ic;
    QTreeWidgetItem *c = new QTreeWidgetItem();
    if(isGroup || members == '__all__'){
        ic = QIcon(":/icons/icons/source_group.png");
    }else{
        ic = QIcon(":/icons/icons/source.png");
    }

    c->setText(0,QString("%1").arg(ui->treeWidget_conditions->topLevelItemCount()));
    c->setIcon(1,ic);
    c->setText(1,members);
    c->setText(2,QString("%1").arg(scans));
    c->setText(3,QString("%1").arg(bls));

    ui->treeWidget_conditions->addTopLevelItem(c);

}

void MainWindow::on_pushButton_removeCondition_clicked()
{
    auto list = ui->treeWidget_conditions->selectedItems();
    if(!list.empty()){
        auto itm = list.at(0);
        delete itm;
        for(int i=0; i<ui->treeWidget_conditions->topLevelItemCount(); ++i){
            auto titm = ui->treeWidget_conditions->topLevelItem(i);
            titm->setText(0,QString("%1").arg(i));
        }
    }
}

void MainWindow::on_spinBox_maxNumberOfIterations_valueChanged(int arg1)
{
    ui->spinBox_gentleSourceReduction->setMaximum(arg1);
}

// ########################################### HIGH IMPACT ###########################################

void MainWindow::on_pushButton_addHighImpactAzEl_clicked()
{
    QString members = ui->comboBox_highImpactStation->currentText();
    bool isGroup = groupSta.find(members.toStdString() ) != groupSrc.end();

    double az = ui->doubleSpinBox_highImpactAzimuth->value();
    double el = ui->doubleSpinBox_highImpactElevation->value();
    double margin = ui->doubleSpinBox_highImpactMargin->value();

    QIcon ic;
    QTreeWidgetItem *c = new QTreeWidgetItem();
    if(isGroup || members == '__all__'){
        ic = QIcon(":/icons/icons/station_group_2.png");
    }else{
        ic = QIcon(":/icons/icons/station.png");
    }

    c->setIcon(0,ic);
    c->setText(0,members);
    c->setText(1,QString("%1").arg(az));
    c->setText(2,QString("%1").arg(el));
    c->setText(3,QString("%1").arg(margin));

    ui->treeWidget_highImpactAzEl->addTopLevelItem(c);

}

void MainWindow::on_pushButton_removeHighImpactAzEl_clicked()
{
    auto list = ui->treeWidget_highImpactAzEl->selectedItems();
    if(!list.empty()){
        auto itm = list.at(0);
        delete itm;
    }
}

// ########################################### AUXILIARY FILES ###########################################

void MainWindow::on_pushButton_readLogFile_read_clicked()
{
    QString path = ui->lineEdit_logFilePath->text().trimmed();
    textfileViewer* myViewer = new textfileViewer(this);
    myViewer->setTextFile(path, textfileViewer::Type::log);
    myViewer->show();
}

void MainWindow::on_pushButton_readSkdFile_read_clicked()
{
    QString path = ui->lineEdit_skdFilePath->text().trimmed();
    textfileViewer* myViewer = new textfileViewer(this);
    if(path.right(3) == "skd"){
        myViewer->setTextFile(path, textfileViewer::Type::skd);
    }else if(path.right(3) == "vex"){
        myViewer->setTextFile(path, textfileViewer::Type::vex);
    }else{
        myViewer->setTextFile(path, textfileViewer::Type::undefined);
    }
    myViewer->show();
}


void MainWindow::on_pushButton_sessionBrowse_clicked()
{
    QString startPath = ui->lineEdit_sessionPath->text();
    QString path = QFileDialog::getOpenFileName(this, "Browse to skd file", startPath, tr("skd files (*.skd)"));
    if( !path.isEmpty() ){
        ui->lineEdit_sessionPath->setText(path);
        ui->lineEdit_sessionPath->setFocus();
        ui->pushButton_sessionAnalyser->click();
    }
}

void MainWindow::on_pushButton_sessionAnalyser_clicked()
{
    QString path = ui->lineEdit_sessionPath->text();
    if(path.length()>4){
        if(path.right(4) == ".skd"){
            try{
                VieVS::SkdParser mySkdParser(path.toStdString());
                mySkdParser.read();
                VieVS::Scheduler sched = mySkdParser.createScheduler();
                std::string start = VieVS::TimeSystem::ptime2string(VieVS::TimeSystem::startTime);
                std::string end = VieVS::TimeSystem::ptime2string(VieVS::TimeSystem::endTime);
                QDateTime qstart = QDateTime::fromString(QString::fromStdString(start),"yyyy.MM.dd HH:mm:ss");
                QDateTime qend   = QDateTime::fromString(QString::fromStdString(end),"yyyy.MM.dd HH:mm:ss");

                VieSchedpp_Analyser *analyser = new VieSchedpp_Analyser(sched,qstart,qend, this);
                analyser->show();

            }catch(...){
                QString message = QString("Error reading session:\n").append(path);
                QMessageBox::critical(this, "error reading session", message);
            }

        }else{
            QString message = QString("Error reading session:\n").append(path);
            QMessageBox::critical(this, "error reading session", message);
        }
    }
}



