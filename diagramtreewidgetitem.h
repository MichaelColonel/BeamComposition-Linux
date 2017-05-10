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

#include <QTreeWidgetItem>

#include "typedefs.h"

class RootCanvasDialog;

class DiagramTreeWidgetItem : public QTreeWidgetItem {
public:
    DiagramTreeWidgetItem(DiagramType diagram_type = NONE)
        :
        QTreeWidgetItem(QTreeWidgetItem::UserType),
        root_diagram_type(diagram_type),
        root_diagram_tuple(std::make_tuple( nullptr, nullptr)),
        root_canvas_dialog(nullptr)
    {
    }

    virtual ~DiagramTreeWidgetItem();
    DiagramType diagramType() const { return root_diagram_type; }
    RootCanvasDialog* canvasDialog() const { return root_canvas_dialog; }
    DiagramTuple diagramTyple() const { return root_diagram_tuple; }
    void setDiagramTuple(DiagramTuple& tuple) { root_diagram_tuple = tuple; }
    void setCanvasDialog(RootCanvasDialog* dialog) { root_canvas_dialog = dialog; }
    TH1* getTH1() const { return std::get<0>(root_diagram_tuple); }
    TH2* getTH2() const { return std::get<1>(root_diagram_tuple); }
private:
    DiagramType root_diagram_type;
    DiagramTuple root_diagram_tuple;
    RootCanvasDialog* root_canvas_dialog;
};
