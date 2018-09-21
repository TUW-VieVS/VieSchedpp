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

void baselineParametersDialog::addDefaultParameters(VieVS::ParameterSettings::ParametersBaselines d)
{
    dp = d;
    ui->doubleSpinBox_weight->setValue(*d.weight);
    ui->spinBox_minScan->setValue(*d.minScan);
    ui->spinBox_maxScan->setValue(*d.maxScan);
}

void baselineParametersDialog::addSelectedParameters(VieVS::ParameterSettings::ParametersBaselines para, QString paraName)
{
    changeParameters(para);
    ui->lineEdit_paraName->setText(paraName);
    if(paraName == "default"){
        ui->groupBox_ignore->setCheckable(false);
        ui->checkBox->setChecked(true);
        ui->checkBox->setEnabled(false);
        ui->groupBox_scanTime->setCheckable(false);

        ui->pushButton_load->setEnabled(false);
        ui->pushButton_save->setEnabled(false);
    }
    ui->lineEdit_paraName->setEnabled(false);
}

void baselineParametersDialog::changeParameters(VieVS::ParameterSettings::ParametersBaselines sp)
{
    QString warningTxt;
    if(sp.ignore.is_initialized()){
        if(*sp.ignore){
            ui->radioButton_ignore_yes->setChecked(true);
        }else{
            ui->radioButton_ignore_no->setChecked(true);
        }
        ui->groupBox_ignore->setChecked(true);
    }else{
        ui->groupBox_ignore->setChecked(false);
    }

    if(sp.weight.is_initialized()){
        ui->doubleSpinBox_weight->setValue(*sp.weight);
        ui->checkBox->setEnabled(true);
    }else{
        ui->doubleSpinBox_weight->setValue(*dp.weight);
        ui->checkBox->setEnabled(false);
    }

    ui->groupBox_scanTime->setChecked(false);
    if(sp.maxScan.is_initialized()){
        ui->spinBox_maxScan->setValue(*sp.maxScan);
        ui->groupBox_scanTime->setChecked(true);
    }else{
        ui->spinBox_maxScan->setValue(*dp.maxScan);
    }

    if(sp.minScan.is_initialized()){
        ui->spinBox_minScan->setValue(*sp.minScan);
        ui->groupBox_scanTime->setChecked(true);
    }else{
        ui->spinBox_minScan->setValue(*dp.minScan);
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
            ui->groupBox_scanTime->setChecked(true);
            qobject_cast<QDoubleSpinBox*>(ui->tableWidget_SNR->cellWidget(idx,0))->setValue(val);
        }else{
            warningTxt.append("    unknown band name: ").append(name).append(" for minimum SNR!\n");
        }
    }
    if(!warningTxt.isEmpty()){
        QString txt = "The following errors occurred while loading the parameters:\n";
        txt.append(warningTxt).append("These parameters were ignored!\nPlease double check parameters again!");
        QMessageBox::warning(this,"Unknown parameters!",txt);
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

    if(ui->groupBox_ignore->isChecked() || !ui->groupBox_ignore->isCheckable()){
        if(ui->radioButton_ignore_yes->isChecked()){
            para.ignore = true;
        }else{
            para.ignore = false;
        }
    }
    if(ui->groupBox_scanTime->isChecked() || !ui->groupBox_scanTime->isCheckable()){
        para.minScan = ui->spinBox_minScan->value();
        para.maxScan = ui->spinBox_maxScan->value();

        for(int i = 0; i<ui->tableWidget_SNR->rowCount(); ++i){
            QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox*> (ui->tableWidget_SNR->cellWidget(i,0));
            if(w->value()!=0){
                para.minSNR[ui->tableWidget_SNR->verticalHeaderItem(i)->text().toStdString()] = w->value();
            }
        }
    }

    if(ui->doubleSpinBox_weight->isEnabled()){
        para.weight = ui->doubleSpinBox_weight->value();
    }

    return std::make_pair(name,para);
}

baselineParametersDialog::baselineParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent) :
    QDialog(parent), settings{settings_},
    ui(new Ui::baselineParametersDialog)
{
    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(":/icons/icons/VieSchedppGUI_logo.png"));
    this->setWindowTitle("VieSched++");

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

void baselineParametersDialog::on_pushButton_load_clicked()
{
    boost::property_tree::ptree parameters = settings.get_child("settings.baseline.parameters");
    QVector<QString> names;
    QVector<VieVS::ParameterSettings::ParametersBaselines> paras;

    for(const auto &it:parameters){
        auto tmp = VieVS::ParameterSettings::ptree2parameterBaseline(it.second);
        std::string name = tmp.first;
        names.push_back(QString::fromStdString(name));
        VieVS::ParameterSettings::ParametersBaselines para = tmp.second;
        paras.push_back(para);
    }
    settingsLoadWindow *dial = new settingsLoadWindow(this);
    dial->setBaselineParameters(names,paras);
    int result = dial->exec();
    if(result == QDialog::Accepted){

        QString itm = dial->selectedItem();
        int idx = dial->selectedIdx();
        VieVS::ParameterSettings::ParametersBaselines sp = paras.at(idx);

        changeParameters(sp);

        ui->lineEdit_paraName->setText(itm);
    }
}

void baselineParametersDialog::on_pushButton_save_clicked()
{
    if(ui->lineEdit_paraName->text().isEmpty()){
        QMessageBox::warning(this,"No parameter Name!","Please add parameter name first!");
    }else{
        if(QMessageBox::Yes == QMessageBox::question(this,"Save?","Are you sure you want to save this settings?\nThis will save the settings to settings.xml file for further use.")){
            std::pair<std::string, VieVS::ParameterSettings::ParametersBaselines> para = getParameters();

            settings.add_child("settings.baseline.parameters.parameter",VieVS::ParameterSettings::parameterBaseline2ptree(para.first,para.second).get_child("parameters"));
            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
        }
    }
}
