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

#include <QtGui>

#include "signalvaluedialog.h"
#include "signalvaluedelegate.h"
#include "backgroundvaluedelegate.h"

BackgroundValueDelegate::BackgroundValueDelegate(QObject* parent)
    :
    QItemDelegate(parent)
{
}

void
BackgroundValueDelegate::paint( QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    if (index.column() == 0 && index.row() > 0 && index.row() <= CHANNELS) {
        QString text = index.model()->data( index, Qt::DisplayRole).toString();

        QStyleOptionViewItem myOption = option;
        myOption.displayAlignment = Qt::AlignHCenter | Qt::AlignVCenter;

        drawDisplay( painter, myOption, myOption.rect, text);
        drawFocus( painter, myOption, myOption.rect);
    } else {
        QItemDelegate::paint( painter, option, index);
    }
}

QWidget*
BackgroundValueDelegate::createEditor( QWidget* parent,
    const QStyleOptionViewItem&,
    const QModelIndex& index) const
{
    if (index.column() == 0 && index.row() > 0 && index.row() <= CHANNELS) {
        SignalValueDialog* dialog = new SignalValueDialog(parent);
        connect( dialog, SIGNAL(accepted()),
            this, SLOT(commitAndCloseEditor()));
        connect( dialog, SIGNAL(rejected()),
            this, SLOT(commitAndCloseEditor()));
        return dialog;
    } else {
        return nullptr;
    }
}

void
BackgroundValueDelegate::setEditorData( QWidget* editor,
    const QModelIndex& index) const
{
    if (index.column() == 0 && index.row() > 0 && index.row() <= CHANNELS) {
        QString str = index.model()->data( index, Qt::DisplayRole).toString();
        SignalValueDialog* dialog = qobject_cast<SignalValueDialog*>(editor);
        SignalPair p = SignalValueDelegate::parse_text(str);
        dialog->setMeanSigmaValues(p);
    } else {
        QItemDelegate::setEditorData( editor, index);
    }
}

void
BackgroundValueDelegate::setModelData( QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() == 0 && index.row() > 0 && index.row() <= CHANNELS) {
        SignalValueDialog* dialog = qobject_cast<SignalValueDialog*>(editor);
        SignalPair pair = dialog->getMeanSigmaValues();
        QString str = SignalValueDelegate::form_text(pair);
        model->setData( index, str);
    } else {
        QItemDelegate::setModelData( editor, model, index);
    }
}

void
BackgroundValueDelegate::commitAndCloseEditor()
{
    SignalValueDialog* editor = qobject_cast<SignalValueDialog*>(sender());
    if (editor) {
        emit commitData(editor);
        emit closeEditor(editor);
    }
}
