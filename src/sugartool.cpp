/***************************************************************************
  sugartool.cpp
  -------------------
  A priming sugar calculator utility for QBrew
  -------------------
  Added by Matthew Caron<matt@mattcaron.net>, 2009. Copyright assigned to:
  Copyright 2004-2008, David Johnson <david@usermode.org>
  Based on code Copyright 2004, Michal Palczewski <michalp@gmail.com>
  Please see the header file for copyright and license information
 ***************************************************************************/

#include <QLabel>
#include <QLocale>

#include "recipe.h"
#include "resource.h"
#include "data.h"
#include "sugartool.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
// AbvcalcTool()
// ----------------
// Constructor

SugarTool::SugarTool(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    setWindowTitle(Resource::TITLE + tr(" - Sugar Tool"));

    QString TEMP_SUFFIX = QString(" %1%2")
        .arg(Resource::DEGREE)
        .arg(Data::instance()->defaultTempUnit().symbol());

    QString VOLUME_SUFFIX = QString(" %1")
    .arg(Data::instance()->defaultSize().unit().symbol());

    // additional setup
    QFont fnt(font());
    fnt.setBold(true);
    ui.sugarbottle->setFont(fnt);
    ui.sugarkeg->setFont(fnt);

    ui.temp->setSuffix(TEMP_SUFFIX);
    ui.volbeer->setSuffix(VOLUME_SUFFIX);

    // convert volume to L
    if(Data::instance()->defaultSize().unit() == Volume::liter){
        ui.volbeer->setMinimum(0.0);
        ui.volbeer->setMaximum(80.0);
        ui.volbeer->setValue(19.0);
    }

    // convert temp to celsius
    if(Data::instance()->defaultTempUnit() == Temperature::celsius){
        ui.temp->setMinimum(0.0);
        ui.temp->setMaximum(100.0);
        ui.temp->setValue(15.0);
    }

    // set up style box
    ui.stylecombo->clear();
    ui.stylecombo->addItems(Data::instance()->stylesList());

    // connections
    connect(ui.volbeer, SIGNAL(valueChanged(double)), this, SLOT(recalc()));
    connect(ui.temp, SIGNAL(valueChanged(double)), this, SLOT(recalc()));
    connect(ui.volco2, SIGNAL(valueChanged(double)), this, SLOT(recalc()));
    connect(ui.stylecombo, SIGNAL(activated(const QString &)),
            this, SLOT(updateCO2(const QString &)));

    updateCO2(ui.stylecombo->currentText());
    recalc();
}

//////////////////////////////////////////////////////////////////////////////
// updateCO2()
// ----------
// update the CO2 based on the beer style

void SugarTool::updateCO2(const QString &newStyle)
{
    Style style;
    QLocale locale = QLocale::system();

    if (Data::instance()->hasStyle(newStyle)){
        style = Data::instance()->style(newStyle);
    }
    else {
        style = Style();
    }

    ui.minco2->setText(locale.toString(style.CO2Low(),'f',1));
    ui.maxco2->setText(locale.toString(style.CO2Hi(),'f',1));
}


//////////////////////////////////////////////////////////////////////////////
// recalc()
// --------
// the signal to calculate the Sugar percentage

void SugarTool::recalc()
{
    double volbeer = ui.volbeer->value();
    double temp = ui.temp->value();
    double volco2 = ui.volco2->value();

    // published calc uses volume in gallons and temp in F, but
    // returns grams....

    if(Data::instance()->defaultTempUnit() == Temperature::celsius){
        temp = celsius2fahrenheit(temp);
    }

    if(Data::instance()->defaultSize().unit() == Volume::liter){
        volbeer = liter2gallon(volbeer);
    }

    double sugar = Recipe::calculateSugar(volbeer, temp, volco2);

    if(Data::instance()->defaultSugarUnit() == Weight::ounce){
        sugar = gram2ounce(sugar);
    }

    QString BOTTLE_STRING = QString("%1 %2")
    .arg(QString::number(sugar,'f',1))
    .arg(Data::instance()->defaultSugarUnit().symbol());

    // Kegging is slightly less than half the sugar of bottling
    // .75 cup for bottling, .33 cup for kegging, and .33 is 44% of
    // .75
    QString KEG_STRING = QString("%1 %2")
    .arg(QString::number(sugar*.44,'f',1))
    .arg(Data::instance()->defaultSugarUnit().symbol());

    ui.sugarbottle->setText(BOTTLE_STRING);
    ui.sugarkeg->setText(KEG_STRING);
}
