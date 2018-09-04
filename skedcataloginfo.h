#ifndef SKEDCATALOGINFO_H
#define SKEDCATALOGINFO_H

#include <QDialog>
#include <QFontDatabase>

namespace Ui {
class SkedCatalogInfo;
}

class SkedCatalogInfo : public QDialog
{
    Q_OBJECT

public:
    explicit SkedCatalogInfo(QWidget *parent = 0);
    void setFonts();
    ~SkedCatalogInfo();

private:
    Ui::SkedCatalogInfo *ui;
};

#endif // SKEDCATALOGINFO_H
