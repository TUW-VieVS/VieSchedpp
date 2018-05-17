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
#include <QDesktopServices>
#include <QDockWidget>
//#include <QTextBrowser>
#include <mytextbrowser.h>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QPolarChart>
#include <QtCharts/QAreaSeries>

#include "chartview.h"
#include "multischededitdialogint.h"
#include "multischededitdialogdouble.h"
#include "multischededitdialogdatetime.h"
#include "callout.h"
#include "addgroupdialog.h"
#include "baselineparametersdialog.h"
#include "stationparametersdialog.h"
#include "sourceparametersdialog.h"
#include "addbanddialog.h"
#include "savetosettingsdialog.h"
#include "VLBI_Scheduler/VieVS_Scheduler.h"
#include "textfileviewer.h"

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

    QString writeXML();

    void readSettings();

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

    void on_treeView_allAvailabeStations_entered(const QModelIndex &index);

    void worldmap_hovered(QPointF point, bool state);

    void skymap_hovered(QPointF point, bool state);

    void on_pushButton_modeCustomAddBAnd_clicked();

    void on_listView_allSelectedStations_entered(const QModelIndex &index);

    void on_actionSky_Coverage_triggered();

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

    void setBackgroundColorOfChildrenWhite(QTreeWidgetItem *item);

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

    void on_pushButton_18_clicked();

    void on_pushButton_19_clicked();

    void on_actionNew_triggered();

    void on_comboBox_sourceSettingMember_currentTextChanged(const QString &arg1);

    void on_comboBox_baselineSettingMember_currentTextChanged(const QString &arg1);

    void on_ComboBox_parameterSource_currentTextChanged(const QString &arg1);

    void on_ComboBox_parameterBaseline_currentTextChanged(const QString &arg1);

    void on_pushButton_multiSchedAddSelected_clicked();

    void on_pushButton_25_clicked();

    QString on_actionSave_triggered();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_17_clicked();

    void on_pushButton_saveCatalogPathes_clicked();

    void on_pushButton_26_clicked();

    void changeDefaultSettings(QStringList path, QStringList value, QString name);

    void on_pushButton_23_clicked();

    void on_pushButton_22_clicked();

    void createDefaultParameterSettings();

    void on_pushButton_saveNetwork_clicked();

    void on_pushButton_loadNetwork_clicked();

    void on_pushButton_saveSourceList_clicked();

    void on_pushButton_loadSourceList_clicked();

    void on_pushButton_saveMode_clicked();

    void on_pushButton_loadMode_clicked();

    void clearGroup(bool sta, bool src, bool bl, QString name);

    void clearSetup(bool sta, bool src, bool bl);

    void splitterMoved();

    void on_pushButton_faqSearch_clicked();

    void on_actionFAQ_triggered();

    void on_actionRun_triggered();

    void networkSizeChanged();

    void sourceListChanged();

    void baselineListChanged();

    void on_comboBox_nThreads_currentTextChanged(const QString &arg1);

    void on_comboBox_jobSchedule_currentTextChanged(const QString &arg1);

    void on_actionsummary_triggered();

    void setupStatisticView();

    void on_pushButton_addStatistic_clicked();

    void on_pushButton_removeStatistic_clicked();

    void on_treeWidget_statisticGeneral_itemChanged(QTreeWidgetItem *item, int column);

    void on_treeWidget_statisticStation_itemChanged(QTreeWidgetItem *item, int column);

    void on_horizontalScrollBar_statistics_valueChanged(int value);

    void on_spinBox_statistics_show_valueChanged(int arg1);

    void plotStatistics(bool animation = false);

    void statisticsHovered(bool status, int index, QBarSet* barset);

    void setupSkyCoverageTemplatePlot();

    void skyCoverageTemplate();

    void on_pushButton_skyCoverageTemplateRandom_clicked();

    void on_influenceTimeSpinBox_valueChanged(int arg1);


    void on_actionConditions_triggered();

    void on_pushButton_addCondition_clicked();

    void on_pushButton_removeCondition_clicked();

    void on_lineEdit_allStationsFilter_3_textChanged(const QString &arg1);

    void on_actionNetwork_triggered();

    void on_actionSource_List_triggered();

    void on_dateTimeEdit_sessionStart_dateChanged(const QDate &date);

    void on_spinBox_doy_valueChanged(int arg1);

    void on_pushButton_clicked();

    void markerWorldmap();

    void markerSkymap();

    void on_radioButton_imageSkymap_toggled(bool checked);

    void on_radioButton_imageWorldmap_toggled(bool checked);

    void on_checkBox_showEcliptic_clicked(bool checked);

    void baselineHovered(QPointF point ,bool flag);

    void on_treeView_allSelectedBaselines_entered(const QModelIndex &index);

    void on_checkBox_showBaselines_clicked(bool checked);

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void initializeInspector();

    void on_pushButton_parameterStation_edit_clicked();

    void on_pushButton_parameterSource_edit_clicked();

    void on_pushButton_parameterBaseline_edit_clicked();

    void on_spinBox_maxNumberOfIterations_valueChanged(int arg1);

    void on_pushButton_31_clicked();

    void on_pushButton_29_clicked();

    void on_pushButton_28_clicked();

    void on_pushButton_30_clicked();

    void on_experimentNameLineEdit_textChanged(const QString &arg1);

    void on_pushButton_41_clicked();

    void on_pushButton_40_clicked();

    void on_pushButton_addHighImpactAzEl_clicked();

    void on_pushButton_removeHighImpactAzEl_clicked();

    void on_actionFix_High_Impact_Scans_triggered();

    void on_pushButton_readLogFile_read_clicked();

    void on_pushButton_readSkdFile_read_clicked();

    void on_actionLog_parser_triggered();

    void on_actionSkd_Parser_triggered();

private:
    Ui::MainWindow *ui;
    QString mainPath;
    boost::property_tree::ptree settings;

    VieVS::ParameterSettings para;

    QStandardItemModel *allStationModel;
    QStandardItemModel *allSourceModel;
    QSortFilterProxyModel *allStationProxyModel;
    QSortFilterProxyModel *allSourceProxyModel;

    QStandardItemModel *selectedStationModel;
    QStandardItemModel *selectedSourceModel;
    QStandardItemModel *selectedBaselineModel;
    bool createBaselines;

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
    QChartView *statisticsView;
    QChartView *skyCoverageTemplateView;
    bool plotSkyCoverageTemplate;

    QScatterSeries *availableStations;
    QScatterSeries *selectedStations;
    QScatterSeries *availableSources;
    QScatterSeries *selectedSources;

    Callout *worldMapCallout;
    Callout *skyMapCallout;
    Callout *statisticsCallout;

    QVector<double> obsAz;
    QVector<double> obsEl;
    QVector<int> obsTime;

    QSignalMapper *deleteModeMapper;

    bool setupChanged;
    VieVS::ParameterSetup setupStationTree;
    VieVS::ParameterSetup setupSourceTree;
    VieVS::ParameterSetup setupBaselineTree;

    std::map<std::string, std::vector<std::string>> groupSta;
    std::map<std::string, std::vector<std::string>> groupSrc;
    std::map<std::string, std::vector<std::string>> groupBl;

    std::map<std::string, VieVS::ParameterSettings::ParametersStations> paraSta;
    std::map<std::string, VieVS::ParameterSettings::ParametersSources> paraSrc;
    std::map<std::string, VieVS::ParameterSettings::ParametersBaselines> paraBl;

    QStringList statisticsName;
    QMap<QString, QMap<int, QVector< int >>> statistics;

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

    void addEmptyStatistic(int idx);

    double interpolate( QVector<double> &xData, QVector<double> &yData, double x, bool extrapolate=false );

    QBarSet* statisticsBarSet(int idx, QString name="");

};

#endif // MAINWINDOW_H
