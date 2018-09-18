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

#ifndef SAVETOSETTINGSDIALOG_H
#define SAVETOSETTINGSDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QStyle>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace Ui {
class saveToSettingsDialog;
}

class saveToSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Type {
        stationNetwork,
        sourceNetwork,
        modes
    };

    explicit saveToSettingsDialog(boost::property_tree::ptree &settings, QWidget *parent = 0);

    void setType(Type type);

    void setNetwork(QVector<QString> newNetwork);

    void setMode(int bits, double srate, QVector<QString> myBands, QVector<double> myFreqs, QVector<int> myChanls);

    ~saveToSettingsDialog();

private slots:
    void on_buttonBox_accepted();

private:
    boost::property_tree::ptree &settings;
    Type type;

    QVector<QString> network;

    QVector<QString> bands;
    QVector<double> freqs;
    QVector<int> chanls;
    int bits;
    double srate;

    Ui::saveToSettingsDialog *ui;
};

#endif // SAVETOSETTINGSDIALOG_H
