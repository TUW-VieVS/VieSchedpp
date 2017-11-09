#include "multischededitdialogint.h"
#include "ui_multischededitdialogint.h"

multiSchedEditDialogInt::multiSchedEditDialogInt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::multiSchedEditDialogInt)
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

void multiSchedEditDialogInt::addMember(QStandardItemModel *model)
{
    ui->groupBox_member->show();
    for(int i = 0; i< model->rowCount(); ++i){
        all->setItem(i,model->item(i)->clone());
    }
}

QStandardItem *multiSchedEditDialogInt::getMember()
{
    return all->item(ui->listView_member->selectionModel()->selectedIndexes().at(0).row());
}

void multiSchedEditDialogInt::on_lineEdit_filter_textChanged(const QString &arg1)
{
    proxy->setFilterRegExp(arg1);
}

void multiSchedEditDialogInt::on_buttonBox_accepted()
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
