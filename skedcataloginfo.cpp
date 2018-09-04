#include "skedcataloginfo.h"
#include "ui_skedcataloginfo.h"

SkedCatalogInfo::SkedCatalogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SkedCatalogInfo)
{
    ui->setupUi(this);
}

void SkedCatalogInfo::setFonts()
{
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    QString family = fixedFont.family();

    ui->textEdit_2->setStyleSheet(QString("font: \"%1\";").arg(family));
    ui->textEdit_3->setFont(fixedFont);
    ui->textEdit_4->setFont(fixedFont);
}

SkedCatalogInfo::~SkedCatalogInfo()
{
    delete ui;
}
