#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QListWidget>
#include <QWhatsThis>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTableWidgetItem>

#include <QtCharts/QChart>
#include "chartview.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>

#include "skdcatalogreader.h"
#include "callout.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:    
    void on_actionMode_triggered();

    void on_actionWelcome_triggered();

    void on_actionGeneral_triggered();

    void on_actionStation_triggered();

    void on_actionSource_triggered();

    void on_actionBaseline_triggered();

    void on_actionRules_triggered();

    void on_actionMulti_Scheduling_triggered();

    void on_actionOutput_triggered();

    void on_actionSettings_triggered();

    void on_actionWeight_factors_triggered();

    void on_pushButton_browseAntenna_clicked();

    void on_pushButton_browseEquip_clicked();

    void on_pushButton_browsePosition_clicked();

    void on_pushButton_browseMask_clicked();

    void on_pushButton_browseSource_clicked();

    void on_pushButton_browseFlux_clicked();

    void on_pushButton_browsModes_clicked();

    void on_pushButton_browseFreq_clicked();

    void on_pushButton_browseTracks_clicked();

    void on_pushButton_browseLoif_clicked();

    void on_pushButton_browseRec_clicked();

    void on_pushButton_browseRx_clicked();

    void on_pushButton_browseHdpos_clicked();

    void on_listView_allSelectedStations_clicked(const QModelIndex &index);

    void on_groupBox_modeSked_toggled(bool arg1);

    void on_groupBox_modeCustom_toggled(bool arg1);

    void on_lineEdit_allStationsFilter_textChanged(const QString &arg1);

    void on_treeView_allAvailabeStations_clicked(const QModelIndex &index);

    void on_actionInput_triggered();

    void on_checkBox_fillinMode_clicked(bool checked);

    void on_doubleSpinBox_weightLowDecStart_valueChanged(double arg1);

    void on_doubleSpinBox_weightLowDecEnd_valueChanged(double arg1);

    void on_doubleSpinBox_weightLowElStart_valueChanged(double arg1);

    void on_doubleSpinBox_weightLowElEnd_valueChanged(double arg1);

    void on_spinBox_scanSequenceCadence_valueChanged(int arg1);

    void on_doubleSpinBox_calibratorLowElStart_valueChanged(double arg1);

    void on_doubleSpinBox_calibratorLowElEnd_valueChanged(double arg1);

    void on_doubleSpinBox_calibratorHighElStart_valueChanged(double arg1);

    void on_doubleSpinBox_calibratorHighElEnd_valueChanged(double arg1);

    void createMultiSchedTable();

    void createModesPolicyTable();

    void addModesPolicyTable(QString name);

    void createModesCustonBandTable();

    void addModesCustomTable(QString name, double freq, int nChannel);

    void deleteModesCustomLine(QString name);

    void on_actionWhat_is_this_triggered();

    void on_spinBox_fontSize_valueChanged(int arg1);

    void on_fontComboBox_font_currentFontChanged(const QFont &f);

    void on_pushButton_worldmapZoomFull_clicked();

    void on_treeView_allAvailabeStations_entered(const QModelIndex &index);

    void worldmap_hovered(QPointF point, bool state);

//    void worldmap_clicked();

    void on_pushButton_modeCustomAddBAnd_clicked();

    void on_listView_allSelectedStations_entered(const QModelIndex &index);

    void on_actionSky_Coverage_triggered();

    void multiSchedEditButton_clicked(QString name);

private:
    Ui::MainWindow *ui;
    QString mainPath;

    QStandardItemModel *allStationModel;
    QSortFilterProxyModel *allStationProxyModel;

    QStringListModel *selectedStationModel;

    QStandardItemModel *allSourceModel;

    QStringListModel *allSkedModesModel;

    VieVS::SkdCatalogReader skdCatalogReader;

    ChartView *worldmap;
    QChart *worldChart;
    QScatterSeries *availableStations;
    QScatterSeries *selectedStations;

//    QMap<QString, QScatterSeries *> scatterPlotStations;
    Callout *worldMapCallout;

    QSignalMapper *deleteModeMapper;
    QSignalMapper *multiSchedMapper;
    void readSkedCatalogs();

    void readStations();

    void readSources();

    void readAllSkedObsModes();

    void plotWorldMap();
};

#endif // MAINWINDOW_H
