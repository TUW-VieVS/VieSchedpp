#ifndef SOURCEPARAMETERSDIALOG_H
#define SOURCEPARAMETERSDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <ParameterSettings.h>

namespace Ui {
class sourceParametersDialog;
}

class sourceParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit sourceParametersDialog(QWidget *parent = 0);
    ~sourceParametersDialog();

    void addBandNames(QStringList bands);

    void addStationModel(QStandardItemModel *stations);

    void addBaselineModel(QStandardItemModel *baselines);

    std::pair<std::string, VieVS::ParameterSettings::ParametersSources> getParameters();

private slots:
    void on_lineEdit_ignoreStations_textChanged(const QString &arg1);

    void on_lineEdit_filterRequiredStations_textChanged(const QString &arg1);

    void on_lineEdit_filter_ignoreBaselines_textChanged(const QString &arg1);

    void on_listWidget_selectedIgnoreStations_clicked(const QModelIndex &index);

    void on_listWidget_selectedRequiredStations_clicked(const QModelIndex &index);

    void on_listWidget_selectedIgnoreBaselines_clicked(const QModelIndex &index);

    void on_listView_ignoreStations_clicked(const QModelIndex &index);

    void on_listView_requiredStations_clicked(const QModelIndex &index);

    void on_listView_ignoreBaselines_clicked(const QModelIndex &index);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::sourceParametersDialog *ui;

    QStandardItemModel *stations;
    QSortFilterProxyModel *stations_proxyIgnore;
    QSortFilterProxyModel *stations_proxyRequired;

    QStandardItemModel *baseline;
    QSortFilterProxyModel *baseline_proxy;

};

#endif // SOURCEPARAMETERSDIALOG_H
