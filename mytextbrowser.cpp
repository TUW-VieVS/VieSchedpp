#include "mytextbrowser.h"

myTextBrowser::myTextBrowser(QWidget *parent) : QTextBrowser(parent)
{

}

void myTextBrowser::readyReadStandardOutput()
{
    QProcess *p = qobject_cast<QProcess*>(sender());
    QString txt (p->readAllStandardOutput());
    QStringList l = txt.split("\n");
    QString currentText = toPlainText();
//    if(currentText.size()>2){
//        currentText = currentText.left(currentText.size()-1);
//    }
    for(int i=0; i<l.size(); ++i){
        QString newTxt = l.at(i);
        newTxt = newTxt.trimmed();
        if(!newTxt.isEmpty()){
            if(newTxt.left(1) == "[" || newTxt.left(1) == "("){
                currentText.append(newTxt).append("\n");
            }else{
                currentText.remove(currentText.count()-1,1);
                currentText.append(newTxt).append("\n");
            }
        }
    }
    setText(currentText);

    highlightWord("[fatal]",QColor(Qt::red));
    highlightWord("[error]",QColor(Qt::red));
    highlightWord("[warning]",QColor(Qt::darkYellow));
    highlightWord("[info]",QColor(Qt::darkGreen));
    highlightWord("[debug]",QColor(Qt::darkCyan));
    highlightWord("[trace]",QColor(Qt::darkCyan));

    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}

void myTextBrowser::readyReadStandardError()
{
    QProcess *p = qobject_cast<QProcess*>(sender());
    QString txt (p->readAllStandardOutput());
    QStringList l = txt.split("\n");
    QString currentText = toPlainText();
//    if(currentText.size()>2){
//        currentText = currentText.left(currentText.size()-1);
//    }
    for(int i=0; i<l.size(); ++i){
        QString newTxt = l.at(i);
        newTxt = newTxt.trimmed();
        currentText.append(newTxt).append("\n");
    }
    setText(currentText);

    highlightWord("[fatal]",QColor(Qt::red));
    highlightWord("[error]",QColor(Qt::red));
    highlightWord("[warning]",QColor(Qt::darkYellow));
    highlightWord("[info]",QColor(Qt::darkGreen));
    highlightWord("[debug]",QColor(Qt::darkCyan));
    highlightWord("[trace]",QColor(Qt::darkCyan));

//    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}

void myTextBrowser::highlightWord(QString word, QColor color){
    QTextDocument *document = this->document();
    QTextCursor highlightCursor(document);
    QTextCursor cursor(document);
    cursor.beginEditBlock();

    QTextCharFormat plainFormat(highlightCursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setForeground(QBrush(color));

    while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
        highlightCursor = document->find(word, highlightCursor);

        if (!highlightCursor.isNull()) {
            highlightCursor.mergeCharFormat(colorFormat);
        }
    }
    cursor.endEditBlock();
}
