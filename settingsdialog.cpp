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

#include <TF1.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <TGaxis.h>
#include <TLegend.h>

#include <QHeaderView>
#include <QDebug>
#include <QResizeEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "rootcanvasdialog.h"
#include "channelscountsfit.h"
#include "channelschargefit.h"

#include "signalvaluedelegate.h"
#include "chargevaluedelegate.h"

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

namespace {

#define HISTOGRAMS 6
struct Hist1Parameters histograms[HISTOGRAMS] = {
    { HIST_CHANNEL1, nullptr, nullptr, 200, 0., 400. },
    { HIST_CHANNEL2, nullptr, nullptr, 200, 0., 400. },
    { HIST_CHANNEL3, nullptr, nullptr, 200, 0., 400. },
    { HIST_CHANNEL4, nullptr, nullptr, 200, 0., 400. },
    { HIST_FITALL, nullptr, nullptr, 200, 0., 400. },
    { HIST_Z, nullptr, nullptr, 200, 0.5, 7.5 },
};

} // namespace

double Hist1Parameters::energy_per_count = 2.0;

SettingsDialog::SettingsDialog(QWidget* parent)
    :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    hist1(nullptr),
    hist2(nullptr),
    settings(nullptr),
    selected_row(-1)
  {
    ui->setupUi(this);

    QHeaderView* hview = new QHeaderView( Qt::Orientation::Vertical, ui->signalCountsTableWidget);
    hview->setClickable(true);
    ui->signalCountsTableWidget->setVerticalHeader(hview);

    // Reference signal to Channel Counts Values
    SignalValueDelegate* reference_signal = new SignalValueDelegate(ui->signalCountsTableWidget);
    ui->signalCountsTableWidget->setItemDelegate(reference_signal);

    params = FitParameters::instance();
    ReferenceSignalMap& ref_signals = params->reference_signals();
    size_t elems = ref_signals.size();

    ui->signalCountsTableWidget->setRowCount(elems);
    int pos = 0;
    for ( const auto& elem : ref_signals) {

        int column = 0;

        int value = elem.first;
        QTableWidgetItem* item1 = new QTableWidgetItem(tr("%1").arg(value));
        ui->signalCountsTableWidget->setItem( pos, column++, item1);

        for ( const SignalPair& sp : elem.second) {
            QString str = SignalValueDelegate::form_text(sp);
            QTableWidgetItem* item = new QTableWidgetItem(str);
            ui->signalCountsTableWidget->setItem( pos, column++, item);
        }
        pos++;
    }
/*
    for ( size_t i = 0; i < elems; ++i) {

        QTableWidgetItem* item1 = new QTableWidgetItem(tr("%1").arg(i));
        ui->signalCountsTableWidget->setItem( i, 0, item1);

        for ( int j = 1; j <= CHANNELS; ++j) {
            SignalPair p( 0.0, 0.0);
            QString str = SignalValueDelegate::form_text(p);
            QTableWidgetItem* item = new QTableWidgetItem(str);
            ui->signalCountsTableWidget->setItem( i, j, item);
        }
    }
*/
    ui->signalCountsTableWidget->addAction(ui->actionAddReferenceRowAbove);
    ui->signalCountsTableWidget->addAction(ui->actionAddReferenceRowBelow);
    QAction* act = new QAction(this);
    act->setSeparator(true);
    ui->signalCountsTableWidget->addAction(act);
    ui->signalCountsTableWidget->addAction(ui->actionDeleteReferenceRow);
    ui->signalCountsTableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    // Ion charge to Channel Counts Values
    ChargeValueDelegate* charge_signal = new ChargeValueDelegate(ui->chargeCountsTableWidget);
    ui->chargeCountsTableWidget->setItemDelegate(charge_signal);

    ChargeSignalMap& ref_charge = params->charge_signals();
/*
    const SignalPair& p = ref_charge[1];
    QString str = SignalValueDelegate::form_text(p);
    QTableWidgetItem* item = new QTableWidgetItem(str);
    ui->chargeCountsTableWidget->setItem( 0, 1, item);
*/
/*
    const SignalPair& p = ref_charge[1];
    QString str = SignalValueDelegate::form_text(p);
    QTableWidgetItem* item = new QTableWidgetItem(str);
    ui->chargeCountsTableWidget->setItem( 0, 1, item);

    const SignalPair& p1 = ref_charge[CARBON_Z];
    str = SignalValueDelegate::form_text(p1);
    item = new QTableWidgetItem(str);
    ui->chargeCountsTableWidget->setItem( CARBON_Z - 1, 1, item);
*/

    QHeaderView* hview1 = new QHeaderView( Qt::Orientation::Vertical, ui->chargeCountsTableWidget);
    hview1->setClickable(true);
    ui->chargeCountsTableWidget->setVerticalHeader(hview1);

    for ( int i = 0; i < CARBON_Z; ++i) {
        const SignalPair& p = ref_charge[i + 1];
        QString str = SignalValueDelegate::form_text(p);
        QTableWidgetItem* item = new QTableWidgetItem(str);
        ui->chargeCountsTableWidget->setItem( i, 1, item);
    }

    // Ion charge radius parameter
    const double* radius = params->charge_radius_parameter();
    for ( int i = 0; i < CARBON_Z; ++i) {
        QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(radius[i]));
        ui->chargeCountsTableWidget->setItem( i, 2, item);
    }

    // Ion charge beta parameter
    const double* beta = params->charge_beta_parameter();
    for ( int i = 0; i < CARBON_Z; ++i) {
        QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(beta[i]));
        ui->chargeCountsTableWidget->setItem( i, 3, item);
    }

    // fit charge
/*
    const bool* fit_charge = CalibrationFitting::BeamCompositionFit::charge_in_fit();
    for ( int i = 0; i < CARBON_Z; ++i) {
//        QString text = (fit_charge[i]) ? tr("true") : tr("false");
        QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg(int(fit_charge[i])));
        ui->chargeCountsTableWidget->setItem( i, 4, item);
    }
*/
    connect( ui->tensionSpinBox, SIGNAL(valueChanged(double)), this, SLOT(tensionParameterChanged(double)));

    connect( ui->actionAddReferenceRowBelow, SIGNAL(triggered()), SLOT(signalCountsAddRowBelowClicked()));
    connect( ui->actionAddReferenceRowAbove, SIGNAL(triggered()), SLOT(signalCountsAddRowAboveClicked()));
    connect( ui->actionDeleteReferenceRow, SIGNAL(triggered()), SLOT(signalCountsDeleteRowClicked()));

    connect( ui->runDirectoryPushButton, SIGNAL(clicked()), this, SLOT(runDirectoryClicked()));
    connect( ui->writeRunFileCheckBox, SIGNAL(clicked(bool)), this, SLOT(writeRunChecked(bool)));
    connect( ui->restoreReferenceMatrixButton, SIGNAL(clicked()), this, SLOT(restoreReferenceMatrixClicked()));
    connect( ui->restoreReferenceChargeButton, SIGNAL(clicked()), this, SLOT(restoreReferenceChargeClicked()));
    connect( ui->showGraphMatrixButton, SIGNAL(clicked()), this, SLOT(showGraphClicked()));

    connect( this, SIGNAL(accepted()), this, SLOT(applyChanges()));
    connect( ui->channelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(channelIndexChanged(int)));
    connect( ui->maxSpinBox, SIGNAL(valueChanged(double)), this, SLOT(maxChanged(double)));
    connect( ui->minSpinBox, SIGNAL(valueChanged(double)), this, SLOT(minChanged(double)));
    connect( ui->binSpinBox, SIGNAL(valueChanged(int)), this, SLOT(binsChanged(int)));
    connect( ui->signalCountsTableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(signalCountsCellClicked( int, int)));
    connect( ui->signalCountsTableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(signalCountsCellChanged( int, int)));
    connect( ui->chargeCountsTableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(chargeCountsCellChanged( int, int)));

    connect( hview, SIGNAL(sectionClicked(int)), this, SLOT(signalCountsRowSelected(int)));
    connect( hview1, SIGNAL(sectionClicked(int)), this, SLOT(chargeSignalRowSelected(int)));

    ui->tensionSpinBox->setValue(params->tension());
//    ui->energyCountSpinBox->setValue(Hist1Parameters::energy_per_count);
//    const int* lfit = params->reference_channel_parameters(lfit_ch);
//    lfit_pos1 = lfit[0];
//    lfit_pos2 = lfit[1];
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void
SettingsDialog::runDirectoryClicked()
{
/*
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOption(QFileDialog::ShowDirsOnly);
    dialog->setDirectory(ui->RunDirectoryLineEdit->text());

    int res = dialog->exec();
    if (res == QDialog::Accepted) {
        QStringList files = dialog->selectedFiles();
        rundir = files[0];
        ui->RunDirectoryLineEdit->setText(rundir);
    }
    delete dialog;
*/
    QString start = ui->runDirectoryLineEdit->text();
    QString dir = QFileDialog::getExistingDirectory( this, tr("Select Directory"),
        start, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        rundir = dir;
        ui->runDirectoryLineEdit->setText(rundir);
    }
}

void
SettingsDialog::setSettingsParameters( QSettings* set,
    Hist1Parameters *h1params, Hist2Parameters *h2params)
{
    settings = set;
    hist1 = h1params;
    hist2 = h2params;

    bool write_run = settings->value( "write-run", true).toBool();
    ui->writeRunFileCheckBox->setChecked(write_run);

    rundir = settings->value( "run-directory", "/home").toString();
    ui->runDirectoryLineEdit->setText(rundir);

    for ( int i = 0; i < CHANNELS; ++i) {
        histograms[i].bins = hist1[i].bins;
        histograms[i].min = hist1[i].min;
        histograms[i].max = hist1[i].max;
    }

    histograms[4].bins = hist1[4].bins;
    histograms[4].min = hist1[4].min;
    histograms[4].max = hist1[4].max;

    histograms[5].bins = hist1[15].bins;
    histograms[5].min = hist1[15].min;
    histograms[5].max = hist1[15].max;

    ui->channelComboBox->setCurrentIndex(0);

    ui->binSpinBox->setValue(histograms[0].bins);
    ui->maxSpinBox->setValue(histograms[0].max);
    ui->minSpinBox->setValue(histograms[0].min);
/*
    settings->beginGroup("LinearFit");
    lfit_ch = settings->value( "reference-channel", 0).toInt();
    lfit_pos1 = settings->value( "reference-pos1", -1).toInt();
    lfit_pos2 = settings->value( "reference-pos2", -1).toInt();
    settings->endGroup();
*/
    int timeout_peroid = settings->value( "update-timeout", 3).toInt();
    ui->timeoutPeriodSpinBox->setValue(timeout_peroid);

    int charge_min = settings->value( "min-fit-charge", 1).toInt();
    int charge_max = settings->value( "max-fit-charge", 6).toInt();
    ui->minChargeSpinBox->setValue(charge_min);
    ui->maxChargeSpinBox->setValue(charge_max);

//    double ecount = settings->value( "energy-per-count", 2.0).toDouble();
//    ui->energyCountSpinBox->setValue(ecount);
}

void
SettingsDialog::channelIndexChanged(int i)
{
    ui->binSpinBox->setValue(histograms[i].bins);
    ui->maxSpinBox->setValue(histograms[i].max);
    ui->minSpinBox->setValue(histograms[i].min);
}

void
SettingsDialog::minChanged(double value)
{
    int ch = ui->channelComboBox->currentIndex();
    histograms[ch].min = value;
}

void
SettingsDialog::maxChanged(double value)
{
    int ch = ui->channelComboBox->currentIndex();
    histograms[ch].max = value;
}

void
SettingsDialog::binsChanged(int value)
{
    int ch = ui->channelComboBox->currentIndex();
    histograms[ch].bins = value;
}

void
SettingsDialog::restoreReferenceMatrixClicked()
{
    int res = QMessageBox::warning( this, tr("Restore"),
        tr("Would you like to restore predefine matrix?"),
        QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (res == QMessageBox::Yes) {
        params->restore_reference_signals();
        ui->signalCountsTableWidget->clear();
        ReferenceSignalMap& ref_signals = params->reference_signals();
        size_t elems = ref_signals.size();

        ui->signalCountsTableWidget->setRowCount(elems);
        int pos = 0;
        for ( const auto& elem : ref_signals) {
            int column = 0;

            int value = elem.first;
            QTableWidgetItem* item1 = new QTableWidgetItem(QString("%1").arg(value));
            ui->signalCountsTableWidget->setItem( pos, column++, item1);

            for ( const SignalPair& sp : elem.second) {
                QString str = SignalValueDelegate::form_text(sp);
                QTableWidgetItem* item = new QTableWidgetItem(str);
                ui->signalCountsTableWidget->setItem( pos, column++, item);
            }
            pos++;
        }
//        lfit_ch = 0;
//        lfit_pos1 = -1;
//        lfit_pos2 = -1;
    }
    else if (res == QMessageBox::No) {

    }
}

void
SettingsDialog::restoreReferenceChargeClicked()
{
    int res = QMessageBox::warning( this, tr("Restore"),
        tr("Would you like to restore predefine matrix?"),
        QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (res == QMessageBox::Yes) {
        params->restore_charge_signals();
        ChargeSignalMap& ref_charge = params->charge_signals();

//        size_t elems = ref_signals.size();

//        ui->chargeCountsTableWidget->setRowCount(elems);
/*
        for ( int i = 0; i < CARBON_Z; ++i) {
            const SignalPair& p = ref_charge[i + 1];
            QString str = SignalValueDelegate::form_text(p);
            QTableWidgetItem* item = new QTableWidgetItem(str);
            ui->chargeCountsTableWidget->setItem( i, 1, item);
        }
*/
        const SignalPair& p = ref_charge[1];
        QString str = SignalValueDelegate::form_text(p);
        QTableWidgetItem* item = ui->chargeCountsTableWidget->item( 0, 1);
        item->setText(str);

        // Ion charge radius parameter
        const double* radius = params->charge_radius_parameter();
        for ( int i = 0; i < CARBON_Z; ++i) {
            QTableWidgetItem* item = ui->chargeCountsTableWidget->item( i, 2);
            item->setText(QString("%1").arg(radius[i]));
        }

        // Ion charge beta parameter
        const double* beta = params->charge_beta_parameter();
        for ( int i = 0; i < CARBON_Z; ++i) {
            QTableWidgetItem* item = ui->chargeCountsTableWidget->item( i, 3);
            item->setText(QString("%1").arg(beta[i]));
        }
    }
    else if (res == QMessageBox::No) {

    }
}

void
SettingsDialog::applyChanges()
{
    params->tension() = ui->tensionSpinBox->value();

    settings->setValue( "update-timeout", ui->timeoutPeriodSpinBox->value());
    settings->setValue( "min-fit-charge", ui->minChargeSpinBox->value());
    settings->setValue( "max-fit-charge", ui->maxChargeSpinBox->value());

    settings->setValue( "run-directory", ui->runDirectoryLineEdit->text());
//    settings->setValue( "energy-per-count", ui->energyCountSpinBox->value());
//    Hist1Parameters::energy_per_count = ui->energyCountSpinBox->value();

    // set parameters for channels
    for ( int i = 0; i < CHANNELS; ++i) {
        Hist1Parameters& p = histograms[i];
        hist1[i].setBins( p.bins, p.min, p.max);
    }

    // set parameters for fitted
    Hist1Parameters& fitted = histograms[4];
    for ( int i = CHANNELS; i < 15; ++i) {
        hist1[i].setBins( fitted.bins, fitted.min, fitted.max);
    }

    // set parameters for charge
    Hist1Parameters& charge = histograms[5];
    hist1[15].setBins( charge.bins, charge.min, charge.max);

    // parameters for channel correlation
    for ( int i = 0; i < 6; ++i) {
        Hist2Parameters& cc = hist2[i];
        cc.setBinsX(fitted);
        cc.setBinsY(fitted);
    }

    // parameters for charge correlation
    Hist1Parameters& z = histograms[5];
    for ( int i = 6; i < 12; ++i) {
        Hist2Parameters& zz = hist2[i];
        zz.setBinsX(z);
        zz.setBinsY(z);
    }

    int rows = ui->signalCountsTableWidget->rowCount();
    int columns = ui->signalCountsTableWidget->columnCount();

    ReferenceSignalMap& ref_signals = params->reference_signals();
    ref_signals.clear();

    for ( int i = 0; i < rows; ++i) {
        QTableWidgetItem* key_item = ui->signalCountsTableWidget->item( i, 0);
        QString key_text = key_item->text();
        double key_value = QVariant(key_text).toDouble();
        SignalArray arr;
        for ( int j = 1; j < columns; ++j) {
            QTableWidgetItem* signal_item = ui->signalCountsTableWidget->item( i, j);
            QString signal_text = signal_item->text();
            SignalPair sp = SignalValueDelegate::parse_text(signal_text);
            arr[j - 1] = sp;
        }
        ref_signals[key_value] = arr;
    }
    params->recalculate();
    params->recalculate_charge_fit();
}

void
SettingsDialog::writeRunChecked(bool state)
{
    settings->setValue( "write-run", state);
}

void
SettingsDialog::signalCountsCellChanged( int row, int column)
{
    QTableWidgetItem* item = ui->signalCountsTableWidget->item( row, column);
    qDebug() << item->text();
}

void
SettingsDialog::chargeCountsCellChanged( int row, int column)
{
    if (column == 1) {
        QTableWidgetItem* item = ui->chargeCountsTableWidget->item( row, column);
        SignalPair charge_signal_pair = SignalValueDelegate::parse_text(item->text());

        ChargeSignalMap& ref_charge = params->charge_signals();
        SignalPair& charge_pair = ref_charge[row + 1];
        charge_pair = charge_signal_pair;
    }
    else if (column == 2) { // charge radius
        QTableWidgetItem* item = ui->chargeCountsTableWidget->item( row, column);
        QVariant variant(item->text());
        double value = variant.toDouble();

        double* radius = params->charge_radius_parameter();
        radius[row] = value;
        qDebug() << "Charge radius: " << value;
    }
    else if (column == 3) { // charge beta
        QTableWidgetItem* item = ui->chargeCountsTableWidget->item( row, column);
        QVariant variant(item->text());
        double value = variant.toDouble();

        double* beta = params->charge_beta_parameter();
        beta[row] = value;
        qDebug() << "Charge beta: " << value;
    }
    else if (column == 4) { // use charge in fit
        QTableWidgetItem* item = ui->chargeCountsTableWidget->item( row, column);
        QVariant variant(item->text());
        bool value = variant.toBool();

        bool* fit_charge = CalibrationFitting::BeamCompositionFit::charge_in_fit();
        fit_charge[row] = value;
        qDebug() << "Use charge in fit: " << value;
    }
}

void
SettingsDialog::signalCountsAddRowAboveClicked()
{
    if (selected_row != -1) {
        addSignalCountsRow(selected_row);
    }

}

void
SettingsDialog::signalCountsAddRowBelowClicked()
{

    if (selected_row != -1) {
        addSignalCountsRow(++selected_row);
    }

}

void
SettingsDialog::addSignalCountsRow(int row)
{
    ui->signalCountsTableWidget->insertRow(row);

    int columns = ui->signalCountsTableWidget->columnCount();
    for ( int i = 0; i < columns; ++i) {
        if (i) {
            QTableWidgetItem* item = new QTableWidgetItem();
            SignalPair p( 0.0, 0.0);
            QString str = SignalValueDelegate::form_text(p);
            item->setText(str);
            ui->signalCountsTableWidget->setItem( selected_row, i, item);
        }
        else {
            QTableWidgetItem* item_above = ui->signalCountsTableWidget->item( selected_row - 1, i);
            QTableWidgetItem* item_below = ui->signalCountsTableWidget->item( selected_row + 1, i);

            QString key_above = item_above->text();
            double value_above = QVariant(key_above).toDouble();

            QString key_below = item_below->text();
            double value_below = QVariant(key_below).toDouble();
            QTableWidgetItem* item = new QTableWidgetItem(QString("%1").arg((value_above + value_below) / 2.));
            ui->signalCountsTableWidget->setItem( selected_row, i, item);
        }
    }
}

void
SettingsDialog::signalCountsDeleteRowClicked()
{
    qDebug() << "delete";
    if (selected_row != -1) {
        ui->signalCountsTableWidget->removeRow(selected_row);
    }
}

void
SettingsDialog::signalCountsRowSelected(int r)
{
    selected_row = r;
    qDebug() << "Signal counts row: " << r;
    int rows = ui->signalCountsTableWidget->rowCount();
    ui->actionDeleteReferenceRow->setEnabled(rows);

    ui->actionAddReferenceRowAbove->setEnabled(selected_row);
    bool end_of_table = (selected_row == (rows - 1));
    ui->actionAddReferenceRowBelow->setEnabled(!end_of_table);
}

void
SettingsDialog::chargeSignalRowSelected(int r)
{
    qDebug() << "Charge signal row: " << r;
    params->reference_charge_parameter() = r + 1;
}

void
SettingsDialog::tensionParameterChanged(double t)
{
    params->tension() = t;
}

void
SettingsDialog::showGraphClicked()
{
    // Reference signal to channels count relation
    TMultiGraph* mg = new TMultiGraph( "mg", "");

    RootCanvasDialog* dialog = new RootCanvasDialog(this);

    double* x[CHANNELS] = {};
    double* xerr[CHANNELS] = {};
    double* y[CHANNELS] = {};
    double* yerr[CHANNELS] = {};
    TGraphErrors* err[CHANNELS] = {};
    Color_t colors[CHANNELS] = { kRed, kBlue, kViolet, kGreen };

    int rows = ui->signalCountsTableWidget->rowCount();

//    const SignalArray& back = params->background();
    for ( int i = 0; i < CHANNELS; ++i) {
        x[i] = new double[rows];
        xerr[i] = new double[rows];
        y[i] = new double[rows];
        yerr[i] = new double[rows];

        double* x_ = x[i];
        double* xerr_ = xerr[i];
        double* y_ = y[i];
        double* yerr_ = yerr[i];

        // table background for channels
        QTableWidgetItem* back_item = ui->signalCountsTableWidget->item( 0, i + 1);
        QString back_text = back_item->text();
        SignalPair ref_back = SignalValueDelegate::parse_text(back_text);

        for ( int j = 0; j < rows; ++j) {
            QTableWidgetItem* key_item = ui->signalCountsTableWidget->item( j, 0);
            QString key_text = key_item->text();
            double key_value = QVariant(key_text).toDouble();

            QTableWidgetItem* signal_item = ui->signalCountsTableWidget->item( j, i + 1);
            QString signal_text = signal_item->text();
            SignalPair sp = SignalValueDelegate::parse_text(signal_text);
            x_[j] = key_value;
            xerr_[j] = 0.0;
            y_[j] = sp.first - ref_back.first;
//            y_[j] += back[0].first;
            yerr_[j] = sp.second;
        }
        err[i] = new TGraphErrors( rows, x_, y_, xerr_, yerr_);
        err[i]->SetLineColor(colors[i]);
        err[i]->SetLineWidth(2);
        err[i]->SetMarkerColor(colors[i]);
//        err[i]->SetMarkerStyle(21);
        mg->Add( err[i], "lp");
    }

    TLegend* leg = new TLegend( 0.15, 0.5, 0.35, 0.85, "Channels");
    leg->AddEntry( err[0], "Channel-1", "LP");
    leg->AddEntry( err[1], "Channel-2", "LP");
    leg->AddEntry( err[2], "Channel-3", "LP");
    leg->AddEntry( err[3], "Channel-4", "LP");

    // draw multigraph in the canvas pad
    TPad* pad = dialog->getPad();

    pad->cd();
    mg->Draw("ap");

    mg->GetHistogram()->SetMaximum(4095.0);   // along
    mg->GetHistogram()->SetMinimum(0.0);  //   Y

    mg->GetXaxis()->SetTitleOffset(1.2);
    mg->GetXaxis()->SetTitleSize(0.03);
    mg->GetXaxis()->SetLabelSize(0.03);
    mg->GetYaxis()->SetTitleOffset(1.3);
    mg->GetXaxis()->SetTitle("Reference signal (mV)");
    mg->GetYaxis()->SetTitle("Amplitude (ADC count)");

    leg->Draw();

    dialog->setModal(true);
    dialog->exec();

    for ( int i = 0; i < CHANNELS; ++i) {
        delete [] x[i];
        delete [] xerr[i];
        delete [] y[i];
        delete [] yerr[i];
        delete err[i];
    }

    delete mg;
    delete leg;

    delete dialog;

}

void
SettingsDialog::signalCountsCellClicked( int, int)
{
    // enable or disable linear fit calculation
}

void
SettingsDialog::recalculate_linear_fit()
{
    double* x[CHANNELS] = {};
    double* y[CHANNELS] = {};

    int rows = ui->signalCountsTableWidget->rowCount();

    for ( int i = 0; i < CHANNELS; ++i) {
        x[i] = new double[rows];
        y[i] = new double[rows];

        double* x_ = x[i];
        double* y_ = y[i];
        for ( int j = 0; j < rows; ++j) {
            QTableWidgetItem* key_item = ui->signalCountsTableWidget->item( j, 0);
            QString key_text = key_item->text();
            double key_value = QVariant(key_text).toDouble();

            QTableWidgetItem* signal_item = ui->signalCountsTableWidget->item( j, i + 1);
            QString signal_text = signal_item->text();
            SignalPair sp = SignalValueDelegate::parse_text(signal_text);
            x_[j] = key_value;
            y_[j] = sp.first;// - ChannelsCountsFit::background[i].first;
        }
    }

    for ( int i = 0; i < CHANNELS; ++i) {
        delete [] x[i];
        delete [] y[i];
    }
}
