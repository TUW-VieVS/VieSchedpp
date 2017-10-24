#ifndef BASELINEPARAMETERSDIALOG_H
#define BASELINEPARAMETERSDIALOG_H

#include <QDialog>

namespace Ui {
class baselineParametersDialog;
}

class baselineParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit baselineParametersDialog(QWidget *parent = 0);
    ~baselineParametersDialog();

private:
    Ui::baselineParametersDialog *ui;
};

#endif // BASELINEPARAMETERSDIALOG_H
