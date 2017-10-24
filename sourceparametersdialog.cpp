#include "sourceparametersdialog.h"
#include "ui_sourceparametersdialog.h"

sourceParametersDialog::sourceParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sourceParametersDialog)
{
    ui->setupUi(this);
}

sourceParametersDialog::~sourceParametersDialog()
{
    delete ui;
}
