#ifndef ADDGROUPDIALOG_H
#define ADDGROUPDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>

namespace Ui {
class AddGroupDialog;
}

class AddGroupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddGroupDialog(QWidget *parent = 0);
    ~AddGroupDialog();

    void addModel(QStandardItemModel *model);

    std::vector<std::string> getSelection();

    std::string getGroupName();

private slots:
    void on_lineEdit_allStationsFilter_textChanged(const QString &arg1);

    void on_listView_all_clicked(const QModelIndex &index);

    void on_listWidget_selected_clicked(const QModelIndex &index);

    void on_buttonBox_accepted();

private:
    QStandardItemModel *all;
    QSortFilterProxyModel *proxy;

    Ui::AddGroupDialog *ui;
};

#endif // ADDGROUPDIALOG_H
