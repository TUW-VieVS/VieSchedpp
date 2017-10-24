#include "stationparametersdialog.h"
#include "ui_stationparametersdialog.h"

stationParametersDialog::stationParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::stationParametersDialog)
{
    ui->setupUi(this);
    sources = new QStringListModel(this);
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

void stationParametersDialog::addSourceNames(QStringList sources)
{
    this->sources->setStringList(sources);
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
        para.maxSlewtime = 600;
    }
    if(ui->spinBox_maxWaitTime->value() != 600){
        para.maxWait = 600;
    }
    if(ui->spinBox_minScanTime->value() != 20){
        para.minScan = 20;
    }
    if(ui->spinBox_maxScanTime->value() != 600){
        para.maxScan = 600;
    }
    if(ui->doubleSpinBox_weight->value() != 1){
        para.weight = 1;
    }
    for(int i = 0; i<ui->listWidget_selectedIgnoreSources->count(); ++i){
        para.ignoreSources_str.push_back(ui->listWidget_selectedIgnoreSources->item(i)->text().toStdString());
    }
    for(int i = 0; i<ui->tableWidget_SNR->rowCount(); ++i){
        QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox*> (ui->tableWidget_SNR->cellWidget(i,0));
        if(w->value()!=0){
            para.minSNR[ui->tableWidget_SNR->verticalHeaderItem(i)->text().toStdString()] = w->value();
        }
    }

    return std::make_pair(name,para);
}
