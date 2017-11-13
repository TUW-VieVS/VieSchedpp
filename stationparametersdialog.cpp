#include "stationparametersdialog.h"
#include "ui_stationparametersdialog.h"

stationParametersDialog::stationParametersDialog(boost::property_tree::ptree &settings_,QWidget *parent) :
    QDialog(parent), settings(settings_),
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

    if(!ui->availableCheckBox->isChecked()){
        para.available = false;
    }
    if(ui->tagalongCheckBox->isChecked()){
        para.tagalong = true;
    }
    if(ui->spinBox_maxSlewTime->value() != 600){
        para.maxSlewtime = ui->spinBox_maxSlewTime->value();
    }
    if(ui->spinBox_maxWaitTime->value() != 600){
        para.maxWait = ui->spinBox_maxWaitTime->value();
    }
    if(ui->spinBox_minScanTime->value() != 20){
        para.minScan = ui->spinBox_minScanTime->value();
    }
    if(ui->spinBox_maxScanTime->value() != 600){
        para.maxScan = ui->spinBox_maxScanTime->value();
    }
    if(ui->doubleSpinBox_weight->value() != 1){
        para.weight = ui->doubleSpinBox_weight->value();
    }
    for(int i = 0; i<ui->listWidget_selectedIgnoreSources->count(); ++i){
        para.ignoreSourcesString.push_back(ui->listWidget_selectedIgnoreSources->item(i)->text().toStdString());
    }
    for(int i = 0; i<ui->tableWidget_SNR->rowCount(); ++i){
        QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox*> (ui->tableWidget_SNR->cellWidget(i,0));
        if(w->value()!=0){
            para.minSNR[ui->tableWidget_SNR->verticalHeaderItem(i)->text().toStdString()] = w->value();
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

}
