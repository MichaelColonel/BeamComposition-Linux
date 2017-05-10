/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "signalvaluedialog.h"
#include "ui_signalvaluedialog.h"

SignalValueDialog::SignalValueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignalValueDialog),
    signal( 0.0, 0.0),
    flag(false)
{
    ui->setupUi(this);
}

SignalValueDialog::~SignalValueDialog()
{
    delete ui;
}

SignalPair
SignalValueDialog::getMeanSigmaValues() const
{
    SignalPair p;
    if (flag) {
        p.first = ui->meanSpinBox->value();
        p.second = ui->sigmaSpinBox->value();
    }
    else {
        p = signal;
    }
    return p;
}

void
SignalValueDialog::setMeanSigmaValues(const SignalPair &values)
{
    signal = values;
    ui->meanSpinBox->setValue(signal.first);
    ui->sigmaSpinBox->setValue(signal.second);
}

void
SignalValueDialog::accept()
{
    flag = true;
    return QDialog::accept();
}

void
SignalValueDialog::reject()
{
    flag = false;
    return QDialog::reject();
}
