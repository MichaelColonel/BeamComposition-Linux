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

#include <TPad.h>
#include <TH1.h>
#include <TH2.h>
#include <TGaxis.h>

#include "typedefs.h"
#include "diagramparameters.h"

#include <typeinfo>

namespace Ui {
class RootCanvasDialog;
}

class RootCanvasDialog : public QDialog {
    Q_OBJECT

public:
    explicit RootCanvasDialog( QWidget* parent = 0, DiagramType type = NONE);
    virtual ~RootCanvasDialog();
    virtual void changeEvent(QEvent* e);
//    void clearCanvas();
    TPad* getPad() { return pad[0]; }
    TPad** getPads() { return pad; }
    void setEnergyRange( double min, double max, double energy_per_count);
    template<class T> void drawDiagram(T*);
    template<class T> void drawChannels(T** ch);

public slots:
    void updateDiagram();

signals:
    void resetDiagram(DiagramType);
    void fitDiagram(DiagramType);

private slots:
    void resetClicked();
    void fitClicked();
    void updateCanvas();

private:
    Ui::RootCanvasDialog* ui;
    TPad* pad[CHANNELS];
    TGaxis* eaxis;
    DiagramType type;
};

template<class T>
void
RootCanvasDialog::drawDiagram(T* diagram)
{
    pad[0]->cd();
    if (typeid(T) == typeid(TH1)) {
        diagram->Draw();
    }
    else if (typeid(T) == typeid(TH2)) {
        diagram->Draw("COLZ");
    }
/*
    if (type == HIST_FIT) {
        double min = diagram->GetXaxis()->GetXmin();
        double max = diagram->GetXaxis()->GetXmax();
        double emin = min * Hist1Parameters::energy_per_count * 1.E-03;
        double emax = max * Hist1Parameters::energy_per_count * 1.E-03;
        diagram->Draw("X+");
        eaxis = new TGaxis( min, 0.0, max, 0.0, emin, emax, 50510, "+L");
        eaxis->SetName("energy_axis");
        eaxis->SetTitle("Energy (MeV)");
        eaxis->Draw();
    }
    else
        diagram->Draw("colz");
*/
}

template<class T>
void
RootCanvasDialog::drawChannels(T** channels)
{
    for ( int i = 0; i < CHANNELS; ++i) {
        pad[i]->cd();
        channels[i]->Draw();
    }
}
