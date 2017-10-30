#include "multischededitdialogdatetime.h"
#include "ui_multischededitdialogdatetime.h"

multiSchedEditDialogDateTime::multiSchedEditDialogDateTime(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::multiSchedEditDialogDateTime)
{
    ui->setupUi(this);
}

multiSchedEditDialogDateTime::~multiSchedEditDialogDateTime()
{
    delete ui;
}

QVector<QDateTime> multiSchedEditDialogDateTime::getValues()
{
    QVector<QDateTime> values;
    for(int i = 0; i<ui->tableWidget_values->rowCount(); ++i){
        QDateTimeEdit *box = qobject_cast<QDateTimeEdit*>(ui->tableWidget_values->cellWidget(i,0));
        values << box->dateTime();
    }
    return values;
}

void multiSchedEditDialogDateTime::on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime > ui->dateTimeEdit_stop->dateTime()){
        ui->dateTimeEdit_stop->setDateTime(dateTime);
    }
}

void multiSchedEditDialogDateTime::on_dateTimeEdit_stop_dateTimeChanged(const QDateTime &dateTime)
{
    if(dateTime < ui->dateTimeEdit_start->dateTime()){
        ui->dateTimeEdit_start->setDateTime(dateTime);
    }
}

void multiSchedEditDialogDateTime::on_pushButton_generate_clicked()
{
    ui->tableWidget_values->setRowCount(0);
    QVector<QDateTime> values;

    QDateTime t = ui->dateTimeEdit_start->dateTime();
    int days = ui->spinBox->value();
    int sec = ui->doubleSpinBox_step->value()*3600;

    while (t<ui->dateTimeEdit_stop->dateTime()) {
        values << t;
        t = t.addDays(days).addSecs(sec);
    }

    int n = values.size();
    for(int i = 0; i<n; ++i){
        ui->tableWidget_values->insertRow(i);
        QDateTimeEdit *spinBox = new QDateTimeEdit(this);
        spinBox->setDateTime(values.at(i));
        ui->tableWidget_values->setCellWidget(i,0, spinBox);
    }
}

void multiSchedEditDialogDateTime::on_pushButton_insert_clicked()
{
    ui->tableWidget_values->insertRow(0);
    QDateTimeEdit *spinBox = new QDateTimeEdit(this);
    spinBox->setDate(QDate(2017,01,01));
    spinBox->setTime(QTime(0,30,0));
    ui->tableWidget_values->setCellWidget(0,0, spinBox);
    spinBox->setFocus();
}

void multiSchedEditDialogDateTime::on_pushButton_delete_clicked()
{
    auto sel = ui->tableWidget_values->selectionModel()->selectedRows(0);
    for(int i = sel.size()-1; i>=0 ; --i){
        int row = sel.at(0).row();
        ui->tableWidget_values->removeRow(row);
    }

}
