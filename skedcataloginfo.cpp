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

#include "skedcataloginfo.h"
#include "ui_skedcataloginfo.h"

SkedCatalogInfo::SkedCatalogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SkedCatalogInfo)
{
    ui->setupUi(this);
    QApplication::setWindowIcon(QIcon(":/icons/icons/VieSchedppGUI_logo.png"));
    this->setWindowTitle("VieSched++");
}

void SkedCatalogInfo::setFonts()
{
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QString family = fixedFont.family();

    ui->textEdit_2->setFont(fixedFont);
    ui->textEdit_3->setFont(fixedFont);
    ui->textEdit_4->setFont(fixedFont);
}

SkedCatalogInfo::~SkedCatalogInfo()
{
    delete ui;
}
