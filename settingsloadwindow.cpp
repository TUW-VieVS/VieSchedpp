#include "settingsloadwindow.h"
#include "ui_settingsloadwindow.h"

settingsLoadWindow::settingsLoadWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsLoadWindow)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(1,2);
    ui->name->setMouseTracking(true);
    connect(ui->name,SIGNAL(itemEntered(QListWidgetItem*)),this,SLOT(refreshList(QListWidgetItem*)));
}

settingsLoadWindow::~settingsLoadWindow()
{
    delete ui;
}

void settingsLoadWindow::setStationParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersStations> &paras)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = 0;
    paraSta = paras;
}

void settingsLoadWindow::setSourceParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersSources> &paras)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = 1;
    paraSrc = paras;
}

void settingsLoadWindow::setBaselineParameters(const QVector<QString> &name, const QVector<VieVS::ParameterSettings::ParametersBaselines> &paras)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = 2;
    paraBl = paras;
}

void settingsLoadWindow::setStationGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = 3;
    membersSta = members;
}

void settingsLoadWindow::setSourceGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = 4;
    membersSrc = members;
}

void settingsLoadWindow::setBaselineGroups(const QVector<QString> &name, const QVector<QVector<QString> > &members)
{
    for(const auto&any:name){
        ui->name->addItem(any);
    }
    type = 5;
    membersBl = members;
}

void settingsLoadWindow::refreshList(QListWidgetItem *itm)
{
    QString name = itm->text();
    int idx = itm->listWidget()->row(itm);


}

