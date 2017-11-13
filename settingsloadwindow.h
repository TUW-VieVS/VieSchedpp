#ifndef SETTINGSLOADWINDOW_H
#define SETTINGSLOADWINDOW_H

#include <QDialog>
#include "ParameterSettings.h"
#include <QListWidgetItem>
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

private slots:
    void refreshList(QListWidgetItem*);

private:
    Ui::settingsLoadWindow *ui;

    int type;

    QVector<VieVS::ParameterSettings::ParametersStations> paraSta;
    QVector<VieVS::ParameterSettings::ParametersSources> paraSrc;
    QVector<VieVS::ParameterSettings::ParametersBaselines> paraBl;

    QVector<QVector<QString> > membersSta;
    QVector<QVector<QString> > membersSrc;
    QVector<QVector<QString> > membersBl;

};

#endif // SETTINGSLOADWINDOW_H
