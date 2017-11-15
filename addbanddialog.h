#ifndef ADDBANDDIALOG_H
#define ADDBANDDIALOG_H

#include <QDialog>
#include "settingsloadwindow.h"
#include "boost/property_tree/ptree.hpp"
namespace Ui {
class addBandDialog;
}

class addBandDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addBandDialog(boost::property_tree::ptree &settings_, QWidget *parent = 0);
    ~addBandDialog();

    QString getBandName();

    double getFrequency();

    int getChannels();

private slots:
    void on_pushButton_Load_clicked();

    void on_pushButton_Save_clicked();

    void on_buttonBox_accepted();

private:
    boost::property_tree::ptree &settings;
    Ui::addBandDialog *ui;
};

#endif // ADDBANDDIALOG_H
