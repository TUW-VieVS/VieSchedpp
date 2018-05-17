#ifndef TEXTFILEVIEWER_H
#define TEXTFILEVIEWER_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <iostream>
#include <fstream>

#include <regex>

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
    void on_radioButton_skd_toggled(bool checked);

    void on_radioButton_vex_toggled(bool checked);

    void on_radioButton_log_toggled(bool checked);

    void on_actionview_triggered();

    void on_actionhighlight_triggered();

    void on_lineEdit_search_editingFinished();

    void addNavigation(const std::string &content, const std::regex &regexStr);

    void linkBlocks();

    void on_treeWidget_navigation_clicked(const QModelIndex &index);

    void on_textBrowser_view_anchorClicked(const QUrl &arg1);

private:
    Ui::textfileViewer *ui;

    QVector<int> navigationPosition;
    QMap<QString, int> linkMap;

    void clearHighlight();

    void addHighlight(QString txt, QColor color);

    void highlight();

    QString linkKeyword(QString word, int pos);

    void addedLink(int pos);
};

#endif // TEXTFILEVIEWER_H
