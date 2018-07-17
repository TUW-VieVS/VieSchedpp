#ifndef VIESCHEDPP_ANALYSER_H
#define VIESCHEDPP_ANALYSER_H

#include <QMainWindow>
#include <QDateTime>
#include <QComboBox>

#include "VieSchedpp/Scheduler.h"

namespace Ui {
class VieSchedpp_Analyser;
}

class VieSchedpp_Analyser : public QMainWindow
{
    Q_OBJECT

public:
    explicit VieSchedpp_Analyser(VieVS::Scheduler schedule, QDateTime start, QDateTime end, QWidget *parent = 0);
    ~VieSchedpp_Analyser();

private slots:
    void on_horizontalSlider_start_valueChanged(int value);

    void on_horizontalSlider_end_valueChanged(int value);

    void on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime);

    void on_dateTimeEdit_end_dateTimeChanged(const QDateTime &dateTime);

    void on_spinBox_duration_valueChanged(int arg1);

    void updateDuration();

    void on_doubleSpinBox_hours_valueChanged(double arg1);

    void setSkyCoverageLayout(int rows, int columns);

    void on_pushButton_skyCoverageLayout_clicked();

private:
    Ui::VieSchedpp_Analyser *ui;

    VieVS::Scheduler schedule_;
    QDateTime sessionStart_;
    QDateTime sessionEnd_;
};

#endif // VIESCHEDPP_ANALYSER_H
