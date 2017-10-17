#ifndef MULTISCHEDEDITDIALOGDOUBLE_H
#define MULTISCHEDEDITDIALOGDOUBLE_H

#include <QDialog>

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

private slots:
    void on_doubleSpinBox_start_valueChanged(double arg1);

    void on_doubleSpinBox_stop_valueChanged(double arg1);

    void on_pushButton_generate_clicked();

    void on_pushButton_insert_clicked();

    void on_pushButton_delete_clicked();

private:
    Ui::multiSchedEditDialogDouble *ui;
};

#endif // MULTISCHEDEDITDIALOGDOUBLE_H
