#ifndef SETTINGSLOADWINDOW_H
#define SETTINGSLOADWINDOW_H

#include <QDialog>
#include "ParameterSettings.h"
#include <QListWidgetItem>
#include <QMessageBox>
namespace Ui {
class settingsLoadWindow;
}

class settingsLoadWindow : public QDialog
{
    Q_OBJECT

public:
    explicit settingsLoadWindow(QWidget *parent = 0);
    ~settingsLoadWindow();

    void setStationParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersStations> &paras);
    void setSourceParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersSources> &paras);
    void setBaselineParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersBaselines> &paras);
    void setStationGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members);
    void setSourceGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members);
    void setBaselineGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members);

    QString selectedItem();

private slots:
    void refreshList(QListWidgetItem*);

    void on_buttonBox_accepted();

private:
    Ui::settingsLoadWindow *ui;

    int type;

    QVector<VieVS::ParameterSettings::ParametersStations> paraSta;
    QVector<VieVS::ParameterSettings::ParametersSources> paraSrc;
    QVector<VieVS::ParameterSettings::ParametersBaselines> paraBl;

    QVector<QVector<QString> > groupSta;
    QVector<QVector<QString> > groupSrc;
    QVector<QVector<QString> > groupBl;

};

#endif // SETTINGSLOADWINDOW_H
