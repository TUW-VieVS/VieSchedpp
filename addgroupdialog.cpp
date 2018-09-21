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

#include "addgroupdialog.h"
#include "ui_addgroupdialog.h"

AddGroupDialog::AddGroupDialog(boost::property_tree::ptree &settings_, Type type, QWidget *parent) :
    QDialog(parent), settings{settings_}, type{type},
    ui(new Ui::AddGroupDialog)
{
    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(":/icons/icons/VieSchedppGUI_logo.png"));
    this->setWindowTitle("VieSched++");

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

void AddGroupDialog::addModel(QStandardItemModel *model)
{
    for(int i = 0; i< model->rowCount(); ++i){
        all->setItem(i,model->item(i)->clone());
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
    proxy->setFilterRegExp(arg1);
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

void AddGroupDialog::on_pushButton_Save_clicked()
{
    if(ui->lineEdit_groupName->text().isEmpty()){
        QMessageBox::warning(this,"No group Name!","Please add group name first!");
    }else{
        if(QMessageBox::Yes == QMessageBox::question(this,"Save?","Are you sure you want to save this group?\nThis will save this group to settings.xml file for further use.")){

            boost::property_tree::ptree pt_group;
            pt_group.add("group.<xmlattr>.name", ui->lineEdit_groupName->text().toStdString());

            for (int i = 0; i<ui->listWidget_selected->count(); ++i) {
                boost::property_tree::ptree tmp;
                tmp.add("member", ui->listWidget_selected->item(i)->text().toStdString());
                pt_group.add_child("group.member", tmp.get_child("member"));
            }

            switch (type) {
            case Type::station:{
                settings.add_child("settings.station.groups.group", pt_group.get_child("group"));
                break;
            }
            case Type::source:{
                settings.add_child("settings.source.groups.group", pt_group.get_child("group"));
                break;
            }
            case Type::baseline:{
                settings.add_child("settings.baseline.groups.group", pt_group.get_child("group"));
                break;
            }
            default:
                break;
            }

            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
        }
    }
}

void AddGroupDialog::on_pushButton_Load_clicked()
{
    boost::optional<boost::property_tree::ptree> groupstree;
    switch (type) {
        case Type::station:{ groupstree = settings.get_child_optional("settings.station.groups"); break; }
        case Type::source:{ groupstree = settings.get_child_optional("settings.source.groups"); break; }
        case Type::baseline:{ groupstree = settings.get_child_optional("settings.baseline.groups"); break; }
        default:{ break; }
    }

    if(groupstree.is_initialized()){
        QVector<QString> names;
        QVector<QVector<QString> > groups;

        for(const auto &it:*groupstree){
            QString name = QString::fromStdString(it.second.get_child("<xmlattr>.name").data());
            QVector<QString> group;

            for(const auto &it2:it.second){
                if(it2.first == "member"){
                    group.push_back(QString::fromStdString(it2.second.data()));
                }
            }

            names.push_back(name);
            groups.push_back(group);
        }
        settingsLoadWindow *dial = new settingsLoadWindow(this);

        switch (type) {
            case Type::station:{ dial->setStationGroups(names,groups); break; }
            case Type::source:{ dial->setSourceGroups(names,groups); break; }
            case Type::baseline:{ dial->setBaselineGroups(names,groups); break; }
            default:{ break; }
        }


        int result = dial->exec();
        if(result == QDialog::Accepted){
            QString warningTxt;

            QString itm = dial->selectedItem();
            int idx = dial->selectedIdx();
            QVector<QString> members = groups.at(idx);

            ui->listWidget_selected->clear();
            for(const auto&any:members){
                auto list = all->findItems(any);
                if(list.size() >= 1){
                    ui->listWidget_selected->addItem(list.at(0)->text());
                }else{
                    warningTxt.append("    unknown member: ").append(any).append("!\n");
                }
            }

            ui->lineEdit_groupName->setText(itm);

            if(!warningTxt.isEmpty()){
                QString txt = "The following errors occurred while loading the group:\n";
                txt.append(warningTxt).append("These members were ignored!\nPlease double check members again!");
                QMessageBox::warning(this,"Unknown group members!",txt);
            }
        }
    }else{
        QMessageBox::warning(this,"No groups found!","No previously saved group found in settings.xml");

    }
}
