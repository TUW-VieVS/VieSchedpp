#ifndef SAVETOSETTINGSDIALOG_H
#define SAVETOSETTINGSDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QStyle>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace Ui {
class saveToSettingsDialog;
}

class saveToSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Type {
        stationNetwork,
        sourceNetwork,
        modes
    };

    explicit saveToSettingsDialog(boost::property_tree::ptree &settings, QWidget *parent = 0);

    void setType(Type type);

    void setNetwork(QVector<QString> newNetwork);

    void setMode(int bits, double srate, QVector<QString> myBands, QVector<double> myFreqs, QVector<int> myChanls);

    ~saveToSettingsDialog();

private slots:
    void on_buttonBox_accepted();

private:
    boost::property_tree::ptree &settings;
    Type type;

    QVector<QString> network;

    QVector<QString> bands;
    QVector<double> freqs;
    QVector<int> chanls;
    int bits;
    double srate;

    Ui::saveToSettingsDialog *ui;
};

#endif // SAVETOSETTINGSDIALOG_H
