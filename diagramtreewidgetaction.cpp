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

#include <QTreeWidget>

#include "rootcanvasdialog.h"
#include "diagramtreewidgetitem.h"

#include "diagramtreewidgetaction.h"

DiagramTreeWidgetAction::DiagramTreeWidgetAction(QTreeWidget* widget)
    :
    treewidget(widget)
{
    QTreeWidgetItemIterator iter(widget);
    Diagrams& d = diagrams;
    while (*iter) {
        DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);

        if (!ditem) {
            ++iter;
            continue;
        }

        TH1* h1 = ditem->getTH1();
        TH2* h2 = ditem->getTH2();

        switch (ditem->diagramType()) {
        case HIST_CHANNEL1:
            d.channels[0] = h1;
            break;
        case HIST_CHANNEL2:
            d.channels[1] = h1;
            break;
        case HIST_CHANNEL3:
            d.channels[2] = h1;
            break;
        case HIST_CHANNEL4:
            d.channels[3] = h1;
            break;
        case HIST_FITALL:
            d.fitall = h1;
            break;
        case HIST_FIT_CHANNEL1:
            d.fit[0] = h1;
            break;
        case HIST_FIT_CHANNEL2:
            d.fit[1] = h1;
            break;
        case HIST_FIT_CHANNEL3:
            d.fit[2] = h1;
            break;
        case HIST_FIT_CHANNEL4:
            d.fit[3] = h1;
            break;
        case HIST_FIT_MEAN:
            d.fit_mean = h1;
            break;
        case HIST_FIT_MEDIAN:
            d.fit_median = h1;
            break;
        case HIST_RANK1:
            d.rank[0] = h1;
            break;
        case HIST_RANK2:
            d.rank[1] = h1;
            break;
        case HIST_RANK3:
            d.rank[2] = h1;
            break;
        case HIST_RANK4:
            d.rank[3] = h1;
            break;
        case HIST_Z:
            d.z = h1;
            break;
        case HIST_CHANNEL12:
            d.c12 = h2;
            break;
        case HIST_CHANNEL23:
            d.c23 = h2;
            break;
        case HIST_CHANNEL34:
            d.c34 = h2;
            break;
        case HIST_CHANNEL13:
            d.c13 = h2;
            break;
        case HIST_CHANNEL14:
            d.c14 = h2;
            break;
        case HIST_CHANNEL24:
            d.c24 = h2;
            break;
        case HIST_Z12:
            d.z12 = h2;
            break;
        case HIST_Z23:
            d.z23 = h2;
            break;
        case HIST_Z34:
            d.z34 = h2;
            break;
        case HIST_Z13:
            d.z13 = h2;
            break;
        case HIST_Z14:
            d.z14 = h2;
            break;
        case HIST_Z24:
            d.z24 = h2;
            break;
        case HIST_CHANNELS:
        case NONE:
        default:
            break;
        }
        ++iter;
    }
}

DiagramTreeWidgetAction::~DiagramTreeWidgetAction()
{
}

void
DiagramTreeWidgetAction::resetDiagram(DiagramType type)
{
    QTreeWidgetItemIterator iter(treewidget);
    while (*iter) {
        DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);

        if (ditem && (type == ditem->diagramType())) {
            if (type == HIST_CHANNELS) {
                for ( int i = 0; i < CHANNELS; ++i) {
                    TH1* h = diagrams.channels[i];
                    h->Reset();
                }
            }
            else {
                TH1* h1 = ditem->getTH1();
                TH2* h2 = ditem->getTH2();

                if (h1) h1->Reset();
                if (h2) h2->Reset();
            }

            RootCanvasDialog* dialog = ditem->canvasDialog();
            if (dialog)
                dialog->updateDiagram();
        }
        ++iter;
    }
}
