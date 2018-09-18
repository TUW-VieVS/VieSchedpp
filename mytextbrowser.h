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

#ifndef MYTEXTBROWSER_H
#define MYTEXTBROWSER_H

#include <QObject>
#include <QTextBrowser>
#include <QProcess>
#include <QRegularExpression>
#include <QScrollBar>

class myTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit myTextBrowser(QWidget *parent = nullptr);

signals:

public slots:
    void readyReadStandardOutput();

    void readyReadStandardError();

private:
    void highlightWord(QString word, QColor color);
};

#endif // MYTEXTBROWSER_H
