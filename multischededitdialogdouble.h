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
