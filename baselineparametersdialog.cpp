#include "baselineparametersdialog.h"
#include "ui_baselineparametersdialog.h"

void baselineParametersDialog::addBandNames(QStringList bands)
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

std::pair<std::string, VieVS::ParameterSettings::ParametersBaselines> baselineParametersDialog::getParameters()
{
    VieVS::ParameterSettings::ParametersBaselines para;

    QString txt = ui->lineEdit_paraName->text();
    txt = txt.trimmed();
    txt = txt.simplified();
    txt.replace(" ","_");

    std::string name = txt.toStdString();

    if(ui->CheckBox_ignore->isChecked()){
        para.ignore = true;
    }
    if(ui->spinBox_minScan->value() != 20){
        para.minScan = ui->spinBox_minScan->value();
    }
    if(ui->spinBox_maxScan->value() != 600){
        para.maxScan = ui->spinBox_maxScan->value();
    }
    if(ui->doubleSpinBox_weight->value() != 1){
        para.weight = ui->doubleSpinBox_weight->value();
    }
    for(int i = 0; i<ui->tableWidget_SNR->rowCount(); ++i){
        QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox*> (ui->tableWidget_SNR->cellWidget(i,0));
        if(w->value()!=0){
            para.minSNR[ui->tableWidget_SNR->verticalHeaderItem(i)->text().toStdString()] = w->value();
        }
    }

    return std::make_pair(name,para);
}

baselineParametersDialog::baselineParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::baselineParametersDialog)
{
    ui->setupUi(this);
}

baselineParametersDialog::~baselineParametersDialog()
{
    delete ui;
}

void baselineParametersDialog::on_buttonBox_accepted()
{
    if(ui->lineEdit_paraName->text().isEmpty()){
        QMessageBox mb;
        mb.warning(this,"missing parameter name","Please add a parameter name");
    }else{
        this->accept();
    }

}

void baselineParametersDialog::on_buttonBox_rejected()
{
    this->reject();
}
