#include "sourceparametersdialog.h"
#include "ui_sourceparametersdialog.h"

sourceParametersDialog::sourceParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent) :
    QDialog(parent), settings{settings_},
    ui(new Ui::sourceParametersDialog)
{
    ui->setupUi(this);

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
    ui->spinBox_minNumberOfStations->setValue(*d.minNumberOfStations);
}

void sourceParametersDialog::addSelectedParameters(VieVS::ParameterSettings::ParametersSources para, QString paraName)
{
    changeParameters(para);
    ui->lineEdit_paraName->setText(paraName);
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
    }else{
        ui->radioButton_available_parent->setChecked(true);
    }

    if(sp.minNumberOfStations.is_initialized()){
        ui->spinBox_minNumberOfStations->setValue(*sp.minNumberOfStations);
    }else{
        ui->spinBox_minNumberOfStations->setValue(2);
    }

    if(sp.minFlux.is_initialized()){
        ui->doubleSpinBox_minFlux->setValue(*sp.minFlux);
    }else{
        ui->doubleSpinBox_minFlux->setValue(*dp.minFlux);
    }

    if(sp.minRepeat.is_initialized()){
        ui->spinBox_minRepeatTime->setValue(*sp.minRepeat);
        ui->groupBox_10->setChecked(true);
    }else{
        ui->spinBox_minRepeatTime->setValue(*dp.minRepeat);
    }

    if(sp.maxScan.is_initialized()){
        ui->spinBox_maxScanTime->setValue(*sp.maxScan);
    }else{
        ui->spinBox_maxScanTime->setValue(*dp.maxScan);
    }

    if(sp.minScan.is_initialized()){
        ui->spinBox_minScanTime->setValue(*sp.minScan);
    }else{
        ui->spinBox_minScanTime->setValue(*dp.minScan);
    }

    if(sp.maxNumberOfScans.is_initialized()){
        ui->spinBox_maxNumberOfScans->setValue(*sp.maxNumberOfScans);
    }else{
        ui->spinBox_maxNumberOfScans->setValue(999);
    }

    if(sp.fixedScanDuration.is_initialized()){
        ui->groupBox_fixedScanDuration->setChecked(true);
        ui->spinBox_fixedScanDuration->setValue(*sp.fixedScanDuration);
    }else{
        ui->groupBox_variableScanDuration->setChecked(true);
        ui->spinBox_fixedScanDuration->setValue(300);
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

    if(sp.tryToObserveXTimesEvenlyDistributed.is_initialized()){
        ui->spinBox_evenlyDistScans->setValue(*sp.tryToObserveXTimesEvenlyDistributed);
        ui->spinBox_variableRepeatMinTime->setValue(*sp.tryToObserveXTimesMinRepeat);
        ui->groupBox_9->setChecked(true);
    }else{
        ui->spinBox_evenlyDistScans->setValue(0);
    }

    if(sp.weight.is_initialized()){
        ui->weightDoubleSpinBox->setValue(*sp.weight);
    }else{
        ui->weightDoubleSpinBox->setValue(*dp.weight);
    }

    if(sp.minElevation.is_initialized()){
        ui->doubleSpinBox_minElevation->setValue(*sp.minElevation);
    }else{
        ui->doubleSpinBox_minElevation->setValue(*dp.minElevation);
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
        }else{
            warningTxt.append("    unknown band name: ").append(name).append(" for minimum SNR!\n");
        }
    }

    ui->listWidget_selectedIgnoreStations->clear();
    for(const auto &any:sp.ignoreStationsString){
        QString name = QString::fromStdString(any);
        auto list = stations->findItems(name);
        if(list.size()==1){
            ui->listWidget_selectedIgnoreStations->insertItem(ui->listWidget_selectedIgnoreStations->count(),name);
        }else{
            warningTxt.append("    unknown station: ").append(name).append(" which should be ignored!\n");
        }
    }
    ui->listWidget_selectedIgnoreStations->sortItems();

    ui->listWidget_selectedRequiredStations->clear();
    for(const auto &any:sp.requiredStationsString){
        QString name = QString::fromStdString(any);
        auto list = stations->findItems(name);
        if(list.size()==1){
            ui->listWidget_selectedRequiredStations->insertItem(ui->listWidget_selectedRequiredStations->count(),name);
        }else{
            warningTxt.append("    unknown station: ").append(name).append(" which should be required!\n");
        }
    }
    ui->listWidget_selectedRequiredStations->sortItems();

    ui->listWidget_selectedIgnoreBaselines->clear();
    for(const auto &any:sp.ignoreBaselinesString){
        QString name = QString::fromStdString(any);
        auto list = baseline->findItems(name);
        if(list.size()==1){
            ui->listWidget_selectedIgnoreBaselines->insertItem(ui->listWidget_selectedIgnoreBaselines->count(),name);
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

    if(!ui->radioButton_available_parent->isChecked()){
        if(ui->radioButton_available_yes->isChecked()){
            para.available = true;
        }else{
            para.available = false;
        }
    }

    if(ui->groupBox_variableScanDuration->isChecked()){
        if(ui->spinBox_minScanTime->value() != *dp.minScan){
            para.minScan = ui->spinBox_minScanTime->value();
        }
        if(ui->spinBox_maxScanTime->value() != *dp.maxScan){
            para.maxScan = ui->spinBox_maxScanTime->value();
        }
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

    if(ui->spinBox_minNumberOfStations->value() != *dp.minNumberOfStations){
        para.minNumberOfStations = ui->spinBox_minNumberOfStations->value();
    }
    if(ui->doubleSpinBox_minFlux->value() != *dp.minFlux){
        para.minFlux = ui->doubleSpinBox_minFlux->value();
    }
    if(ui->spinBox_maxNumberOfScans->value() != *dp.maxNumberOfScans){
        para.maxNumberOfScans = ui->spinBox_maxNumberOfScans->value();
    }

    if(ui->groupBox_9->isChecked()){
        para.tryToObserveXTimesEvenlyDistributed = ui->spinBox_evenlyDistScans->value();
        para.tryToObserveXTimesMinRepeat = ui->spinBox_variableRepeatMinTime->value();
    }
    if(ui->groupBox_10->isChecked()){
        if(ui->spinBox_minRepeatTime->value() != *dp.minRepeat){
            para.minRepeat = ui->spinBox_minRepeatTime->value();
        }
    }


    if(ui->weightDoubleSpinBox->value() != *dp.weight){
        para.weight = ui->weightDoubleSpinBox->value();
    }
    if(ui->doubleSpinBox_minElevation->value() != *dp.minElevation){
        para.minElevation = ui->doubleSpinBox_minElevation->value();
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
    for(int i = 0; i<ui->listWidget_selectedIgnoreStations->count(); ++i){
        para.ignoreStationsString.push_back(ui->listWidget_selectedIgnoreStations->item(i)->text().toStdString());
    }
    for(int i = 0; i<ui->listWidget_selectedRequiredStations->count(); ++i){
        para.requiredStationsString.push_back(ui->listWidget_selectedRequiredStations->item(i)->text().toStdString());
    }
    for(int i = 0; i<ui->listWidget_selectedIgnoreBaselines->count(); ++i){
        para.ignoreBaselinesString.push_back(ui->listWidget_selectedIgnoreBaselines->item(i)->text().toStdString());
    }

    if(name == "default"){
        para.available = true;
        para.minRepeat = ui->spinBox_minRepeatTime->value();
        para.minScan = ui->spinBox_minScanTime->value();
        para.maxScan = ui->spinBox_maxScanTime->value();
        para.weight = ui->weightDoubleSpinBox->value();
        para.maxNumberOfScans = ui->spinBox_maxNumberOfScans->value();
        para.minFlux = ui->doubleSpinBox_minFlux->value();
        para.minElevation = ui->doubleSpinBox_minElevation->value();
        para.minNumberOfStations = ui->spinBox_minNumberOfStations->value();
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
        ui->groupBox_increaseWeightIfObservedOnce->setChecked(true);
    }else{
        ui->groupBox_increaseWeightIfObservedOnce->setChecked(false);
    }
}

void sourceParametersDialog::on_groupBox_10_toggled(bool arg1)
{
    ui->groupBox_9->setChecked(!arg1);
}

void sourceParametersDialog::on_groupBox_9_toggled(bool arg1)
{
    ui->groupBox_10->setChecked(!arg1);
}

void sourceParametersDialog::on_groupBox_variableScanDuration_toggled(bool arg1)
{
    ui->groupBox_fixedScanDuration->setChecked(!arg1);
}

void sourceParametersDialog::on_groupBox_fixedScanDuration_toggled(bool arg1)
{
    ui->groupBox_variableScanDuration->setChecked(!arg1);
}
