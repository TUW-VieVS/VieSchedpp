#include "vieschedpp_analyser.h"
#include "ui_vieschedpp_analyser.h"

VieSchedpp_Analyser::VieSchedpp_Analyser(VieVS::Scheduler schedule, QDateTime start, QDateTime end, QWidget *parent) :
    QMainWindow(parent), schedule_{schedule}, sessionStart_{start}, sessionEnd_{end},
    ui(new Ui::VieSchedpp_Analyser)
{
    ui->setupUi(this);

    ui->dateTimeEdit_start->setDateTimeRange(sessionStart_,sessionEnd_);
    ui->dateTimeEdit_end->setDateTimeRange(sessionStart_,sessionEnd_);
    ui->dateTimeEdit_start->setDateTime(sessionStart_);
    ui->dateTimeEdit_end->setDateTime(sessionEnd_);

    int duration = sessionStart_.secsTo(sessionEnd_);
    ui->spinBox_duration->setRange(0,duration);
    ui->spinBox_duration->setValue(duration);
    ui->horizontalSlider_start->setRange(0,duration);
    ui->horizontalSlider_end->setRange(0,duration);

    updateDuration();

    ui->splitter_skyCoverage->setSizes(QList<int>({INT_MAX, INT_MAX/5}));

    const std::vector<VieVS::Source> &source = schedule.getSources();
    for(const auto &any:source){
        QTreeWidgetItem *itm = new QTreeWidgetItem(QStringList() << QString::fromStdString(any.getName()));
        itm->setIcon(0,QIcon(":/icons/icons/source.png"));

        ui->treeWidget_skyCoverage_sources->addTopLevelItem(itm);
    }
    ui->treeWidget_skyCoverage_sources->sortItems(0,Qt::AscendingOrder);

    ui->label_fileName->setText(QString::fromStdString(schedule.getName()));


    setSkyCoverageLayout(1,2);
}

VieSchedpp_Analyser::~VieSchedpp_Analyser()
{
    delete ui;
}

void VieSchedpp_Analyser::on_horizontalSlider_start_valueChanged(int value)
{
    QDateTime newStart = sessionStart_.addSecs(value);
    ui->dateTimeEdit_start->setDateTime(newStart);

    if(value>ui->horizontalSlider_end->value()){
        ui->horizontalSlider_end->setValue(value);
    }
    if(ui->checkBox_fixDuration->isChecked()){
        ui->horizontalSlider_end->setValue(value+ui->spinBox_duration->value());
    }else{
        updateDuration();
    }
}

void VieSchedpp_Analyser::on_horizontalSlider_end_valueChanged(int value)
{
    QDateTime newEnd = sessionStart_.addSecs(value);
    ui->dateTimeEdit_end->setDateTime(newEnd);

    if(value<ui->horizontalSlider_start->value()){
        ui->horizontalSlider_start->setValue(value);
    }
    if(ui->checkBox_fixDuration->isChecked()){
        ui->horizontalSlider_start->setValue(value-ui->spinBox_duration->value());
    }else{
        updateDuration();
    }
}

void VieSchedpp_Analyser::on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime)
{
    int start = sessionStart_.secsTo(dateTime);
    ui->horizontalSlider_start->setValue(start);
}

void VieSchedpp_Analyser::on_dateTimeEdit_end_dateTimeChanged(const QDateTime &dateTime)
{
    int end = sessionStart_.secsTo(dateTime);
    ui->horizontalSlider_end->setValue(end);
}

void VieSchedpp_Analyser::on_spinBox_duration_valueChanged(int arg1)
{
    int newEnd = ui->horizontalSlider_start->value() + arg1;
    ui->horizontalSlider_end->setValue(newEnd);
}

void VieSchedpp_Analyser::updateDuration()
{
    int dur = ui->dateTimeEdit_start->dateTime().secsTo(ui->dateTimeEdit_end->dateTime());
    ui->spinBox_duration->setValue(dur);
    double dur_h = static_cast<double>(dur)/3600.;
    ui->doubleSpinBox_hours->setValue(dur_h);
}


void VieSchedpp_Analyser::on_doubleSpinBox_hours_valueChanged(double arg1)
{
    int dur = arg1*3600;
    ui->spinBox_duration->setValue(dur);
}

void VieSchedpp_Analyser::setSkyCoverageLayout(int rows, int columns)
{
    for(int i=0; i<rows; ++i){
        for(int j=0; j<columns; ++j){
            QVBoxLayout *layout = new QVBoxLayout(this);
            QComboBox *c = new QComboBox(this);
            layout->addWidget(c);
            ui->gridLayout_skyCoverage->addLayout(layout,i,j);
        }
    }
}

void VieSchedpp_Analyser::on_pushButton_skyCoverageLayout_clicked()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);
    form.addRow(new QLabel("The question ?"));
    // Add the lineEdits with their respective labels

    QSpinBox *rowBox = new QSpinBox(&dialog);
    rowBox->setMinimum(1);
    form.addRow("rows: ", rowBox);
    QSpinBox *colBox = new QSpinBox(&dialog);
    colBox->setMinimum(1);
    form.addRow("columns: ", colBox);


    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);

    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields

        setSkyCoverageLayout(rowBox->value(), colBox->value());
    }

}
