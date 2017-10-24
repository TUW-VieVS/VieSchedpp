#include "baselineparametersdialog.h"
#include "ui_baselineparametersdialog.h"

baselineParametersDialog::baselineParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::baselineParametersDialog)
{
    ui->setupUi(this);
}

baselineParametersDialog::~baselineParametersDialog()
{
    delete ui;
}
