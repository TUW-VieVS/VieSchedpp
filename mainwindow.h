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
#include <QTreeWidgetItem>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDateTimeEdit>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>

#include "chartview.h"
#include "multischededitdialogint.h"
#include "multischededitdialogdouble.h"
#include "multischededitdialogdatetime.h"
#include "skdcatalogreader.h"
#include "callout.h"
#include "ParameterSettings.h"
#include "addgroupdialog.h"
#include "baselineparametersdialog.h"
#include "stationparametersdialog.h"
#include "sourceparametersdialog.h"

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
    void closeEvent(QCloseEvent *event);

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

    void skymap_hovered(QPointF point, bool state);

    void on_pushButton_modeCustomAddBAnd_clicked();

    void on_listView_allSelectedStations_entered(const QModelIndex &index);

    void on_actionSky_Coverage_triggered();

    void multiSchedEditButton_clicked(QString name);

    void on_actionExit_triggered();

    void on_iconSizeSpinBox_valueChanged(int arg1);

    void on_treeWidget_2_itemChanged(QTreeWidgetItem *item, int column);

    void addGroupStation();

    void addGroupSource();

    void addGroupBaseline();

    void on_pushButton_stationParameter_clicked();

    void on_dateTimeEdit_sessionStart_dateTimeChanged(const QDateTime &dateTime);

    void on_doubleSpinBox_sessionDuration_valueChanged(double arg1);

    void on_DateTimeEdit_startParameterStation_dateTimeChanged(const QDateTime &dateTime);

    void on_DateTimeEdit_endParameterStation_dateTimeChanged(const QDateTime &dateTime);

    void on_pushButton_3_clicked();

    void addSetup(QTreeWidget *targetTreeWidget, QDateTimeEdit *paraStart, QDateTimeEdit *paraEnd,
                  QComboBox *transition, QComboBox *member, QComboBox *parameter, VieVS::ParameterSetup &paraSetup,
                  QChartView *setupChartView, QComboBox *targetStationPlot);

    void on_pushButton_4_clicked();

    void deleteSetupSelection(VieVS::ParameterSetup &setup, QChartView *setupChartView, QComboBox *setupCB, QTreeWidget *setupTW);

    void on_treeWidget_setupStation_itemEntered(QTreeWidgetItem *item, int column);

    void prepareSetupPlot(QChartView *figure, QVBoxLayout *container);

    void drawSetupPlot(QChartView *cv, QComboBox *cb, QTreeWidget *tw);

    void on_comboBox_stationSettingMember_currentTextChanged(const QString &arg1);

    void on_ComboBox_parameterStation_currentTextChanged(const QString &arg1);

    void displayStationSetupParameterFromPlot();

    void displaySourceSetupParameterFromPlot();

    void displayBaselineSetupParameterFromPlot();

    void on_comboBox_setupStation_currentTextChanged(const QString &arg1);

    void on_pushButton_sourceParameter_clicked();

    void on_comboBox_setupSource_currentTextChanged(const QString &arg1);

    void on_treeWidget_setupSource_itemEntered(QTreeWidgetItem *item, int column);

    void on_pushButton_removeSetupSource_clicked();

    void on_pushButton_addSetupSource_clicked();

    void on_DateTimeEdit_startParameterSource_dateTimeChanged(const QDateTime &dateTime);

    void on_DateTimeEdit_endParameterSource_dateTimeChanged(const QDateTime &dateTime);

    void createBaselineModel();

    void on_pushButton__baselineParameter_clicked();

    void on_DateTimeEdit_startParameterBaseline_dateTimeChanged(const QDateTime &dateTime);

    void on_DateTimeEdit_endParameterBaseline_dateTimeChanged(const QDateTime &dateTime);

    void on_pushButton_addSetupBaseline_clicked();

    void on_pushButton_removeSetupBaseline_clicked();

    void on_treeWidget_setupBaseline_itemEntered(QTreeWidgetItem *item, int column);

    void on_comboBox_setupBaseline_currentTextChanged(const QString &arg1);

    void setupStationWaitAddRow();

    void setupStationAxisBufferAddRow();

    void on_pushButton_14_clicked();

    void on_pushButton_16_clicked();

    void on_treeView_allAvailabeSources_clicked(const QModelIndex &index);

    void on_listView_allSelectedSources_clicked(const QModelIndex &index);

    void on_pushButton_13_clicked();

    void on_pushButton_15_clicked();

    void worldmap_clicked(QPointF point);

    void on_treeView_allAvailabeSources_entered(const QModelIndex &index);

    void on_listView_allSelectedSources_entered(const QModelIndex &index);

    void on_pushButton_skymapZoomFull_clicked();

private:
    Ui::MainWindow *ui;
    QString mainPath;

    VieVS::ParameterSettings para;

    QStandardItemModel *allStationModel;
    QStandardItemModel *allSourceModel;
    QSortFilterProxyModel *allStationProxyModel;
    QSortFilterProxyModel *allSourceProxyModel;

    QStandardItemModel *selectedStationModel;
    QStandardItemModel *selectedSourceModel;
    QStandardItemModel *selectedBaselineModel;

    QStandardItemModel *allSourcePlusGroupModel;
    QStandardItemModel *allStationPlusGroupModel;
    QStandardItemModel *allBaselinePlusGroupModel;

    QStringListModel *allSkedModesModel;

    VieVS::SkdCatalogReader skdCatalogReader;

    ChartView *worldmap;
    ChartView *skymap;
    QChartView *setupStation;
    QChartView *setupSource;
    QChartView *setupBaseline;

    QScatterSeries *availableStations;
    QScatterSeries *selectedStations;
    QScatterSeries *availableSources;
    QScatterSeries *selectedSources;

//    QMap<QString, QScatterSeries *> scatterPlotStations;
    Callout *worldMapCallout;
    Callout *skyMapCallout;

    QSignalMapper *deleteModeMapper;
    QSignalMapper *multiSchedMapper;

    VieVS::ParameterSetup setupStationTree;
    VieVS::ParameterSetup setupSourceTree;
    VieVS::ParameterSetup setupBaselineTree;

    void readSkedCatalogs();

    void readStations();

    void readSources();

    void readAllSkedObsModes();

    void plotWorldMap();

    void plotSkyMap();

    void defaultParameters();

    bool eventFilter(QObject *watched, QEvent *event);

    void displayStationSetupMember(QString name);

    void displaySourceSetupMember(QString name);

    void displayBaselineSetupMember(QString name);

    void displayStationSetupParameter(QString name);

    void displaySourceSetupParameter(QString name);

    void displayBaselineSetupParameter(QString name);

    int plotParameter(QChart* targetChart, QTreeWidgetItem *item, int level, int plot, QString target, const std::map<std::string, std::vector<std::string> > &map);

};

#endif // MAINWINDOW_H
