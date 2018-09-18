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

#ifndef TEXTFILEVIEWER_H
#define TEXTFILEVIEWER_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <QFileDialog>
#include <QTextDocumentWriter>
#include <fstream>
#include <regex>
#include <QShortcut>

namespace Ui {
class textfileViewer;
}

class textfileViewer : public QMainWindow
{
    Q_OBJECT

public:
    enum class Type{
        log,
        skd,
        vex,
        undefined
    };

    explicit textfileViewer(QWidget *parent = 0);
    ~textfileViewer();

    bool setTextFile(QString path, Type type);

private slots:
    void on_actionview_triggered();

    void on_actionhighlight_triggered();

    void on_lineEdit_search_editingFinished();


    void on_treeWidget_navigation_clicked(const QModelIndex &index);

    void on_textBrowser_view_anchorClicked(const QUrl &arg1);

    void on_pushButton_jumpBack_clicked();

    void on_actionSave_triggered();

    void on_pushButton_writemode_toggled(bool checked);

private:
    Ui::textfileViewer *ui;
    int lastPosition_;

    std::string addAnchors(const std::string &prefix, const std::string & content);

    std::string addReferences(const std::string &prefix, const std::string & content);

    std::string highlight_vex(const std::string &prefix,  const std::string & content);
    std::string highlight_skd(const std::string & content);
    std::string highlight_log(const std::string & content);

};

#endif // TEXTFILEVIEWER_H
