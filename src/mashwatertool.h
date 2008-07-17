// mashwaterdialog.h
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

#ifndef MASHWATERTOOL_H
#define MASHWATERTOOL_H
#include <QDialog>
#include "ui_mashwatertool.h"
class MashWaterDialog : public QDialog, public Ui::MashWaterDialog
{
    Q_OBJECT
public:
    MashWaterDialog(QWidget *parent = 0);

private slots:
    void on_strikeWaterVolumeDoubleSpinBox_valueChanged();
    void on_grainMassDoubleSpinBox_valueChanged();
    void on_targetStrikeTempSpinBox_valueChanged();
    void on_grainTempSpinBox_valueChanged();
    void on_step2TargetTempSpinBox_valueChanged();
    void on_step2WaterTempSpinBox_valueChanged();
    void on_step3TargetTempSpinBox_valueChanged();
    void on_step3WaterTempSpinBox_valueChanged();
    void on_moreButton_toggled(bool checked);
    void initUnits();

private:
    QString calculateWaterTemp() const;
    QString calculateRatio() const;
    QString calculateStep2WaterVolume() const;
    QString calculateStep3WaterVolume() const;
    void preSetMultiRest();

    int units;
    double therm_constant;
    bool more_was_checked;
    enum { US_UNITS, METRIC_UNITS };
};
#endif
