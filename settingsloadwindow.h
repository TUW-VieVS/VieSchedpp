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

#ifndef SETTINGSLOADWINDOW_H
#define SETTINGSLOADWINDOW_H

#include <QDialog>
#include "../VieSchedpp/ParameterSettings.h"
#include <QListWidgetItem>
#include <QMessageBox>
namespace Ui {
class settingsLoadWindow;
}

class settingsLoadWindow : public QDialog
{
    Q_OBJECT

public:
    enum class Type {
        stationParameters,
        sourceParameters,
        baselineParameters,
        stationGroup,
        sourceGroup,
        baselineGroup,
        bands,
        network,
        sourceList,
        modes
    };


    explicit settingsLoadWindow(QWidget *parent = 0);
    ~settingsLoadWindow();

    void setStationParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersStations> &paras);

    void setSourceParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersSources> &paras);

    void setBaselineParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersBaselines> &paras);

    void setStationGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members);

    void setSourceGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members);

    void setBaselineGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members);

    void setBands(const QVector<QString> &name, const QVector<QPair<double, int> > &bands);

    void setNetwork(const QVector<QString> &name, const QVector<QVector<QString> > &members);

    void setSourceList(const QVector<QString> &name, const QVector<QVector<QString> > &members);

    void setModes(const QVector<QString> &names, const QVector<int> &bits, const QVector<double> &srates,
                  const QVector<QVector<QString> > &bands, const QVector<QVector<int> > &channels,
                  const QVector<QVector<double> > &freqs);

    QString selectedItem();

    int selectedIdx();

private slots:
    void refreshList(QListWidgetItem*);

    void on_buttonBox_accepted();

private:
    Ui::settingsLoadWindow *ui;

    Type type;

    QVector<VieVS::ParameterSettings::ParametersStations> paraSta;
    QVector<VieVS::ParameterSettings::ParametersSources> paraSrc;
    QVector<VieVS::ParameterSettings::ParametersBaselines> paraBl;

    QVector<QVector<QString> > groupSta;
    QVector<QVector<QString> > groupSrc;
    QVector<QVector<QString> > groupBl;

    QVector<QPair<double,int> > bands;

    QVector<QVector<QString> > network;
    QVector<QVector<QString> > sourceList;

    QVector<int> bits;
    QVector<double> srates;
    QVector<QVector<QString> > modes_bands;
    QVector<QVector<int> > channels;
    QVector<QVector<double> > freqs;


};

#endif // SETTINGSLOADWINDOW_H
