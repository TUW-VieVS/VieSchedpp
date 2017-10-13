#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainPath = QCoreApplication::applicationFilePath();

    QPushButton *savePara = new QPushButton("write xml",this);
    ui->statusBar->addPermanentWidget(savePara);

    allStationModel = new QStandardItemModel(0,5,this);
    allStationProxyModel = new QSortFilterProxyModel();
    allStationProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    selectedStationModel = new QStringListModel();
    allSourceModel = new QStandardItemModel(0,3,this);
    allSkedModesModel = new QStringListModel();

    ui->treeView_allAvailabeStations->setModel(allStationProxyModel);
    ui->treeView_allAvailabeStations->setRootIsDecorated(false);
    ui->treeView_allAvailabeStations->setSortingEnabled(true);
    ui->treeView_allAvailabeStations->sortByColumn(0, Qt::AscendingOrder);

    ui->listView_allSelectedStations->setModel(selectedStationModel);
    ui->comboBox_skedObsModes->setModel(allSkedModesModel);

    deleteModeMapper = new QSignalMapper(this);
    connect (deleteModeMapper, SIGNAL(mapped(QString)), this, SLOT(deleteModesCustomLine(QString))) ;
    multiSchedMapper = new QSignalMapper(this);
    connect (multiSchedMapper, SIGNAL(mapped(QString)), this, SLOT(multiSchedEditButton_clicked(QString))) ;

    readAllSkedObsModes();
    readSkedCatalogs();
    readStations();
    readSources();

    createMultiSchedTable();
    createModesPolicyTable();
    createModesCustonBandTable();
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
            QString sourceName = line.at(0);
            QString rah = line.at(2);
            QString ram = line.at(3);
            QString ras = line.at(4);
            double ra = (rah.toDouble() + ram.toDouble()/60 + ras.toDouble()/3600)*15;
            QString ded = line.at(5);
            QString dem = line.at(6);
            QString des = line.at(7);
            double de = ded.toDouble() + dem.toDouble()/60 + des.toDouble()/3600;

            allSourceModel->insertRow(0);
            allSourceModel->setData(allSourceModel->index(0,0), sourceName);
            allSourceModel->setData(allSourceModel->index(0, 2), (double)((int)(ra*100 +0.5))/100.0);
            allSourceModel->setData(allSourceModel->index(0, 3), (double)((int)(de*100 +0.5))/100.0);
        }
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
            ui->tableWidget_scanSequence->setCellWidget(ui->tableWidget_scanSequence->rowCount()-1,0, new QComboBox);
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
    ui->tableWidget_multiSched->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("session start"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("subnetting"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("fillin mode"));

    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("sky coverage"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("number of observations"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("duration"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("average stations"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("average sources"));

    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("max slew time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("max wait time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("max scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("max scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("min scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("weight"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());

    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("min number of stations"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("min flux"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("min repeat time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("max scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("min scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("weight"));

    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem(""));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("max scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("min scan time"));
    ui->tableWidget_multiSched->insertRow(ui->tableWidget_multiSched->rowCount());
    ui->tableWidget_multiSched->setVerticalHeaderItem(ui->tableWidget_multiSched->rowCount()-1,new QTableWidgetItem("weight"));




    QBrush brush(Qt::black,Qt::HorPattern);
    QSet<int> breakes = {3,9,16,23};
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

        QListWidget *list = new QListWidget(this);
        ui->tableWidget_multiSched->setCellWidget(i, 0, list);

        QPushButton *button = new QPushButton("edit",this);
        ui->tableWidget_multiSched->setCellWidget(i, 2, button);

        connect(button,SIGNAL(clicked(bool)),multiSchedMapper,SLOT(map()));
        multiSchedMapper->setMapping(button,ui->tableWidget_multiSched->verticalHeaderItem(i)->text());
    }
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
    for(int i = 0; i<ui->tableWidget_multiSched->rowCount(); ++i){
        QString rname = ui->tableWidget_multiSched->verticalHeaderItem(i)->text();
        if(rname == name){

        }
    }
}
