#ifndef MULTISCHEDEDITDIALOGDATETIME_H
#define MULTISCHEDEDITDIALOGDATETIME_H

#include <QDialog>

namespace Ui {
class multiSchedEditDialogDateTime;
}

class multiSchedEditDialogDateTime : public QDialog
{
    Q_OBJECT

public:
    explicit multiSchedEditDialogDateTime(QWidget *parent = 0);
    ~multiSchedEditDialogDateTime();

    QVector<QDateTime> getValues();

private slots:
    void on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime);

    void on_dateTimeEdit_stop_dateTimeChanged(const QDateTime &dateTime);

    void on_pushButton_generate_clicked();

    void on_pushButton_insert_clicked();

    void on_pushButton_delete_clicked();

private:
    Ui::multiSchedEditDialogDateTime *ui;
};

#endif // MULTISCHEDEDITDIALOGDATETIME_H
