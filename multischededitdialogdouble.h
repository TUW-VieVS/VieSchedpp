#ifndef MULTISCHEDEDITDIALOGDOUBLE_H
#define MULTISCHEDEDITDIALOGDOUBLE_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>

namespace Ui {
class multiSchedEditDialogDouble;
}

class multiSchedEditDialogDouble : public QDialog
{
    Q_OBJECT

public:
    explicit multiSchedEditDialogDouble(QWidget *parent = 0);
    ~multiSchedEditDialogDouble();

    QVector<double> getValues();

    void addMember(QStandardItemModel *model);

    QStandardItem* getMember();

private slots:
    void on_doubleSpinBox_start_valueChanged(double arg1);

    void on_doubleSpinBox_stop_valueChanged(double arg1);

    void on_pushButton_generate_clicked();

    void on_pushButton_insert_clicked();

    void on_pushButton_delete_clicked();

    void on_lineEdit_filter_textChanged(const QString &arg1);

    void on_buttonBox_accepted();

private:
    Ui::multiSchedEditDialogDouble *ui;
    QStandardItemModel *all;
    QSortFilterProxyModel *proxy;
};

#endif // MULTISCHEDEDITDIALOGDOUBLE_H
