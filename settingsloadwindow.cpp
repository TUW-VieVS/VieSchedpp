/* 
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settingsloadwindow.h"
#include "ui_settingsloadwindow.h"

settingsLoadWindow::settingsLoadWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsLoadWindow)
{
    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(":/icons/icons/VieSchedppGUI_logo.png"));
    this->setWindowTitle("VieSched++");

    ui->splitter->setStretchFactor(1,2);
    ui->name->setMouseTracking(true);
    connect(ui->name,SIGNAL(itemEntered(QListWidgetItem*)),this,SLOT(refreshList(QListWidgetItem*)));
}

settingsLoadWindow::~settingsLoadWindow()
{
    delete ui;
}

void settingsLoadWindow::setStationParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersStations> &paras)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::stationParameters;
    paraSta = paras;
}

void settingsLoadWindow::setSourceParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersSources> &paras)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::sourceParameters;
    paraSrc = paras;
}

void settingsLoadWindow::setBaselineParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersBaselines> &paras)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::baselineParameters;
    paraBl = paras;
}

void settingsLoadWindow::setStationGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::stationGroup;
    groupSta = members;
}

void settingsLoadWindow::setSourceGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::sourceGroup;
    groupSrc = members;
}

void settingsLoadWindow::setBaselineGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::baselineGroup;
    groupBl = members;
}

void settingsLoadWindow::setBands(const QVector<QString> &name, const QVector<QPair<double, int> > &bands_)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::bands;
    bands = bands_;

}

void settingsLoadWindow::setNetwork(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::network;
    network = members;
}

void settingsLoadWindow::setSourceList(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = Type::sourceList;
    sourceList = members;
}

void settingsLoadWindow::setModes(const QVector<QString> &names, const QVector<int> &bits,
                                  const QVector<double> &srates,
                                  const QVector<QVector<QString> > &bands,
                                  const QVector<QVector<int> > &channels,
                                  const QVector<QVector<double> > &freqs)
{
    for(const auto&any:names){
        ui->name->addItem(any);
    }
    type = Type::modes;

    this->bits = bits;
    this->srates = srates;
    this->modes_bands = bands;
    this->channels = channels;
    this->freqs = freqs;
}

QString settingsLoadWindow::selectedItem()
{
    auto list = ui->name->selectedItems();
    return list.at(0)->text();
}

int settingsLoadWindow::selectedIdx()
{
    int row = ui->name->selectionModel()->selectedRows(0).at(0).row();
    return row;
}

void settingsLoadWindow::refreshList(QListWidgetItem *itm)
{
    QString name = itm->text();
    int idx = itm->listWidget()->row(itm);
    switch (type) {
    case Type::stationParameters:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Parameter: %1").arg(name)));
        VieVS::ParameterSettings::ParametersStations para = paraSta.at(idx);
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
                t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore source"));
                t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/source.png"),QString::fromStdString(any)));
                ++r;
            }
        }
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);

        break;
    }
    case Type::sourceParameters:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Parameter: %1").arg(name)));
        VieVS::ParameterSettings::ParametersSources para = paraSrc.at(idx);
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
        if(para.fixedScanDuration.is_initialized()){
            t->insertRow(r);
            t->setItem(r,0,new QTableWidgetItem(QString::number(*para.fixedScanDuration)));
            t->setVerticalHeaderItem(r,new QTableWidgetItem("fixed scan duration [s]"));
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
            t->setVerticalHeaderItem(r,new QTableWidgetItem("min elevation [deg]"));
            ++r;
        }
        if(para.minSunDistance.is_initialized()){
            t->insertRow(r);
            t->setItem(r,0,new QTableWidgetItem(QString::number(*para.minSunDistance)));
            t->setVerticalHeaderItem(r,new QTableWidgetItem("min sun distance [deg]"));
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
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore station"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),QString::fromStdString(any)));
                  ++r;
              }
        }
        if(para.requiredStationsString.size() > 0){
              for(const auto &any: para.requiredStationsString){
                  t->insertRow(r);
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("required stations"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),QString::fromStdString(any)));
                  ++r;
              }

        }
        if(para.ignoreBaselinesString.size() > 0){
              for(const auto &any: para.ignoreBaselinesString){
                  t->insertRow(r);
                  t->setVerticalHeaderItem(r,new QTableWidgetItem("ignore baseline"));
                  t->setItem(r,0,new QTableWidgetItem(QIcon(":/icons/icons/baseline.png"),QString::fromStdString(any)));
                  ++r;
              }

        }
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
    case Type::baselineParameters:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Parameter: %1").arg(name)));
        VieVS::ParameterSettings::ParametersBaselines para = paraBl.at(idx);
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
        break;
    }
    case Type::stationGroup:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/station_group_2.png"),QString("Group: %1").arg(name)));
        auto members = groupSta.at(idx);
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = members.at(i);
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),txt));
        }
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
    case Type::sourceGroup:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/source_group.png"),QString("Group: %1").arg(name)));
        auto members = groupSrc.at(idx);
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = members.at(i);
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/source.png"),txt));
        }
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
    case Type::baselineGroup:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QIcon(":/icons/icons/baseline_group.png"),QString("Group: %1").arg(name)));
        auto members = groupBl.at(idx);
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = members.at(i);
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/baseline.png"),txt));
        }
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);

        break;
    }
    case Type::bands:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Band: %1").arg(name)));
        auto band = bands.at(idx);
        t->setRowCount(2);
        t->setItem(0,0,new QTableWidgetItem(QString::number(band.first)));
        t->setVerticalHeaderItem(0,new QTableWidgetItem("frequency [GHz]"));
        t->setItem(1,0,new QTableWidgetItem(QString::number(band.second)));
        t->setVerticalHeaderItem(1,new QTableWidgetItem("channels"));
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
    case Type::network:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("Network: %1").arg(name)));
        auto members = network.at(idx);
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = members.at(i);
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/station.png"),txt));
        }
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
    case Type::sourceList:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("source list: %1").arg(name)));
        auto members = sourceList.at(idx);
        t->setRowCount(members.size());
        for(int i=0; i<members.size(); ++i){
            QString txt = members.at(i);
            t->setItem(i,0,new QTableWidgetItem(QIcon(":/icons/icons/source.png"),txt));
        }
        QHeaderView *hv = t->verticalHeader();
        hv->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
    case Type::modes:{
        auto t = ui->para;
        t->clear();
        t->setColumnCount(1);
        t->setRowCount(0);
        t->setHorizontalHeaderItem(0,new QTableWidgetItem(QString("mode: %1").arg(name)));

        t->insertRow(0);
        t->setItem(0,0,new QTableWidgetItem(QString::number(srates.at(idx))));
        t->setVerticalHeaderItem(0,new QTableWidgetItem("sample rate [MHz]"));

        t->insertRow(1);
        t->setItem(1,0,new QTableWidgetItem(QString::number(bits.at(idx))));
        t->setVerticalHeaderItem(1,new QTableWidgetItem("sample bits"));

        int c=2;
        for(int i = 0; i< modes_bands.at(idx).size(); ++i){
            QString name = modes_bands.at(idx).at(i);
            t->insertRow(c);
            t->setItem(c,0,new QTableWidgetItem(QString::number(channels.at(idx).at(i))));
            t->setVerticalHeaderItem(c,new QTableWidgetItem(name+": channels"));
            ++c;

            t->insertRow(c);
            t->setItem(c,0,new QTableWidgetItem(QString::number(freqs.at(idx).at(i))));
            t->setVerticalHeaderItem(c,new QTableWidgetItem(name+": frequency [GHz]"));
            ++c;
        }
        break;
    }
    default:{
        break;
    }
    }
}

void settingsLoadWindow::on_buttonBox_accepted()
{
    auto list = ui->name->selectedItems();
    int i = list.size();
    if(i==1){
        this->accept();
    }else{
        QMessageBox::warning(this,"Nothing selected!","Please select a item from the left list!");
    }
}
