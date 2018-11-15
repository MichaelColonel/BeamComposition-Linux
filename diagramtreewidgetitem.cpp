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

#include <TH1D.h>
#include <TH2D.h>

#include <QDebug>

#include "rootcanvasdialog.h"

#include "diagramtreewidgetitem.h"

DiagramTreeWidgetItem::~DiagramTreeWidgetItem()
{
    if (root_canvas_dialog) delete root_canvas_dialog;

    TH1* h1;
    TH2* h2;
    std::tie( h1, h2) = root_diagram_tuple;
    if (h1) delete h1;
    if (h2) delete h2;

//    qDebug() << "GUI: DiagramTreeWidgetItem Destructor: " << root_diagrams_type;
}

QString
DiagramTreeWidgetItem::getDiagramName() const
{
    QString dname;
    if (root_diagram_type != HIST_CHANNELS) {
        if (TH1* h1 = getTH1())
            dname = QString(h1->GetName());
        else if (TH2* h2 = getTH2())
            dname = QString(h2->GetName());
    }
    else {
        dname = "Channels";
    }
    return dname;
}
