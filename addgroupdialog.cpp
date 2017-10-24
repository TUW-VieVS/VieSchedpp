#include "addgroupdialog.h"
#include "ui_addgroupdialog.h"

AddGroupDialog::AddGroupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddGroupDialog)
{
    ui->setupUi(this);
    all = new QStandardItemModel(0,1,this);
    proxy = new QSortFilterProxyModel();
    proxy->setSourceModel(all);

    ui->listView_all->setModel(proxy);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

AddGroupDialog::~AddGroupDialog()
{
    delete ui;
}

void AddGroupDialog::addModel(QStringListModel *model)
{
    for(int i = 0; i< model->rowCount(); ++i){
        all->insertRow(i, new QStandardItem(model->index(i,0).data().toString()));
    }

}

std::vector<std::string> AddGroupDialog::getSelection()
{
    std::vector<std::string> sel;
    for (int i = 0; i<ui->listWidget_selected->count(); ++i){
        sel.push_back(ui->listWidget_selected->item(i)->text().toStdString());
    }

    return sel;
}

std::string AddGroupDialog::getGroupName()
{
    QString txt = ui->lineEdit_groupName->text();
    txt = txt.trimmed();
    txt = txt.simplified();
    txt.replace(" ","_");
    return txt.toStdString();
}

void AddGroupDialog::on_lineEdit_allStationsFilter_textChanged(const QString &arg1)
{
    proxy->setFilterRegExp(ui->lineEdit_allStationsFilter->text());
}


void AddGroupDialog::on_listView_all_clicked(const QModelIndex &index)
{
    QString itm = index.data().toString();

    if( ui->listWidget_selected->findItems(itm,Qt::MatchExactly).isEmpty() ){
        ui->listWidget_selected->addItem(itm);
    }
}

void AddGroupDialog::on_listWidget_selected_clicked(const QModelIndex &index)
{
    auto itm = ui->listWidget_selected->takeItem(index.row());
    delete(itm);
}

void AddGroupDialog::on_buttonBox_accepted()
{
    if(ui->lineEdit_groupName->text().isEmpty()){
        QMessageBox mb;
        mb.warning(this,"missing group name","Please add a group name");
    }else{
        this->accept();
    }
}
