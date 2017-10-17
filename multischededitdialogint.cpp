#include "multischededitdialogint.h"
#include "ui_multischededitdialogint.h"

multiSchedEditDialogInt::multiSchedEditDialogInt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::multiSchedEditDialogInt)
{
    ui->setupUi(this);
}

multiSchedEditDialogInt::~multiSchedEditDialogInt()
{
    delete ui;
}

QVector<int> multiSchedEditDialogInt::getValues()
{
    QVector<int> values;
    for(int i = 0; i<ui->tableWidget_values->rowCount(); ++i){
        QSpinBox *box = qobject_cast<QSpinBox*>(ui->tableWidget_values->cellWidget(i,0));
        values << box->value();
    }
    return values;
}

void multiSchedEditDialogInt::on_pushButton_generate_clicked()
{
    ui->tableWidget_values->setRowCount(0);
    QVector<int> values;
    for(int i = ui->spinBox_start->value(); i<= ui->spinBox_stop->value(); i+=ui->spinBox_step->value()){
        values << i;
    }

    int n = values.size();
    for(int i = 0; i<n; ++i){
        ui->tableWidget_values->insertRow(i);
        QSpinBox *spinBox = new QSpinBox(this);
        spinBox->setSingleStep(60);
        spinBox->setMaximum(10000);
        spinBox->setValue(values.at(i));
        ui->tableWidget_values->setCellWidget(i,0, spinBox);
    }

}

void multiSchedEditDialogInt::on_pushButton_insert_clicked()
{
    ui->tableWidget_values->insertRow(0);
    QSpinBox *spinBox = new QSpinBox(this);
    spinBox->setSingleStep(60);
    spinBox->setMaximum(10000);
    ui->tableWidget_values->setCellWidget(0,0, spinBox);
    spinBox->setFocus();

}

void multiSchedEditDialogInt::on_pushButton_delete_clicked()
{
    auto sel = ui->tableWidget_values->selectionModel()->selectedRows(0);
    for(int i = sel.size()-1; i>=0 ; --i){
        int row = sel.at(0).row();
        ui->tableWidget_values->removeRow(row);
    }

}

void multiSchedEditDialogInt::on_spinBox_start_valueChanged(int arg1)
{
    if(arg1 > ui-> spinBox_stop->value()){
        ui->spinBox_stop->setValue(arg1);
    }
}

void multiSchedEditDialogInt::on_spinBox_stop_valueChanged(int arg1)
{
    if(arg1 < ui->spinBox_start->value()){
        ui->spinBox_start->setValue(arg1);
    }

}
