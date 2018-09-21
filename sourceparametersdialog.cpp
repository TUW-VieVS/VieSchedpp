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

#include "sourceparametersdialog.h"
#include "ui_sourceparametersdialog.h"

sourceParametersDialog::sourceParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent) :
    QDialog(parent), settings{settings_},
    ui(new Ui::sourceParametersDialog)
{
    ui->setupUi(this);
    QApplication::setWindowIcon(QIcon(":/icons/icons/VieSchedppGUI_logo.png"));
    this->setWindowTitle("VieSched++");

    stations = new QStandardItemModel(this);
    stations_proxyIgnore = new QSortFilterProxyModel(this);
    stations_proxyRequired = new QSortFilterProxyModel(this);
    stations_proxyIgnore->setFilterCaseSensitivity(Qt::CaseInsensitive);
    stations_proxyRequired->setFilterCaseSensitivity(Qt::CaseInsensitive);
    stations_proxyIgnore->setSourceModel(stations);
    stations_proxyRequired->setSourceModel(stations);

    baseline = new QStandardItemModel(this);
    baseline_proxy = new QSortFilterProxyModel(this);
    baseline_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    baseline_proxy->setSourceModel(baseline);

    ui->listView_ignoreStations->setModel(stations_proxyIgnore);
    ui->listView_requiredStations->setModel(stations_proxyRequired);
    ui->listView_ignoreBaselines->setModel(baseline_proxy);
}

sourceParametersDialog::~sourceParametersDialog()
{
    delete ui;
}

void sourceParametersDialog::addBandNames(QStringList bands)
{
    ui->tableWidget_minSNR->clear();
    ui->tableWidget_minSNR->setRowCount(0);
    ui->tableWidget_minSNR->setColumnCount(1);
    ui->tableWidget_minSNR->setHorizontalHeaderItem(0,new QTableWidgetItem("min SNR"));

    for(int i = 0; i<bands.length(); ++i){
        int nextrow = ui->tableWidget_minSNR->rowCount();
        ui->tableWidget_minSNR->insertRow(nextrow);
        QString text = bands.at(i);
        ui->tableWidget_minSNR->setVerticalHeaderItem(nextrow,new QTableWidgetItem(text));
        QDoubleSpinBox *spin = new QDoubleSpinBox(this);
        spin->setSuffix(" [Jy]");
        ui->tableWidget_minSNR->setCellWidget(nextrow,0,spin);
    }
}

void sourceParametersDialog::addStationModel(QStandardItemModel *otherStations)
{
    for(int i=0; i<otherStations->rowCount();++i){
        stations->appendRow(otherStations->item(i)->clone());
    }
}

void sourceParametersDialog::addBaselineModel(QStandardItemModel *otherBaselines)
{
    for(int i=0; i<otherBaselines->rowCount();++i){
        baseline->appendRow(otherBaselines->item(i)->clone());
    }
}

void sourceParametersDialog::addDefaultParameters(VieVS::ParameterSettings::ParametersSources d)
{
    dp = d;
    ui->spinBox_minRepeatTime->setValue(*d.minRepeat);
    ui->spinBox_minScanTime->setValue(*d.minScan);
    ui->spinBox_maxScanTime->setValue(*d.maxScan);
    ui->weightDoubleSpinBox->setValue(*d.weight);
    ui->spinBox_maxNumberOfScans->setValue(*d.maxNumberOfScans);
    ui->doubleSpinBox_minFlux->setValue(*d.minFlux);
    ui->doubleSpinBox_minElevation->setValue(*d.minElevation);
    ui->doubleSpinBox_minSunDistance->setValue(*d.minSunDistance);
    ui->spinBox_minNumberOfStations->setValue(*d.minNumberOfStations);
}

void sourceParametersDialog::addSelectedParameters(VieVS::ParameterSettings::ParametersSources para, QString paraName)
{
    changeParameters(para);
    ui->lineEdit_paraName->setText(paraName);
    if(paraName == "default"){
        ui->groupBox_available->setCheckable(false);
        ui->groupBox_availableForFillinmode->setCheckable(false);

        ui->checkBox_maxNumberOfScans->setChecked(true);
        ui->checkBox_maxNumberOfScans->setEnabled(false);
        ui->checkBox_minElevation->setChecked(true);
        ui->checkBox_minElevation->setEnabled(false);
        ui->checkBox_minSunDistance->setChecked(true);
        ui->checkBox_minSunDistance->setEnabled(false);
        ui->checkBox_minFlux->setChecked(true);
        ui->checkBox_minFlux->setEnabled(false);
        ui->checkBox_minNumberOfStations->setChecked(true);
        ui->checkBox_minNumberOfStations->setEnabled(false);
        ui->checkBox_weight->setChecked(true);
        ui->checkBox_weight->setEnabled(false);

        ui->groupBox_variableScanDuration->setCheckable(false);
        ui->groupBox_fixedScanDuration->setCheckable(false);
        ui->groupBox_fixedScanDuration->setEnabled(false);
        ui->groupBox_9->setCheckable(false);
        ui->groupBox_9->setEnabled(false);
        ui->groupBox_10->setCheckable(false);
        ui->groupBox_increaseWeightIfObservedOnce->setCheckable(false);
        ui->groupBox_increaseWeightIfObservedOnce->setEnabled(false);

        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(false);

    }
    ui->lineEdit_paraName->setEnabled(false);
}

void sourceParametersDialog::changeParameters(VieVS::ParameterSettings::ParametersSources sp)
{
    QString warningTxt;
    if(sp.available.is_initialized()){
        if(*sp.available){
            ui->radioButton_available_yes->setChecked(true);
        }else{
            ui->radioButton_available_no->setChecked(true);
        }
        ui->groupBox_available->setChecked(true);
    }else{
        ui->groupBox_available->setChecked(false);
    }

    if(sp.availableForFillinmode.is_initialized()){
        if(*sp.availableForFillinmode){
            ui->radioButton_availableForFillin_yes->setChecked(true);
        }else{
            ui->radioButton_availableForFillin_no->setChecked(true);
        }
        ui->groupBox_availableForFillinmode->setChecked(true);
    }else{
        ui->groupBox_availableForFillinmode->setChecked(false);
    }

    if(sp.minNumberOfStations.is_initialized()){
        ui->spinBox_minNumberOfStations->setValue(*sp.minNumberOfStations);
        ui->checkBox_minNumberOfStations->setChecked(true);
    }else{
        ui->spinBox_minNumberOfStations->setValue(*dp.minNumberOfStations);
        ui->checkBox_minNumberOfStations->setChecked(false);
    }

    if(sp.minFlux.is_initialized()){
        ui->doubleSpinBox_minFlux->setValue(*sp.minFlux);
        ui->checkBox_minFlux->setChecked(true);
    }else{
        ui->doubleSpinBox_minFlux->setValue(*dp.minFlux);
        ui->checkBox_minFlux->setChecked(false);
    }

    if(sp.maxNumberOfScans.is_initialized()){
        ui->spinBox_maxNumberOfScans->setValue(*sp.maxNumberOfScans);
        ui->checkBox_maxNumberOfScans->setChecked(true);
    }else{
        ui->spinBox_maxNumberOfScans->setValue(999);
        ui->checkBox_maxNumberOfScans->setChecked(false);
    }

    if(sp.weight.is_initialized()){
        ui->weightDoubleSpinBox->setValue(*sp.weight);
        ui->checkBox_weight->setChecked(true);
    }else{
        ui->weightDoubleSpinBox->setValue(*dp.weight);
        ui->checkBox_weight->setChecked(false);
    }

    if(sp.minElevation.is_initialized()){
        ui->doubleSpinBox_minElevation->setValue(*sp.minElevation);
        ui->checkBox_minElevation->setChecked(true);
    }else{
        ui->doubleSpinBox_minElevation->setValue(*dp.minElevation);
        ui->checkBox_minElevation->setChecked(false);
    }

    if(sp.minSunDistance.is_initialized()){
        ui->doubleSpinBox_minSunDistance->setValue(*sp.minSunDistance);
        ui->checkBox_minSunDistance->setChecked(true);
    }else{
        ui->doubleSpinBox_minSunDistance->setValue(*dp.minSunDistance);
        ui->checkBox_minSunDistance->setChecked(false);
    }


    if(sp.minRepeat.is_initialized()){
        ui->spinBox_minRepeatTime->setValue(*sp.minRepeat);
        ui->groupBox_10->setChecked(true);
    }else{
        ui->spinBox_minRepeatTime->setValue(*dp.minRepeat);
        ui->groupBox_10->setChecked(false);
    }

    if(sp.tryToObserveXTimesEvenlyDistributed.is_initialized()){
        ui->spinBox_evenlyDistScans->setValue(*sp.tryToObserveXTimesEvenlyDistributed);
        ui->spinBox_variableRepeatMinTime->setValue(*sp.tryToObserveXTimesMinRepeat);
        ui->groupBox_9->setChecked(true);
    }else{
        ui->spinBox_evenlyDistScans->setValue(0);
        ui->groupBox_9->setChecked(false);
    }

    if(sp.tryToFocusIfObservedOnce.is_initialized()){
        ui->groupBox_increaseWeightIfObservedOnce->setChecked(true);
        ui->doubleSpinBox_factor->setValue(*sp.tryToFocusFactor);
        if(*sp.tryToFocusOccurrency == VieVS::ParameterSettings::TryToFocusOccurrency::once){
            ui->radioButton_increaseWeight_once->setChecked(true);
        }else{
            ui->radioButton_increaseWeight_perScan->setChecked(true);
        }
        if(*sp.tryToFocusType == VieVS::ParameterSettings::TryToFocusType::additive){
            ui->radioButton_addWeightAdditive->setChecked(true);
        }else{
            ui->radioButton_addWeightMultiplicative->setChecked(true);
        }
    }else{
        ui->groupBox_increaseWeightIfObservedOnce->setChecked(false);
    }



    ui->groupBox_variableScanDuration->setChecked(false);
    if(sp.maxScan.is_initialized()){
        ui->spinBox_maxScanTime->setValue(*sp.maxScan);
        ui->groupBox_variableScanDuration->setChecked(true);
    }else{
        ui->spinBox_maxScanTime->setValue(*dp.maxScan);
    }
    if(sp.minScan.is_initialized()){
        ui->spinBox_minScanTime->setValue(*sp.minScan);
        ui->groupBox_variableScanDuration->setChecked(true);
    }else{
        ui->spinBox_minScanTime->setValue(*dp.minScan);
    }
    QVector<QString> bands;
    for(int i=0; i<ui->tableWidget_minSNR->rowCount(); ++i){
        qobject_cast<QDoubleSpinBox*>(ui->tableWidget_minSNR->cellWidget(i,0))->setValue(0);
        bands.push_back(ui->tableWidget_minSNR->verticalHeaderItem(i)->text());
    }
    for(const auto &any:sp.minSNR){
        QString name = QString::fromStdString(any.first);
        double val = any.second;
        int idx = bands.indexOf(name);
        if(idx != -1){
            qobject_cast<QDoubleSpinBox*>(ui->tableWidget_minSNR->cellWidget(idx,0))->setValue(val);
            ui->groupBox_variableScanDuration->setChecked(true);
        }else{
            warningTxt.append("    unknown band name: ").append(name).append(" for minimum SNR!\n");
        }
    }

    if(sp.fixedScanDuration.is_initialized()){
        ui->spinBox_fixedScanDuration->setValue(*sp.fixedScanDuration);
        ui->groupBox_fixedScanDuration->setChecked(true);
    }else{
        ui->spinBox_fixedScanDuration->setValue(300);
        ui->groupBox_fixedScanDuration->setChecked(false);
    }


    ui->listWidget_selectedIgnoreStations->clear();
    ui->groupBox_ignoreStations->setChecked(false);
    for(const auto &any:sp.ignoreStationsString){
        QString name = QString::fromStdString(any);
        auto list = stations->findItems(name);
        if(list.size()==1){
            ui->listWidget_selectedIgnoreStations->insertItem(ui->listWidget_selectedIgnoreStations->count(),name);
            ui->groupBox_ignoreStations->setChecked(true);
        }else{
            warningTxt.append("    unknown station: ").append(name).append(" which should be ignored!\n");
        }
    }
    ui->listWidget_selectedIgnoreStations->sortItems();

    ui->listWidget_selectedRequiredStations->clear();
    ui->groupBox_requiredStations->setChecked(false);
    for(const auto &any:sp.requiredStationsString){
        QString name = QString::fromStdString(any);
        auto list = stations->findItems(name);
        if(list.size()==1){
            ui->listWidget_selectedRequiredStations->insertItem(ui->listWidget_selectedRequiredStations->count(),name);
            ui->groupBox_requiredStations->setChecked(true);
        }else{
            warningTxt.append("    unknown station: ").append(name).append(" which should be required!\n");
        }
    }
    ui->listWidget_selectedRequiredStations->sortItems();

    ui->listWidget_selectedIgnoreBaselines->clear();
    ui->groupBox_ignoreBaselines->setChecked(false);
    for(const auto &any:sp.ignoreBaselinesString){
        QString name = QString::fromStdString(any);
        auto list = baseline->findItems(name);
        if(list.size()==1){
            ui->listWidget_selectedIgnoreBaselines->insertItem(ui->listWidget_selectedIgnoreBaselines->count(),name);
            ui->groupBox_ignoreBaselines->setChecked(true);
        }else{
            warningTxt.append("    unknown baseline: ").append(name).append(" which should be ignored!\n");
        }
    }
    ui->listWidget_selectedIgnoreBaselines->sortItems();

    if(!warningTxt.isEmpty()){
        QString txt = "The following errors occurred while loading the parameters:\n";
        txt.append(warningTxt).append("These parameters were ignored!\nPlease double check parameters again!");
        QMessageBox::warning(this,"Unknown parameters!",txt);
    }
}

void sourceParametersDialog::on_lineEdit_ignoreStations_textChanged(const QString &arg1)
{
    stations_proxyIgnore->setFilterRegExp(arg1);
}

void sourceParametersDialog::on_lineEdit_filterRequiredStations_textChanged(const QString &arg1)
{
    stations_proxyRequired->setFilterRegExp(arg1);
}

void sourceParametersDialog::on_lineEdit_filter_ignoreBaselines_textChanged(const QString &arg1)
{
    baseline_proxy->setFilterRegExp(arg1);
}

void sourceParametersDialog::on_listWidget_selectedIgnoreStations_clicked(const QModelIndex &index)
{
    QListWidgetItem *itm = ui->listWidget_selectedIgnoreBaselines->takeItem(index.row());
    delete(itm);
}

void sourceParametersDialog::on_listWidget_selectedRequiredStations_clicked(const QModelIndex &index)
{
    QListWidgetItem *itm = ui->listWidget_selectedRequiredStations->takeItem(index.row());
    delete(itm);
}

void sourceParametersDialog::on_listWidget_selectedIgnoreBaselines_clicked(const QModelIndex &index)
{
    QListWidgetItem *itm = ui->listWidget_selectedIgnoreBaselines->takeItem(index.row());
    delete(itm);
}


void sourceParametersDialog::on_listView_ignoreStations_clicked(const QModelIndex &index)
{
    QString itm = index.data().toString();

    bool found = false;
    for(int i = 0; i<ui->listWidget_selectedIgnoreStations->count(); ++i){
        if(itm == ui->listWidget_selectedIgnoreStations->item(i)->text()){
            found = true;
            break;
        }
    }

    if(!found){
        ui->listWidget_selectedIgnoreStations->insertItem(0,itm);
        ui->listWidget_selectedIgnoreStations->sortItems();
    }

}

void sourceParametersDialog::on_listView_requiredStations_clicked(const QModelIndex &index)
{
    QString itm = index.data().toString();

    bool found = false;
    for(int i = 0; i<ui->listWidget_selectedRequiredStations->count(); ++i){
        if(itm == ui->listWidget_selectedRequiredStations->item(i)->text()){
            found = true;
            break;
        }
    }

    if(!found){
        ui->listWidget_selectedRequiredStations->insertItem(0,itm);
        ui->listWidget_selectedRequiredStations->sortItems();
    }
}

void sourceParametersDialog::on_listView_ignoreBaselines_clicked(const QModelIndex &index)
{
    QString itm = index.data().toString();

    bool found = false;
    for(int i = 0; i<ui->listWidget_selectedIgnoreBaselines->count(); ++i){
        if(itm == ui->listWidget_selectedIgnoreBaselines->item(i)->text()){
            found = true;
            break;
        }
    }

    if(!found){
        ui->listWidget_selectedIgnoreBaselines->insertItem(0,itm);
        ui->listWidget_selectedIgnoreBaselines->sortItems();
    }
}

void sourceParametersDialog::on_buttonBox_accepted()
{
    if(ui->lineEdit_paraName->text().isEmpty()){
        QMessageBox mb;
        mb.warning(this,"missing parameter name","Please add a parameter name");
    }else{
        this->accept();
    }
}

std::pair<std::string, VieVS::ParameterSettings::ParametersSources> sourceParametersDialog::getParameters()
{
    VieVS::ParameterSettings::ParametersSources para;

    QString txt = ui->lineEdit_paraName->text();
    txt = txt.trimmed();
    txt = txt.simplified();
    txt.replace(" ","_");

    std::string name = txt.toStdString();

    if(ui->groupBox_available->isChecked() || !ui->groupBox_available->isCheckable()){
        if(ui->radioButton_available_yes->isChecked()){
            para.available = true;
        }else{
            para.available = false;
        }
    }
    if(ui->groupBox_availableForFillinmode->isChecked() || !ui->groupBox_availableForFillinmode->isCheckable()){
        if(ui->radioButton_availableForFillin_yes->isChecked()){
            para.availableForFillinmode = true;
        }else{
            para.availableForFillinmode = false;
        }
    }

    if(ui->spinBox_minNumberOfStations->isEnabled()){
        para.minNumberOfStations = ui->spinBox_minNumberOfStations->value();
    }
    if(ui->doubleSpinBox_minFlux->isEnabled()){
        para.minFlux = ui->doubleSpinBox_minFlux->value();
    }
    if(ui->spinBox_maxNumberOfScans->isEnabled()){
        para.maxNumberOfScans = ui->spinBox_maxNumberOfScans->value();
    }
    if(ui->weightDoubleSpinBox->isEnabled()){
        para.weight = ui->weightDoubleSpinBox->value();
    }
    if(ui->doubleSpinBox_minElevation->isEnabled()){
        para.minElevation = ui->doubleSpinBox_minElevation->value();
    }
    if(ui->doubleSpinBox_minSunDistance->isEnabled()){
        para.minSunDistance = ui->doubleSpinBox_minSunDistance->value();
    }

    if(ui->groupBox_9->isChecked()){
        para.tryToObserveXTimesEvenlyDistributed = ui->spinBox_evenlyDistScans->value();
        para.tryToObserveXTimesMinRepeat = ui->spinBox_variableRepeatMinTime->value();
    }
    if(ui->groupBox_10->isChecked() || !ui->groupBox_10->isCheckable()){
        para.minRepeat = ui->spinBox_minRepeatTime->value();
    }


    if(ui->groupBox_increaseWeightIfObservedOnce->isChecked()){
        para.tryToFocusIfObservedOnce = true;
        para.tryToFocusFactor = ui->doubleSpinBox_factor->value();
        if(ui->radioButton_increaseWeight_once->isChecked()){
            para.tryToFocusOccurrency = VieVS::ParameterSettings::TryToFocusOccurrency::once;
        }else{
            para.tryToFocusOccurrency = VieVS::ParameterSettings::TryToFocusOccurrency::perScan;
        }
        if(ui->radioButton_addWeightAdditive->isChecked()){
            para.tryToFocusType = VieVS::ParameterSettings::TryToFocusType::additive;
        }else{
            para.tryToFocusType = VieVS::ParameterSettings::TryToFocusType::multiplicative;
        }
    }

    if(ui->groupBox_variableScanDuration->isChecked() || !ui->groupBox_variableScanDuration->isCheckable()){
        para.minScan = ui->spinBox_minScanTime->value();
        para.maxScan = ui->spinBox_maxScanTime->value();
        for(int i = 0; i<ui->tableWidget_minSNR->rowCount(); ++i){
            QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox*> (ui->tableWidget_minSNR->cellWidget(i,0));
            if(w->value()!=0){
                para.minSNR[ui->tableWidget_minSNR->verticalHeaderItem(i)->text().toStdString()] = w->value();
            }
        }
    }
    if(ui->groupBox_fixedScanDuration->isChecked()){
        para.fixedScanDuration = ui->spinBox_fixedScanDuration->value();
    }

    if(ui->groupBox_ignoreStations->isChecked()){
        for(int i = 0; i<ui->listWidget_selectedIgnoreStations->count(); ++i){
            para.ignoreStationsString.push_back(ui->listWidget_selectedIgnoreStations->item(i)->text().toStdString());
        }
    }

    if(ui->groupBox_requiredStations->isChecked()){
        for(int i = 0; i<ui->listWidget_selectedRequiredStations->count(); ++i){
            para.requiredStationsString.push_back(ui->listWidget_selectedRequiredStations->item(i)->text().toStdString());
        }
    }

    if(ui->groupBox_ignoreBaselines->isChecked()){
        for(int i = 0; i<ui->listWidget_selectedIgnoreBaselines->count(); ++i){
            para.ignoreBaselinesString.push_back(ui->listWidget_selectedIgnoreBaselines->item(i)->text().toStdString());
        }
    }

    return std::make_pair(name,para);
}


void sourceParametersDialog::on_buttonBox_rejected()
{
    this->reject();
}

void sourceParametersDialog::on_pushButton_2_clicked()
{
    boost::property_tree::ptree parameters = settings.get_child("settings.source.parameters");
    QVector<QString> names;
    QVector<VieVS::ParameterSettings::ParametersSources> paras;

    for(const auto &it:parameters){
        auto tmp = VieVS::ParameterSettings::ptree2parameterSource(it.second);
        std::string name = tmp.first;
        names.push_back(QString::fromStdString(name));
        VieVS::ParameterSettings::ParametersSources para = tmp.second;
        paras.push_back(para);
    }
    settingsLoadWindow *dial = new settingsLoadWindow(this);
    dial->setSourceParameters(names,paras);
    int result = dial->exec();
    if(result == QDialog::Accepted){

        QString itm = dial->selectedItem();
        int idx = dial->selectedIdx();
        VieVS::ParameterSettings::ParametersSources sp = paras.at(idx);

        changeParameters(sp);

        ui->lineEdit_paraName->setText(itm);

    }
}

void sourceParametersDialog::on_pushButton_clicked()
{
    if(ui->lineEdit_paraName->text().isEmpty()){
        QMessageBox::warning(this,"No parameter Name!","Please add parameter name first!");
    }else{
        if(QMessageBox::Yes == QMessageBox::question(this,"Save?","Are you sure you want to save this settings?\nThis will save the settings to settings.xml file for further use.")){
            std::pair<std::string, VieVS::ParameterSettings::ParametersSources> para = getParameters();

            settings.add_child("settings.source.parameters.parameter",VieVS::ParameterSettings::parameterSource2ptree(para.first,para.second).get_child("parameters"));
            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
        }
    }
}

void sourceParametersDialog::on_spinBox_evenlyDistScans_valueChanged(int arg1)
{
    if(arg1 == 0){
        ui->groupBox_increaseWeightIfObservedOnce->setChecked(false);
    }else{
        ui->groupBox_increaseWeightIfObservedOnce->setChecked(true);
    }
}

void sourceParametersDialog::on_groupBox_10_toggled(bool arg1)
{
    if(arg1){
        ui->groupBox_9->setChecked(!arg1);
    }
}

void sourceParametersDialog::on_groupBox_9_toggled(bool arg1)
{
    if(arg1){
        ui->groupBox_10->setChecked(!arg1);
    }
}

void sourceParametersDialog::on_groupBox_variableScanDuration_toggled(bool arg1)
{
    if(arg1){
        ui->groupBox_fixedScanDuration->setChecked(!arg1);
    }
}

void sourceParametersDialog::on_groupBox_fixedScanDuration_toggled(bool arg1)
{
    if(arg1){
        ui->groupBox_variableScanDuration->setChecked(!arg1);
    }
}
