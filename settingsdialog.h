/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#pragma once

#include <QDialog>

#include "typedefs.h"
#include "channelscountsfit.h"
#include "diagramparameters.h"

namespace Ui {
class SettingsDialog;
}

class QSettings;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = 0);
    virtual ~SettingsDialog();
    void setSettingsParameters( QSettings* set,
        Hist1Parameters *h1params, Hist2Parameters *h2params);

private slots:
    void channelIndexChanged(int);
    void runDirectoryClicked();
    void minChanged(double value);
    void maxChanged(double value);
    void binsChanged(int value);
    void applyChanges();
    void writeRunChecked(bool);
    void tensionParameterChanged(double);
    void signalCountsCellClicked( int, int);
    void signalCountsCellChanged( int, int);
    void chargeCountsCellChanged( int, int);
    void signalCountsRowSelected(int);
    void signalCountsDeleteRowClicked();
    void signalCountsAddRowBelowClicked();
    void signalCountsAddRowAboveClicked();
    void restoreReferenceMatrixClicked();
    void restoreReferenceChargeClicked();
    void showGraphClicked();

private:
    void addSignalCountsRow(int row);
    void recalculate_linear_fit();

    Ui::SettingsDialog* ui;
    Hist1Parameters* hist1;
    Hist2Parameters* hist2;
    QString rundir;
    QSettings* settings;
    int selected_row;
//    int lfit_ch;
//    int lfit_pos1;
//    int lfit_pos2;
    SharedFitParameters params;
};
