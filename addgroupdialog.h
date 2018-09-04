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