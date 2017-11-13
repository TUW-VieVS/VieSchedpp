#ifndef STATIONPARAMETERSDIALOG_H
#define STATIONPARAMETERSDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <ParameterSettings.h>

namespace Ui {
class stationParametersDialog;
}

class stationParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit stationParametersDialog(boost::property_tree::ptree &settings_, QWidget *parent = 0);
    ~stationParametersDialog();

    void addBandNames(QStringList bands);

    void addSourceNames(QStandardItemModel *sources);

    std::pair<std::string, VieVS::ParameterSettings::ParametersStations> getParameters();

private slots:
    void on_listView_ignoreSources_clicked(const QModelIndex &index);

    void on_lineEdit_filter_textChanged(const QString &arg1);

    void on_listWidget_selectedIgnoreSources_clicked(const QModelIndex &index);

    void on_buttonBox_accepted();

    void on_pushButton_save_clicked();

    void on_pushButton_load_clicked();

private:
    Ui::stationParametersDialog *ui;

    boost::property_tree::ptree &settings;
    QStandardItemModel *sources;
    QSortFilterProxyModel *sources_proxy;
};

#endif // STATIONPARAMETERSDIALOG_H
