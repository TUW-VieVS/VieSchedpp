#include "textfileviewer.h"
#include "ui_textfileviewer.h"




textfileViewer::textfileViewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::textfileViewer)
{
    ui->setupUi(this);
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
        ui->radioButton_log->setChecked(true);
        ui->dockWidget_navigate->setVisible(false);
        break;
    case Type::skd:
        ui->radioButton_skd->setChecked(true);
        ui->dockWidget_navigate->setVisible(true);
        break;
    case Type::vex:
        ui->radioButton_vex->setChecked(true);
        ui->dockWidget_navigate->setVisible(true);
        break;
    default:
        ui->radioButton_log->setChecked(false);
        ui->radioButton_skd->setChecked(false);
        ui->radioButton_vex->setChecked(false);
        break;
    }

    ui->textBrowser_view->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QFile file(path);
    if(!file.exists()){
        return false;
    }
    QString line;
    ui->textBrowser_view->clear();
    QMessageBox info;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&file);
        int i=1;

        qDebug() << QTime::currentTime().toString() << "read file\n";
        std::ifstream ifs(path.toStdString(), std::ifstream::in);
        std::string content;
        std::string sline;
        while(std::getline(ifs,sline)){
            content.append(sline).append("\n");
        }

        QTextCursor cursor = ui->textBrowser_view->textCursor();
        cursor.setPosition(0);
        ui->textBrowser_view->setTextCursor(cursor);

        if(type == Type::skd || type == Type::vex){
            qDebug() << QTime::currentTime().toString() << "link navigation bar\n";
            addNavigation(content,std::regex("(^|\n)(\\$\\w*)"));
        }
        ui->textBrowser_view->setText(QString::fromStdString(std::move(content)));

        if(type == Type::vex){
            QString html = ui->textBrowser_view->toHtml();
            qDebug() << QTime::currentTime().toString() << "link blocks\n";
            linkBlocks();
        }

        qDebug() << QTime::currentTime().toString() << "highlight\n";
        highlight();
        qDebug() << QTime::currentTime().toString() << "finished\n";
        info.close();
        ui->textBrowser_view->setUndoRedoEnabled(true);
    }
    return true;
}

void textfileViewer::on_radioButton_skd_toggled(bool checked)
{
    if(checked){
        clearHighlight();
        addHighlight("^\\$\\w*",QColor(255,0,255));
        addHighlight("^\\*.*",QColor(0,125,0));
        addHighlight("\\d{11}",QColor(0,255,255));
    }
}

void textfileViewer::on_radioButton_vex_toggled(bool checked)
{
    if(checked){
        clearHighlight();

        addHighlight("^\\$\\w*",QColor(255,0,255));
        addHighlight("\\sdef[^;]*",QColor(0,255,255));
        addHighlight("\\senddef",QColor(0,255,255));
        addHighlight("\\sref\\s+[^\\s]*",QColor(57,255,20));
        addHighlight("\\sscan[^;]*",QColor(255,204,0));
        addHighlight("\\sendscan",QColor(255,204,0));
        addHighlight("&\\w*",QColor(255,0,20));
        addHighlight("^\\*.*",QColor(0,125,0));
    }

}

void textfileViewer::on_radioButton_log_toggled(bool checked)
{
    if(checked){
        clearHighlight();
        addHighlight("#antcn#Commanding to a new source/new offsets",QColor(0,255,0));
        addHighlight("#flagr#flagr/antenna,new-source",QColor(0,255,0));
        addHighlight("#flagr#flagr/antenna,acquired",QColor(0,255,0));


        addHighlight("disk_record(.*)",QColor(0,0,255));
        addHighlight("data_valid=(.*)",QColor(0,0,255));

        addHighlight("preob",QColor(255,0,255));
        addHighlight("midob",QColor(255,0,255));
        addHighlight("postob",QColor(255,0,255));

        addHighlight("scan_name=(.*)",QColor(0,255,255));
        addHighlight("source=(.*)",QColor(0,255,255));

        addHighlight("ERROR(.*)",QColor(255,0,0));
        addHighlight("/onsource/SLEWING",QColor(255,0,0));

        addHighlight("/onsource/TRACKING",QColor(255,255,0));
        addHighlight("Az,.{6},El,.{5}",QColor(255,255,0));

        addHighlight("WARNING(.*)",QColor(255,153,0));

        addHighlight("!\\d{4}\\.\\d{3}.\\d{2}:\\d{2}:\\d{2}",QColor(204,255,051));
    }
}

void textfileViewer::clearHighlight()
{
    ui->treeWidget_highlights->clear();
}

void textfileViewer::addHighlight(QString txt, QColor color)
{
    QTreeWidget* tree = ui->treeWidget_highlights;

    QTreeWidgetItem *c = new QTreeWidgetItem();
    c->setText(0,txt);
    c->setText(1,"");
    c->setBackgroundColor(1,color);
    tree->addTopLevelItem(c);
}

void textfileViewer::highlight()
{
    QTreeWidget* tree = ui->treeWidget_highlights;
    QTextDocument *document = ui->textBrowser_view->document();

    for(int i=0; i<tree->topLevelItemCount(); ++i){
        QString searchString = tree->topLevelItem(i)->text(0);

        QTextCursor highlightCursor(document);
        QTextCursor cursor(document);

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setForeground(tree->topLevelItem(i)->backgroundColor(1));

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document->find(QRegularExpression(searchString), highlightCursor);

            if (!highlightCursor.isNull()) {
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }
    }
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

void textfileViewer::addNavigation(const std::string &content, const std::regex &regexStr)
{
    QTreeWidget* tree = ui->treeWidget_navigation;

    auto words_begin = std::sregex_iterator(content.begin(), content.end(), regexStr);

    for (std::sregex_iterator i = words_begin; i != std::sregex_iterator(); ++i) {
        std::smatch match = *i;
        std::string str = match.str(2);

        int pos = match.position();
        tree->insertTopLevelItem(tree->topLevelItemCount(),new QTreeWidgetItem(QStringList() << QString::fromStdString(str)));
        navigationPosition.push_back(pos);
    }
}

void textfileViewer::linkBlocks()
{
    QTextDocument *document = ui->textBrowser_view->document();



    QTextCursor cursor(document);
    QRegularExpression regex("(\\sdef\\s+)([^;]*)");
    QRegularExpression regextmp("(def\\s+)([^;]*)");

    while (!cursor.isNull() && !cursor.atEnd()) {
        if (!cursor.isNull()) {
            cursor = document->find(regex, cursor);

            if (!cursor.isNull()) {
                int n = cursor.selectedText().length();
                cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,1);
                cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,1);
                cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,n-1);
                QString txt = cursor.selectedText();
                QString def = regextmp.match(txt).captured(2);
                QString pre = regextmp.match(txt).captured(1);

                int pos = cursor.position();
                QString keyword = linkKeyword(def,pos);

                cursor.removeSelectedText();

                QString newText = "<a id=\""+keyword+"\"></a>"+def+"";
                cursor.insertHtml(regextmp.match(txt).captured(1)+newText);

                linkMap[keyword] = pos;
            }
        }
    }

    QTextCursor cursor1(document);
    QRegularExpression regex1("ref\\s+(\\$[^\\s]*)\\s*=\\s*([^;\\s:]*)(\\s*[:\\s*(\\w*)\\s*]*)");

    while (!cursor1.isNull() && !cursor1.atEnd()) {
        if (!cursor1.isNull()) {
            cursor1 = document->find(regex1, cursor1);

            if (!cursor1.isNull()) {
                QString txt = cursor1.selectedText();
                QString block = regex1.match(txt).captured(1);
                QString name = regex1.match(txt).captured(2);

                QString link = block.right(block.size()-1).append(name);
                QString txtLeft;
                QString newText = "<a href=\"#"+link+"\" style=\"color: rgb(255,255,255)\">"+name+"</a>";
                if(regex1.captureCount() == 3){
                    QString stations = regex1.match(txt).captured(3);
                    QRegularExpression re("(\\s*:\\s*)(\\w*)");
                    QRegularExpressionMatchIterator i = re.globalMatch(stations);
                    while (i.hasNext()) {
                        QRegularExpressionMatch match = i.next();
                        QString between = match.captured(1);
                        QString id = "STATION";
                        id.append(match.captured(2));
                        QString sta = "<a href=\"#"+id+"\" style=\"color: rgb(255,255,255)\">"+match.captured(2)+"</a>";
                        newText.append(between).append(sta);
                    }

                    txtLeft = txt.left(txt.size()-name.size()-stations.size());
                }else{
                    txtLeft = txt.left(txt.size()-name.size());
                }
                cursor1.removeSelectedText();
                cursor1.insertHtml(txtLeft+newText);
            }
        }
    }


    QTextCursor cursor2(document);
    QRegularExpression regex2("source\\s*=\\s*([^;\\s]*)");
    while (!cursor2.isNull() && !cursor2.atEnd()) {
        if (!cursor2.isNull()) {
            cursor2 = document->find(regex2, cursor2);

            if (!cursor2.isNull()) {
                QString txt = cursor2.selectedText();
                QString name = regex2.match(txt).captured(1);

                QString txtLeft = txt.left(txt.size()-name.size());

                QString link = QString("SOURCE").append(name);
                QString newText = "<a href=\"#"+link+"\" style=\"color: rgb(255,255,255)\">"+name+"</a>";

                cursor2.removeSelectedText();
                cursor2.insertHtml(txtLeft+newText);
            }
        }
    }

    QTextCursor cursor3(document);
    QRegularExpression regex3("station\\s*=\\s*([^;\\s]*)");
    while (!cursor3.isNull() && !cursor3.atEnd()) {
        if (!cursor3.isNull()) {
            cursor3 = document->find(regex3, cursor3);

            if (!cursor3.isNull()) {
                QString txt = cursor3.selectedText();
                QString name = regex3.match(txt).captured(1);

                QString txtLeft = txt.left(txt.size()-name.size());

                QString link = QString("STATION").append(name);
                QString newText = "<a href=\"#"+link+"\" style=\"color: rgb(255,255,255)\">"+name+"</a>";

                cursor3.removeSelectedText();
                cursor3.insertHtml(txtLeft+newText);
            }
        }
    }

    QTextCursor cursor4(document);
    QRegularExpression regex4("mode\\s*=\\s*([^;\\s]*)");
    while (!cursor4.isNull() && !cursor4.atEnd()) {
        if (!cursor4.isNull()) {
            cursor4 = document->find(regex4, cursor4);

            if (!cursor4.isNull()) {
                QString txt = cursor4.selectedText();
                QString name = regex4.match(txt).captured(1);

                QString txtLeft = txt.left(txt.size()-name.size());

                QString link = QString("MODE").append(name);
                QString newText = "<a href=\"#"+link+"\" style=\"color: rgb(255,255,255)\">"+name+"</a>";

                cursor4.removeSelectedText();
                cursor4.insertHtml(txtLeft+newText);
            }
        }
    }

}

QString textfileViewer::linkKeyword(QString word, int pos)
{
    QString prefix;
    for(int i=1; i<navigationPosition.size(); ++i){
        if(navigationPosition[i]>pos){
            prefix = ui->treeWidget_navigation->topLevelItem(i-1)->text(0);
            break;
        }
        if(i==navigationPosition.size()-1){
            prefix = ui->treeWidget_navigation->topLevelItem(i)->text(0);
        }
    }
    QString keyword = prefix.right(prefix.size()-1).append(word);
    return keyword;
}

void textfileViewer::addedLink(int pos)
{
    for(int i=0; i<navigationPosition.size(); ++i){
        if(navigationPosition[i]>pos){
            navigationPosition[i]+=15;
        }
    }
}


void textfileViewer::on_treeWidget_navigation_clicked(const QModelIndex &index)
{
    int pos = navigationPosition[index.row()];

    QTextDocument *document = ui->textBrowser_view->document();
    QTextCursor cursor(document);


    cursor.movePosition(QTextCursor::End);
    ui->textBrowser_view->setTextCursor(cursor);
    cursor.setPosition(pos);
    ui->textBrowser_view->setTextCursor(cursor);

}

void textfileViewer::on_textBrowser_view_anchorClicked(const QUrl &arg1)
{
//    QString link = arg1.url(QUrl::StripTrailingSlash);
//    if(linkMap.find(link) != linkMap.end()){
//        int pos = linkMap[link];

//        QTextDocument *document = ui->textBrowser_view->document();
//        QTextCursor cursor(document);

//        cursor.movePosition(QTextCursor::End);
//        ui->textBrowser_view->setTextCursor(cursor);
//        cursor.setPosition(pos);
//        ui->textBrowser_view->setTextCursor(cursor);
//    }
}
