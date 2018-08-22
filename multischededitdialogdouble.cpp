#include "multischededitdialogdouble.h"
#include "ui_multischededitdialogdouble.h"

multiSchedEditDialogDouble::multiSchedEditDialogDouble(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::multiSchedEditDialogDouble)
{
    ui->setupUi(this);
    ui->groupBox_member->hide();
    all = new QStandardItemModel(0,1,this);
    proxy = new QSortFilterProxyModel();
    proxy->setSourceModel(all);

    ui->listView_member->setModel(proxy);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tableWidget_values->verticalHeader()->show();
    ui->tableWidget_values->horizontalHeader()->show();
}

multiSchedEditDialogDouble::~multiSchedEditDialogDouble()
{
    delete ui;
}

QVector<double> multiSchedEditDialogDouble::getValues()
{
    QVector<double> values;
    for(int i = 0; i<ui->tableWidget_values->rowCount(); ++i){
        QDoubleSpinBox *box = qobject_cast<QDoubleSpinBox*>(ui->tableWidget_values->cellWidget(i,0));
        values << box->value();
    }
    return values;
}

void multiSchedEditDialogDouble::addMember(QStandardItemModel *model)
{
    ui->groupBox_member->show();
    for(int i = 0; i< model->rowCount(); ++i){
        all->setItem(i,model->item(i)->clone());
    }
}

QStandardItem *multiSchedEditDialogDouble::getMember()
{
    return all->item(ui->listView_member->selectionModel()->selectedIndexes().at(0).row());
}

void multiSchedEditDialogDouble::on_doubleSpinBox_start_valueChanged(double arg1)
{
    if(arg1 > ui->doubleSpinBox_stop->value()){
        ui->doubleSpinBox_stop->setValue(arg1);
    }
}

void multiSchedEditDialogDouble::on_doubleSpinBox_stop_valueChanged(double arg1)
{
    if(arg1 < ui->doubleSpinBox_start->value()){
        ui->doubleSpinBox_start->setValue(arg1);
    }
}

void multiSchedEditDialogDouble::on_pushButton_generate_clicked()
{
    ui->tableWidget_values->setRowCount(0);
    QVector<double> values;
    for(double i = ui->doubleSpinBox_start->value(); i<= ui->doubleSpinBox_stop->value()+1e-5; i+=ui->doubleSpinBox_step->value()){
        values << i;
    }

    int n = values.size();
    for(int i = 0; i<n; ++i){
        ui->tableWidget_values->insertRow(i);
        QDoubleSpinBox *spinBox = new QDoubleSpinBox(this);
        spinBox->setSingleStep(.1);
        spinBox->setValue(values.at(i));
        ui->tableWidget_values->setCellWidget(i,0, spinBox);
    }
}

void multiSchedEditDialogDouble::on_pushButton_insert_clicked()
{
    ui->tableWidget_values->insertRow(0);
    QDoubleSpinBox *spinBox = new QDoubleSpinBox(this);
    spinBox->setSingleStep(.1);
    ui->tableWidget_values->setCellWidget(0,0, spinBox);
    spinBox->setFocus();
}

void multiSchedEditDialogDouble::on_pushButton_delete_clicked()
{
    auto sel = ui->tableWidget_values->selectionModel()->selectedRows(0);
    for(int i = sel.size()-1; i>=0 ; --i){
        int row = sel.at(0).row();
        ui->tableWidget_values->removeRow(row);
    }
}

void multiSchedEditDialogDouble::on_lineEdit_filter_textChanged(const QString &arg1)
{
    proxy->setFilterRegExp(arg1);
}

void multiSchedEditDialogDouble::on_buttonBox_accepted()
{
    if(ui->tableWidget_values->rowCount()>0){
        if(ui->groupBox_member->isHidden()){
            this->accept();
        }else{
            if(ui->listView_member->selectionModel()->selectedIndexes().count() == 1){
                this->accept();
            }else{
                QMessageBox ms;
                ms.warning(this,"Select member!","You forgot to select a member!");
            }
        }
    }else{
        this->reject();
    }
}