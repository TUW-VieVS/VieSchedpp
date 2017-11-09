#include "sourceparametersdialog.h"
#include "ui_sourceparametersdialog.h"

sourceParametersDialog::sourceParametersDialog(QWidget *parent) :
    QDialog(parent),
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

    if(!ui->availableCheckBox->isChecked()){
        para.available = false;
    }
    if(ui->spinBox_minScanTime->value() != 20){
        para.minScan = ui->spinBox_minScanTime->value();
    }
    if(ui->spinBox_maxScanTime->value() != 600){
        para.maxScan = ui->spinBox_maxScanTime->value();
    }
    if(ui->spinBox_minNumberOfStations->value() != 1){
        para.minNumberOfStations = ui->spinBox_minNumberOfStations->value();
    }
    if(ui->doubleSpinBox_minFlux->value() != 0){
        para.minFlux = ui->doubleSpinBox_minFlux->value();
    }
    if(ui->spinBox_minRepeatTime->value() != 1800){
        para.minRepeat = ui->spinBox_minRepeatTime->value();
    }
    if(ui->spinBox_maxNumberOfScans->value() != 999){
        para.maxNumberOfScans = ui->spinBox_maxNumberOfScans->value();
    }
    if(ui->spinBox_evenlyDistScans->value() != 0){
        para.tryToObserveXTimesEvenlyDistributed = ui->spinBox_evenlyDistScans->value();
    }
    if(ui->weightDoubleSpinBox->value() != 1){
        para.weight = ui->weightDoubleSpinBox->value();
    }
    if(ui->checkBox_focusIfObsOnce->isChecked()){
        para.tryToFocusIfObservedOnce = true;
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
    for(int i = 0; i<ui->tableWidget_minSNR->rowCount(); ++i){
        QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox*> (ui->tableWidget_minSNR->cellWidget(i,0));
        if(w->value()!=0){
            para.minSNR[ui->tableWidget_minSNR->verticalHeaderItem(i)->text().toStdString()] = w->value();
        }
    }

    return std::make_pair(name,para);
}


void sourceParametersDialog::on_buttonBox_rejected()
{
    this->reject();
}
