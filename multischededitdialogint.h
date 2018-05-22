#ifndef MULTISCHEDEDITDIALOGINT_H
#define MULTISCHEDEDITDIALOGINT_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>

namespace Ui {
class multiSchedEditDialogInt;
}

class multiSchedEditDialogInt : public QDialog
{
    Q_OBJECT

public:
    explicit multiSchedEditDialogInt(QWidget *parent = 0);
    ~multiSchedEditDialogInt();

    QVector<int> getValues();

    void addMember(QStandardItemModel *model);

    QStandardItem* getMember();

private slots:
    void on_pushButton_generate_clicked();

    void on_pushButton_insert_clicked();

    void on_pushButton_delete_clicked();

    void on_spinBox_start_valueChanged(int arg1);

    void on_spinBox_stop_valueChanged(int arg1);

    void on_lineEdit_filter_textChanged(const QString &arg1);

    void on_buttonBox_accepted();

private:
    Ui::multiSchedEditDialogInt *ui;
    QStandardItemModel *all;
    QSortFilterProxyModel *proxy;
};

#endif // MULTISCHEDEDITDIALOGINT_H
