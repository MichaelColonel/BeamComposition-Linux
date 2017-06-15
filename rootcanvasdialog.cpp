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

#include <TCanvas.h>

#include <QResizeEvent>
#include <QMessageBox>
#include <QPushButton>
//#include <QDebug>

//#include <iostream>

#include "rootcanvasdialog.h"
#include "ui_rootcanvasdialog.h"

RootCanvasDialog::RootCanvasDialog( QWidget* parent, DiagramType dtype)
    :
    QDialog(parent),
    ui(new Ui::RootCanvasDialog),
    eaxis(nullptr),
    type(dtype)
{
    ui->setupUi(this);

    // TCanvas from ROOT Widget
    TCanvas* canvas = ui->widget->getCanvas();

    if (type == HIST_CHANNELS) {
        canvas->Divide( 2, 2);
        for ( int i = 0; i < CHANNELS; ++i) {
            TPad* p = dynamic_cast<TPad*>(canvas->GetPad(i + 1));
            canvas->cd(i + 1);
//            p->SetGrid();
            pad[i] = p;
        }
    }
    else {
        TPad* p = new TPad( "pad", "Grid", 0., 0., 1., 1.);
//        p->SetGrid();
        p->Draw();
        pad[0] = p;
    }

    QPushButton* reset_button = ui->buttonBox->button(QDialogButtonBox::Reset);
    reset_button->setText(tr("Reset..."));
    if (reset_button && type != NONE) {
        connect( reset_button, SIGNAL(clicked()), this, SLOT(resetClicked()));
    }

    QPushButton* update_button = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    update_button->setText(tr("Update"));
    if (update_button && type != NONE) {
        connect( update_button, SIGNAL(clicked()), this, SLOT(updateCanvas()));
    }

    if (type == NONE) {
        reset_button->hide();
        update_button->hide();
    }

    QPushButton* fit_button = ui->buttonBox->button(QDialogButtonBox::Help);
    if (fit_button && type == HIST_Z) {
        fit_button->setText(tr("Fit"));
        connect( fit_button, SIGNAL(clicked()), this, SLOT(fitClicked()));
    }
    else {
        fit_button->hide();
    }
}

RootCanvasDialog::~RootCanvasDialog()
{
//    qDebug() << "GUI: RootCanvasDialog Destructor: " << root_diagrams_type;

    if (type != HIST_CHANNELS) {
        delete pad[0];
    }

    if (eaxis && (type == HIST_FIT)) {
        delete eaxis;
    }

    delete ui;
}
/*
void
RootCanvasDialog::clearCanvas()
{
    TCanvas* canvas = ui->widget->getCanvas();
    canvas->Clear();
}
*/
void
RootCanvasDialog::updateCanvas()
{
    if (type == HIST_CHANNELS) {
        for ( int i = 0; i < CHANNELS; ++i) {
            TPad* p = pad[i];
            p->Modified();
            p->Update();
        }
    }
    else {
        pad[0]->Modified();
        pad[0]->Update();
    }
}

void
RootCanvasDialog::setEnergyRange(double min, double max, double energy_per_count)
{
    if (eaxis) {
        eaxis->SetX1(min);
        eaxis->SetX2(max);
        eaxis->SetWmin( min * energy_per_count * 1.E-03);
        eaxis->SetWmax( max * energy_per_count * 1.E-03);
    }
}

void
RootCanvasDialog::updateDiagram()
{
    if (!isHidden()) {
        updateCanvas();
    }
}

void
RootCanvasDialog::resetClicked()
{
    int res = QMessageBox::warning( this, tr("Reset diagram"),
        tr("Diagram data will be lost. Do you want to proceed?"),
        QMessageBox::Ok, QMessageBox::Cancel | QMessageBox::Default);

    if (res == QMessageBox::Ok)
        emit resetDiagram(type);
}

void
RootCanvasDialog::fitClicked()
{
    emit fitDiagram(type);
}

void
RootCanvasDialog::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent* event = static_cast<QWindowStateChangeEvent*>(e);
        if ((event->oldState() & Qt::WindowMaximized) ||
          (event->oldState() & Qt::WindowMinimized) ||
          (event->oldState() == Qt::WindowNoState &&
            this->windowState() == Qt::WindowMaximized)) {
            TCanvas* canvas = ui->widget->getCanvas();
            if (canvas) {
                canvas->Resize();
                canvas->Update();
         }
      }
   }
}
