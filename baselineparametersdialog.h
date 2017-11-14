#ifndef BASELINEPARAMETERSDIALOG_H
#define BASELINEPARAMETERSDIALOG_H

#include <QDialog>
#include <ParameterSettings.h>
#include <QMessageBox>
#include <settingsloadwindow.h>
namespace Ui {
class baselineParametersDialog;
}

class baselineParametersDialog : public QDialog
{
    Q_OBJECT


public:
    explicit baselineParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent = 0);
    ~baselineParametersDialog();

    void addBandNames(QStringList bands);

    void addDefaultParameters(VieVS::ParameterSettings::ParametersBaselines defaultPara);

    std::pair<std::string, VieVS::ParameterSettings::ParametersBaselines> getParameters();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_pushButton_load_clicked();

    void on_pushButton_save_clicked();

private:
    Ui::baselineParametersDialog *ui;
    VieVS::ParameterSettings::ParametersBaselines dp;
    boost::property_tree::ptree &settings;
};

#endif // BASELINEPARAMETERSDIALOG_H
