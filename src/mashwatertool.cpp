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
    mashoutGroupBox->hide();
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

    strikeWaterTempLabel->setText(
        calculateWaterTemp()
        + Resource::DEGREE
        + Data::instance()->defaultTempUnit().symbol());
}

void MashWaterDialog::calculateDerivedValues()
{
    if(moreButton->isChecked())
    {
        step2WaterVolumeLabel->setText(calculateStep2WaterVolume()
                                       + (getUnitString()));
        step3WaterVolumeLabel->setText(calculateStep3WaterVolume()
                                       + (getUnitString()));
    }

    if(mashoutButton->isChecked())
    {
        mashoutWaterVolumeLabel->setText(calculateMashoutWaterVolume()
                                         + (getUnitString()));
    }

    totalWaterVolumeLabel->setText(calculateTotalWaterVolume()
                                   + (getUnitString()));
    estimatedWaterVolumeOutputLabel->
    setText(calculateEstimatedWaterOutputVolume()
            + (getUnitString()));
    totalVolumeLabel->setText(calculateTotalVolume() + (getUnitString()));
}

QString MashWaterDialog::getUnitString() const
{
    return QString((units==METRIC_UNITS) ? " liters" : " quarts");
}

void MashWaterDialog::on_strikeWaterVolumeDoubleSpinBox_valueChanged()
{
    ratioLabel->setText(calculateRatio());
    strikeWaterTempLabel->setText(
        calculateWaterTemp()
        + Resource::DEGREE
        + Data::instance()->defaultTempUnit().symbol());

    calculateDerivedValues();
}

void MashWaterDialog::on_grainMassDoubleSpinBox_valueChanged()
{
    ratioLabel->setText(calculateRatio());
    strikeWaterTempLabel->setText(
        calculateWaterTemp()
        + Resource::DEGREE
        + Data::instance()->defaultTempUnit().symbol());

    calculateDerivedValues();
}

void MashWaterDialog::on_targetStrikeTempSpinBox_valueChanged()
{
    strikeWaterTempLabel->setText(
        calculateWaterTemp()
        + Resource::DEGREE
        + Data::instance()->defaultTempUnit().symbol());

    calculateDerivedValues();
}

void MashWaterDialog::on_grainTempSpinBox_valueChanged()
{
    strikeWaterTempLabel->setText(
        calculateWaterTemp()
        + Resource::DEGREE
        + Data::instance()->defaultTempUnit().symbol());
}

void MashWaterDialog::on_step2TargetTempSpinBox_valueChanged()
{
    calculateDerivedValues();
}

void MashWaterDialog::on_step2WaterTempSpinBox_valueChanged()
{
    calculateDerivedValues();
}

void MashWaterDialog::on_step3TargetTempSpinBox_valueChanged()
{
    calculateDerivedValues();
}

void MashWaterDialog::on_step3WaterTempSpinBox_valueChanged()
{
    calculateDerivedValues();
}

void MashWaterDialog::on_mashoutTargetTempSpinBox_valueChanged()
{
    calculateDerivedValues();
}

void MashWaterDialog::on_mashoutWaterTempSpinBox_valueChanged()
{
    calculateDerivedValues();
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

    return calculateWaterVolume(targetTemp, initialTemp,
                                grainMass, waterVolume, infWaterTemp);
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
                           calculateStep2WaterVolume().toDouble();
    double infWaterTemp  = step3WaterTempSpinBox->value();

    return calculateWaterVolume(targetTemp, initialTemp,
                                grainMass, waterVolume, infWaterTemp);
}

// This uses the same math as above, but we get
// amount of water in the mash by adding the last
// three additions together.
QString MashWaterDialog::calculateMashoutWaterVolume() const
{
    double targetTemp    = mashoutTargetTempSpinBox->value();
    double initialTemp   = step3TargetTempSpinBox->value();
    double grainMass     = grainMassDoubleSpinBox->value();
    double waterVolume   = strikeWaterVolumeDoubleSpinBox->value() +
                           step2WaterVolumeLabel->text().replace(
                               getUnitString(),"").toDouble() +
                           step3WaterVolumeLabel->text().replace(
                               getUnitString(),"").toDouble();
    double infWaterTemp  = mashoutWaterTempSpinBox->value();

    return calculateWaterVolume(targetTemp, initialTemp,
                                grainMass, waterVolume, infWaterTemp);
}

QString MashWaterDialog::calculateWaterVolume(double targetTemp,
                                              double initialTemp,
                                              double grainMass,
                                              double waterVolume,
                                              double infWaterTemp) const
{
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

QString MashWaterDialog::calculateTotalWaterVolume() const
{
    double volume = strikeWaterVolumeDoubleSpinBox->value();

    if(mashoutButton->isChecked())
    {
        volume += 
        mashoutWaterVolumeLabel->text().replace(getUnitString(),"").toDouble();
    }

    if(moreButton->isChecked())
    {
        volume +=
        step2WaterVolumeLabel->text().replace(getUnitString(),"").toDouble() +
        step3WaterVolumeLabel->text().replace(getUnitString(),"").toDouble();
    }

    QString totalVolumeString;
    QTextStream totalVolumeStream(&totalVolumeString);
    totalVolumeStream.setRealNumberPrecision(2); // 2 decimal points
    totalVolumeStream.setRealNumberNotation(QTextStream::FixedNotation);
    totalVolumeStream << volume;
    return totalVolumeString;
}

QString MashWaterDialog::calculateEstimatedWaterOutputVolume() const
{
    // You lose 1/2 quart per pound (about)

    // this is somewhat redundant...
    double volume = calculateTotalWaterVolume().toDouble();
    double grain = grainMassDoubleSpinBox->value();

    volume -= grain * .5;

    QString estimatedVolumeString;
    QTextStream estimatedVolumeStream(&estimatedVolumeString);
    estimatedVolumeStream.setRealNumberPrecision(2); // 2 decimal points
    estimatedVolumeStream.setRealNumberNotation(QTextStream::FixedNotation);
    estimatedVolumeStream << volume;
    return estimatedVolumeString;
}

QString MashWaterDialog::calculateTotalVolume() const
{
    // 1 pound of grain + 1 quart of water = 1.3125 quarts of volume,
    // up until you get a number of quarts equal to the pounds of
    // grain - after that, it just adds.

    double volume = calculateTotalWaterVolume().toDouble();
    double grain = grainMassDoubleSpinBox->value();

    volume -= grain;
    volume += grain * 1.3125;

    QString totalVolumeString;
    QTextStream totalVolumeStream(&totalVolumeString);
    totalVolumeStream.setRealNumberPrecision(2); // 2 decimal points
    totalVolumeStream.setRealNumberNotation(QTextStream::FixedNotation);
    totalVolumeStream << volume;
    return totalVolumeString;
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
        strikeWaterTempLabel->setText(calculateWaterTemp()
                                  + Resource::DEGREE
                                  + Data::instance()->defaultTempUnit().symbol());
        more_was_checked = true;
    }
  }

  calculateDerivedValues();

  // set label as appropriate
  moreButton->setText(checked ? tr("&Less <<") : tr("&More >>"));
}

void MashWaterDialog::on_mashoutButton_toggled(bool checked){
    if(checked)
    {
        mashoutGroupBox->show();
    }
    else
    {
        mashoutGroupBox->hide();
    }

    calculateDerivedValues();
    
    mashoutButton->setText(checked ? tr("Mashout <<") : tr("Mashout >>"));
}

// Different presets for different unit bases.
void MashWaterDialog::initUnits()
{
    if(units == US_UNITS)
    {
        strikeWaterVolumeLabel->setText(tr("Quarts of &water"));
        grainMassLabel->setText(tr("Pounds of &grain"));

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
