// mashwaterdialog.cpp
// a single infusion or multi-step mash calculator for QBrew
//
// Copyright (c) 2008, Timothy Howe <timh at dirtymonday dot net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
///////////////////////////////////////////////////////////////////////////

#include <QtGui>

#include "resource.h"
#include "data.h"
#include "mashwatertool.h"

MashWaterDialog::MashWaterDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    secondStepGroupBox->hide();
    thirdStepGroupBox->hide();
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    more_was_checked = false;

    setWindowTitle(Resource::TITLE + tr(" - Mash Water Tool"));

    if(Data::instance()->defaultTempUnit() == Temperature::fahrenheit)
    {
        units = US_UNITS;
        therm_constant = 0.2;
    }
    else if(Data::instance()->defaultTempUnit() == Temperature::celsius)
    {
        units = METRIC_UNITS;
        therm_constant = 0.41;
    }

    initUnits();

    strikeWaterTempLabel->setText(calculateWaterTemp());
    tempUnitsLabel->setText(Resource::DEGREE +
        Data::instance()->defaultTempUnit().symbol());
}

void MashWaterDialog::on_strikeWaterVolumeDoubleSpinBox_valueChanged()
{
    ratioLabel->setText(calculateRatio());
    strikeWaterTempLabel->setText(calculateWaterTemp());

    if(moreButton->isChecked())
    {
        step2WaterVolumeLabel->setText(calculateStep2WaterVolume());
        step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
    }
}

void MashWaterDialog::on_grainMassDoubleSpinBox_valueChanged()
{
    ratioLabel->setText(calculateRatio());
    strikeWaterTempLabel->setText(calculateWaterTemp());

    if(moreButton->isChecked())
    {
        step2WaterVolumeLabel->setText(calculateStep2WaterVolume());
        step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
    }
}

void MashWaterDialog::on_targetStrikeTempSpinBox_valueChanged()
{
    strikeWaterTempLabel->setText(calculateWaterTemp());

    if(moreButton->isChecked())
    {
        step2WaterVolumeLabel->setText(calculateStep2WaterVolume());
        step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
    }
}

void MashWaterDialog::on_grainTempSpinBox_valueChanged()
{
    strikeWaterTempLabel->setText(calculateWaterTemp());
}

void MashWaterDialog::on_step2TargetTempSpinBox_valueChanged()
{
    step2WaterVolumeLabel->setText(calculateStep2WaterVolume());
    step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
}

void MashWaterDialog::on_step2WaterTempSpinBox_valueChanged()
{
    step2WaterVolumeLabel->setText(calculateStep2WaterVolume());
    step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
}

void MashWaterDialog::on_step3TargetTempSpinBox_valueChanged()
{
    step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
}

void MashWaterDialog::on_step3WaterTempSpinBox_valueChanged()
{
    step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
}

// The ratio is an interesting bit of info to know,
// especially for single infusion mash.
QString MashWaterDialog::calculateRatio() const
{
    double ratio = strikeWaterVolumeDoubleSpinBox->value() /
                   grainMassDoubleSpinBox->value();
    QString ratioString;
    QTextStream ratioStream(&ratioString);
    ratioStream.setRealNumberPrecision(2); // 2 decimal points
    ratioStream.setRealNumberNotation(QTextStream::FixedNotation);
    ratioStream << ratio;
    return ratioString;
}

// This calculation is taken from the Initial Infusion Equation
// on page 170 of John J. Palmer's "How to Brew".
// Strike Water Temperature
// Tw = (0.2/R)(T2 - T1) + T2
// Tw = The Actual Temperature in F
// T1 = The Initial Temperature in F
// T2 = The Target Temperature in F
// R = Ratio of Water to grain
QString MashWaterDialog::calculateWaterTemp() const
{
    double ratio         = strikeWaterVolumeDoubleSpinBox->value() /
                           grainMassDoubleSpinBox->value();
    double targetTemp    = targetStrikeTempSpinBox->value();
    double initialTemp   = grainTempSpinBox->value();
    double temp;

    temp = ((therm_constant / ratio) * (targetTemp - initialTemp)) + targetTemp;

    QString tempString;
    QTextStream tempStream(&tempString);
    tempStream.setRealNumberPrecision(2); // 2 decimal points
    tempStream.setRealNumberNotation(QTextStream::FixedNotation);
    tempStream << temp; // << QChar(0x00B0);
    //tempStream << "F";
    return tempString;
}

// also from p 170 of Palmer's How To Brew
// second infusion
// Wa = (T2 - T1)(0.2G + Wm) / (Tw - T2)
// Wa = The amount of boiling water to add
// Wm = total amount of water in the mash
// G  = amount of grain in the mash
QString MashWaterDialog::calculateStep2WaterVolume() const
{
    double targetTemp    = step2TargetTempSpinBox->value();
    double initialTemp   = targetStrikeTempSpinBox->value();
    double grainMass     = grainMassDoubleSpinBox->value();
    double waterVolume   = strikeWaterVolumeDoubleSpinBox->value();
    double infWaterTemp  = step2WaterTempSpinBox->value();
    double volume;

    volume = (targetTemp - initialTemp) *
             ((therm_constant * grainMass) + waterVolume) /
             (infWaterTemp - targetTemp);

    if(volume < 0)
        volume = 0;

    QString volumeString;
    QTextStream volumeStream(&volumeString);
    volumeStream.setRealNumberPrecision(2); // 2 decimal points
    volumeStream.setRealNumberNotation(QTextStream::FixedNotation);
    volumeStream << volume;
    return volumeString;
}

// This uses the same math as above, but we get
// amount of water in the mash by adding the last
// two additions together.
QString MashWaterDialog::calculateStep3WaterVolume() const
{
    double targetTemp    = step3TargetTempSpinBox->value();
    double initialTemp   = step2TargetTempSpinBox->value();
    double grainMass     = grainMassDoubleSpinBox->value();
    double waterVolume   = strikeWaterVolumeDoubleSpinBox->value() +
                           step2WaterVolumeLabel->text().toDouble();
    double infWaterTemp  = step3WaterTempSpinBox->value();
    double volume;

    volume = (targetTemp - initialTemp) *
             ((therm_constant * grainMass) + waterVolume) /
             (infWaterTemp - targetTemp);

    if(volume < 0)
        volume = 0;

    QString volumeString;
    QTextStream volumeStream(&volumeString);
    volumeStream.setRealNumberPrecision(2); // 2 decimal points
    volumeStream.setRealNumberNotation(QTextStream::FixedNotation);
    volumeStream << volume;
    return volumeString;
}

// If moreButton is checked, we need to calculate
// the second and third mash steps.
// If it is the first time it is checked, it will
// automatically preset the mash for George Fix's
// 104 to 140 to 158 multi-rest mash schedule.
void MashWaterDialog::on_moreButton_toggled(bool checked){
  if(checked)
  {
    if(!more_was_checked)
    {
        preSetMultiRest();
        ratioLabel->setText(calculateRatio());
        strikeWaterTempLabel->setText(calculateWaterTemp());
        more_was_checked = true;
    }
    step2WaterVolumeLabel->setText(calculateStep2WaterVolume());
    step3WaterVolumeLabel->setText(calculateStep3WaterVolume());
  }

  // set label as appropriate
  moreButton->setText(checked ? tr("&Less <<") : tr("&More >>"));
}

// Different presets for different unit bases.
void MashWaterDialog::initUnits()
{
    if(units == US_UNITS)
    {
        strikeWaterVolumeLabel->setText(tr("Quarts of &water"));
        grainMassLabel->setText(tr("Pounds of &grain"));
        tempUnitsLabel->setText(QString(QChar(0x00B0)) + "F");
        step2WaterVolumeUnitsLabel->setText(tr("quarts"));
        step3WaterVolumeUnitsLabel->setText(tr("quarts"));

        targetStrikeTempSpinBox->setMinimum(80);
        targetStrikeTempSpinBox->setMaximum(180);
        targetStrikeTempSlider->setMinimum(80);
        targetStrikeTempSlider->setMaximum(180);
        targetStrikeTempSpinBox->setValue(153);

        grainTempSpinBox->setMinimum(32);
        grainTempSpinBox->setMaximum(100);
        grainTempSlider->setMinimum(32);
        grainTempSlider->setMaximum(100);
        grainTempSpinBox->setValue(70);

        step2TargetTempSpinBox->setMinimum(100);
        step2TargetTempSpinBox->setMaximum(180);
        step2TargetTempSlider->setMinimum(100);
        step2TargetTempSlider->setMaximum(180);
        step2TargetTempSpinBox->setValue(140);

        step2WaterTempSpinBox->setMinimum(140);
        step2WaterTempSpinBox->setMaximum(212);
        step2WaterTempSlider->setMinimum(140);
        step2WaterTempSlider->setMaximum(212);
        step2WaterTempSpinBox->setValue(205);

        step3TargetTempSpinBox->setMinimum(120);
        step3TargetTempSpinBox->setMaximum(200);
        step3TargetTempSlider->setMinimum(120);
        step3TargetTempSlider->setMaximum(200);
        step3TargetTempSpinBox->setValue(158);

        step3WaterTempSpinBox->setMinimum(140);
        step3WaterTempSpinBox->setMaximum(212);
        step3WaterTempSlider->setMinimum(140);
        step3WaterTempSlider->setMaximum(212);
        step3WaterTempSpinBox->setValue(205);

        strikeWaterVolumeDoubleSpinBox->setValue(20.00);
        grainMassDoubleSpinBox->setValue(10.00);
    }
    else if(units == METRIC_UNITS)
    {
        strikeWaterVolumeLabel->setText(tr("Liters of &water"));
        grainMassLabel->setText(tr("Kilograms of &grain"));
        tempUnitsLabel->setText(QString(QChar(0x00B0)) + "C");
        step2WaterVolumeUnitsLabel->setText(tr("liters"));
        step3WaterVolumeUnitsLabel->setText(tr("liters"));

        targetStrikeTempSpinBox->setMinimum(27);
        targetStrikeTempSpinBox->setMaximum(82);
        targetStrikeTempSlider->setMinimum(27);
        targetStrikeTempSlider->setMaximum(82);
        targetStrikeTempSpinBox->setValue(67);

        grainTempSpinBox->setMinimum(0);
        grainTempSpinBox->setMaximum(38);
        grainTempSlider->setMinimum(0);
        grainTempSlider->setMaximum(38);
        grainTempSpinBox->setValue(21);

        step2TargetTempSpinBox->setMinimum(38);
        step2TargetTempSpinBox->setMaximum(82);
        step2TargetTempSlider->setMinimum(38);
        step2TargetTempSlider->setMaximum(82);
        step2TargetTempSpinBox->setValue(60);

        step2WaterTempSpinBox->setMinimum(60);
        step2WaterTempSpinBox->setMaximum(100);
        step2WaterTempSlider->setMinimum(60);
        step2WaterTempSlider->setMaximum(100);
        step2WaterTempSpinBox->setValue(96);

        step3TargetTempSpinBox->setMinimum(49);
        step3TargetTempSpinBox->setMaximum(93);
        step3TargetTempSlider->setMinimum(120);
        step3TargetTempSlider->setMaximum(49);
        step3TargetTempSpinBox->setValue(70);

        step3WaterTempSpinBox->setMinimum(60);
        step3WaterTempSpinBox->setMaximum(100);
        step3WaterTempSlider->setMinimum(60);
        step3WaterTempSlider->setMaximum(100);
        step3WaterTempSpinBox->setValue(96);

        strikeWaterVolumeDoubleSpinBox->setValue(19.00);
        grainMassDoubleSpinBox->setValue(4.50);
    }
}

// Set some values for a 104-140-158 mash schedule.
void MashWaterDialog::preSetMultiRest()
{
    if(units == US_UNITS)
    {
        strikeWaterVolumeDoubleSpinBox->setValue(
                grainMassDoubleSpinBox->value());
        targetStrikeTempSpinBox->setValue(104);
    }
    else if(units == METRIC_UNITS)
    {
        strikeWaterVolumeDoubleSpinBox->setValue(
                grainMassDoubleSpinBox->value() * 2.0);
        targetStrikeTempSpinBox->setValue(40);
    }
}
