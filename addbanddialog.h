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

#ifndef ADDBANDDIALOG_H
#define ADDBANDDIALOG_H

#include <QDialog>
#include "settingsloadwindow.h"
#include "boost/property_tree/ptree.hpp"
namespace Ui {
class addBandDialog;
}

class addBandDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addBandDialog(boost::property_tree::ptree &settings_, QWidget *parent = 0);
    ~addBandDialog();

    QString getBandName();

    double getFrequency();

    int getChannels();

private slots:
    void on_pushButton_Load_clicked();

    void on_pushButton_Save_clicked();

    void on_buttonBox_accepted();

private:
    boost::property_tree::ptree &settings;
    Ui::addBandDialog *ui;
};

#endif // ADDBANDDIALOG_H
