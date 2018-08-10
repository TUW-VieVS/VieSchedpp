#include "vieschedpp_analyser.h"
#include "ui_vieschedpp_analyser.h"

VieSchedpp_Analyser::VieSchedpp_Analyser(VieVS::Scheduler schedule, QDateTime start, QDateTime end, QWidget *parent) :
    QMainWindow(parent), schedule_{schedule}, sessionStart_{start}, sessionEnd_{end},
    ui(new Ui::VieSchedpp_Analyser)
{
    ui->setupUi(this);
    srcModel = new QStandardItemModel(0,6,this);
    staModel = new QStandardItemModel(0,6,this);
    blModel = new QStandardItemModel(0,4,this);
    setup();
    setupWorldmap();
    setupSkymap();
    statisticsGeneralSetup();
    statisticsStationsSetup();
    statisticsSourceSetup();
    statisticsBaselineSetup();

    ui->dateTimeEdit_start->setDateTimeRange(sessionStart_,sessionEnd_);
    ui->dateTimeEdit_end->setDateTimeRange(sessionStart_,sessionEnd_);
    ui->dateTimeEdit_start->setDateTime(sessionStart_);
    ui->dateTimeEdit_end->setDateTime(sessionEnd_);

    int duration = sessionStart_.secsTo(sessionEnd_);
    ui->spinBox_duration->setRange(0,duration);
    ui->spinBox_duration->setValue(duration);
    ui->horizontalSlider_start->setRange(0,duration);
    ui->horizontalSlider_end->setRange(0,duration);

    ui->doubleSpinBox_hours->setRange(0,duration/3600.0);

    updateDuration();

    ui->splitter_skyCoverage->setStretchFactor(0,6);
    ui->splitter_skyCoverage->setStretchFactor(1,1);
    ui->splitter_skyCoverage->setSizes({5000,1000});

    ui->splitter_worldmap->setStretchFactor(0,5);
    ui->splitter_worldmap->setStretchFactor(1,1);
    ui->splitter_worldmap->setSizes({5000,1000});

    ui->splitter_skymap->setStretchFactor(0,5);
    ui->splitter_skymap->setStretchFactor(1,1);
    ui->splitter_skymap->setSizes({5000,1000});


    QHeaderView *hv = ui->tableWidget_general->verticalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);

}

VieSchedpp_Analyser::~VieSchedpp_Analyser()
{
    delete ui;
}

void VieSchedpp_Analyser::on_actionsky_coverage_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
    updateSkyCoverageTimes();
}

void VieSchedpp_Analyser::on_actionworld_map_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
    updateWorldmapTimes();
}

void VieSchedpp_Analyser::on_actionuv_coverage_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);

}

void VieSchedpp_Analyser::on_actionsky_map_triggered()
{
    ui->stackedWidget->setCurrentIndex(3);
    updateSkymapTimes();
}


void VieSchedpp_Analyser::on_actiongeneral_triggered()
{
    ui->stackedWidget->setCurrentIndex(4);
    updateGeneralStatistics();
}

void VieSchedpp_Analyser::on_actionper_station_triggered()
{
    ui->stackedWidget->setCurrentIndex(5);
    updateStatisticsStations();
}

void VieSchedpp_Analyser::on_actionper_source_triggered()
{
    ui->stackedWidget->setCurrentIndex(6);
    updateStatisticsSource();
}

void VieSchedpp_Analyser::on_actionper_baseline_triggered()
{
    ui->stackedWidget->setCurrentIndex(7);
    updateStatisticsBaseline();
}


void VieSchedpp_Analyser::setup()
{
    ui->label_fileName->setText(QString::fromStdString(schedule_.getName()));

    srcModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));
    srcModel->setHeaderData(1, Qt::Horizontal, QObject::tr("2nd name"));
    srcModel->setHeaderData(2, Qt::Horizontal, QObject::tr("#scans"));
    srcModel->setHeaderData(3, Qt::Horizontal, QObject::tr("#obs"));
    srcModel->setHeaderData(4, Qt::Horizontal, QObject::tr("ra [deg]"));
    srcModel->setHeaderData(5, Qt::Horizontal, QObject::tr("de [deg]"));
    srcModel->setRowCount(schedule_.getSources().size());

    int i = 0;
    for(const VieVS::Source &any : schedule_.getSources()){
        QString sourceName = QString::fromStdString(any.getName());
        QString aSourceName = QString::fromStdString(any.getAlternativeName());
        double ra = qRadiansToDegrees(any.getRa());
        double de = qRadiansToDegrees(any.getDe());
        srcModel->setData(srcModel->index(i,0), sourceName);
        srcModel->setData(srcModel->index(i,1), aSourceName);
        srcModel->item(i,0)->setIcon(QIcon(":/icons/icons/source.png"));
        srcModel->setData(srcModel->index(i,2), static_cast<int>(any.getNTotalScans()));
        srcModel->setData(srcModel->index(i,3), static_cast<int>(any.getNObs()));
        srcModel->setData(srcModel->index(i,4), (double)((int)(ra*100000 +0.5))/100000.0);
        srcModel->setData(srcModel->index(i,5), (double)((int)(de*100000 +0.5))/100000.0);
        srcModel->item(i,2)->setTextAlignment(Qt::AlignRight);
        srcModel->item(i,3)->setTextAlignment(Qt::AlignRight);
        srcModel->item(i,4)->setTextAlignment(Qt::AlignRight);
        srcModel->item(i,5)->setTextAlignment(Qt::AlignRight);
        ++i;
    }
    QSortFilterProxyModel *srcSkyCoverage = new QSortFilterProxyModel(this);
    srcSkyCoverage->setSourceModel(srcModel);
    srcSkyCoverage->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView_skyCoverage_sources->setModel(srcSkyCoverage);
    for( int i=0 ; i<ui->treeView_skyCoverage_sources->model()->columnCount(); ++i){
        ui->treeView_skyCoverage_sources->resizeColumnToContents(i);
    }
    srcSkyCoverage->sort(0);

    QSortFilterProxyModel *srcSkymap = new QSortFilterProxyModel(this);
    srcSkymap->setSourceModel(srcModel);
    srcSkymap->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView_skymap_sources->setModel(srcSkymap);
    for( int i=0 ; i<ui->treeView_skymap_sources->model()->columnCount(); ++i){
        ui->treeView_skymap_sources->resizeColumnToContents(i);
    }
    srcSkymap->sort(0);

    staModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));
    staModel->setHeaderData(1, Qt::Horizontal, QObject::tr("id"));
    staModel->setHeaderData(2, Qt::Horizontal, QObject::tr("#scans"));
    staModel->setHeaderData(3, Qt::Horizontal, QObject::tr("#obs"));
    staModel->setHeaderData(4, Qt::Horizontal, QObject::tr("lat [deg]"));
    staModel->setHeaderData(5, Qt::Horizontal, QObject::tr("lon [deg]"));
    staModel->setRowCount(schedule_.getNetwork().getStations().size());

    i=0;
    for(const VieVS::Station &any : schedule_.getNetwork().getStations()){
        QString staName = QString::fromStdString(any.getName());
        QString id = QString::fromStdString(any.getAlternativeName());
        double lat = qRadiansToDegrees(any.getPosition().getLat());
        double lon = qRadiansToDegrees(any.getPosition().getLon());
        staModel->setData(staModel->index(i,0), staName);
        staModel->item(i,0)->setIcon(QIcon(":/icons/icons/station.png"));
        staModel->setData(staModel->index(i,1), id);
        staModel->setData(staModel->index(i,2), static_cast<int>(any.getNTotalScans()));
        staModel->setData(staModel->index(i,3), static_cast<int>(any.getNObs()));
        staModel->setData(staModel->index(i,4), (double)((int)(lat*100000 +0.5))/100000.0);
        staModel->setData(staModel->index(i,5), (double)((int)(lon*100000 +0.5))/100000.0);
        staModel->item(i,2)->setTextAlignment(Qt::AlignRight);
        staModel->item(i,3)->setTextAlignment(Qt::AlignRight);
        staModel->item(i,4)->setTextAlignment(Qt::AlignRight);
        staModel->item(i,5)->setTextAlignment(Qt::AlignRight);
        ++i;
    }



    blModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));
    blModel->setHeaderData(1, Qt::Horizontal, QObject::tr("long name"));
    blModel->setHeaderData(2, Qt::Horizontal, QObject::tr("#obs"));
    blModel->setHeaderData(3, Qt::Horizontal, QObject::tr("distance [km]"));
    blModel->setRowCount(schedule_.getNetwork().getBaselines().size());
    i=0;
    for(const VieVS::Baseline &any : schedule_.getNetwork().getBaselines()){
        QString name = QString::fromStdString(any.getName());
        std::string id1 = name.left(2).toStdString();
        std::string id2 = name.right(2).toStdString();
        std::string name1 = schedule_.getNetwork().getStation(id1).getName();
        std::string name2 = schedule_.getNetwork().getStation(id2).getName();
        double dist = schedule_.getNetwork().getStation(id1).distance(schedule_.getNetwork().getStation(id2));
        QString longName = QString("%1-%2").arg(QString::fromStdString(name1)).arg(QString::fromStdString(name2));

        blModel->setData(blModel->index(i,0), name);
        blModel->item(i,0)->setIcon(QIcon(":/icons/icons/baseline.png"));
        blModel->setData(blModel->index(i,1), longName);
        blModel->setData(blModel->index(i,2), static_cast<int>(any.getNObs()));
        blModel->setData(blModel->index(i,3), dist/1000);
        blModel->item(i,2)->setTextAlignment(Qt::AlignRight);
        blModel->item(i,3)->setTextAlignment(Qt::AlignRight);
        ++i;
    }

    QSortFilterProxyModel *staWorldmap = new QSortFilterProxyModel(this);
    staWorldmap->setSourceModel(staModel);
    staWorldmap->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView_worldmap_stations->setModel(staWorldmap);
    for( int i=0 ; i<ui->treeView_worldmap_stations->model()->columnCount(); ++i){
        ui->treeView_worldmap_stations->resizeColumnToContents(i);
    }
    staWorldmap->sort(0);

    QSortFilterProxyModel *staStat = new QSortFilterProxyModel(this);
    staStat->setSourceModel(staModel);
    staStat->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView_statistics_station_model->setModel(staStat);
    for( int i=0 ; i<ui->treeView_statistics_station_model->model()->columnCount(); ++i){
        ui->treeView_statistics_station_model->resizeColumnToContents(i);
    }
    staStat->sort(0);

    QSortFilterProxyModel *blsWorldmap = new QSortFilterProxyModel(this);
    blsWorldmap->setSourceModel(blModel);
    blsWorldmap->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView_worldmap_baselines->setModel(blsWorldmap);
    for( int i=0 ; i<ui->treeView_worldmap_baselines->model()->columnCount(); ++i){
        ui->treeView_worldmap_baselines->resizeColumnToContents(i);
    }
    blsWorldmap->sort(0);


//    comboBox2skyCoverage = new QSignalMapper(this);

    int stas = staModel->rowCount();
    if(stas == 2){
        setSkyCoverageLayout(1,2);
    }else if(stas == 3){
        setSkyCoverageLayout(1,3);
    }else if(stas == 4){
        setSkyCoverageLayout(2,2);
    }else if(stas == 5){
        setSkyCoverageLayout(2,3);
    }else if(stas == 6){
        setSkyCoverageLayout(2,3);
    }else if(stas == 7){
        setSkyCoverageLayout(2,4);
    }else if(stas >= 8){
        setSkyCoverageLayout(2,4);
    }

//    ui->splitter_skyCoverage->setSizes(QList<int>({std::numeric_limits<int>::max(), std::numeric_limits<int>::max()/4}));
//    ui->splitter_worldmap->setSizes(QList<int>({std::numeric_limits<int>::max(), std::numeric_limits<int>::max()/4}));
//    ui->splitter_worldmap->setSizes(QList<int>({std::numeric_limits<int>::max(), std::numeric_limits<int>::max()/4}));


    int totalObs = 0;
    const std::vector<VieVS::Scan> &scans = schedule_.getScans();
    for(const VieVS::Scan &any: scans){
        totalObs += any.getNObs();
    }
    ui->spinBox_observations->setValue(totalObs);
    ui->spinBox_observations_total->setValue(totalObs);
    ui->spinBox_scans->setValue(scans.size());
    ui->spinBox_scans_total->setValue(scans.size());
    ui->spinBox_sources->setValue(srcModel->rowCount());
    ui->spinBox_sources_total->setValue(srcModel->rowCount());
    ui->spinBox_stations->setValue(staModel->rowCount());
    ui->spinBox_stations_total->setValue(staModel->rowCount());
    ui->spinBox_baselines->setValue(schedule_.getNetwork().getNBls());
    ui->spinBox_baselines_total->setValue(schedule_.getNetwork().getNBls());

}

// -----------------------------------------------------------------------------

void VieSchedpp_Analyser::on_horizontalSlider_start_valueChanged(int value)
{
    const QSignalBlocker b1(ui->horizontalSlider_end);

    QDateTime newStart = sessionStart_.addSecs(value);
    ui->dateTimeEdit_start->setDateTime(newStart);

    if(value>ui->horizontalSlider_end->value()){
        ui->horizontalSlider_end->setValue(value);
        ui->dateTimeEdit_end->setDateTime(newStart);
    }
    if(ui->checkBox_fixDuration->isChecked()){
        ui->horizontalSlider_end->setValue(value+ui->spinBox_duration->value());
        QDateTime newEnd = sessionStart_.addSecs(value+ui->spinBox_duration->value());
        ui->dateTimeEdit_end->setDateTime(newEnd);
    }else{
        updateDuration();
    }
    updatePlotsAndModels();
}

void VieSchedpp_Analyser::on_horizontalSlider_end_valueChanged(int value)
{
    const QSignalBlocker b0(ui->horizontalSlider_start);

    QDateTime newEnd = sessionStart_.addSecs(value);
    ui->dateTimeEdit_end->setDateTime(newEnd);

    if(value<ui->horizontalSlider_start->value()){
        ui->horizontalSlider_start->setValue(value);
        ui->dateTimeEdit_start->setDateTime(newEnd);
    }
    if(ui->checkBox_fixDuration->isChecked()){
        ui->horizontalSlider_start->setValue(value-ui->spinBox_duration->value());
        QDateTime newStart = sessionStart_.addSecs(value-ui->spinBox_duration->value());
        ui->dateTimeEdit_start->setDateTime(newStart);
    }else{
        updateDuration();
    }
    updatePlotsAndModels();
}

void VieSchedpp_Analyser::on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime)
{
    int start = sessionStart_.secsTo(dateTime);
    ui->horizontalSlider_start->setValue(start);
}

void VieSchedpp_Analyser::on_dateTimeEdit_end_dateTimeChanged(const QDateTime &dateTime)
{
    int end = sessionStart_.secsTo(dateTime);
    ui->horizontalSlider_end->setValue(end);
}

void VieSchedpp_Analyser::on_spinBox_duration_valueChanged(int arg1)
{
    int newEnd = ui->horizontalSlider_start->value() + arg1;
    ui->horizontalSlider_end->setValue(newEnd);   

//    if(arg1 < 3600){
//        ui->horizontalSlider_start->setSingleStep(arg1/2);
//        ui->horizontalSlider_end->setSingleStep(arg1/2);
//    }else{
//        ui->horizontalSlider_start->setSingleStep(1800);
//        ui->horizontalSlider_end->setSingleStep(1800);
//    }
}

void VieSchedpp_Analyser::updateDuration()
{
    int dur = ui->dateTimeEdit_start->dateTime().secsTo(ui->dateTimeEdit_end->dateTime());
    ui->spinBox_duration->setValue(dur);
    double dur_h = static_cast<double>(dur)/3600.;
    ui->doubleSpinBox_hours->setValue(dur_h);
}


void VieSchedpp_Analyser::on_doubleSpinBox_hours_valueChanged(double arg1)
{
    int dur = arg1*3600;
    ui->spinBox_duration->setValue(dur);
}

// -----------------------------------------------------------------------------

void VieSchedpp_Analyser::setSkyCoverageLayout(int rows, int columns)
{

    while( ui->gridLayout_skyCoverage->count() >0){
        auto itm = ui->gridLayout_skyCoverage->takeAt(0);
        if(itm->widget()){
            delete itm->widget();
        }
        if(itm->layout()){
            delete itm->layout();
        }
    }

    int counter = 0;
    for(int i=0; i<rows; ++i){
        for(int j=0; j<columns; ++j){
            QVBoxLayout *layout = new QVBoxLayout();
            QComboBox *c1 = new QComboBox();
            c1->setModel(staModel);

            layout->addWidget(c1);

            QGroupBox *groupBox = new QGroupBox(this);
            QPolarChart *chart = new QPolarChart();
            chart->setAnimationOptions(QPolarChart::NoAnimation);
            QChartView *chartView = new QChartView(chart,groupBox);

            chart->layout()->setContentsMargins(0, 0, 0, 0);
            chart->setBackgroundRoundness(0);
            chart->legend()->hide();
            chart->acceptHoverEvents();
            chartView->setMouseTracking(true);
            Callout *callout = new Callout(chart);
            callout->hide();

            QValueAxis *angularAxis = new QValueAxis();
            angularAxis->setTickCount(13); // First and last ticks are co-located on 0/360 angle.
            angularAxis->setLabelFormat("%.0f");
            angularAxis->setShadesVisible(true);
            angularAxis->setShadesBrush(QBrush(QColor(230, 238, 255)));
            angularAxis->setRange(0,360);
            chart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);

            QValueAxis *radialAxis = new QValueAxis();
            radialAxis->setTickCount(10);
            radialAxis->setLabelFormat("");
            radialAxis->setRange(0,90);
            chart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);

            chartView->setRenderHint(QPainter::Antialiasing);

            layout->addWidget(chartView);

            c1->setCurrentIndex(counter);
            connect(c1,SIGNAL(currentIndexChanged(QString)), this, SLOT(updateSkyCoverage(QString)));

            groupBox->setLayout(layout);

            ui->gridLayout_skyCoverage->addWidget(groupBox,i,j);
            updateSkyCoverage(counter, c1->currentText());
            ++counter;
        }
    }
    for(int i=0; i<rows; ++i){
        ui->gridLayout_skyCoverage->setRowStretch(i,1);
    }
    for(int i=rows; i< ui->gridLayout_skyCoverage->rowCount(); ++i){
        ui->gridLayout_skyCoverage->setRowStretch(i,0);
    }
    for(int j=0; j<columns; ++j){
        ui->gridLayout_skyCoverage->setColumnStretch(j,1);
    }
    for(int i=columns; i< ui->gridLayout_skyCoverage->columnCount(); ++i){
        ui->gridLayout_skyCoverage->setColumnStretch(i,0);
    }

}

void VieSchedpp_Analyser::on_pushButton_skyCoverageLayout_clicked()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel("The question ?"));
    // Add the lineEdits with their respective labels

    QSpinBox *rowBox = new QSpinBox(&dialog);
    rowBox->setMinimum(1);
    rowBox->setValue(ui->gridLayout_skyCoverage->rowCount());
    form.addRow("rows: ", rowBox);
    QSpinBox *colBox = new QSpinBox(&dialog);
    colBox->setMinimum(1);
    colBox->setValue(ui->gridLayout_skyCoverage->columnCount());
    form.addRow("columns: ", colBox);


    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);

    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields

        setSkyCoverageLayout(rowBox->value(), colBox->value());
    }

}

void VieSchedpp_Analyser::updateSkyCoverage(QString name)
{
    QObject *obj = sender();
    QObject *parent = obj->parent();

    int idx = -1;
    for(int i=0; i<ui->gridLayout_skyCoverage->count(); ++i){
        auto itm = ui->gridLayout_skyCoverage->itemAt(i)->widget();
        if(parent == itm){
            idx = i;
            break;
        }
    }
    updateSkyCoverage(idx, name);
}

void VieSchedpp_Analyser::updateSkyCoverage(int idx, QString name)
{
    if(name.isEmpty()){
        return;
    }
    QGroupBox *box = qobject_cast<QGroupBox*>(ui->gridLayout_skyCoverage->itemAt(idx)->widget());
    QChartView *chartView = qobject_cast<QChartView*>(box->layout()->itemAt(1)->widget());
    QChart *chart = chartView->chart();
    chart->removeAllSeries();
    chart->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);

    const VieVS::Network &network = schedule_.getNetwork();
    const VieVS::Station &thisSta = network.getStation(name.toStdString());

    std::pair<std::vector<double>, std::vector<double>> mask = thisSta.getHorizonMask();
    const std::vector<double> &az = mask.first;
    const std::vector<double> &el = mask.second;

    QLineSeries *hmaskUp = new QLineSeries();
    for(int i=0; i<az.size(); ++i){
        hmaskUp->append(az[i]*rad2deg,90-el[i]*rad2deg);
    }
    if(hmaskUp->count() == 0){
        for(int i=0; i<=360; ++i){
            hmaskUp->append(static_cast<double>(i),89);
        }
    }
    QLineSeries *hmaskDown = new QLineSeries();
    for(int i=0; i<=360; ++i){
        hmaskDown->append(static_cast<double>(i),90);
    }
    QAreaSeries *hmask = new QAreaSeries();
    hmask->setName("horizon mask");
    hmask->setUpperSeries(hmaskDown);
    hmask->setLowerSeries(hmaskUp);
    hmask->setBrush(Qt::gray);
    hmask->setOpacity(0.5);
    chart->addSeries(hmask);
    hmask->attachAxis(chart->axisX());
    hmask->attachAxis(chart->axisY());

    const VieVS::SkyCoverage &thisSkyCoverage = network.getSkyCoverage(network.getStaid2skyCoverageId().at(thisSta.getId()));

    QList<qtUtil::ObsData> list = qtUtil::getObsData(thisSta.getId(), schedule_.getScans());
//    QList<std::tuple<int, double, double, int>> list = qtUtil::pointingVectors2Lists(thisSkyCoverage.getPointingVectors());

    QScatterSeriesExtended *data = new QScatterSeriesExtended();
    for(const auto &any : list){
        double unaz = any.az;
        double az = VieVS::util::wrapToPi(unaz)*rad2deg;
        if(az<0){
            az+=360;
        }
        VieVS::CableWrap::CableWrapFlag flag = thisSta.getCableWrap().cableWrapFlag(unaz);
        data->append(az, 90-any.el*rad2deg, any.startTime, any.endTime, flag, any.srcid, any.nsta);
    }
    data->setBrush(Qt::gray);
    data->setMarkerSize(7);
    data->setName("outside timespan");

    chart->addSeries(data);
    data->attachAxis(chart->axisX());
    data->attachAxis(chart->axisY());
    connect(data, SIGNAL(hovered(QPointF,bool)), this, SLOT(skyCoverageHovered(QPointF,bool)));

    QScatterSeriesExtended *ccw = new QScatterSeriesExtended();
    ccw->setBrush(QBrush(QColor(228,26,28)));
    ccw->setName("ccw");

    chart->addSeries(ccw);
    ccw->attachAxis(chart->axisX());
    ccw->attachAxis(chart->axisY());
    connect(ccw, SIGNAL(hovered(QPointF,bool)), this, SLOT(skyCoverageHovered(QPointF,bool)));

    QScatterSeriesExtended *cw = new QScatterSeriesExtended();
    cw->setBrush(QBrush(QColor(55,126,184)));
    cw->setName("cw");

    chart->addSeries(cw);
    cw->attachAxis(chart->axisX());
    cw->attachAxis(chart->axisY());
    connect(cw, SIGNAL(hovered(QPointF,bool)), this, SLOT(skyCoverageHovered(QPointF,bool)));

    QScatterSeriesExtended *n = new QScatterSeriesExtended();
    n->setBrush(QBrush(QColor(77,174,74)));
    n->setName("n");

    chart->addSeries(n);
    n->attachAxis(chart->axisX());
    n->attachAxis(chart->axisY());
    connect(n, SIGNAL(hovered(QPointF,bool)), this, SLOT(skyCoverageHovered(QPointF,bool)));

    QScatterSeriesExtended *selected = new QScatterSeriesExtended();
    selected->setBrush(Qt::yellow);
    selected->setPen(QPen(Qt::black));
    selected->setName("selected");

    chart->addSeries(selected);
    selected->attachAxis(chart->axisX());
    selected->attachAxis(chart->axisY());
    connect(selected, SIGNAL(hovered(QPointF,bool)), this, SLOT(skyCoverageHovered(QPointF,bool)));

    chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    updateSkyCoverageTimes(idx);
}

void VieSchedpp_Analyser::updateSkyCoverageTimes()
{
    for(int i=0; i<ui->gridLayout_skyCoverage->count(); ++i){
        updateSkyCoverageTimes(i);
    }
}

void VieSchedpp_Analyser::updateSkyCoverageTimes(int idx)
{
    QGroupBox *box = qobject_cast<QGroupBox*>(ui->gridLayout_skyCoverage->itemAt(idx)->widget());
    QChartView *chartView = qobject_cast<QChartView*>(box->layout()->itemAt(1)->widget());
    QChart *chart = chartView->chart();

    QList<QAbstractSeries *> series = chart->series();
    QScatterSeriesExtended * data;
    QScatterSeriesExtended * ccw;
    QScatterSeriesExtended * cw;
    QScatterSeriesExtended * n;
    QScatterSeriesExtended * selected;

    for(const auto &any:series){
        if(any->name() == "outside timespan"){
            data = static_cast<QScatterSeriesExtended *>(any);
        }
        if(any->name() == "ccw"){
            ccw = static_cast<QScatterSeriesExtended *>(any);
        }
        if(any->name() == "cw"){
            cw = static_cast<QScatterSeriesExtended *>(any);
        }
        if(any->name() == "n"){
            n = static_cast<QScatterSeriesExtended *>(any);
        }
        if(any->name() == "selected"){
            selected = static_cast<QScatterSeriesExtended *>(any);
        }
    }

    ccw->clear();
    cw->clear();
    n->clear();
    selected->clear();

    int start = ui->horizontalSlider_start->value();
    int end = ui->horizontalSlider_end->value();

    for(int i=0; i<data->count(); ++i){
        bool flag1 = data->getStartTime(i) >= start && data->getStartTime(i) <= end;
        bool flag2 = data->getEndTime(i) >= start && data->getEndTime(i) <= end;
        bool flag3 = data->getStartTime(i) <= start && data->getEndTime(i) >= end;
        bool flag = flag1 || flag2 || flag3;

        if(flag){
            switch(data->getCableWrapFlag(i)){
                case VieVS::CableWrap::CableWrapFlag::n:{
                    n->append(data->at(i).x(), data->at(i).y(), data->getStartTime(i), data->getEndTime(i), data->getCableWrapFlag(i), data->getSrcid(i), data->getNSta(i));
                    break;
                }
                case VieVS::CableWrap::CableWrapFlag::ccw:{
                    ccw->append(data->at(i).x(), data->at(i).y(), data->getStartTime(i), data->getEndTime(i), data->getCableWrapFlag(i), data->getSrcid(i), data->getNSta(i));
                    break;
                }
                case VieVS::CableWrap::CableWrapFlag::cw:{
                    cw->append(data->at(i).x(), data->at(i).y(), data->getStartTime(i), data->getEndTime(i), data->getCableWrapFlag(i), data->getSrcid(i), data->getNSta(i));
                    break;
                }
            }
        }
    }

    on_treeView_skyCoverage_sources_clicked(QModelIndex());
}

void VieSchedpp_Analyser::skyCoverageHovered(QPointF point, bool flag)
{
    QObject *obj = sender();
    QScatterSeriesExtended *series = static_cast<QScatterSeriesExtended *>(obj);

    QChart *chart = qobject_cast<QChart *>(obj->parent()->parent());

    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *c = dynamic_cast<Callout *>(childItem)){
            if(flag){
                c->setAnchor(point);

                QString name = series->name();
                int idx = 0;
                while(idx<series->count()){
                    const QPointF &p = series->at(idx);

                    if(point == p){
                        break;
                    }
                    ++idx;
                }

                int srcid = series->getSrcid(idx);
                QString source = srcModel->item(srcid,0)->text().append("\n");
                int startTime = series->getStartTime(idx);
                int endTime = series->getEndTime(idx);
                QDateTime qStartTime = sessionStart_.addSecs(startTime);
                QDateTime qEndTime   = sessionStart_.addSecs(endTime);
                QString startTimeStr = qStartTime.toString("hh:mm:ss");
                QString endTimeStr   = qEndTime.toString("hh:mm:ss");
                QString timeStr = startTimeStr.append("-").append(endTimeStr).append("\n");
                QString az = QString().sprintf("az: %.2f\n", point.x());
                QString el = QString().sprintf("el: %.2f\n", 90-point.y());
                QString nsta = QString().sprintf("#sta: %d", series->getNSta(idx));

                QString txt = source;
                txt.append(timeStr).append(az).append(el).append(nsta);

                c->setText(txt);
                c->setZValue(11);
                c->updateGeometry();
                c->show();
            }else{
                c->hide();
            }
            break;
        }
    }
}

void VieSchedpp_Analyser::on_lineEdit_skyCoverageSourceFilter_textChanged(const QString &arg1)
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(ui->treeView_skyCoverage_sources->model());
    model->setFilterFixedString(arg1);
}

void VieSchedpp_Analyser::on_treeView_skyCoverage_sources_clicked(const QModelIndex &index)
{
    QModelIndexList sel = ui->treeView_skyCoverage_sources->selectionModel()->selectedRows();
    QVector<int> ids;
    for(const auto &any : sel){
        QString name = ui->treeView_skyCoverage_sources->model()->data(any).toString();
        int id = srcModel->findItems(name).at(0)->row();
        ids.push_back(id);
    }

    for(int idx = 0; idx < ui->gridLayout_skyCoverage->count(); ++idx){
        QGroupBox *box = qobject_cast<QGroupBox*>(ui->gridLayout_skyCoverage->itemAt(idx)->widget());
        QChartView *chartView = qobject_cast<QChartView*>(box->layout()->itemAt(1)->widget());
        QChart *chart = chartView->chart();

        QList<QAbstractSeries *> series = chart->series();
        QScatterSeriesExtended * data;
        QScatterSeriesExtended * selected;

        for(const auto &any:series){
            if(any->name() == "outside timespan"){
                data = static_cast<QScatterSeriesExtended *>(any);
            }
            if(any->name() == "selected"){
                selected = static_cast<QScatterSeriesExtended *>(any);
            }
        }
        selected->clear();

        int start = ui->horizontalSlider_start->value();
        int end = ui->horizontalSlider_end->value();

        for(int i=0; i<data->count(); ++i){

            bool flag1 = data->getStartTime(i) >= start && data->getStartTime(i) <= end;
            bool flag2 = data->getEndTime(i) >= start && data->getEndTime(i) <= end;
            bool flag3 = data->getStartTime(i) <= start && data->getEndTime(i) >= end;
            bool flag = flag1 || flag2 || flag3;

            if(flag){
                if( ids.indexOf(data->getSrcid(i)) != -1){
                    selected->append(data->at(i).x(), data->at(i).y(), data->getStartTime(i), data->getEndTime(i), data->getCableWrapFlag(i), data->getSrcid(i), data->getNSta(i));
                }
            }
        }
    }
}


void VieSchedpp_Analyser::on_checkBox_skyCoverageLegend_toggled(bool checked)
{
    if(ui->checkBox_skyCoverageLegend->isChecked()){
        for(int i=0; i<ui->gridLayout_skyCoverage->count(); ++i){
            QGroupBox *box = qobject_cast<QGroupBox*>(ui->gridLayout_skyCoverage->itemAt(i)->widget());
            QChartView *chartView = qobject_cast<QChartView*>(box->layout()->itemAt(1)->widget());
            QChart *chart = chartView->chart();
            QLegend *l = chart->legend();
            l->show();
        }

    }else{

        for(int i=0; i<ui->gridLayout_skyCoverage->count(); ++i){
            QGroupBox *box = qobject_cast<QGroupBox*>(ui->gridLayout_skyCoverage->itemAt(i)->widget());
            QChartView *chartView = qobject_cast<QChartView*>(box->layout()->itemAt(1)->widget());
            QChart *chart = chartView->chart();
            QLegend *l = chart->legend();
            l->hide();
        }

    }

}


void VieSchedpp_Analyser::setupWorldmap()
{
    ChartView *worldmap = new ChartView(this);
    qtUtil::worldMap(worldmap);
    QChart *worldChart = worldmap->chart();

    QScatterSeries *selectedStations = new QScatterSeries(worldChart);
    selectedStations->setName("stations");
    selectedStations->setMarkerSize(10);
    selectedStations->setBrush(QBrush(Qt::red,Qt::SolidPattern));
    selectedStations->setPen(QColor(Qt::white));

    QScatterSeries *observingStations = new QScatterSeries(worldChart);
    observingStations->setName("observing stations");
    QImage img(":/icons/icons/station_white.png");
    img = img.scaled(27,27);
    observingStations->setBrush(QBrush(img));
    observingStations->setMarkerSize(27);
    observingStations->setPen(QColor(Qt::transparent));


    connect(selectedStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));
    connect(observingStations,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));

    const std::vector<VieVS::Station> &stations = schedule_.getNetwork().getStations();
    for(const VieVS::Station &station : stations){
        double lat = station.getPosition().getLat()*rad2deg;
        double lon = station.getPosition().getLon()*rad2deg;
        selectedStations->append(lon,lat);
    }
    for(int i=0; i<stations.size(); ++i){
        double lat1 = stations.at(i).getPosition().getLat()*rad2deg;
        double lon1 = stations.at(i).getPosition().getLon()*rad2deg;
        QString name1 = QString::fromStdString(stations.at(i).getAlternativeName());

        for(int j=i+1; j<stations.size(); ++j){
            double lat2 = stations.at(j).getPosition().getLat()*rad2deg;
            double lon2 = stations.at(j).getPosition().getLon()*rad2deg;
            QString name2 = QString::fromStdString(stations.at(j).getAlternativeName());

            QList<QLineSeries *>series = qtUtil::baselineSeries(lat1,lon1,name1,lat2,lon2,name2);
            for(const auto &any: series){
                worldChart->addSeries(any);
                any->attachAxis(worldChart->axisX());
                any->attachAxis(worldChart->axisY());
                connect(any,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_baseline_hovered(QPointF,bool)));

            }

        }
    }

    worldChart->addSeries(selectedStations);
    worldChart->addSeries(observingStations);

    Callout *callout = new Callout(worldChart);
    callout->hide();
    selectedStations->attachAxis(worldChart->axisX());
    selectedStations->attachAxis(worldChart->axisY());
    observingStations->attachAxis(worldChart->axisX());
    observingStations->attachAxis(worldChart->axisY());

    ui->horizontalLayout_worldmap->insertWidget(0,worldmap,1);

    connect(ui->checkBox_showBaselines,SIGNAL(toggled(bool)),this,SLOT(updateWorldmapTimes()));
}

void VieSchedpp_Analyser::worldmap_hovered(QPointF point, bool state)
{
    QObject *obj = sender();

    QChart *chart = qobject_cast<QChart *>(obj->parent()->parent());

    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *worldMapCallout = dynamic_cast<Callout *>(childItem)){

            if (state) {
                QString sta;
                QString scans;
                QString obs;
                const std::vector<VieVS::Station> &stations = schedule_.getNetwork().getStations();
                for(const VieVS::Station &station : stations){
                    double y = station.getPosition().getLat()*rad2deg;
                    double x = station.getPosition().getLon()*rad2deg;

                    auto dx = x-point.x();
                    auto dy = y-point.y();
                    if(dx*dx+dy*dy < 1e-3){
                        if(sta.size()==0){
                            sta.append(QString("%1 (%2)").arg(QString::fromStdString(station.getName())).arg(QString::fromStdString(station.getAlternativeName())));
                        }else{
                            sta.append(QString(", %1 (%2)").arg(QString::fromStdString(station.getName())).arg(QString::fromStdString(station.getAlternativeName())));
                        }
                        auto stations = staModel->findItems(QString::fromStdString(station.getName()));
                        int row = stations.at(0)->row();
                        QString nscans = staModel->item(row,2)->text();
                        QString nobs = staModel->item(row,3)->text();

                        if(scans.size()==0){
                            scans.append(QString("%1").arg(nscans));
                        }else{
                            scans.append(QString(", %1").arg(nscans));
                        }
                        if(obs.size()==0){
                            obs.append(QString("%1").arg(nobs));
                        }else{
                            obs.append(QString(", %1").arg(nobs));
                        }
                    }
                }

                QString text = QString("%1 \n#scans %2\n#obs %3\nlat %4 [deg] \nlon %5 [deg] ").arg(sta).arg(scans).arg(obs).arg(point.y()).arg(point.x());
                worldMapCallout->setText(text);
                worldMapCallout->setAnchor(point);
                worldMapCallout->setZValue(11);
                worldMapCallout->updateGeometry();
                worldMapCallout->show();
            } else {
                worldMapCallout->hide();
            }
            break;
        }
    }
}

void VieSchedpp_Analyser::worldmap_baseline_hovered(QPointF point, bool state)
{
    QObject *obj = sender();

    QLineSeries *series = qobject_cast<QLineSeries *>(obj);
    QChart *chart = qobject_cast<QChart *>(obj->parent()->parent());

    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *worldMapCallout = dynamic_cast<Callout *>(childItem)){

            if (state) {

                QString name = series->name().left(5);
                int row = blModel->findItems(name).at(0)->row();
                int obs = blModel->index(row,2).data().toInt();

                QString longName = blModel->index(row,1).data().toString();
                double dist = blModel->index(row,3).data().toDouble();

                QString text = QString("%1 \n%2 \n#scans %3\ndist %4 [km] ").arg(name).arg(longName).arg(obs).arg(dist);

                worldMapCallout->setText(text);
                worldMapCallout->setAnchor(point);
                worldMapCallout->setZValue(11);
                worldMapCallout->updateGeometry();
                worldMapCallout->show();
            } else {
                worldMapCallout->hide();
            }
            break;
        }
    }

}


void VieSchedpp_Analyser::setupSkymap()
{
    ChartView *skymap = new ChartView();

    qtUtil::skyMap(skymap);

    QChart *skyChart = skymap->chart();



    QScatterSeries *selectedSources = new QScatterSeries(skyChart);
    selectedSources->setMarkerSize(10);
    selectedSources->setBrush(QBrush(Qt::gray,Qt::SolidPattern));
    selectedSources->setPen(QColor(Qt::white));

    QScatterSeries *observedSources = new QScatterSeries(skyChart);
    observedSources->setName("observed sources");
    QImage img(":/icons/icons/source_white.png");
    img = img.scaled(24,24);
    observedSources->setBrush(QBrush(img));
    observedSources->setMarkerSize(24);
    observedSources->setPen(QColor(Qt::transparent));

    skyChart->addSeries(selectedSources);
    skyChart->addSeries(observedSources);

    connect(selectedSources,SIGNAL(hovered(QPointF,bool)),this,SLOT(skymap_hovered(QPointF,bool)));
    connect(observedSources,SIGNAL(hovered(QPointF,bool)),this,SLOT(skymap_hovered(QPointF,bool)));

    const std::vector<VieVS::Source> &sources = schedule_.getSources();
    for(const VieVS::Source &source : sources){
        double ra = source.getRa();
        double lambda = ra;

        double phi = source.getDe();

        auto xy = qtUtil::radec2xy(lambda, phi);

        selectedSources->append(xy.first, xy.second);
    }
    Callout *callout = new Callout(skyChart);
    callout->hide();

    selectedSources->attachAxis(skyChart->axisX());
    selectedSources->attachAxis(skyChart->axisY());
    observedSources->attachAxis(skyChart->axisX());
    observedSources->attachAxis(skyChart->axisY());

    ui->horizontalLayout_skymap->insertWidget(0,skymap,1);
}

void VieSchedpp_Analyser::skymap_hovered(QPointF point, bool state)
{
    QObject *obj = sender();

    QChart *chart = qobject_cast<QChart *>(obj->parent()->parent());

    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *skyMapCallout = dynamic_cast<Callout *>(childItem)){

            if (state) {
                const std::vector<VieVS::Source> &sources = schedule_.getSources();
                QString text;
                for(const VieVS::Source &source : sources){
                    double ra = source.getRa();
                    double dec = source.getDe();

                    auto xy = qtUtil::radec2xy(ra, dec);

                    auto dx = xy.first-point.x();
                    auto dy = xy.second-point.y();
                    if(dx*dx+dy*dy < 1e-3){
                        if(source.hasAlternativeName()){
                            text = QString("%1 (%2)\n#scans %3 \n#obs %4\nra %5 [deg] \ndec %6 [deg] ").arg(QString::fromStdString(source.getName())).arg(QString::fromStdString(source.getAlternativeName())).arg(source.getNscans()).arg(source.getNObs()).arg(ra*rad2deg).arg(dec*rad2deg);
                        }else{
                            text = QString("%1 \n#scans %2 \n#obs %3\nra %4 [deg] \ndec %5 [deg] ").arg(QString::fromStdString(source.getName())).arg(source.getNscans()).arg(source.getNObs()).arg(ra*rad2deg).arg(dec*rad2deg);
                        }
                        break;
                    }
                }

                skyMapCallout->setText(text);
                skyMapCallout->setAnchor(point);
                skyMapCallout->setZValue(11);
                skyMapCallout->updateGeometry();
                skyMapCallout->show();
            } else {
                skyMapCallout->hide();
            }

            break;
        }
    }

}

void VieSchedpp_Analyser::on_lineEdit_skymapSourceFilter_textChanged(const QString &arg1)
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(ui->treeView_skymap_sources->model());
    model->setFilterFixedString(arg1);
}



void VieSchedpp_Analyser::on_lineEdit_worldmapStationFilter_textChanged(const QString &arg1)
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(ui->treeView_worldmap_stations->model());
    model->setFilterFixedString(arg1);
}

void VieSchedpp_Analyser::on_lineEdit_worldmapBaselineFilter_textChanged(const QString &arg1)
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(ui->treeView_worldmap_baselines->model());
    model->setFilterFixedString(arg1);
}


void VieSchedpp_Analyser::on_treeView_worldmap_stations_entered(const QModelIndex &index)
{
    auto model = ui->treeView_worldmap_stations->model();
    int row = index.row();
    QString name = model->index(row,0).data().toString();
    QString id = model->index(row,1).data().toString();

    int scans = model->index(row,2).data().toInt();
    int obs = model->index(row,3).data().toInt();
    double lat = model->index(row,4).data().toDouble();
    double lon = model->index(row,5).data().toDouble();

    QString text = QString("%1 (%2) \n#scans %3\n#obs %4\nlat %5 [deg] \nlon %6 [deg] ").arg(name).arg(id).arg(scans).arg(obs).arg(lat).arg(lon);

    auto chartview = static_cast<ChartView *>(ui->horizontalLayout_worldmap->itemAt(0)->widget());
    QChart *chart = chartview->chart();
    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *worldMapCallout = dynamic_cast<Callout *>(childItem)){

            worldMapCallout->setText(text);
            worldMapCallout->setAnchor(QPointF(lon, lat));
            worldMapCallout->setZValue(11);
            worldMapCallout->updateGeometry();
            worldMapCallout->show();
            break;
        }
    }
}

void VieSchedpp_Analyser::on_treeView_worldmap_baselines_entered(const QModelIndex &index)
{

    auto chartview = static_cast<ChartView *>(ui->horizontalLayout_worldmap->itemAt(0)->widget());
    QChart *chart = chartview->chart();

    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *worldMapCallout = dynamic_cast<Callout *>(childItem)){

            auto model = ui->treeView_worldmap_baselines->model();
            int row = index.row();
            QString name = model->index(row,0).data().toString();
            QString longName = model->index(row,1).data().toString();
            int obs = model->index(row,2).data().toInt();
            double dist = model->index(row,3).data().toDouble();

            if(obs>0){
                QString text = QString("%1 \n%2 \n#scans %3\ndist %4 [km] ").arg(name).arg(longName).arg(obs).arg(dist);

                auto series = chart->series();

                QList<QLineSeries *>found;
                for(const auto &any:series){
                    if(any->name().left(5) == name){
                        found.append(qobject_cast<QLineSeries *>(any));
                    }
                }
                QList<double> fd;
                for(const auto &any: found){

                    QPointF delta = any->at(0) - any->at(1);
                    fd.append( std::sqrt(delta.x()*delta.x()+delta.y()*delta.y()) );
                }

                int i = std::distance(fd.begin(),std::max_element(fd.begin(),fd.end()));
                QPointF latlon = (found.at(i)->at(0)+found.at(i)->at(1))/2;

                worldMapCallout->setText(text);
                worldMapCallout->setAnchor(latlon);
                worldMapCallout->setZValue(11);
                worldMapCallout->updateGeometry();
                worldMapCallout->show();
                break;
            } else {
                worldMapCallout->hide();
                break;
            }
        }

    }
}


void VieSchedpp_Analyser::on_treeView_skymap_sources_entered(const QModelIndex &index)
{
    auto model = ui->treeView_skymap_sources->model();
    int row = index.row();
    QString name = model->index(row,0).data().toString();
    QString id = model->index(row,1).data().toString();

    int scans = model->index(row,2).data().toInt();
    int obs = model->index(row,3).data().toInt();
    double ra = model->index(row,4).data().toDouble();
    double dec = model->index(row,5).data().toDouble();
    auto xy = qtUtil::radec2xy(ra*deg2rad,dec*deg2rad);

    QString text;
    if(id.isEmpty()){
        text = QString("%1 \n#scans %2\n#obs %3\nra %4 [deg] \ndec %5 [deg] ").arg(name).arg(scans).arg(obs).arg(ra).arg(dec);
    }else{
        text = QString("%1 (%2) \n#scans %3\n#obs %4\nra %5 [deg] \ndec %6 [deg] ").arg(name).arg(id).arg(scans).arg(obs).arg(ra).arg(dec);
    }

    auto chartview = static_cast<ChartView *>(ui->horizontalLayout_skymap->itemAt(0)->widget());
    QChart *chart = chartview->chart();
    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *skymapCallout = dynamic_cast<Callout *>(childItem)){

            skymapCallout->setText(text);
            skymapCallout->setAnchor(QPointF(xy.first, xy.second));
            skymapCallout->setZValue(11);
            skymapCallout->updateGeometry();
            skymapCallout->show();

        }
    }
}

void VieSchedpp_Analyser::statisticsGeneralSetup()
{

    ui->treeWidget_general_highlight->setColumnCount(2);
    ui->label_general_title->setStyleSheet("font-weight: bold");

    int nsta = schedule_.getNetwork().getNSta();
    QVector<int> nstaPerScan(nsta+1,0);

    for(const VieVS::Scan &scan : schedule_.getScans()){
        ++nstaPerScan[scan.getNSta()];
    }

    QPieSeries * staPerScan = new QPieSeries();
    staPerScan->setName("#stations per scan");

    for(int i=2; i<=nsta; ++i){
        QString name = QString("%1 sta").arg(i);
        int n = nstaPerScan[i];
        if(n>0){
            staPerScan->append(name,n);
        }
    }

    staPerScan->setLabelsVisible(true);
    staPerScan->setLabelsPosition(QPieSlice::LabelOutside);

    QChart *staPerScanChart = new QChart();
    staPerScanChart->setAnimationOptions(QChart::NoAnimation);
    staPerScanChart->createDefaultAxes();
    staPerScanChart->addSeries(staPerScan);
    QChartView *staPerScanChartView = new QChartView(staPerScanChart,this);
    staPerScanChart->setTitle("number of station per scan");
    staPerScanChartView->setRenderHint(QPainter::Antialiasing);
    ui->horizontalLayout_general_total->insertWidget(1,staPerScanChartView,1);

    QBarSeries *barSeries = new QBarSeries();
    QBarSet *barSet = new QBarSet("time [s]");
    barSeries->append(barSet);

    QChart *obsDurChart = new QChart();
    obsDurChart->legend()->hide();
    obsDurChart->setAnimationOptions(QChart::NoAnimation);
    obsDurChart->addSeries(barSeries);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    QValueAxis *axisY = new QValueAxis;
    axisX->setTitleText("[s]");
    axisY->setTitleText("# obs");
    obsDurChart->setAxisY(axisY, barSeries);
    obsDurChart->setAxisX(axisX, barSeries);
    obsDurChart->setTitle("time per observation");
    QChartView *obsDurChartView = new QChartView(obsDurChart,this);
    obsDurChartView->setRenderHint(QPainter::Antialiasing);
    ui->horizontalLayout_general_total->insertWidget(2,obsDurChartView,1);

    int maxObsDur = 0;
    for(const VieVS::Scan &scan: schedule_.getScans()){
        for(int i=0; i<scan.getNSta(); ++i){
            if(scan.getTimes().getObservingDuration(i) > maxObsDur){
                maxObsDur = scan.getTimes().getObservingDuration(i);
            }
        }
    }

    int newMax = maxObsDur/10*10+10;
    int cache = (1+newMax/100)*10;

    int upper_bound = cache;
    while (upper_bound < newMax+cache){
        histogram_upperLimits_.append(upper_bound);
        upper_bound += cache;
    }

    int lower = 0;
    QStringList labels;
    for(int i=0; i<histogram_upperLimits_.count(); ++i){
        int higher = histogram_upperLimits_.at(i)-1;
        labels << QString("%0-%1").arg(lower).arg(higher);
        lower = higher+1;
    }
    axisX->append(labels);
    axisX->setLabelsAngle(-90);

//    barSeries->attachAxis(axisX);
//    barSeries->attachAxis(axisY);


    staPerScanChart->legend()->hide();
    staPerScanChartView->setMouseTracking(true);
    connect(staPerScan,SIGNAL(hovered(QPieSlice*,bool)),this,SLOT(staPerScanPieHovered(QPieSlice*,bool)));
    connect(barSet,SIGNAL(hovered(bool,int)),this,SLOT(timePerObservation_hovered(bool,int)));
}

void VieSchedpp_Analyser::staPerScanPieHovered(QPieSlice *slice, bool state)
{

    if(state){
        QString name = slice->label().split(" ").at(0);
        int sta = name.toInt();
        int n = slice->value();
        double p = slice->percentage();

        ui->treeWidget_general_highlight->clear();
        ui->label_general_title->setText("number of stations per scan");
        ui->treeWidget_general_highlight->addTopLevelItem(new QTreeWidgetItem(QStringList() << "highlighted" << QString("%1 station scans").arg(sta)));
        ui->treeWidget_general_highlight->addTopLevelItem(new QTreeWidgetItem(QStringList() << "#scans" << QString("%1").arg(n)));
        ui->treeWidget_general_highlight->addTopLevelItem(new QTreeWidgetItem(QStringList() << "percentage" << QString("%1 [%]").arg(p*100,0,'f',2)));

        ui->treeWidget_general_highlight->resizeColumnToContents(0);
        ui->treeWidget_general_highlight->resizeColumnToContents(1);

        slice->setExploded(true);
        slice->setPen(QPen(Qt::darkRed));
    }else{
        ui->treeWidget_general_highlight->clear();
        ui->label_general_title->setText("hovered item");
        slice->setExploded(false);
        slice->setPen(QPen(Qt::white));
    }


}


void VieSchedpp_Analyser::timePerObservation_hovered(bool state, int idx)
{
    if(state){
        QBarSet *set = qobject_cast<QBarSet *>(sender());
        int n = set->at(idx);
        double total = 0;
        for(int i=0; i<set->count(); ++i){
            total += set->at(i);
        }
        QChart *chart = qobject_cast<QChart *>(set->parent()->parent()->parent());
        QBarCategoryAxis *axis =  qobject_cast<QBarCategoryAxis *>(chart->axisX());
        QString label = axis->at(idx);
        QString min = label.split("-").at(0);
        QString max = label.split("-").at(1);

        ui->treeWidget_general_highlight->clear();
        ui->label_general_title->setText("time per observation");
        ui->treeWidget_general_highlight->addTopLevelItem(new QTreeWidgetItem(QStringList() << "min time" << QString("%1 [s]").arg(min)));
        ui->treeWidget_general_highlight->addTopLevelItem(new QTreeWidgetItem(QStringList() << "max time" << QString("%1 [s]").arg(max)));
        ui->treeWidget_general_highlight->addTopLevelItem(new QTreeWidgetItem(QStringList() << "#obs" << QString("%1").arg(n)));
        ui->treeWidget_general_highlight->addTopLevelItem(new QTreeWidgetItem(QStringList() << "percentage" << QString("%1 [%]").arg(n/total*100,0,'f',2)));

        ui->treeWidget_general_highlight->resizeColumnToContents(0);
        ui->treeWidget_general_highlight->resizeColumnToContents(1);

    }else{
        ui->treeWidget_general_highlight->clear();
        ui->label_general_title->setText("hovered item");

    }

}

void VieSchedpp_Analyser::updatePlotsAndModels()
{
    int nsta = staModel->rowCount();
    QVector<int> scansStation(nsta,0);
    QVector<int> obsStation(nsta,0);
    int nsrc = srcModel->rowCount();
    QVector<int> scansSource(nsrc,0);
    QVector<int> obsSource(nsrc,0);
    int nbl = blModel->rowCount();
    QVector<int> scansBaseline(nbl,0);

    int start = ui->horizontalSlider_start->value();
    int end = ui->horizontalSlider_end->value();

    for(const VieVS::Scan &scan : schedule_.getScans()){
        bool flag1 = scan.getTimes().getObservingTime(VieVS::Timestamp::start) >= start && scan.getTimes().getObservingTime(VieVS::Timestamp::start) <= end;
        bool flag2 = scan.getTimes().getObservingTime(VieVS::Timestamp::end)   >= start && scan.getTimes().getObservingTime(VieVS::Timestamp::end) <= end;
        bool flag3 = scan.getTimes().getObservingTime(VieVS::Timestamp::start) <= start && scan.getTimes().getObservingTime(VieVS::Timestamp::end) >= end;
        bool flag = flag1 || flag2 || flag3;

        if(flag){
            ++scansSource[scan.getSourceId()];
            for(int i = 0; i<scan.getNSta(); ++i){
                unsigned int pvStart = scan.getPointingVector(i,VieVS::Timestamp::start).getTime();
                unsigned int pvEnd = scan.getPointingVector(i,VieVS::Timestamp::end).getTime();
                unsigned int obsStart = scan.getTimes().getObservingTime(i,VieVS::Timestamp::start);
                unsigned int obsEnd = scan.getTimes().getObservingTime(i,VieVS::Timestamp::end);

                bool flag11 = scan.getPointingVector(i,VieVS::Timestamp::start).getTime() >= start && scan.getPointingVector(i,VieVS::Timestamp::start).getTime() <= end;
                bool flag21 = scan.getPointingVector(i,VieVS::Timestamp::end).getTime()   >= start && scan.getPointingVector(i,VieVS::Timestamp::end).getTime()   <= end;
                bool flag31 = scan.getPointingVector(i,VieVS::Timestamp::start).getTime() <= start && scan.getPointingVector(i,VieVS::Timestamp::end).getTime()   >= end;
                bool flag01 = flag11 || flag21 || flag31;

                if(flag01){
                    ++scansStation[scan.getPointingVector(i).getStaid()];
                }
            }
            for(int i = 0; i<scan.getNObs(); ++i){

                bool flag11 = scan.getObservation(i).getStartTime() >= start && scan.getObservation(i).getStartTime() <= end;
                bool flag21 = scan.getObservation(i).getStartTime()+scan.getObservation(i).getObservingTime() >= start && scan.getObservation(i).getStartTime()+scan.getObservation(i).getObservingTime() <= end;
                bool flag31 = scan.getObservation(i).getStartTime() <= start && scan.getObservation(i).getStartTime()+scan.getObservation(i).getObservingTime()   >= end;
                bool flag01 = flag11 || flag21 || flag31;

                if(flag01){
                    ++obsStation[scan.getObservation(i).getStaid1()];
                    ++obsStation[scan.getObservation(i).getStaid2()];
                    ++obsSource[scan.getSourceId()];
                    ++scansBaseline[scan.getObservation(i).getBlid()];
                }
            }
        }
    }

    for(int i=0; i<staModel->rowCount();++i){
        staModel->setData(staModel->index(i,2), scansStation[i]);
        staModel->setData(staModel->index(i,3), obsStation[i]);
    }
    for(int i=0; i<blModel->rowCount();++i){
        blModel->setData(blModel->index(i,2), scansBaseline[i]);
    }
    for(int i=0; i<srcModel->rowCount();++i){
        srcModel->setData(srcModel->index(i,2), scansSource[i]);
        srcModel->setData(srcModel->index(i,3), obsSource[i]);
    }

    int idx = ui->stackedWidget->currentIndex();
    switch(idx){
        case 0:{ updateSkyCoverageTimes(); break; }
        case 1:{ updateWorldmapTimes(); break; }
        case 2:{ break; }
        case 3:{ updateSkymapTimes(); break; }
        case 4:{ updateGeneralStatistics(); break; }
        case 5:{ updateStatisticsStations(); break; }
        case 6:{ updateStatisticsSourceTimes(); break; }
        case 7:{ updateStatisticsBaseline(); break; }
        default:{ break;}
    }

}

void VieSchedpp_Analyser::updateWorldmapTimes()
{
    auto chartview = static_cast<ChartView *>(ui->horizontalLayout_worldmap->itemAt(0)->widget());
    QChart *chart = chartview->chart();
    const auto &aseries = chart->series();
    for(const auto &any:aseries){
        if(any->name() == "observing stations"){
            QScatterSeries *series = qobject_cast<QScatterSeries *>(any);
            series->clear();

            for(int i=0; i<staModel->rowCount(); ++i){
                int n = staModel->index(i,2).data().toInt();
                if( n > 0){
                    series->append(staModel->index(i,5).data().toDouble(), staModel->index(i,4).data().toDouble());
                }
            }
        }else if(any->name() != "stations" && any->name() != "coast" ){
            QString name = any->name().left(5);

            int row = blModel->findItems(name).at(0)->row();
            int n = blModel->index(row,2).data().toInt();
            if(n>0 && ui->checkBox_showBaselines->isChecked()){
                any->show();
            }else{
                any->hide();
            }
        }
    }
}

void VieSchedpp_Analyser::updateSkymapTimes()
{
    auto chartview = static_cast<ChartView *>(ui->horizontalLayout_skymap->itemAt(0)->widget());
    QChart *chart = chartview->chart();
    const auto &aseries = chart->series();
    for(const auto &any:aseries){
        if(any->name() == "observed sources"){
            QScatterSeries *series = qobject_cast<QScatterSeries *>(any);
            series->clear();

            for(int i=0; i<srcModel->rowCount(); ++i){
                int n = srcModel->index(i,2).data().toInt();
                if( n > 0){

                    double ra = srcModel->index(i,4).data().toDouble();
                    double phi = srcModel->index(i,5).data().toDouble();
                    double lambda = ra;


                    auto xy = qtUtil::radec2xy(lambda*deg2rad, phi*deg2rad);

                    series->append(xy.first, xy.second);
                }
            }
        }
    }
}

void VieSchedpp_Analyser::updateGeneralStatistics()
{
    int nsta = schedule_.getNetwork().getNSta();
    QVector<int> nstaPerScan(nsta+1,0);
    int nsrc = schedule_.getSources().size();
    QVector<int> stations(nsta,0);
    QVector<int> sources(nsrc,0);


    int start = ui->horizontalSlider_start->value();
    int end = ui->horizontalSlider_end->value();

    for(const VieVS::Scan &scan : schedule_.getScans()){
        bool flag1 = scan.getTimes().getObservingTime(VieVS::Timestamp::start) >= start && scan.getTimes().getObservingTime(VieVS::Timestamp::start) <= end;
        bool flag2 = scan.getTimes().getObservingTime(VieVS::Timestamp::end)   >= start && scan.getTimes().getObservingTime(VieVS::Timestamp::end) <= end;
        bool flag3 = scan.getTimes().getObservingTime(VieVS::Timestamp::start) <= start && scan.getTimes().getObservingTime(VieVS::Timestamp::end) >= end;
        bool flag = flag1 || flag2 || flag3;
        if(flag){
            ++nstaPerScan[scan.getNSta()];
        }
    }
    using namespace boost::accumulators;

    accumulator_set<double, stats< tag::mean, tag::median, tag::variance, tag::min, tag::max> > accStaScan;
    accumulator_set<double, stats< tag::mean, tag::median, tag::variance, tag::min, tag::max> > accStaObs;

    int sta = 0;
    for(int i=0; i<staModel->rowCount();++i){
        int thisScan = staModel->data(staModel->index(i,2)).toInt();
        int thisObs =  staModel->data(staModel->index(i,3)).toInt();
        if(thisScan > 0){
            ++sta;
        }
        accStaScan (thisScan);
        accStaObs (thisObs);
    }
    double h = (ui->horizontalSlider_end->value()-ui->horizontalSlider_start->value())/3600.;
    ui->tableWidget_general->setItem(0,0,new QTableWidgetItem(QString::number(mean(accStaScan),'f',2)));
    ui->tableWidget_general->setItem(0,1,new QTableWidgetItem(QString::number(std::sqrt(variance(accStaScan)),'f',2)));
    ui->tableWidget_general->setItem(0,2,new QTableWidgetItem(QString::number(median(accStaScan),'f',2)));
    ui->tableWidget_general->setItem(0,3,new QTableWidgetItem(QString::number(min(accStaScan))));
    ui->tableWidget_general->setItem(0,4,new QTableWidgetItem(QString::number(max(accStaScan))));
    ui->tableWidget_general->setItem(0,5,new QTableWidgetItem(QString::number(mean(accStaScan)/h,'f',2)));


    ui->tableWidget_general->setItem(3,0,new QTableWidgetItem(QString::number(mean(accStaObs),'f',2)));
    ui->tableWidget_general->setItem(3,1,new QTableWidgetItem(QString::number(std::sqrt(variance(accStaObs)),'f',2)));
    ui->tableWidget_general->setItem(3,2,new QTableWidgetItem(QString::number(median(accStaObs),'f',2)));
    ui->tableWidget_general->setItem(3,3,new QTableWidgetItem(QString::number(min(accStaObs))));
    ui->tableWidget_general->setItem(3,4,new QTableWidgetItem(QString::number(max(accStaObs))));
    ui->tableWidget_general->setItem(3,5,new QTableWidgetItem(QString::number(mean(accStaObs)/h,'f',2)));


    accumulator_set<double, stats< tag::mean, tag::median, tag::variance, tag::min, tag::max> > accSrcScan;
    accumulator_set<double, stats< tag::mean, tag::median, tag::variance, tag::min, tag::max> > accSrcObs;

    int scans = 0;
    int obs = 0;
    int src = 0;
    for(int i=0; i<srcModel->rowCount();++i){
        int thisScan = srcModel->data(srcModel->index(i,2)).toInt();
        int thisObs =  srcModel->data(srcModel->index(i,3)).toInt();

        scans += thisScan;
        obs   += thisObs;
        if(thisScan > 0){
            ++src;
        }
        accSrcScan (thisScan);
        accSrcObs (thisObs);
    }
    ui->tableWidget_general->setItem(1,0,new QTableWidgetItem(QString::number(mean(accSrcScan),'f',2)));
    ui->tableWidget_general->setItem(1,1,new QTableWidgetItem(QString::number(std::sqrt(variance(accSrcScan)),'f',2)));
    ui->tableWidget_general->setItem(1,2,new QTableWidgetItem(QString::number(median(accSrcScan),'f',2)));
    ui->tableWidget_general->setItem(1,3,new QTableWidgetItem(QString::number(min(accSrcScan))));
    ui->tableWidget_general->setItem(1,4,new QTableWidgetItem(QString::number(max(accSrcScan))));
    ui->tableWidget_general->setItem(1,5,new QTableWidgetItem(QString::number(mean(accSrcScan)/h,'f',2)));

    ui->tableWidget_general->setItem(4,0,new QTableWidgetItem(QString::number(mean(accSrcObs),'f',2)));
    ui->tableWidget_general->setItem(4,1,new QTableWidgetItem(QString::number(std::sqrt(variance(accSrcObs)),'f',2)));
    ui->tableWidget_general->setItem(4,2,new QTableWidgetItem(QString::number(median(accSrcObs),'f',2)));
    ui->tableWidget_general->setItem(4,3,new QTableWidgetItem(QString::number(min(accSrcObs))));
    ui->tableWidget_general->setItem(4,4,new QTableWidgetItem(QString::number(max(accSrcObs))));
    ui->tableWidget_general->setItem(4,5,new QTableWidgetItem(QString::number(mean(accSrcObs)/h,'f',2)));

    accumulator_set<double, stats< tag::mean, tag::median, tag::variance, tag::min, tag::max> > accBlScan;

    int bl = 0;
    for(int i=0; i<blModel->rowCount();++i){
        int thisScan = blModel->data(blModel->index(i,2)).toInt();
        if(thisScan > 0){
            ++bl;
        }
        accBlScan(thisScan);
    }
    ui->tableWidget_general->setItem(2,0,new QTableWidgetItem(QString::number(mean(accBlScan),'f',2)));
    ui->tableWidget_general->setItem(2,1,new QTableWidgetItem(QString::number(std::sqrt(variance(accBlScan)),'f',2)));
    ui->tableWidget_general->setItem(2,2,new QTableWidgetItem(QString::number(median(accBlScan),'f',2)));
    ui->tableWidget_general->setItem(2,3,new QTableWidgetItem(QString::number(min(accBlScan))));
    ui->tableWidget_general->setItem(2,4,new QTableWidgetItem(QString::number(max(accBlScan))));
    ui->tableWidget_general->setItem(2,5,new QTableWidgetItem(QString::number(mean(accBlScan)/h,'f',2)));


    ui->spinBox_scans->setValue(scans);
    ui->spinBox_observations->setValue(obs);
    ui->spinBox_stations->setValue(sta);
    ui->spinBox_sources->setValue(src);
    ui->spinBox_baselines->setValue(bl);

    QChartView *chartview = qobject_cast<QChartView *>(ui->horizontalLayout_general_total->itemAt(1)->widget());
    QChart *staPerScanChart = chartview->chart();
    QPieSeries *staPerScan= qobject_cast<QPieSeries *>(staPerScanChart->series().at(0));
    staPerScan->clear();

    for(int i=2; i<=nsta; ++i){
        QString name = QString("%1 sta").arg(i);
        int n = nstaPerScan[i];
        if(n>0){
            staPerScan->append(name,n);
        }
    }

    staPerScan->setLabelsVisible(true);
    staPerScan->setLabelsPosition(QPieSlice::LabelOutside);

    for(int r = 0; r<ui->tableWidget_general->rowCount(); ++r){
        for(int c = 0; c<ui->tableWidget_general->columnCount(); ++c){
            ui->tableWidget_general->item(r,c)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    QChartView *barChartView = qobject_cast<QChartView *>(ui->horizontalLayout_general_total->itemAt(2)->widget());
    QChart *barChart = barChartView->chart();
    QBarSeries *barSeries= qobject_cast<QBarSeries *>(barChart->series().at(0));
    QBarSet *barSet = barSeries->barSets().at(0);
    barSet->remove(0,barSet->count());

    QValueAxis *axisY = qobject_cast<QValueAxis *>(barChart->axisY());

    QVector<int> values;
    for(const VieVS::Scan &scan: schedule_.getScans()){
        for(int i=0; i<scan.getNSta(); ++i){
            int tstart = scan.getTimes().getObservingTime(i,VieVS::Timestamp::start);
            int tend = scan.getTimes().getObservingTime(i,VieVS::Timestamp::end);
            bool flag1 = tstart >= start && tstart <= end;
            bool flag2 = tend   >= start && tend <= end;
            bool flag3 = tstart <= start && tend >= end;
            bool flag = flag1 || flag2 || flag3;
            if(flag){
                values.append(scan.getTimes().getObservingDuration(i));
            }
        }
    }
    qSort(values.begin(),values.end());

    if(values.empty()){
        return;
    }

    QVector<int> hist(histogram_upperLimits_.count(),0);
    for (int any:values) {
        int i = 0;
        while(any>=histogram_upperLimits_[i]){
            ++i;
        }
        ++hist[i];
    }
    for(auto any: hist){
        barSet->append(any);
    }

    axisY->setRange(0, *std::max_element(hist.begin(),hist.end())/10*10+10);
}

void VieSchedpp_Analyser::statisticsStationsSetup()
{

    QPieSeries *scans = new QPieSeries();
    QPieSeries *obs = new QPieSeries();
    scans->setName("#scans");
    obs->setName("#obs");

    for(int i=0; i<staModel->rowCount(); ++i){
        QString name = staModel->data(staModel->index(i,0)).toString();
        int thisScan = staModel->data(staModel->index(i,2)).toInt();
        int thisObs =  staModel->data(staModel->index(i,3)).toInt();

        if(thisScan>0){
            scans->append(name,thisScan);
        }
        if(thisObs>0){
            obs->append(name,thisObs);
        }
    }

    scans->setLabelsVisible(true);
    scans->setLabelsPosition(QPieSlice::LabelOutside);

    obs->setLabelsVisible(true);
    obs->setLabelsPosition(QPieSlice::LabelOutside);

    QChart *scansChart = new QChart();
    scansChart->legend()->hide();
    scansChart->setAnimationOptions(QChart::NoAnimation);
    scansChart->createDefaultAxes();
    scansChart->addSeries(scans);
    scansChart->setTitle("#scans");

    QChart *obsChart = new QChart();
    obsChart->legend()->hide();
    obsChart->setAnimationOptions(QChart::NoAnimation);
    obsChart->createDefaultAxes();
    obsChart->addSeries(obs);
    obsChart->setTitle("#obs");

    QChartView *scansChartView = new QChartView(scansChart,this);
    scansChartView->setRenderHint(QPainter::Antialiasing);

    QChartView *obsChartView = new QChartView(obsChart,this);
    obsChartView->setRenderHint(QPainter::Antialiasing);

    connect(scans,SIGNAL(hovered(QPieSlice*,bool)),this,SLOT(stationsScansPieHovered(QPieSlice*,bool)));
    connect(obs  ,SIGNAL(hovered(QPieSlice*,bool)),this,SLOT(stationsObsPieHovered(QPieSlice*,bool)));

    ui->horizontalLayout_statistics_station_model->insertWidget(1,scansChartView,2);
    ui->horizontalLayout_statistics_station_model->insertWidget(2,obsChartView,2);

    for(int i=0; i<staModel->rowCount(); ++i){
        QTreeWidgetItem *itm = new QTreeWidgetItem();
        itm->setIcon(0,QIcon(":/icons/icons/station.png"));
        itm->setText(0,staModel->item(i,0)->text());
        ui->treeWidget_statistics_station_time->insertTopLevelItem(i+1, itm);
    }
    ui->treeWidget_statistics_station_time->sortByColumn(0,Qt::AscendingOrder);
    for(int i=0; i<6 ; ++i){
        ui->treeWidget_statistics_station_time->resizeColumnToContents(i);
    }

    QBarSet *bfs = new QBarSet("field system");
    QBarSet *bslew = new QBarSet("slew");
    QBarSet *bidle = new QBarSet("idle");
    QBarSet *bpreob = new QBarSet("preob");
    QBarSet *bobs= new QBarSet("observation");

    QPercentBarSeries *series = new QPercentBarSeries();
    series->append(bfs);
    series->append(bslew);
    series->append(bidle);
    series->append(bpreob);
    series->append(bobs);
    connect(series,SIGNAL(hovered(bool,int,QBarSet*)),this,SLOT(stationsTimeBarHovered(bool,int,QBarSet*)));

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("time spent");
    chart->setAnimationOptions(QChart::NoAnimation);

    QStringList categories;
    categories << "Average";
    for(int i=0; i<staModel->rowCount(); ++i){
        QString name = staModel->data(staModel->index(i,0)).toString();
        categories << name;
    }
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    axis->setLabelsAngle(-90);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->horizontalLayout_statistics_station_times->insertWidget(1,chartView,10);

}

void VieSchedpp_Analyser::updateStatisticsStations()
{
    QChartView *scansChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(1)->widget());
    QChartView *obsChartView   = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(2)->widget());

    QPieSeries *scans_ = qobject_cast<QPieSeries *>(scansChartView->chart()->series().at(0));
    QPieSeries *obs_   = qobject_cast<QPieSeries *>(obsChartView->chart()->series().at(0));

    scans_->clear();
    obs_->clear();

    for(int i=0; i<staModel->rowCount(); ++i){
        QString name = staModel->data(staModel->index(i,0)).toString();
        int thisScan = staModel->data(staModel->index(i,2)).toInt();
        int thisObs =  staModel->data(staModel->index(i,3)).toInt();

        if(thisScan>0){
            scans_->append(name,thisScan);
        }
        if(thisObs>0){
            obs_->append(name,thisObs);
        }
    }

    scans_->setLabelsVisible(true);
    scans_->setLabelsPosition(QPieSlice::LabelOutside);

    obs_->setLabelsVisible(true);
    obs_->setLabelsPosition(QPieSlice::LabelOutside);

    ui->label_statistics_station_model_hover_title->setStyleSheet("font-weight: bold");

    int start = ui->horizontalSlider_start->value();
    int end = ui->horizontalSlider_end->value();

    int nsta = staModel->rowCount();
    QVector<double> fs(nsta+1,0);
    QVector<double> slew(nsta+1,0);
    QVector<double> idle(nsta+1,0);
    QVector<double> preob(nsta+1,0);
    QVector<double> obs(nsta+1,0);
    QVector<int> eols(nsta+1,0);
    for(const VieVS::Scan &scan:schedule_.getScans()){
        for(int i=0; i<scan.getNSta(); ++i){
            int pos = scan.getStationId(i)+1;
            VieVS::ScanTimes t = scan.getTimes();

            int fsStart = t.getFieldSystemTime(i,VieVS::Timestamp::start);
            int fsEnd   = t.getFieldSystemTime(i,VieVS::Timestamp::end);
            if((fsStart >= start && fsEnd <= end)){
                fs[pos] += fsEnd-fsStart;     // inside
            }else if(fsEnd <= start || fsStart >=end){
                                                // outside
            }else if(fsStart <= start && fsEnd <= end){
                fs[pos] += fsEnd-start;       // start outside
            }else if(fsStart >= start && fsEnd >= end){
                fs[pos] += end-fsStart;       // end outside
            }else if((fsStart <= start && fsEnd >= end)){
                fs[pos] += end-start;         // over
            }

            int slewStart = t.getSlewTime(i,VieVS::Timestamp::start);
            int slewEnd   = t.getSlewTime(i,VieVS::Timestamp::end);
            if((slewStart >= start && slewEnd <= end)){
                slew[pos] += slewEnd-slewStart;     // inside
            }else if(slewEnd <= start || slewStart >=end){
                                                // outside
            }else if(slewStart <= start && slewEnd <= end){
                slew[pos] += slewEnd-start;       // start outside
            }else if(slewStart >= start && slewEnd >= end){
                slew[pos] += end-slewStart;       // end outside
            }else if((slewStart <= start && slewEnd >= end)){
                slew[pos] += end-start;         // over
            }


            int idleStart = t.getIdleTime(i,VieVS::Timestamp::start);
            int idleEnd   = t.getIdleTime(i,VieVS::Timestamp::end);
            if((idleStart >= start && idleEnd <= end)){
                idle[pos] += idleEnd-idleStart;     // inside
            }else if(idleEnd <= start || idleStart >=end){
                                                // outside
            }else if(idleStart <= start && idleEnd <= end){
                idle[pos] += idleEnd-start;       // start outside
            }else if(idleStart >= start && idleEnd >= end){
                idle[pos] += end-idleStart;       // end outside
            }else if((idleStart <= start && idleEnd >= end)){
                idle[pos] += end-start;         // over
            }

            int preobStart = t.getPreobTime(i,VieVS::Timestamp::start);
            int preobEnd   = t.getPreobTime(i,VieVS::Timestamp::end);
            if((preobStart >= start && preobEnd <= end)){
                preob[pos] += preobEnd-preobStart;     // inside
            }else if(preobEnd <= start || preobStart >=end){
                                                // outside
            }else if(preobStart <= start && preobEnd <= end){
                preob[pos] += preobEnd-start;       // start outside
            }else if(preobStart >= start && preobEnd >= end){
                preob[pos] += end-preobStart;       // end outside
            }else if((preobStart <= start && preobEnd >= end)){
                preob[pos] += end-start;         // over
            }

            int obsStart = t.getObservingTime(i,VieVS::Timestamp::start);
            int obsEnd   = t.getObservingTime(i,VieVS::Timestamp::end);
            if((obsStart >= start && obsEnd <= end)){
                obs[pos] += obsEnd-obsStart;     // inside
            }else if(obsEnd <= start || obsStart >=end){
                                                // outside
            }else if(obsStart <= start && obsEnd <= end){
                obs[pos] += obsEnd-start;       // start outside
            }else if(obsStart >= start && obsEnd >= end){
                obs[pos] += end-obsStart;       // end outside
            }else if((obsStart <= start && obsEnd >= end)){
                obs[pos] += end-start;         // over
            }

            int fs_eols = eols[pos];
            if((fs_eols != fsStart) || (fsEnd != slewStart) || (slewEnd != idleStart) || (idleEnd != preobStart) || (preobEnd != obsStart)){
                bool invalid = false;
            }
            eols[pos] = obsEnd;
        }
    }
    fs[0] = std::accumulate(fs.begin(),fs.end(),0)/static_cast<double>(nsta);
    slew[0] = std::accumulate(slew.begin(),slew.end(),0)/static_cast<double>(nsta);
    idle[0] = std::accumulate(idle.begin(),idle.end(),0)/static_cast<double>(nsta);
    preob[0] = std::accumulate(preob.begin(),preob.end(),0)/static_cast<double>(nsta);
    obs[0] = std::accumulate(obs.begin(),obs.end(),0)/static_cast<double>(nsta);

    QChartView *chartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_times->itemAt(1)->widget());
    QChart *chart = chartView->chart();
    QPercentBarSeries *series = qobject_cast<QPercentBarSeries *>(chart->series().at(0));
    for(const auto &barSet : series->barSets()){
        barSet->remove(0,barSet->count());

        if(barSet->label() == "field system"){
            for(auto &any: fs){
                barSet->append(any);
            }
        }else if(barSet->label() == "slew"){
            for(auto &any: slew){
                barSet->append(any);
            }
        }else if(barSet->label() == "idle"){
            for(auto &any: idle){
                barSet->append(any);
            }
        }else if(barSet->label() == "preob"){
            for(auto &any: preob){
                barSet->append(any);
            }
        }else if(barSet->label() == "observation"){
            for(auto &any: obs){
                barSet->append(any);
            }
        }
    }



    QVector<double> pfs(nsta+1,0);
    QVector<double> pslew(nsta+1,0);
    QVector<double> pidle(nsta+1,0);
    QVector<double> ppreob(nsta+1,0);
    QVector<double> pobs(nsta+1,0);
    for(int i=0; i<nsta+1; ++i){
        double total = fs[i]+slew[i]+idle[i]+preob[i]+obs[i];
        pfs[i]    = fs[i]/total*100;
        pslew[i]  = slew[i]/total*100;
        pidle[i]  = idle[i]/total*100;
        ppreob[i] = preob[i]/total*100;
        pobs[i]   = obs[i]/total*100;
    }

    for(int r=0; r<ui->treeWidget_statistics_station_time->topLevelItemCount(); ++r){
        auto itm = ui->treeWidget_statistics_station_time->topLevelItem(r);
        QString name = itm->text(0);
        int idx;
        if(name == "Average"){
            idx = 0;
        }else{
            for(int i=0; i<staModel->rowCount();++i){
                if(staModel->item(i,0)->text() == name){
                    idx = i+1;
                    break;
                }
            }
        }
        itm->setData(1,0,std::round(pfs[idx]*100)/100.0);
        itm->setData(2,0,std::round(pslew[idx]*100)/100.0);
        itm->setData(3,0,std::round(pidle[idx]*100)/100.0);
        itm->setData(4,0,std::round(ppreob[idx]*100)/100.0);
        itm->setData(5,0,std::round(pobs[idx]*100)/100.0);
    }


}

void VieSchedpp_Analyser::statisticsSourceSetup()
{
    QSortFilterProxyModel *pmodel = new QSortFilterProxyModel(this);
    pmodel->setSourceModel(srcModel);
    pmodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView_statistics_source->setModel(pmodel);
    for( int i=0 ; i<ui->treeView_statistics_source->model()->columnCount(); ++i){
        ui->treeView_statistics_source->resizeColumnToContents(i);
    }
    pmodel->sort(0);

//    connect(ui->treeView_statistics_source,SIGNAL(clicked(QModelIndex)),this,SLOT(updateStatisticsSource()));

    ui->splitter_statistics_source->setStretchFactor(0,1);
    ui->splitter_statistics_source->setStretchFactor(1,5);
    ui->splitter_statistics_source->setSizes({1000,5000});

    QChart *chart = new QChart();

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    axisX->setFormat("hh:mm");
    axisX->setRange(sessionStart_,sessionEnd_);
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0,90);
    axisY->setTitleText("elevation [deg]");
    chart->addAxis(axisY, Qt::AlignLeft);


    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QList<QColor> c;

    c.append(QColor(228,26,28));
    c.append(QColor(55,126,184));
    c.append(QColor(77,175,74));
    c.append(QColor(152,78,163));
    c.append(QColor(255,127,0));
    c.append(QColor(255,255,51));
    c.append(QColor(166,86,40));
    c.append(QColor(247,129,191));
    c.append(QColor(153,153,153));

    for(int i=0; i<staModel->rowCount(); ++i){
        QString name = staModel->item(i,0)->text();
        QLineSeries *series = new QLineSeries();
        series->setName(name);

        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
        int nc = i/9;
        int cc = i%9;
        if(nc == 0){
            series->setPen(QPen(QBrush(c.at(cc)),1.5,Qt::SolidLine));
        }else if(nc == 1){
            series->setPen(QPen(QBrush(c.at(cc)),1.5,Qt::DashLine));
        }else if(nc == 2){
            series->setPen(QPen(QBrush(c.at(cc)),1.5,Qt::DotLine));
        }else if(nc == 3){
            series->setPen(QPen(QBrush(c.at(cc)),1.5,Qt::DashDotLine));
        }else if(nc == 4){
            series->setPen(QPen(QBrush(c.at(cc)),1.5,Qt::DashDotDotLine));
        }
        connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(statisticsSourceHovered(QPointF, bool)));
    }

    QLineSeries *series = new QLineSeries();
    series->setName("observed");
    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    series->setPen(QPen(QBrush(Qt::black),4,Qt::SolidLine));

    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
    chart->setAcceptHoverEvents(true);
    Callout *callout = new Callout(chart);
    callout->hide();

    ui->horizontalLayout_statistics_source->insertWidget(0,chartView,1);

    connect(ui->treeView_statistics_source->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),SLOT(updateStatisticsSource()));
    ui->treeView_statistics_source->setCurrentIndex(ui->treeView_statistics_source->model()->index(0,0));

}

void VieSchedpp_Analyser::updateStatisticsSource()
{
    QChartView *chartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_source->itemAt(0)->widget());
    QChart *chart = chartView->chart();
    QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis *>(chart->axisX());

    QModelIndexList sel = ui->treeView_statistics_source->selectionModel()->selectedRows();

    QVector<int> ids;
    for(const auto &any : sel){
        QString name = ui->treeView_statistics_source->model()->data(any).toString();
        int id = srcModel->findItems(name).at(0)->row();
        ids.push_back(id);
    }
    int idx = ids[0];
    QString name = ui->treeView_statistics_source->model()->index(idx,0).data().toString();

    const VieVS::Source &src = schedule_.getSources().at(idx);
    chart->setTitle(QString::fromStdString(src.getName()));
    const std::vector<VieVS::Station> stations = schedule_.getNetwork().getStations();

    int istart = ui->horizontalSlider_start->value();
    int iend   = ui->horizontalSlider_end->value();
    QDateTime start = sessionStart_.addSecs(istart);
    QDateTime end = sessionStart_.addSecs(iend);

    auto series = chart->series();

    for(const VieVS::Station &sta : stations){

        QLineSeries *serie;
        for(const auto &any:series){
            if(any->name() == QString::fromStdString(sta.getName())){
                serie = qobject_cast<QLineSeries *>(any);
                break;
            }
        }
        serie->clear();

        for(int i = 0; i<=ui->horizontalSlider_end->maximum()+300; i+=300){
            QDateTime t = sessionStart_.addSecs(i);

            VieVS::PointingVector pv(sta.getId(),src.getId());
            pv.setTime(i);

            sta.calcAzEl(src,pv);
            double el = pv.getEl()*rad2deg;
            serie->append(t.toMSecsSinceEpoch(),el);
        }
    }

    for(const auto &any:series){
        if(any->name().left(4) == "obs_"){
            chart->removeSeries(any);
            delete(any);
        }
    }


    chart->legend()->setMarkerShape(QLegend::MarkerShapeRectangle);
    for(const VieVS::Scan &scan : schedule_.getScans()){
        if(scan.getSourceId() == idx){

            for(int i=0; i<scan.getNSta(); ++i){
                QLineSeriesExtended *series = new QLineSeriesExtended();
                int staid = scan.getStationId(i);
                QString name = QString::fromStdString(stations.at(staid).getName());
                series->setName("obs_" + name);
                chart->addSeries(series);
                series->attachAxis(axisX);
                series->attachAxis(chart->axisY());
                series->setPen(QPen(QBrush(Qt::black),4,Qt::SolidLine));
                chart->legend()->markers(series).at(0)->setVisible(false);

                VieVS::PointingVector s = scan.getPointingVector(i,VieVS::Timestamp::start);
                VieVS::PointingVector e = scan.getPointingVector(i,VieVS::Timestamp::end);

                double sel = s.getEl()*rad2deg;
                double eel = e.getEl()*rad2deg;
                int st = s.getTime();
                int et = e.getTime();

                series->append(sessionStart_.addSecs(st).toMSecsSinceEpoch(),sel);
                series->append(sessionStart_.addSecs(et).toMSecsSinceEpoch(),eel);
                series->setNSta(scan.getNSta());
                series->setNObs(scan.getNObs());
                series->setStartTime(st);
                series->setEndTime(et);

                connect(series,SIGNAL(hovered(QPointF,bool)),this,SLOT(statisticsSourceHovered(QPointF, bool)));
            }
        }
    }
    chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    updateStatisticsSourceTimes();
}

void VieSchedpp_Analyser::updateStatisticsSourceTimes()
{
    QChartView *chartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_source->itemAt(0)->widget());
    QChart *chart = chartView->chart();
    QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis *>(chart->axisX());
    int istart = ui->horizontalSlider_start->value();
    int iend   = ui->horizontalSlider_end->value();
    QDateTime start = sessionStart_.addSecs(istart);
    QDateTime end = sessionStart_.addSecs(iend);
    axisX->setRange(start,end);

}

void VieSchedpp_Analyser::statisticsSourceHovered(QPointF point, bool state)
{


    QChartView *chartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_source->itemAt(0)->widget());
    QChart *chart = chartView->chart();

    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *c = dynamic_cast<Callout *>(childItem)){
            if(state){
                QAbstractSeries *as = qobject_cast<QAbstractSeries *>(sender());
                QString txt;
                if(as->name().left(4) == "obs_"){
                    QLineSeriesExtended *series = static_cast<QLineSeriesExtended *>(as);

                    int startTime = series->getStartTime();
                    int endTime = series->getEndTime();
                    QDateTime qStartTime = sessionStart_.addSecs(startTime);
                    QDateTime qEndTime   = sessionStart_.addSecs(endTime);
                    QString startTimeStr = qStartTime.toString("hh:mm:ss");
                    QString endTimeStr   = qEndTime.toString("hh:mm:ss");
                    QString timeStr = startTimeStr.append("-").append(endTimeStr).append("\n");
                    QString el = QString().sprintf("elevation: %.2f [deg]\n", point.y());
                    QString nsta = QString().sprintf("#sta: %d\n#obs: %d", series->getNSta(), series->getNObs());

                    txt.append(as->name().mid(4)+"\n").append(timeStr).append(el).append(nsta);


                }else{
                    QLineSeries *series = qobject_cast<QLineSeries *>(as);
                    QString name = series->name();
                    QDateTime time = QDateTime::fromMSecsSinceEpoch(point.x());
                    double el = point.y();

                    txt = QString("%1\n%2\nelevation: %3 [deg]").arg(name).arg(time.toString("yy.MM.dd hh:mm:ss")).arg(el,0,'f',2);
                }


                c->setText(txt);
                c->setAnchor(point);
                c->setZValue(11);
                c->updateGeometry();
                c->show();
            }else{
                c->hide();
            }
        }
    }

}

void VieSchedpp_Analyser::stationsScansPieHovered(QPieSlice *slice, bool state)
{
    QChartView *scansChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(1)->widget());
    QPieSeries *scans = qobject_cast<QPieSeries *>(scansChartView->chart()->series().at(0));
    for(const auto &any: scans->slices()){
        any->setExploded(false);
    }

    QTreeWidget *t = ui->treeWidget_statistics_station_selected;
    if(state){
        QString name = slice->label();
        int n = slice->value();
        double p = slice->percentage();

        t->clear();
        ui->label_statistics_station_model_hover_title->setText("number of scans");
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "station" << name));
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "#scans" << QString("%1").arg(n)));
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "percentage" << QString("%1 [%]").arg(p*100,0,'f',2)));

        t->resizeColumnToContents(0);
        t->resizeColumnToContents(1);

        slice->setExploded(true);
        slice->setPen(QPen(Qt::darkRed));
    }else{
        t->clear();
        ui->label_statistics_station_model_hover_title->setText("hovered item");
        slice->setExploded(false);
        slice->setPen(QPen(Qt::white));
    }

    QChartView *obsChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(2)->widget());
    QPieSeries *obs = qobject_cast<QPieSeries *>(obsChartView->chart()->series().at(0));
    for(const auto &any: obs->slices()){
        any->setExploded(false);
    }

}

void VieSchedpp_Analyser::stationsObsPieHovered(QPieSlice *slice, bool state)
{
    QChartView *obsChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(2)->widget());
    QPieSeries *obs = qobject_cast<QPieSeries *>(obsChartView->chart()->series().at(0));
    for(const auto &any: obs->slices()){
        any->setExploded(false);
    }

    QTreeWidget *t = ui->treeWidget_statistics_station_selected;
    if(state){
        QString name = slice->label();
        int n = slice->value();
        double p = slice->percentage();

        t->clear();
        ui->label_statistics_station_model_hover_title->setText("number of observations");
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "station" << name));
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "#obs" << QString("%1").arg(n)));
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "percentage" << QString("%1 [%]").arg(p*100,0,'f',2)));

        t->resizeColumnToContents(0);
        t->resizeColumnToContents(1);

        slice->setExploded(true);
        slice->setPen(QPen(Qt::darkRed));
    }else{
        t->clear();
        ui->label_statistics_station_model_hover_title->setText("hovered item");
        slice->setExploded(false);
        slice->setPen(QPen(Qt::white));
    }

    QChartView *scansChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(1)->widget());
    QPieSeries *scans = qobject_cast<QPieSeries *>(scansChartView->chart()->series().at(0));
    for(const auto &any: scans->slices()){
        any->setExploded(false);
    }

}

void VieSchedpp_Analyser::stationsTimeBarHovered(bool state, int idx, QBarSet *set)
{
    QChartView *obsChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(2)->widget());
    QPieSeries *obs = qobject_cast<QPieSeries *>(obsChartView->chart()->series().at(0));
    for(const auto &any: obs->slices()){
        any->setExploded(false);
    }
    QChartView *scansChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(1)->widget());
    QPieSeries *scans = qobject_cast<QPieSeries *>(scansChartView->chart()->series().at(0));
    for(const auto &any: scans->slices()){
        any->setExploded(false);
    }

    QTreeWidget *t = ui->treeWidget_statistics_station_selected;
    if(state){
        QChartView *chartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_times->itemAt(1)->widget());
        QChart *chart = chartView->chart();
        QBarCategoryAxis *axis =  qobject_cast<QBarCategoryAxis *>(chart->axisX());
        QString station = axis->at(idx);
        QString name = set->label();
        int n = std::roundl(set->at(idx));

        t->clear();
        ui->label_statistics_station_model_hover_title->setText(name+" time");
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "station" << station));
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "seconds" << QString("%1").arg(set->at(idx),0,'f',2)));
        double min = n/60.0;
        if(min>1){
            t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "minutes" << QString("%2").arg(min,0,'f',2)));
        }
        double hours = min/60.0;
        if(hours>1){
            t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "hours" << QString("%2").arg(hours,0,'f',2)));
        }
        QString duration;
        duration.sprintf("%02d:%02d:%02d", n/3600, n/60%60, n%60);
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "duration" << duration));
    }else{
        t->clear();
        ui->label_statistics_station_model_hover_title->setText("hovered item");
    }
}

void VieSchedpp_Analyser::on_treeWidget_statistics_station_time_entered(const QModelIndex &index)
{
    QChartView *obsChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(2)->widget());
    QPieSeries *obs = qobject_cast<QPieSeries *>(obsChartView->chart()->series().at(0));
    for(const auto &any: obs->slices()){
        any->setExploded(false);
    }
    QChartView *scansChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(1)->widget());
    QPieSeries *scans = qobject_cast<QPieSeries *>(scansChartView->chart()->series().at(0));
    for(const auto &any: scans->slices()){
        any->setExploded(false);
    }


    QTreeWidget *t = ui->treeWidget_statistics_station_selected;
    t->clear();

    QTreeWidget *b = ui->treeWidget_statistics_station_time;
    int row = index.row();
    QString name = b->topLevelItem(row)->text(0);

    QChartView *chartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_times->itemAt(1)->widget());
    QChart *chart = chartView->chart();
    QBarCategoryAxis *axis =  qobject_cast<QBarCategoryAxis *>(chart->axisX());
    int idx;
    for(int i=0; i<axis->count(); ++i){
        if(axis->at(i) == name){
            idx = i;
            break;
        }
    }

    t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "station" << name));
    QPercentBarSeries *series = qobject_cast<QPercentBarSeries *>(chart->series().at(0));
    for(const auto &set: series->barSets()){
        t->addTopLevelItem(new QTreeWidgetItem(QStringList() << set->label()+" time" << QString("%1 [s]").arg(set->at(idx))));
    }
    ui->label_statistics_station_model_hover_title->setText("time spent");

}


void VieSchedpp_Analyser::on_treeView_statistics_station_model_entered(const QModelIndex &index)
{

    QTreeWidget *t = ui->treeWidget_statistics_station_selected;
    t->clear();

    auto model = ui->treeView_statistics_station_model->model();
    int row = index.row();
    QString name = model->index(row,0).data().toString();


    QChartView *scansChartView = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(1)->widget());
    QChartView *obsChartView   = qobject_cast<QChartView *>(ui->horizontalLayout_statistics_station_model->itemAt(2)->widget());

    QPieSeries *scans = qobject_cast<QPieSeries *>(scansChartView->chart()->series().at(0));
    QPieSeries *obs   = qobject_cast<QPieSeries *>(obsChartView->chart()->series().at(0));

    t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "station" << name));

    ui->label_statistics_station_model_hover_title->setText("number of scans/observations");
    for(const auto &any: scans->slices()){
        if(any->label() == name){
            any->setExploded(true);
            t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "#scans" << QString("%1").arg(any->value())));
            t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "percentage" << QString("%1 [%]").arg(any->percentage()*100,0,'f',2)));
        }else{
            any->setExploded(false);
        }
    }

    for(const auto &any: obs->slices()){
        if(any->label() == name){
            any->setExploded(true);
            t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "#obs" << QString("%1").arg(any->value())));
            t->addTopLevelItem(new QTreeWidgetItem(QStringList() << "percentage" << QString("%1 [%]").arg(any->percentage()*100,0,'f',2)));
        }else{
            any->setExploded(false);
        }
    }

}


void VieSchedpp_Analyser::on_lineEdit_statistics_source_filter_textChanged(const QString &arg1)
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(ui->treeView_statistics_source->model());
    model->setFilterFixedString(arg1);
}

void VieSchedpp_Analyser::on_lineEdit_statistics_baseline_filter_textChanged(const QString &arg1)
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(ui->treeView_statistics_baseline->model());
    model->setFilterFixedString(arg1);
    updateStatisticsBaseline();
}

void VieSchedpp_Analyser::statisticsBaselineSetup()
{

    ui->splitter_statistics_baseline->setStretchFactor(0,1);
    ui->splitter_statistics_baseline->setStretchFactor(1,5);
    ui->splitter_statistics_baseline->setSizes({1000,5000});

    QSortFilterProxyModel *m = new QSortFilterProxyModel(this);
    m->setSourceModel(blModel);
    m->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView_statistics_baseline->setModel(m);
    for( int i=0 ; i<ui->treeView_statistics_baseline->model()->columnCount(); ++i){
        ui->treeView_statistics_baseline->resizeColumnToContents(i);
    }
    m->sort(0);


    ChartView *worldmap = new ChartView(this);
    qtUtil::worldMap(worldmap);
    QChart *worldChart = worldmap->chart();

    QScatterSeries *stationsSeries = new QScatterSeries(worldChart);
    stationsSeries->setName("stations");
    QImage img(":/icons/icons/station_white.png");
    img = img.scaled(27,27);
    stationsSeries->setBrush(QBrush(img));
    stationsSeries->setMarkerSize(27);
    stationsSeries->setPen(QColor(Qt::transparent));


    connect(stationsSeries,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_hovered(QPointF,bool)));

    const std::vector<VieVS::Station> &stations = schedule_.getNetwork().getStations();
    for(const VieVS::Station &station : stations){
        double lat = station.getPosition().getLat()*rad2deg;
        double lon = station.getPosition().getLon()*rad2deg;
        stationsSeries->append(lon,lat);
    }
    for(int i=0; i<stations.size(); ++i){
        double lat1 = stations.at(i).getPosition().getLat()*rad2deg;
        double lon1 = stations.at(i).getPosition().getLon()*rad2deg;
        QString name1 = QString::fromStdString(stations.at(i).getAlternativeName());

        for(int j=i+1; j<stations.size(); ++j){
            double lat2 = stations.at(j).getPosition().getLat()*rad2deg;
            double lon2 = stations.at(j).getPosition().getLon()*rad2deg;
            QString name2 = QString::fromStdString(stations.at(j).getAlternativeName());

            QList<QLineSeries *>series = qtUtil::baselineSeries(lat1,lon1,name1,lat2,lon2,name2);
            for(const auto &any: series){
                worldChart->addSeries(any);
                any->attachAxis(worldChart->axisX());
                any->attachAxis(worldChart->axisY());
                connect(any,SIGNAL(hovered(QPointF,bool)),this,SLOT(worldmap_baseline_hovered(QPointF,bool)));
            }
        }
    }

    worldChart->addSeries(stationsSeries);

    Callout *callout = new Callout(worldChart);
    callout->hide();
    stationsSeries->attachAxis(worldChart->axisX());
    stationsSeries->attachAxis(worldChart->axisY());

    ui->verticalLayout_statistics_baseline->insertWidget(1,worldmap,1);

}

void VieSchedpp_Analyser::updateStatisticsBaseline()
{

    QTreeView *t = ui->treeView_statistics_baseline;
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(t->model());

    QStringList allBls;
    for(int i=0; i<blModel->rowCount();++i){
        allBls.append(blModel->data(blModel->index(i,0)).toString());
    }


    QList<QColor>ref;
    ref.append(QColor(255,255,229));
    ref.append(QColor(247,252,185));
    ref.append(QColor(217,240,163));
    ref.append(QColor(173,221,142));
    ref.append(QColor(120,198,121));
    ref.append(QColor(65,171,93));
    ref.append(QColor(35,132,67));
    ref.append(QColor(0,104,55));
    ref.append(QColor(0,69,41));

    QStringList bls;
    QVector<int> n;

    for(int i=0; i<model->rowCount(); ++i){
        QString name = model->data(model->index(i,0)).toString();
        int thisObs =  model->data(model->index(i,2)).toInt();
        bls.append(name);
        n.append(thisObs);
    }

    double nMax = *std::max_element(n.begin(),n.end());
    double nMin = 0;

    double step = (nMax-nMin)/8;
    QVector<double> edges;

    for(int i=0; i<9; ++i){
        edges.append(nMin+i*step);
    }
    edges.append(nMax);

    QList<QColor> colors;
    for(const auto &any: n){

        for(int istart=0; istart<edges.size()-1; ++istart){
            if(any>= edges[istart] && any<=edges[istart+1]){
                QColor sColor = ref[istart];
                QColor eColor = ref[istart+1];
                double sv = edges[istart];
                double ev = edges[istart+1];

                double d = (any-sv)/(ev-sv);

                int r = std::roundl(sColor.red()+d*(eColor.red()-sColor.red()));
                int g = std::roundl(sColor.green()+d*(eColor.green()-sColor.green()));
                int b = std::roundl(sColor.blue()+d*(eColor.blue()-sColor.blue()));

                colors.append(QColor(r,g,b));
                break;
            }
        }
    }

    auto chartview = static_cast<ChartView *>(ui->verticalLayout_statistics_baseline->itemAt(1)->widget());
    QChart *chart = chartview->chart();
    auto series = chart->series();


    for(const auto &s : series){
        QString name = s->name().left(5);

        if( allBls.indexOf(name) != -1 ){
            QLineSeries *ls = qobject_cast<QLineSeries *>(s);
            int idx = bls.indexOf(name);
            if( idx == -1){
                ls->hide();
            }else{
                ls->setPen(QPen(QBrush(colors.at(idx)),1.5,Qt::DashLine));
                ls->show();
            }
        }
    }
}

void VieSchedpp_Analyser::on_treeView_statistics_baseline_entered(const QModelIndex &index)
{
    auto chartview = static_cast<ChartView *>(ui->verticalLayout_statistics_baseline->itemAt(1)->widget());
    QChart *chart = chartview->chart();

    for(QGraphicsItem *childItem: chart->childItems()){
        if(Callout *worldMapCallout = dynamic_cast<Callout *>(childItem)){

            auto model = ui->treeView_statistics_baseline->model();
            int row = index.row();
            QString name = model->index(row,0).data().toString();
            QString longName = model->index(row,1).data().toString();
            int obs = model->index(row,2).data().toInt();
            double dist = model->index(row,3).data().toDouble();

            if(obs>0){
                QString text = QString("%1 \n%2 \n#scans %3\ndist %4 [km] ").arg(name).arg(longName).arg(obs).arg(dist);

                auto series = chart->series();

                QList<QLineSeries *>found;
                for(const auto &any:series){
                    if(any->name().left(5) == name){
                        found.append(qobject_cast<QLineSeries *>(any));
                    }
                }
                QList<double> fd;
                for(const auto &any: found){

                    QPointF delta = any->at(0) - any->at(1);
                    fd.append( std::sqrt(delta.x()*delta.x()+delta.y()*delta.y()) );
                }

                int i = std::distance(fd.begin(),std::max_element(fd.begin(),fd.end()));
                QPointF latlon = (found.at(i)->at(0)+found.at(i)->at(1))/2;

                worldMapCallout->setText(text);
                worldMapCallout->setAnchor(latlon);
                worldMapCallout->setZValue(11);
                worldMapCallout->updateGeometry();
                worldMapCallout->show();
                break;
            } else {
                worldMapCallout->hide();
                break;
            }
        }

    }
}

void VieSchedpp_Analyser::on_checkBox_statistics_baseline_showStations_toggled(bool checked)
{
    auto chartview = static_cast<ChartView *>(ui->verticalLayout_statistics_baseline->itemAt(1)->widget());
    QChart *chart = chartview->chart();
    auto series = chart->series();
    for(const auto &s : series){
        QString name = s->name();
        if(name == "stations"){
            if(checked){
                s->show();
            }else{
                s->hide();
            }
            break;
        }
    }
}
