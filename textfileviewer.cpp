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

#include "textfileviewer.h"
#include "ui_textfileviewer.h"




textfileViewer::textfileViewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::textfileViewer)
{
    ui->setupUi(this);
    QApplication::setWindowIcon(QIcon(":/icons/icons/VieSchedppGUI_logo.png"));
    this->setWindowTitle("VieSched++");

    ui->pushButton_jumpBack->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Down));
}

textfileViewer::~textfileViewer()
{
    delete ui;
}

void textfileViewer::on_actionview_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void textfileViewer::on_actionhighlight_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
}

bool textfileViewer::setTextFile(QString path, Type type)
{
    ui->label_fname->setText(path);

    switch (type) {
    case Type::log:
        ui->pushButton_jumpBack->setVisible(false);
        ui->dockWidget_navigate->setVisible(true);
        break;
    case Type::skd:
        ui->pushButton_jumpBack->setVisible(false);
        ui->dockWidget_navigate->setVisible(true);
        break;
    case Type::vex:
        ui->dockWidget_navigate->setVisible(true);
        break;
    default:
        break;
    }

    ui->textBrowser_view->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QFile file(path);
    if(!file.exists()){
        return false;
    }
    ui->textBrowser_view->clear();
    QTreeWidget* tree = ui->treeWidget_navigation;

    QMessageBox info;

    std::ifstream ifs (path.toStdString(), std::ifstream::in);

    std::regex head;
    if(type == Type::vex){
        head = std::regex("\\s*(\\$\\w*);", std::regex::optimize);
    }else if(type == Type::skd){
        head = std::regex("(\\$\\w*)", std::regex::optimize);
    }else if(type == Type::log){
        head = std::regex("scan_name=([^,]*)");
    }
    std::string line;
    std::getline(ifs,line);

    int i=0;
    while (ifs.good()){
        std::string prefix;
        std::smatch m;

        if(std::regex_match(line,m,head)){
            prefix = m.str(1);
            tree->insertTopLevelItem(tree->topLevelItemCount(),new QTreeWidgetItem(QStringList() << QString::fromStdString(prefix)));
            line = std::regex_replace(line,head,"<a id=\"$1\"/><font color=\"#FF00FF\">$0</font>");
        }

        std::string content = "<PRE>";
        content.append(line).append("<br>");
        while (std::getline(ifs,line)) {
            ++i;
            if(std::regex_match(line,m,head)){
                break;
            }else{
                content.append(line).append("<br>");
            }
        }

        content.append("</PRE>");
        switch (type) {
        case Type::log:
            content = highlight_log(content);
            break;
        case Type::skd:
            content = highlight_skd(content);
            break;
        case Type::vex:
            content = addAnchors(prefix, content);
            content = addReferences(prefix, content);
            content = highlight_vex(prefix, content);
            break;
        default:
            break;
        }
        ui->textBrowser_view->insertHtml(QString::fromStdString(content));
    }


    QTextCursor cursor = ui->textBrowser_view->textCursor();
    cursor.setPosition(0);
    ui->textBrowser_view->setTextCursor(cursor);
    return true;
}

std::string textfileViewer::addAnchors(const std::string &prefix, const std::string &content)
{
    if(prefix != "$SCHED"){
        std::regex r("\\s(def\\s+)([^;]*)");
        std::string replace = "<a id=\""+prefix+"_$2\"/>"
                                                "<font color=\"#00FFFF\">"
                                                "$0"
                                                "</font>";
        return std::regex_replace(content,r,replace);
    }else{
        return content;
    }

}

std::string textfileViewer::addReferences(const std::string &prefix, const std::string &content)
{
    std::string newContent;
    if(prefix == "$GLOBAL"){
        std::regex ref("(ref\\s+)(\\$[^\\s]*)(\\s*=\\s*)([^;\\s:]*)");
        std::string replace = "<font color=\"#39FF14\">"
                                "$1"
                              "</font>"
                              "<a href=\"#$2\">"
                                "<font color=\"#39FF14\">"
                                  "$2"
                                "</font>"
                              "</a>"
                              "$3"
                              "<a href=\"#$2_$4\">"
                                "<font color=\"#FFFFFF\">"
                                  "$4"
                                "</font>"
                              "</a>";
        newContent = regex_replace(content,ref,replace);
    }else if(prefix == "$STATION"){
        std::regex ref("(ref\\s+)(\\$[^\\s]*)(\\s*=\\s*)([^;\\s:]*)");
        std::string replace = "<font color=\"#39FF14\">"
                                "$1"
                              "</font>"
                              "<a href=\"#$2\">"
                                "<font color=\"#39FF14\">"
                                  "$2"
                                "</font>"
                              "</a>"
                              "$3"
                              "<a href=\"#$2_$4\">"
                                "<font color=\"#FFFFFF\">"
                                  "$4"
                                "</font>"
                              "</a>";
        newContent = regex_replace(content,ref,replace);

    }else if(prefix == "$MODE"){
        std::regex tlc("(\\s*:\\s*)(\\w{2})");
        std::string replace = "$1"
                              "<a href=\"#$STATION_$2\">"
                                "<font color=\"#FFFFFF\">"
                                  "$2"
                                "</font>"
                              "</a>";
        newContent = regex_replace(content,tlc,replace);

        std::regex ref("(ref\\s+)(\\$[^\\s]*)(\\s*=\\s*)([^;\\s:]*)");
        std::string replace2= "<font color=\"#39FF14\">"
                                "$1"
                              "</font>"
                              "<a href=\"#$2\">"
                                "<font color=\"#39FF14\">"
                                  "$2"
                                "</font>"
                              "</a>"
                              "$3"
                              "<a href=\"#$2_$4\">"
                                "<font color=\"#FFFFFF\">"
                                  "$4"
                                "</font>"
                              "</a>";
        newContent = regex_replace(newContent,ref,replace2);


    }else if(prefix == "$SCHED"){
        std::regex source("(source\\s*=\\s*)([^;\\s]*)");
        std::string replace = "$1"
                              "<a href=\"#$SOURCE_$2\">"
                                "<font color=\"#FFFFFF\">"
                                  "$2"
                                "</font>"
                              "</a>";
        newContent = regex_replace(content,source,replace);

        std::regex station("(station\\s*=\\s*)([^;\\s]*)");
        std::string replace2 = "$1"
                               "<a href=\"#$STATION_$2\">"
                                 "<font color=\"#FFFFFF\">"
                                   "$2"
                                 "</font>"
                               "</a>";
        newContent = regex_replace(newContent,station,replace2);

        std::regex mode("(mode\\s*=\\s*)([^;\\s]*)");
        std::string replace3 = "$1"
                               "<a href=\"#$MODE_$2\">"
                                 "<font color=\"#FFFFFF\">"
                                   "$2"
                                 "</font>"
                               "</a>";
        newContent = regex_replace(newContent,mode,replace3);

    }else{
        return content;
    }
    return newContent;
}

std::string textfileViewer::highlight_vex(const std::string &prefix, const std::string &content)
{
    std::string newContent;
    if(prefix == "$SCHED"){
        std::regex scan("[\\s|>]scan[^;]*");
        newContent = regex_replace(content,scan,"<font color=\"#FFCC00\">$0</font>");
        std::regex endScan("[\\s|>]endscan");
        newContent = regex_replace(newContent,endScan,"<font color=\"#FFCC00\">$0</font>");

        std::regex link("&\\w*");
        newContent = regex_replace(newContent,link,"<font color=\"#FF0014\">$0</font>");

    }else{
        std::regex enddef("[\\s|>]enddef");
        newContent = regex_replace(content,enddef,"<font color=\"#00FFFF\">$0</font>");

        if(prefix == "$ANTENNA" || prefix == "$BBC" || prefix == "$IF" || prefix == "$TRACKS" || prefix == "$FREQ" || prefix == "$PHASE_CAL_DETECT"){
            std::regex link("&\\w*");
            newContent = regex_replace(newContent,link,"<font color=\"#FF0000\">$0</font>");
        }
    }

    std::regex comments("(\\*.*?)(<br>)");

    newContent = regex_replace(newContent,comments,"<font color=\"#007D00\">$1</font>$2");
    return newContent;
}

std::string textfileViewer::highlight_skd(const std::string &content)
{
    std::regex comments("(\\*.*?)(<br>)");
    return regex_replace(content,comments,"<font color=\"#007D00\">$1</font>$2");
}

std::string textfileViewer::highlight_log(const std::string &content)
{
    std::regex r("source=[^,]*");
    std::string newContent = regex_replace(content,r,"<font color=\"#FF00FF\">$0</font>");

    r = std::regex("scan_name=([^,]*)");
    auto words_begin = std::sregex_iterator(content.begin(), content.end(), r);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string prefix = match.str(1);
        ui->treeWidget_navigation->insertTopLevelItem(ui->treeWidget_navigation->topLevelItemCount(),new QTreeWidgetItem(QStringList() << QString::fromStdString(prefix)));
    }
    newContent = std::regex_replace(newContent,r,"<a id=\"$1\"/><font color=\"#FF00FF\">$0</font>");

    r = std::regex("preob");
    newContent = regex_replace(newContent,r,"<font color=\"#00FFFF\">$0</font>");
    r = std::regex("midob");
    newContent = regex_replace(newContent,r,"<font color=\"#00FFFF\">$0</font>");

    r = std::regex("postob");
    newContent = regex_replace(newContent,r,"<font color=\"#00FFFF\">$0</font>");
    r = std::regex("disk_record=[^<]*");
    newContent = regex_replace(newContent,r,"<font color=\"#0000FF\">$0</font>");
    r = std::regex("data_valid=[^<]*");
    newContent = regex_replace(newContent,r,"<font color=\"#0000FF\">$0</font>");

    r = std::regex("#antcn#Commanding to a new source/new offsets");
    newContent = regex_replace(newContent,r,"<font color=\"#00FF00\">$0</font>");
    r = std::regex("#flagr#flagr[^<]*");
    newContent = regex_replace(newContent,r,"<font color=\"#00FF00\">$0</font>");
    r = std::regex("/onsource/TRACKING");
    newContent = regex_replace(newContent,r,"<font color=\"#00FF00\">$0</font>");

    r = std::regex("ERROR[^<]*");
    newContent = regex_replace(newContent,r,"<font color=\"#FF0000\">$0</font>");
    r = std::regex("/onsource/SLEWING");
    newContent = regex_replace(newContent,r,"<font color=\"#FF0000\">$0</font>");

    r = std::regex("Az,.{6},El,.{5}");
    newContent = regex_replace(newContent,r,"<font color=\"#FFFF00\">$0</font>");

    r = std::regex("WARNING[^<]*");
    newContent = regex_replace(newContent,r,"<font color=\"#FF9900\">$0</font>");

    r = std::regex("!\\d{4}\\.\\d{3}.\\d{2}:\\d{2}:\\d{2}");
    newContent = regex_replace(newContent,r,"<font color=\"#CCFF33\">$0</font>");

    return newContent;
}

void textfileViewer::on_lineEdit_search_editingFinished()
{
    QString searchString = ui->lineEdit_search->text();
    QTextDocument *document = ui->textBrowser_view->document();

    if(document->isUndoAvailable()){
        document->undo();
    }

    QTextCursor cursor(document);
    QTextCursor highlightCursor(document);

    cursor.beginEditBlock();

    QTextCharFormat plainFormat(highlightCursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setBackground(Qt::yellow);

    int counter = 0;
    while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
        highlightCursor = document->find(searchString, highlightCursor);
        if (!highlightCursor.isNull()) {
            highlightCursor.mergeCharFormat(colorFormat);
            ++counter;
            if(counter == 1){
                ui->textBrowser_view->setTextCursor(highlightCursor);
            }
        }
    }
    cursor.endEditBlock();
    ui->label_found->setText(QString("%1 found").arg(counter));

}


void textfileViewer::on_treeWidget_navigation_clicked(const QModelIndex &index)
{
    QTextCursor cursor = ui->textBrowser_view->cursorForPosition(QPoint(0, 0));
    lastPosition_ = cursor.position();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser_view->setTextCursor(cursor);
    QString txt = ui->treeWidget_navigation->topLevelItem(index.row())->text(0);
    ui->textBrowser_view->scrollToAnchor(txt);
}

void textfileViewer::on_textBrowser_view_anchorClicked(const QUrl &arg1)
{
    QTextCursor cursor = ui->textBrowser_view->cursorForPosition(QPoint(0, 0));
    lastPosition_ = cursor.position();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser_view->setTextCursor(cursor);


}

void textfileViewer::on_pushButton_jumpBack_clicked()
{

    QTextCursor cursor = ui->textBrowser_view->cursorForPosition(QPoint(0, 0));
    int tmp = lastPosition_;
    lastPosition_ = cursor.position();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser_view->setTextCursor(cursor);
    cursor.setPosition(tmp);
    ui->textBrowser_view->setTextCursor(cursor);
}

void textfileViewer::on_actionSave_triggered()
{
    QFileInfo file (ui->label_fname->text());

    QFileDialog fileDialog(this, "Save as...");
    fileDialog.setDirectory(file.dir());
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    fileDialog.setDefaultSuffix(file.suffix());
    if (fileDialog.exec() != QDialog::Accepted){
        return;
    }

    const QString fileName = fileDialog.selectedFiles().first();

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(ui->textBrowser_view->document());
    if (success) {
        ui->textBrowser_view->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }

}

void textfileViewer::on_pushButton_writemode_toggled(bool checked)
{
    if(checked){
        ui->textBrowser_view->setReadOnly(false);
    }else{
        ui->textBrowser_view->setReadOnly(true);
    }
}

