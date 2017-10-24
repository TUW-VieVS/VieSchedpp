#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainPath = QCoreApplication::applicationFilePath();

    ui->iconSizeSpinBox->setValue(ui->basicToolBar->iconSize().width());

    QPushButton *savePara = new QPushButton("write xml",this);
    ui->statusBar->addPermanentWidget(savePara);
    ui->dateTimeEdit_sessionStart->setDate(QDate::currentDate());

    allStationModel = new QStandardItemModel(0,5,this);
    allSourceModel = new QStandardItemModel(0,3,this);
    allStationProxyModel = new QSortFilterProxyModel();
    allStationProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    selectedStationModel = new QStringListModel();
    selectedSourceModel = new QStringListModel();

    allSourcePlusGroupModel = new QStandardItemModel();
    allSourcePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/source_group.png"),"__all__"));

    allStationPlusGroupModel = new QStandardItemModel();
    allStationPlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/station_group_2.png"),"__all__"));

    allSkedModesModel = new QStringListModel();

    ui->treeView_allAvailabeStations->setModel(allStationProxyModel);
    ui->treeView_allAvailabeStations->setRootIsDecorated(false);
    ui->treeView_allAvailabeStations->setSortingEnabled(true);
    ui->treeView_allAvailabeStations->sortByColumn(0, Qt::AscendingOrder);

    ui->listView_allSelectedStations->setModel(selectedStationModel);
    ui->comboBox_skedObsModes->setModel(allSkedModesModel);

    ui->comboBox_stationSettingMember->setModel(allStationPlusGroupModel);
    ui->comboBox_stationSettingMember_cable->setModel(allStationPlusGroupModel);
    ui->comboBox_stationSettingMember_wait->setModel(allStationPlusGroupModel);

    ui->comboBox_calibratorBlock_calibratorSources->setModel(allSourcePlusGroupModel);

    deleteModeMapper = new QSignalMapper(this);
    connect (deleteModeMapper, SIGNAL(mapped(QString)), this, SLOT(deleteModesCustomLine(QString))) ;
    multiSchedMapper = new QSignalMapper(this);
    connect (multiSchedMapper, SIGNAL(mapped(QString)), this, SLOT(multiSchedEditButton_clicked(QString))) ;


    connect(ui->pushButton_addGroupStationSetup, SIGNAL(clicked(bool)), this, SLOT(addGroupStation()));
    connect(ui->pushButton_addGroupStationWait, SIGNAL(clicked(bool)), this, SLOT(addGroupStation()));
    connect(ui->pushButton_addGroupStationCable, SIGNAL(clicked(bool)), this, SLOT(addGroupStation()));

    connect(ui->pushButton_addSourceGroup_Calibrator,SIGNAL(clicked(bool)), this, SLOT(addGroupSource()));
    connect(ui->pushButton_addSourceGroup_Sequence,SIGNAL(clicked(bool)), this, SLOT(addGroupSource()));

    readAllSkedObsModes();
    readSkedCatalogs();
    readStations();
    readSources();

    createMultiSchedTable();
    createModesPolicyTable();
    createModesCustonBandTable();

    defaultParameters();
}

MainWindow::~MainWindow()
{
    delete ui;
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
            allStationModel->setData(allStationModel->index(0, 1), antId);
            allStationModel->setData(allStationModel->index(0, 2), (double)((int)(qRadiansToDegrees(lat)*100 +0.5))/100.0);
            allStationModel->setData(allStationModel->index(0, 3), (double)((int)(qRadiansToDegrees(lon)*100 +0.5))/100.0);
            allStationModel->setData(allStationModel->index(0, 4), (double)((int)(diam*10 +0.5))/10.0);
        }catch(...){

        }
    }

    allStationProxyModel->setSourceModel(allStationModel);
    ui->treeView_allAvailabeStations->resizeColumnToContents(0);
    ui->treeView_allAvailabeStations->resizeColumnToContents(1);
    ui->treeView_allAvailabeStations->resizeColumnToContents(2);
    ui->treeView_allAvailabeStations->resizeColumnToContents(3);
    ui->treeView_allAvailabeStations->resizeColumnToContents(4);

    plotWorldMap();
    worldMapCallout = new Callout(worldChart);
    worldMapCallout->hide();
}

void MainWindow::readSources()
{
    QString sourcePath = ui->lineEdit_pathSource->text();

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
            allSourceModel->setData(allSourceModel->index(0, 2), (double)((int)(ra*100 +0.5))/100.0);
            allSourceModel->setData(allSourceModel->index(0, 3), (double)((int)(de*100 +0.5))/100.0);

            selectedSourceModel->insertRow(0);
            QModelIndex index_new = selectedSourceModel->index(0);
            selectedSourceModel->setData(index_new,sourceName);
            selectedSourceModel->sort(0);

            allSourcePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/source.png"),sourceName));
        }
        allSourcePlusGroupModel->sort(0);
        sourceFile.close();
    }
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
    ui->pushButton_worldmapZoomFull->setText("");
    ui->pushButton_worldmapZoomFull->setIcon(QIcon(":/icons/icons/zoom-fit-best-3.png"));
    ui->pushButton_worldmapZoomFull->setIconSize(ui->pushButton_worldmapZoomFull->size());
    worldChart = new QChart();
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
    selectedStations->setColor(Qt::darkGreen);
    selectedStations->setMarkerSize(15);

    worldChart->addSeries(availableStations);
    worldChart->addSeries(selectedStations);

    connect(availableStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));
    connect(selectedStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));

    for(int row = 0; row<allStationModel->rowCount(); ++row){

        QScatterSeries *stations = new QScatterSeries(worldChart);
        double lat = allStationModel->index(row,2).data().toDouble();
        double lon = allStationModel->index(row,3).data().toDouble();
        availableStations->append(lon,lat);

    }

    worldChart->createDefaultAxes();
    worldChart->setAcceptHoverEvents(true);
    worldChart->legend()->hide();
    worldChart->axisX()->setRange(-180,180);
    worldChart->axisY()->setRange(-90,90);

    worldmap = new ChartView(worldChart);
    worldmap->setRenderHint(QPainter::Antialiasing);
    worldmap->setFrameStyle(QFrame::Raised | QFrame::StyledPanel);
    worldmap->setBackgroundBrush(QBrush(Qt::white));
    worldmap->setMouseTracking(true);

    ui->verticalLayout_worldmap->addWidget(worldmap,8);

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

    para.parameters("default",sta);
    ui->ComboBox_parameterStation->addItem("default");

    QTreeWidgetItem *d = new QTreeWidgetItem();
    d->setText(0,"__all__");
    d->setText(1,"default");
    d->setText(2,ui->dateTimeEdit_sessionStart->dateTime().toString("dd.MM.yy hh:mm"));
    QDateTime e = ui->dateTimeEdit_sessionStart->dateTime().addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    d->setText(3,e.toString("dd.MM.yy hh:mm"));
    d->setText(4,"hard");
    d->setIcon(0,QIcon(":/icons/icons/station_group_2.png"));
    ui->treeWidget_setupStation->insertTopLevelItem(0,d);


//    QTreeWidgetItem *c = new QTreeWidgetItem();
//    d->addChild(c);

//    QComboBox *mem = new QComboBox(this);
//    mem->setModel(allStationPlusGroupModel);
//    ui->treeWidget_setupStation->setItemWidget(ui->treeWidget_setupStation->topLevelItem(0)->child(0),0,mem);

//    QDateTimeEdit *start = new QDateTimeEdit(this);
//    start->setMinimumDateTime(ui->dateTimeEdit_sessionStart->dateTime());
//    start->setMaximumDateTime(e);
//    ui->treeWidget_setupStation->setItemWidget(ui->treeWidget_setupStation->topLevelItem(0)->child(0),2,start);

//    QDateTimeEdit *end = new QDateTimeEdit(this);
//    end->setMinimumDateTime(ui->dateTimeEdit_sessionStart->dateTime());
//    end->setMaximumDateTime(e);
//    ui->treeWidget_setupStation->setItemWidget(ui->treeWidget_setupStation->topLevelItem(0)->child(0),3,end);

//    QComboBox *trans = new QComboBox(this);
//    trans->insertItem(0,"soft");
//    trans->insertItem(1,"hard");
//    ui->treeWidget_setupStation->setItemWidget(ui->treeWidget_setupStation->topLevelItem(0)->child(0),4,trans);



}

void MainWindow::on_listView_allSelectedStations_clicked(const QModelIndex &index)
{
    QString name = selectedStationModel->stringList().at(index.row());
    int idx = selectedStationModel->stringList().indexOf(name);
    selectedStationModel->removeRow(idx);

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

//    selectedStations->remove(allStationModel->index(row,3).data().toDouble(),
//                             allStationModel->index(row,2).data().toDouble());

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

    if(selectedStationModel->stringList().indexOf(name) == -1){
        selectedStationModel->insertRow(0);
        QModelIndex index_new = selectedStationModel->index(0);
        selectedStationModel->setData(index_new,name);
        selectedStationModel->sort(0);
        selectedStations->append(allStationProxyModel->index(row,3).data().toDouble(),
                                 allStationProxyModel->index(row,2).data().toDouble());


        allStationPlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/station.png"),name));
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
    ui->tableWidget_ModesPolicy->resizeColumnsToContents();
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
            ui->tableWidget_multiSched->item(i, j)->setBackground(brush);
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
    QApplication::setFont(myFont);
}

void MainWindow::on_fontComboBox_font_currentFontChanged(const QFont &f)
{
    QFont myFont = f;
    myFont.setPointSize(ui->spinBox_fontSize->value());
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

    double x = allStationProxyModel->index(row,3).data().toDouble();;
    double y = allStationProxyModel->index(row,2).data().toDouble();;
    QString text = QString("%1 \nlat: %2 [deg] \nlon: %3 [deg] ").arg(name).arg(x).arg(y);
    worldMapCallout->setText(text);
    worldMapCallout->setAnchor(QPointF(x,y));
    worldMapCallout->setZValue(11);
    worldMapCallout->updateGeometry();
    worldMapCallout->show();
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
        para.group(VieVS::ParameterSettings::Type::station,newGroup);
        allStationPlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/station_group_2.png"),QString::fromStdString(stdname) ));
    }
    delete(dial);
}

void MainWindow::addGroupSource()
{
    AddGroupDialog *dial = new AddGroupDialog(this);
    int i = selectedSourceModel->rowCount();
    dial->addModel(selectedSourceModel);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        std::vector<std::string> stdlist = dial->getSelection();
        std::string stdname = dial->getGroupName();
        VieVS::ParameterGroup newGroup(stdname, stdlist);
        para.group(VieVS::ParameterSettings::Type::source,newGroup);
        allSourcePlusGroupModel->appendRow(new QStandardItem(QIcon(":/icons/icons/source_group.png"),QString::fromStdString(stdname) ));
    }
    delete(dial);
}

void MainWindow::addGroupBaseline()
{

}

void MainWindow::on_pushButton_stationParameter_clicked()
{
    stationParametersDialog *dial = new stationParametersDialog(this);
    QStringList bands;
    for(int i = 0; i<ui->tableWidget_ModesPolicy->rowCount(); ++i){
        bands << ui->tableWidget_ModesPolicy->verticalHeaderItem(i)->text();
    }
    dial->addBandNames(bands);

    QStringList sources = selectedSourceModel->stringList();
    dial->addSourceNames(sources);

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

void MainWindow::on_dateTimeEdit_sessionStart_dateTimeChanged(const QDateTime &dateTime)
{
    ui->DateTimeEdit_startParameterStation->setMinimumDateTime(dateTime);
    ui->DateTimeEdit_endParameterStation->setMinimumDateTime(dateTime);
    QDateTime dateTimeEnd = dateTime.addSecs(ui->doubleSpinBox_sessionDuration->value()*3600);
    ui->DateTimeEdit_startParameterStation->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterStation->setMaximumDateTime(dateTimeEnd);

    ui->DateTimeEdit_startParameterStation->setDateTime(dateTime);
    ui->DateTimeEdit_endParameterStation->setDateTime(dateTimeEnd);
}

void MainWindow::on_doubleSpinBox_sessionDuration_valueChanged(double arg1)
{
    QDateTime dateTimeEnd = ui->dateTimeEdit_sessionStart->dateTime().addSecs(arg1*3600);
    ui->DateTimeEdit_startParameterStation->setMaximumDateTime(dateTimeEnd);
    ui->DateTimeEdit_endParameterStation->setMaximumDateTime(dateTimeEnd);
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

void MainWindow::on_pushButton_3_clicked()
{
    QList<QTreeWidgetItem *> sel = ui->treeWidget_setupStation->selectedItems();
    if(sel.size() != 1){
        QMessageBox *ms = new QMessageBox;
        ms->warning(this,"Wrong selection","Please select one parent in the right window!");
    }else{
        QTreeWidgetItem *c = new QTreeWidgetItem();

        c->setText(0,ui->comboBox_stationSettingMember->currentText());
        c->setText(1,ui->ComboBox_parameterStation->currentText());
        c->setText(2,ui->DateTimeEdit_startParameterStation->dateTime().toString("dd.MM.yy hh:mm"));
        c->setText(3,ui->DateTimeEdit_endParameterStation->dateTime().toString("dd.MM.yy hh:mm"));
        c->setText(4,ui->comboBox_parameterStationTransition->currentText());

        sel.at(0)->addChild(c);
        sel.at(0)->setExpanded(true);
    }


}

void MainWindow::on_pushButton_4_clicked()
{
    QList<QTreeWidgetItem *> sel = ui->treeWidget_setupStation->selectedItems();
    for(int i = 0; i<sel.size(); ++i){
        if(sel.at(0)->parent()){
            delete(sel.at(0));
        }else{
            QMessageBox *ms = new QMessageBox;
            ms->warning(this,"Wrong selection","You can not delete top level default parameter item!");
        }

    }
}
