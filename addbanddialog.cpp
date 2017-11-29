#include "addbanddialog.h"
#include "ui_addbanddialog.h"

addBandDialog::addBandDialog(boost::property_tree::ptree &settings_, QWidget *parent) :
    QDialog(parent), settings{settings_},
    ui(new Ui::addBandDialog)
{
    ui->setupUi(this);
}

addBandDialog::~addBandDialog()
{
    delete ui;
}

QString addBandDialog::getBandName()
{
    return ui->lineEdit_bandName->text();
}

double addBandDialog::getFrequency()
{
    return ui->doubleSpinBox_freq->value();
}

int addBandDialog::getChannels()
{
    return ui->spinBox_channels->value();
}

void addBandDialog::on_pushButton_Load_clicked()
{
    auto child = settings.get_child_optional( "settings.bands" );
    if(!child){
        return;
    }

    boost::property_tree::ptree parameters = settings.get_child("settings.bands");

    QVector<QString> names;
    QVector<QPair<double,int> > bands;

    for(const auto &it:parameters){
        std::string name = it.second.get<std::string>("<xmlattr>.name");
        names.push_back(QString::fromStdString(name));

        double freq = it.second.get<double>("frequency");
        int ch = it.second.get<int>("channels");
        bands.push_back({freq,ch});
    }
    settingsLoadWindow *dial = new settingsLoadWindow(this);
    dial->setBands(names,bands);
    int result = dial->exec();
    if(result == QDialog::Accepted){
        QString warningTxt;

        QString itm = dial->selectedItem();
        int idx = dial->selectedIdx();
        QPair<double,int> band = bands.at(idx);

        ui->doubleSpinBox_freq->setValue(band.first);
        ui->spinBox_channels->setValue(band.second);

        ui->lineEdit_bandName->setText(itm);

        if(!warningTxt.isEmpty()){
            QString txt = "The following errors occurred while loading the band:\n";
            txt.append(warningTxt).append("These members were ignored!\nPlease double check again!");
            QMessageBox::warning(this,"Unknown band members!",txt);
        }
    }
}

void addBandDialog::on_pushButton_Save_clicked()
{
    if(ui->lineEdit_bandName->text().isEmpty()){
        QMessageBox::warning(this,"No band name!","Please add band name first!");
    }else{
        if(QMessageBox::Yes == QMessageBox::question(this,"Save?","Are you sure you want to save this band?\nThis will save the band to settings.xml file for further use.")){

            boost::property_tree::ptree bt;
            bt.add("band.frequency",ui->doubleSpinBox_freq->value());
            bt.add("band.channels",ui->spinBox_channels->value());
            bt.add("band.<xmlattr>.name",ui->lineEdit_bandName->text().toStdString());

            settings.add_child("settings.bands.band",bt.get_child("band"));
            std::ofstream os;
            os.open("settings.xml");
            boost::property_tree::xml_parser::write_xml(os, settings,
                                                        boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
            os.close();
        }
    }
}

void addBandDialog::on_buttonBox_accepted()
{
    if(!ui->lineEdit_bandName->text().isEmpty()){
        this->accept();
    }else{
        QMessageBox::warning(this,"Missing band name!","Please add band name first!");
    }
}
