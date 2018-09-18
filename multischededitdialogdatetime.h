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

    void on_buttonBox_accepted();

private:
    Ui::multiSchedEditDialogDateTime *ui;
};

#endif // MULTISCHEDEDITDIALOGDATETIME_H
