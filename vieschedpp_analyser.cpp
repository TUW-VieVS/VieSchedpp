#include "vieschedpp_analyser.h"
#include "ui_vieschedpp_analyser.h"

VieSchedpp_Analyser::VieSchedpp_Analyser(VieVS::Scheduler schedule, QDateTime start, QDateTime end, QWidget *parent) :
    QMainWindow(parent), schedule_{schedule}, sessionStart_{start}, sessionEnd_{end},
    ui(new Ui::VieSchedpp_Analyser)
{
    ui->setupUi(this);

    ui->dateTimeEdit_start->setDateTimeRange(sessionStart_,sessionEnd_);
    ui->dateTimeEdit_end->setDateTimeRange(sessionStart_,sessionEnd_);
    ui->dateTimeEdit_start->setDateTime(sessionStart_);
    ui->dateTimeEdit_end->setDateTime(sessionEnd_);

    int duration = sessionStart_.secsTo(sessionEnd_);
    ui->spinBox_duration->setRange(0,duration);
    ui->spinBox_duration->setValue(duration);
    ui->horizontalSlider_start->setRange(0,duration);
    ui->horizontalSlider_end->setRange(0,duration);

    updateDuration();
}

VieSchedpp_Analyser::~VieSchedpp_Analyser()
{
    delete ui;
}

void VieSchedpp_Analyser::setup()
{
    ui->label_fileName->setText(QString::fromStdString(schedule_.getName()));

    srcModel = new QStandardItemModel(0,3,this);
    srcModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));
    srcModel->setHeaderData(1, Qt::Horizontal, QObject::tr("ra [deg]"));
    srcModel->setHeaderData(2, Qt::Horizontal, QObject::tr("de [deg]"));

    for(const VieVS::Source &any : schedule_.getSources()){
        QString sourceName = QString::fromStdString(any.getName());
        double ra = qRadiansToDegrees(any.getRa());
        double de = qRadiansToDegrees(any.getDe());
        srcModel->insertRow(0);
        srcModel->setData(srcModel->index(0,0), sourceName);
        srcModel->item(0,0)->setIcon(QIcon(":/icons/icons/source.png"));
        srcModel->setData(srcModel->index(0, 1), (double)((int)(ra*100000 +0.5))/100000.0);
        srcModel->setData(srcModel->index(0, 2), (double)((int)(de*100000 +0.5))/100000.0);
    }

    ui->treeView_skyCoverage_sources->setModel(srcModel);
    ui->treeView_skyCoverage_sources->resizeColumnToContents(0);

    staModel = new QStandardItemModel(0,1,this);
    staModel->setHeaderData(0, Qt::Horizontal, QObject::tr("name"));

    for(const VieVS::Station &any : schedule_.getNetwork().getStations()){
        QString staName = QString::fromStdString(any.getName());
        double lat = qRadiansToDegrees(any.getPosition().getLat());
        double lon = qRadiansToDegrees(any.getPosition().getLon());
        staModel->insertRow(0);
        staModel->setData(staModel->index(0,0), staName);
        staModel->item(0,0)->setIcon(QIcon(":/icons/icons/station.png"));
        staModel->setData(staModel->index(0, 1), (double)((int)(lat*100000 +0.5))/100000.0);
        staModel->setData(staModel->index(0, 2), (double)((int)(lon*100000 +0.5))/100000.0);
    }

//    comboBox2skyCoverage = new QSignalMapper(this);

    int stas = staModel->rowCount();
    if(stas == 2){
        setSkyCoverageLayout(1,2);
    }else if(stas == 3){
        setSkyCoverageLayout(1,3);
    }else if(stas == 4){
        setSkyCoverageLayout(2,2);
    }else if(stas >4){
        setSkyCoverageLayout(2,3);
    }

    ui->splitter_skyCoverage->setStretchFactor(0,5);
    ui->splitter_skyCoverage->setStretchFactor(1,1);
}

void VieSchedpp_Analyser::on_horizontalSlider_start_valueChanged(int value)
{
    QDateTime newStart = sessionStart_.addSecs(value);
    ui->dateTimeEdit_start->setDateTime(newStart);

    if(value>ui->horizontalSlider_end->value()){
        ui->horizontalSlider_end->setValue(value);
    }
    if(ui->checkBox_fixDuration->isChecked()){
        ui->horizontalSlider_end->setValue(value+ui->spinBox_duration->value());
    }else{
        updateDuration();
    }
    updateSkyCoverageTimes();
}

void VieSchedpp_Analyser::on_horizontalSlider_end_valueChanged(int value)
{
    QDateTime newEnd = sessionStart_.addSecs(value);
    ui->dateTimeEdit_end->setDateTime(newEnd);

    if(value<ui->horizontalSlider_start->value()){
        ui->horizontalSlider_start->setValue(value);
    }
    if(ui->checkBox_fixDuration->isChecked()){
        ui->horizontalSlider_start->setValue(value-ui->spinBox_duration->value());
    }else{
        updateDuration();
    }
    updateSkyCoverageTimes();
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
            angularAxis->setShadesBrush(QBrush(QColor(249, 249, 255)));
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

    QList<std::tuple<int, double, double, int>> list = qtUtil::pointingVectors2Lists(thisSkyCoverage.getPointingVectors());

    QScatterSeriesExtended *data = new QScatterSeriesExtended();
    for(const auto &any : list){
        double unaz = std::get<1>(any);
        double az = VieVS::util::wrapToPi(unaz)*rad2deg;
        if(az<0){
            az+=360;
        }
        VieVS::CableWrap::CableWrapFlag flag = thisSta.getCableWrap().cableWrapFlag(unaz);
        data->append(std::get<0>(any), az, 90-std::get<2>(any)*rad2deg, flag, std::get<3>(any));
    }
    data->setBrush(Qt::gray);
    data->setMarkerSize(7);
    data->setName("data");

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

    QScatterSeriesExtended *highlight = new QScatterSeriesExtended();
    highlight->setBrush(Qt::yellow);
    highlight->setPen(QPen(Qt::black));
    highlight->setName("highlight");

    chart->addSeries(highlight);
    highlight->attachAxis(chart->axisX());
    highlight->attachAxis(chart->axisY());
    connect(highlight, SIGNAL(hovered(QPointF,bool)), this, SLOT(skyCoverageHovered(QPointF,bool)));

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
    QScatterSeriesExtended * highlight;

    for(const auto &any:series){
        if(any->name() == "data"){
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
        if(any->name() == "highlight"){
            highlight = static_cast<QScatterSeriesExtended *>(any);
        }
    }

    ccw->clear();
    cw->clear();
    n->clear();
    highlight->clear();

    int start = ui->horizontalSlider_start->value();
    int end = ui->horizontalSlider_end->value();

    for(int i=0; i<data->count(); ++i){
        if(data->getTime(i) >= start && data->getTime(i) <= end){
            switch(data->getCableWrapFlag(i)){
                case VieVS::CableWrap::CableWrapFlag::n:{
                    n->append(data->getTime(i), data->at(i).x(), data->at(i).y(), data->getCableWrapFlag(i), data->getSrcid(i));
                    break;
                }
                case VieVS::CableWrap::CableWrapFlag::ccw:{
                    ccw->append(data->getTime(i), data->at(i).x(), data->at(i).y(), data->getCableWrapFlag(i), data->getSrcid(i));
                    break;
                }
                case VieVS::CableWrap::CableWrapFlag::cw:{
                    cw->append(data->getTime(i), data->at(i).x(), data->at(i).y(), data->getCableWrapFlag(i), data->getSrcid(i));
                    break;
                }
            }
        }
    }


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
                int time = series->getTime(idx);
                QDateTime qtime = sessionStart_.addSecs(time);
                QString timeStr = qtime.toString("hh:mm:ss").append("\n");
                QString az = QString().sprintf("az: %.2f\n", point.x());
                QString el = QString().sprintf("el: %.2f", point.y());

                QString txt = source;
                txt.append(timeStr).append(az).append(el);

                c->setText(txt);
                c->setZValue(11);
                c->updateGeometry();
                c->show();
            }else{
                c->hide();
            }

        }
    }
}















