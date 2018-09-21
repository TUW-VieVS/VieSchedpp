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

#include "savetosettingsdialog.h"
#include "ui_savetosettingsdialog.h"

saveToSettingsDialog::saveToSettingsDialog(boost::property_tree::ptree &settings, QWidget *parent) :
    QDialog(parent), settings{settings},
    ui(new Ui::saveToSettingsDialog)
{
    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(":/icons/icons/VieSchedppGUI_logo.png"));
    this->setWindowTitle("VieSched++");

    QIcon ic = style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    QPixmap pix = ic.pixmap(100,100);
    ui->label_img->setPixmap(pix);
}

saveToSettingsDialog::~saveToSettingsDialog()
{
    delete ui;
}

void saveToSettingsDialog::setType(saveToSettingsDialog::Type type)
{
    this->type = type;
}

void saveToSettingsDialog::setNetwork(QVector<QString> newNetwork)
{
    network = newNetwork;
}

void saveToSettingsDialog::setMode(int myBits, double mySrate, QVector<QString> myBands, QVector<double> myFreqs, QVector<int> myChanls)
{
    bands = myBands;
    freqs = myFreqs;
    chanls = myChanls;
    bits = myBits;
    srate = mySrate;
}

void saveToSettingsDialog::on_buttonBox_accepted()
{
    if(ui->lineEdit->text().isEmpty()){
        QMessageBox::warning(this,"Missing name!","Please add name first!");
    }else{
        switch (type) {
        case Type::stationNetwork:{
            boost::property_tree::ptree pt;
            pt.add("network.<xmlattr>.name", ui->lineEdit->text().toStdString());

            for (int i = 0; i<network.size(); ++i) {
                boost::property_tree::ptree tmp;
                tmp.add("member", network.at(i).toStdString());
                pt.add_child("network.member", tmp.get_child("member"));
            }
            auto x = pt.get_child("network");
            settings.add_child("settings.networks.network", x);

            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
            break;
        }
        case Type::sourceNetwork:{
            boost::property_tree::ptree pt;
            pt.add("source_list.<xmlattr>.name", ui->lineEdit->text().toStdString());

            for (int i = 0; i<network.size(); ++i) {
                boost::property_tree::ptree tmp;
                tmp.add("member", network.at(i).toStdString());
                pt.add_child("source_list.member", tmp.get_child("member"));
            }
            settings.add_child("settings.source_lists.source_list", pt.get_child("source_list"));

            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
            break;
        }
        case Type::modes:{
            boost::property_tree::ptree pt;
            pt.add("mode.<xmlattr>.name", ui->lineEdit->text().toStdString());
            pt.add("mode.bits",bits);
            pt.add("mode.sampleRate",srate);
            for (int i = 0; i<bands.size(); ++i) {
                boost::property_tree::ptree tmp;
                tmp.add("band.<xmlattr>.name", bands.at(i).toStdString());
                tmp.add("band.frequency",freqs.at(i));
                tmp.add("band.channels",chanls.at(i));
                pt.add_child("mode.band", tmp.get_child("band"));
            }
            settings.add_child("settings.modes.mode", pt.get_child("mode"));
            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
            break;
        }
        default:
            break;
        }
        this->accept();
    }
}
