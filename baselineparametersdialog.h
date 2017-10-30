#ifndef BASELINEPARAMETERSDIALOG_H
#define BASELINEPARAMETERSDIALOG_H

#include <QDialog>
#include <ParameterSettings.h>
#include <QMessageBox>

namespace Ui {
class baselineParametersDialog;
}

class baselineParametersDialog : public QDialog
{
    Q_OBJECT


public:
    explicit baselineParametersDialog(QWidget *parent = 0);
    ~baselineParametersDialog();

    void addBandNames(QStringList bands);

    std::pair<std::string, VieVS::ParameterSettings::ParametersBaselines> getParameters();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::baselineParametersDialog *ui;
};

#endif // BASELINEPARAMETERSDIALOG_H
