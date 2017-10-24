#ifndef SOURCEPARAMETERSDIALOG_H
#define SOURCEPARAMETERSDIALOG_H

#include <QDialog>

namespace Ui {
class sourceParametersDialog;
}

class sourceParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit sourceParametersDialog(QWidget *parent = 0);
    ~sourceParametersDialog();

private:
    Ui::sourceParametersDialog *ui;
};

#endif // SOURCEPARAMETERSDIALOG_H
