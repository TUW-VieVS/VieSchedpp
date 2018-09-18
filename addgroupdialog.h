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

#ifndef ADDGROUPDIALOG_H
#define ADDGROUPDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>

#include "settingsloadwindow.h"
#include "boost/property_tree/ptree.hpp"

namespace Ui {
class AddGroupDialog;
}

class AddGroupDialog : public QDialog
{
    Q_OBJECT

public:

    enum class Type {
        station,
        source,
        baseline,
    };

    explicit AddGroupDialog(boost::property_tree::ptree &settings_, Type type, QWidget *parent = 0);

    ~AddGroupDialog();

    void addModel(QStandardItemModel *model);

    std::vector<std::string> getSelection();

    std::string getGroupName();

private slots:
    void on_lineEdit_allStationsFilter_textChanged(const QString &arg1);

    void on_listView_all_clicked(const QModelIndex &index);

    void on_listWidget_selected_clicked(const QModelIndex &index);

    void on_buttonBox_accepted();

    void on_pushButton_Save_clicked();

    void on_pushButton_Load_clicked();

private:
    QStandardItemModel *all;
    QSortFilterProxyModel *proxy;
    boost::property_tree::ptree &settings;

    Type type;

    Ui::AddGroupDialog *ui;
};

#endif // ADDGROUPDIALOG_H
