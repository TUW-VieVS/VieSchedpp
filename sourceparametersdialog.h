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

#ifndef SOURCEPARAMETERSDIALOG_H
#define SOURCEPARAMETERSDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <../VieSchedpp/ParameterSettings.h>
#include <settingsloadwindow.h>

namespace Ui {
class sourceParametersDialog;
}

class sourceParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit sourceParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent = 0);
    ~sourceParametersDialog();

    void addBandNames(QStringList bands);

    void addStationModel(QStandardItemModel *stations);

    void addBaselineModel(QStandardItemModel *baselines);

    void addDefaultParameters(VieVS::ParameterSettings::ParametersSources defaultPara);

    void addSelectedParameters(VieVS::ParameterSettings::ParametersSources para, QString paraName);

    void changeParameters(VieVS::ParameterSettings::ParametersSources sp);

    std::pair<std::string, VieVS::ParameterSettings::ParametersSources> getParameters();

private slots:
    void on_lineEdit_ignoreStations_textChanged(const QString &arg1);

    void on_lineEdit_filterRequiredStations_textChanged(const QString &arg1);

    void on_lineEdit_filter_ignoreBaselines_textChanged(const QString &arg1);

    void on_listWidget_selectedIgnoreStations_clicked(const QModelIndex &index);

    void on_listWidget_selectedRequiredStations_clicked(const QModelIndex &index);

    void on_listWidget_selectedIgnoreBaselines_clicked(const QModelIndex &index);

    void on_listView_ignoreStations_clicked(const QModelIndex &index);

    void on_listView_requiredStations_clicked(const QModelIndex &index);

    void on_listView_ignoreBaselines_clicked(const QModelIndex &index);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_spinBox_evenlyDistScans_valueChanged(int arg1);

    void on_groupBox_10_toggled(bool arg1);

    void on_groupBox_9_toggled(bool arg1);

    void on_groupBox_variableScanDuration_toggled(bool arg1);

    void on_groupBox_fixedScanDuration_toggled(bool arg1);

private:
    Ui::sourceParametersDialog *ui;
    VieVS::ParameterSettings::ParametersSources dp;
    boost::property_tree::ptree &settings;

    QStandardItemModel *stations;
    QSortFilterProxyModel *stations_proxyIgnore;
    QSortFilterProxyModel *stations_proxyRequired;

    QStandardItemModel *baseline;
    QSortFilterProxyModel *baseline_proxy;

};

#endif // SOURCEPARAMETERSDIALOG_H
