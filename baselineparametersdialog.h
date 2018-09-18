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

#ifndef BASELINEPARAMETERSDIALOG_H
#define BASELINEPARAMETERSDIALOG_H

#include <QDialog>
#include <../VieSchedpp/ParameterSettings.h>
#include <QMessageBox>
#include <settingsloadwindow.h>
namespace Ui {
class baselineParametersDialog;
}

class baselineParametersDialog : public QDialog
{
    Q_OBJECT


public:
    explicit baselineParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent = 0);
    ~baselineParametersDialog();

    void addBandNames(QStringList bands);

    void addDefaultParameters(VieVS::ParameterSettings::ParametersBaselines defaultPara);

    void addSelectedParameters(VieVS::ParameterSettings::ParametersBaselines para, QString paraName);

    void changeParameters(VieVS::ParameterSettings::ParametersBaselines sp);

    std::pair<std::string, VieVS::ParameterSettings::ParametersBaselines> getParameters();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_pushButton_load_clicked();

    void on_pushButton_save_clicked();

private:
    Ui::baselineParametersDialog *ui;
    VieVS::ParameterSettings::ParametersBaselines dp;
    boost::property_tree::ptree &settings;
};

#endif // BASELINEPARAMETERSDIALOG_H
