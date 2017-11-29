#ifndef MYTEXTBROWSER_H
#define MYTEXTBROWSER_H

#include <QObject>
#include <QTextBrowser>
#include <QProcess>
#include <QRegularExpression>

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
