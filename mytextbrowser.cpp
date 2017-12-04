#include "mytextbrowser.h"

myTextBrowser::myTextBrowser(QWidget *parent) : QTextBrowser(parent)
{

}

void myTextBrowser::readyReadStandardOutput()
{
    QProcess *p = qobject_cast<QProcess*>(sender());
    QString txt (p->readAllStandardOutput());
    txt = txt.replace(QRegularExpression("[\\s\\n]+"), " ");
    QStringList l = txt.split(";");
    QString currentText = toPlainText();
    if(currentText.size()>2){
        currentText = currentText.left(currentText.size()-1);
    }
    for(int i=0; i<l.size(); ++i){
        QString newTxt = l.at(i);
        newTxt = newTxt.trimmed();
        currentText.append(newTxt).append("\n");
    }
    setText(currentText);

    highlightWord("Processing file:",QColor(Qt::darkGreen));
    highlightWord("WARNING:",QColor(Qt::darkYellow));
    highlightWord("ERROR:",QColor(Qt::red));
    highlightWord("writing",QColor(Qt::darkCyan));
    highlightWord("version",QColor(Qt::darkMagenta));
    highlightWord("thread",QColor(Qt::red));
    highlightWord("threads",QColor(Qt::red));
    highlightWord("log file",QColor(Qt::darkCyan));
    highlightWord("everything finally finished!!!",QColor(Qt::darkGreen));
    highlightWord("finished",QColor(Qt::darkGreen));
}

void myTextBrowser::readyReadStandardError()
{
    QProcess *p = qobject_cast<QProcess*>(sender());
    QString txt (p->readAllStandardError());
    txt = txt.replace(QRegularExpression("[\\s\\n]+"), " ");
    QStringList l = txt.split(";");
    QString currentText = toPlainText();
    if(currentText.size()>2){
        currentText = currentText.left(currentText.size()-1);
    }
    for(int i=0; i<l.size(); ++i){
        QString newTxt = l.at(i);
        newTxt = newTxt.trimmed();
        currentText.append(newTxt).append("\n");
    }
    setText(currentText);

    highlightWord("Processing file:",QColor(Qt::darkGreen));
    highlightWord("WARNING:",QColor(Qt::darkYellow));
    highlightWord("ERROR:",QColor(Qt::red));
    highlightWord("writing",QColor(Qt::darkCyan));
    highlightWord("version",QColor(Qt::darkMagenta));
    highlightWord("thread",QColor(Qt::red));
    highlightWord("threads",QColor(Qt::red));
    highlightWord("log file",QColor(Qt::darkCyan));
    highlightWord("everything finally finished!!!",QColor(Qt::darkGreen));
    highlightWord("finished",QColor(Qt::darkGreen));
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
