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
