#include "stationparametersdialog.h"
#include "ui_stationparametersdialog.h"

stationParametersDialog::stationParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent) :
    QDialog(parent), settings{settings_},
    ui(new Ui::stationParametersDialog)
{
    ui->setupUi(this);
    sources = new QStandardItemModel(this);
    sources_proxy = new QSortFilterProxyModel(this);
    sources_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    sources_proxy->setSourceModel(sources);
    ui->listView_ignoreSources->setModel(sources_proxy);
}

stationParametersDialog::~stationParametersDialog()
{
    delete ui;
}

void stationParametersDialog::addBandNames(QStringList bands)
{
    ui->tableWidget_SNR->clear();
    ui->tableWidget_SNR->setRowCount(0);
    ui->tableWidget_SNR->setColumnCount(1);
    ui->tableWidget_SNR->setHorizontalHeaderItem(0,new QTableWidgetItem("min SNR"));

    for(int i = 0; i<bands.length(); ++i){
        int nextrow = ui->tableWidget_SNR->rowCount();
        ui->tableWidget_SNR->insertRow(nextrow);
        QString text = bands.at(i);
        ui->tableWidget_SNR->setVerticalHeaderItem(nextrow,new QTableWidgetItem(text));
        QDoubleSpinBox *spin = new QDoubleSpinBox(this);
        spin->setSuffix(" [Jy]");
        ui->tableWidget_SNR->setCellWidget(nextrow,0,spin);
    }
}

void stationParametersDialog::addDefaultParameters(VieVS::ParameterSettings::ParametersStations d)
{
    dp = d;
    ui->doubleSpinBox_weight->setValue(*d.weight);
    ui->doubleSpinBox_minElevation->setValue(*d.minElevation);
    ui->spinBox_minScanTime->setValue(*d.minScan);
    ui->spinBox_maxScanTime->setValue(*d.maxScan);
    ui->spinBox_maxWaitTime->setValue(*d.maxWait);
    ui->spinBox_maxSlewTime->setValue(*d.maxSlewtime);
    ui->doubleSpinBox_minSlewDistance->setValue(*d.minSlewDistance);
    ui->doubleSpinBox_maxSlewDistance->setValue(*d.maxSlewDistance);
    ui->spinBox_maxNumberOfScans->setValue(*d.maxNumberOfScans);
}

void stationParametersDialog::addSelectedParameters(VieVS::ParameterSettings::ParametersStations para, QString paraName)
{
    changeParameters(para);
    ui->lineEdit->setText(paraName);
    if(paraName == "default"){
        ui->groupBox_available->setCheckable(false);
        ui->groupBox_availableForFillinmode->setCheckable(false);

        ui->groupBox_tagalong->setCheckable(false);
        ui->groupBox_tagalong->setEnabled(false);

        ui->checkBox_weight->setChecked(true);
        ui->checkBox_weight->setEnabled(false);
        ui->checkBox_minElevation->setChecked(true);
        ui->checkBox_minElevation->setEnabled(false);
        ui->checkBox_maxWaitTime->setChecked(true);
        ui->checkBox_maxWaitTime->setEnabled(false);
        ui->checkBox_maxSlewTime->setChecked(true);
        ui->checkBox_maxSlewTime->setEnabled(false);
        ui->checkBox_maxSlewDistance->setChecked(true);
        ui->checkBox_maxSlewDistance->setEnabled(false);
        ui->checkBox_minSlewDistance->setChecked(true);
        ui->checkBox_minSlewDistance->setEnabled(false);
        ui->checkBox_maxNumberOfScans->setChecked(true);
        ui->checkBox_maxNumberOfScans->setEnabled(false);

        ui->groupBox_scanTime->setCheckable(false);

    }
    ui->lineEdit->setEnabled(false);
}

void stationParametersDialog::changeParameters(VieVS::ParameterSettings::ParametersStations sp)
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
            ui->radioButton_availableFillin_yes->setChecked(true);
        }else{
            ui->radioButton_availableFillin_no->setChecked(true);
        }
        ui->groupBox_availableForFillinmode->setChecked(true);
    }else{
        ui->groupBox_availableForFillinmode->setChecked(false);
    }

    if(sp.tagalong.is_initialized()){
        if(*sp.tagalong){
            ui->radioButton_tagalong_yes->setChecked(true);
        }else{
            ui->radioButton_tagalong_no->setChecked(true);
        }
        ui->groupBox_tagalong->setChecked(true);
    }else{
        ui->groupBox_tagalong->setChecked(false);
    }

    if(sp.maxSlewtime.is_initialized()){
        ui->spinBox_maxSlewTime->setValue(*sp.maxSlewtime);
        ui->checkBox_maxSlewTime->setChecked(true);
    }else{
        ui->spinBox_maxSlewTime->setValue(*dp.maxSlewtime);
        ui->checkBox_maxSlewTime->setChecked(false);
    }
    if(sp.maxSlewDistance.is_initialized()){
        ui->doubleSpinBox_maxSlewDistance->setValue(*sp.maxSlewDistance);
        ui->checkBox_maxSlewDistance->setChecked(true);
    }else{
        ui->doubleSpinBox_maxSlewDistance->setValue(*dp.maxSlewDistance);
        ui->checkBox_maxSlewDistance->setChecked(false);
    }
    if(sp.minSlewDistance.is_initialized()){
        ui->doubleSpinBox_minSlewDistance->setValue(*sp.minSlewDistance);
        ui->checkBox_minSlewDistance->setChecked(true);
    }else{
        ui->doubleSpinBox_minSlewDistance->setValue(*dp.minSlewDistance);
        ui->checkBox_minSlewDistance->setChecked(false);
    }

    if(sp.maxWait.is_initialized()){
        ui->spinBox_maxWaitTime->setValue(*sp.maxWait);
        ui->checkBox_maxWaitTime->setChecked(true);
    }else{
        ui->spinBox_maxWaitTime->setValue(*dp.maxWait);
        ui->checkBox_maxWaitTime->setChecked(false);
    }

    if(sp.maxNumberOfScans.is_initialized()){
        ui->spinBox_maxNumberOfScans->setValue(*sp.maxNumberOfScans);
        ui->checkBox_maxNumberOfScans->setChecked(true);
    }else{
        ui->spinBox_maxNumberOfScans->setValue(*dp.maxNumberOfScans);
        ui->checkBox_maxNumberOfScans->setChecked(false);
    }

    if(sp.minElevation.is_initialized()){
        ui->doubleSpinBox_minElevation->setValue(*sp.minElevation);
        ui->checkBox_minElevation->setChecked(true);
    }else{
        ui->doubleSpinBox_minElevation->setValue(*dp.minElevation);
        ui->checkBox_minElevation->setChecked(false);
    }

    if(sp.weight.is_initialized()){
        ui->doubleSpinBox_weight->setValue(*sp.weight);
        ui->checkBox_weight->setChecked(true);
    }else{
        ui->doubleSpinBox_weight->setValue(*dp.weight);
        ui->checkBox_weight->setChecked(false);
    }

    ui->groupBox_scanTime->setChecked(false);
    if(sp.maxScan.is_initialized()){
        ui->spinBox_maxScanTime->setValue(*sp.maxScan);
        ui->groupBox_scanTime->setChecked(true);
    }else{
        ui->spinBox_maxScanTime->setValue(*dp.maxScan);
    }

    if(sp.minScan.is_initialized()){
        ui->spinBox_minScanTime->setValue(*sp.minScan);
        ui->groupBox_scanTime->setChecked(true);
    }else{
        ui->spinBox_minScanTime->setValue(*dp.minScan);
    }

    QVector<QString> bands;
    for(int i=0; i<ui->tableWidget_SNR->rowCount(); ++i){
        qobject_cast<QDoubleSpinBox*>(ui->tableWidget_SNR->cellWidget(i,0))->setValue(0);
        bands.push_back(ui->tableWidget_SNR->verticalHeaderItem(i)->text());
    }
    for(const auto &any:sp.minSNR){
        QString name = QString::fromStdString(any.first);
        double val = any.second;
        int idx = bands.indexOf(name);
        if(idx != -1){
            qobject_cast<QDoubleSpinBox*>(ui->tableWidget_SNR->cellWidget(idx,0))->setValue(val);
            ui->groupBox_scanTime->setChecked(true);
        }else{
            warningTxt.append("    unknown band name: ").append(name).append(" for minimum SNR!\n");
        }
    }

    ui->listWidget_selectedIgnoreSources->clear();
    ui->groupBox_ignoreSources->setChecked(false);
    for(const auto &any:sp.ignoreSourcesString){
        QString name = QString::fromStdString(any);
        auto list = sources->findItems(name);
        if(list.size()==1){
            ui->listWidget_selectedIgnoreSources->insertItem(ui->listWidget_selectedIgnoreSources->count(),name);
            ui->groupBox_ignoreSources->setChecked(true);
        }else{
            warningTxt.append("    unknown source: ").append(name).append(" which should be ignored!\n");
        }
    }
    ui->listWidget_selectedIgnoreSources->sortItems();

    if(!warningTxt.isEmpty()){
        QString txt = "The following errors occurred while loading the parameters:\n";
        txt.append(warningTxt).append("These parameters were ignored!\nPlease double check parameters again!");
        QMessageBox::warning(this,"Unknown parameters!",txt);
    }

}

void stationParametersDialog::addSourceNames(QStandardItemModel *otherSources)
{
    for(int i=0; i<otherSources->rowCount();++i){
        sources->appendRow(otherSources->item(i)->clone());
    }
}



void stationParametersDialog::on_listView_ignoreSources_clicked(const QModelIndex &index)
{
    QString itm = index.data().toString();

    bool found = false;
    for(int i = 0; i<ui->listWidget_selectedIgnoreSources->count(); ++i){
        if(itm == ui->listWidget_selectedIgnoreSources->item(i)->text()){
            found = true;
            break;
        }
    }

    if(!found){
        ui->listWidget_selectedIgnoreSources->insertItem(0,itm);
        ui->listWidget_selectedIgnoreSources->sortItems();
    }
}

void stationParametersDialog::on_lineEdit_filter_textChanged(const QString &arg1)
{
    sources_proxy->setFilterRegExp(arg1);
}

void stationParametersDialog::on_listWidget_selectedIgnoreSources_clicked(const QModelIndex &index)
{
    QListWidgetItem *itm = ui->listWidget_selectedIgnoreSources->takeItem(index.row());
    delete(itm);
}

void stationParametersDialog::on_buttonBox_accepted()
{
    if(ui->lineEdit->text().isEmpty()){
        QMessageBox mb;
        mb.warning(this,"missing parameter name","Please add a parameter name");
    }else{
        this->accept();
    }
}

std::pair<std::string, VieVS::ParameterSettings::ParametersStations> stationParametersDialog::getParameters()
{
    VieVS::ParameterSettings::ParametersStations para;

    QString txt = ui->lineEdit->text();
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
        if(ui->radioButton_availableFillin_yes->isChecked()){
            para.availableForFillinmode = true;
        }else{
            para.availableForFillinmode = false;
        }
    }
    if(ui->groupBox_tagalong->isChecked()){
        if(ui->radioButton_tagalong_yes->isChecked()){
            para.tagalong = true;
        }else{
            para.tagalong = false;
        }

    }

    if(ui->spinBox_maxSlewTime->isEnabled()){
        para.maxSlewtime = ui->spinBox_maxSlewTime->value();
    }
    if(ui->doubleSpinBox_maxSlewDistance->isEnabled()){
        para.maxSlewDistance = ui->doubleSpinBox_maxSlewDistance->value();
    }
    if(ui->doubleSpinBox_minSlewDistance->isEnabled()){
        para.minSlewDistance = ui->doubleSpinBox_minSlewDistance->value();
    }
    if(ui->spinBox_maxWaitTime->isEnabled() ){
        para.maxWait = ui->spinBox_maxWaitTime->value();
    }
    if(ui->spinBox_maxNumberOfScans->isEnabled() ){
        para.maxNumberOfScans = ui->spinBox_maxNumberOfScans->value();
    }
    if(ui->doubleSpinBox_minElevation->isEnabled()){
        para.minElevation = ui->doubleSpinBox_minElevation->value();
    }
    if(ui->doubleSpinBox_weight->isEnabled()){
        para.weight = ui->doubleSpinBox_weight->value();
    }

    if(ui->groupBox_scanTime->isChecked()){
        para.minScan = ui->spinBox_minScanTime->value();
        para.maxScan = ui->spinBox_maxScanTime->value();
        for(int i = 0; i<ui->tableWidget_SNR->rowCount(); ++i){
            QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox*> (ui->tableWidget_SNR->cellWidget(i,0));
            if(w->value()!=0){
                para.minSNR[ui->tableWidget_SNR->verticalHeaderItem(i)->text().toStdString()] = w->value();
            }
        }
    }

    if(ui->groupBox_ignoreSources->isChecked()){
        for(int i = 0; i<ui->listWidget_selectedIgnoreSources->count(); ++i){
            para.ignoreSourcesString.push_back(ui->listWidget_selectedIgnoreSources->item(i)->text().toStdString());
        }
    }

    return std::make_pair(name,para);
}

void stationParametersDialog::on_pushButton_save_clicked()
{
    if(ui->lineEdit->text().isEmpty()){
        QMessageBox::warning(this,"No parameter Name!","Please add parameter name first!");
    }else{
        if(QMessageBox::Yes == QMessageBox::question(this,"Save?","Are you sure you want to save this settings?\nThis will save the settings to settings.xml file for further use.")){
            std::pair<std::string, VieVS::ParameterSettings::ParametersStations> para = getParameters();

            settings.add_child("settings.station.parameters.parameter",VieVS::ParameterSettings::parameterStation2ptree(para.first,para.second).get_child("parameters"));
            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
        }
    }
}

void stationParametersDialog::on_pushButton_load_clicked()
{
    boost::property_tree::ptree parameters = settings.get_child("settings.station.parameters");
    QVector<QString> names;
    QVector<VieVS::ParameterSettings::ParametersStations> paras;

    for(const auto &it:parameters){
        auto tmp = VieVS::ParameterSettings::ptree2parameterStation(it.second);
        std::string name = tmp.first;
        names.push_back(QString::fromStdString(name));
        VieVS::ParameterSettings::ParametersStations para = tmp.second;
        paras.push_back(para);
    }
    settingsLoadWindow *dial = new settingsLoadWindow(this);
    dial->setStationParameters(names,paras);
    int result = dial->exec();
    if(result == QDialog::Accepted){

        QString itm = dial->selectedItem();
        int idx = dial->selectedIdx();
        VieVS::ParameterSettings::ParametersStations sp = paras.at(idx);

        changeParameters(sp);

        ui->lineEdit->setText(itm);
    }
}
