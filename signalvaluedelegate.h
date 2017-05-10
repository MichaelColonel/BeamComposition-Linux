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

#pragma once

#include <QItemDelegate>

#include "typedefs.h"

class SignalValueDelegate : public QItemDelegate {
    Q_OBJECT
public:
    SignalValueDelegate( QObject* parent = 0, bool charge_signal_flag = false);

    void paint( QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const;
    QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const;
    void setEditorData( QWidget* editor, const QModelIndex& index) const;
    void setModelData( QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const;

    static SignalPair parse_text(const QString& text);
    static QString form_text(const SignalPair&);

private slots:
    void commitAndCloseEditor();
private:
    bool charge_flag;
};
