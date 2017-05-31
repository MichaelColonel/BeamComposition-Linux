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
#include "chargevaluedelegate.h"

ChargeValueDelegate::ChargeValueDelegate(QObject* parent)
    :
    QItemDelegate(parent)
{
}

void
ChargeValueDelegate::paint( QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    if (index.column() >= 1 && index.column() <= 4) {
        QString text = index.model()->data(index, Qt::DisplayRole).toString();

        QStyleOptionViewItem myOption = option;
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

        drawDisplay(painter, myOption, myOption.rect, text);
        drawFocus(painter, myOption, myOption.rect);
    }   
    else {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget*
ChargeValueDelegate::createEditor( QWidget* parent,
    const QStyleOptionViewItem&,
    const QModelIndex& index) const
{
    if (index.column() == 1) {
        SignalValueDialog* dialog = new SignalValueDialog(parent);
        connect( dialog, SIGNAL(accepted()),
            this, SLOT(commitAndCloseEditor()));
        connect( dialog, SIGNAL(rejected()),
            this, SLOT(commitAndCloseEditor()));
        dialog->setModal(true);
        return dialog;
    }
    else if (index.column() == 2) {
        QDoubleSpinBox *spinbox = new QDoubleSpinBox(parent);
        spinbox->setRange( 0.10, 1.50);
        spinbox->setSingleStep(0.1);
        spinbox->setDecimals(2);
        connect( spinbox, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseEditor()));
        return spinbox;
    }
    else if (index.column() == 3) {
        QDoubleSpinBox *spinbox = new QDoubleSpinBox(parent);
        spinbox->setRange( 0.00000, 0.99999);
        spinbox->setSingleStep(0.01);
        spinbox->setDecimals(5);
        connect( spinbox, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseEditor()));
        return spinbox;
    }
    else if (index.column() == 4) {
        QSpinBox *spinbox = new QSpinBox(parent);
        spinbox->setRange( 0, 1);
        connect( spinbox, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseEditor()));
        return spinbox;
    }
    else {
        return nullptr;
    }
}

void
ChargeValueDelegate::setEditorData( QWidget* editor,
    const QModelIndex& index) const
{
    if (index.column() == 1) {
        QString str = index.model()->data( index, Qt::DisplayRole).toString();
        SignalValueDialog* dialog = qobject_cast<SignalValueDialog*>(editor);
        SignalPair p = SignalValueDelegate::parse_text(str);
        dialog->setMeanSigmaValues(p);
    }
    else if (index.column() == 2) {
        double value = index.model()->data( index, Qt::DisplayRole).toDouble();
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        spinbox->setValue(value);
    }
    else if (index.column() == 3) {
        double value = index.model()->data( index, Qt::DisplayRole).toDouble();
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        spinbox->setValue(value);
    }
    else if (index.column() == 4) {
        int value = index.model()->data( index, Qt::DisplayRole).toInt();
        QSpinBox* spinbox = qobject_cast<QSpinBox*>(editor);
        spinbox->setValue(bool(value));
    }
    else {
        QItemDelegate::setEditorData( editor, index);
    }
}

void
ChargeValueDelegate::setModelData( QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() == 1) {
        SignalValueDialog* dialog = qobject_cast<SignalValueDialog*>(editor);
        SignalPair pair = dialog->getMeanSigmaValues();
        QString str = SignalValueDelegate::form_text(pair);
        model->setData( index, str);
    }
    else if (index.column() == 2) {
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        double value = spinbox->value();
        model->setData( index, value);
    }
    else if (index.column() == 3) {
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        double value = spinbox->value();
        model->setData( index, value);
    }
    else if (index.column() == 4) {
        QSpinBox* spinbox = qobject_cast<QSpinBox*>(editor);
        int value = spinbox->value();
        model->setData( index, value);
    }
    else {
        QItemDelegate::setModelData( editor, model, index);
    }
}

void
ChargeValueDelegate::commitAndCloseEditor()
{
    SignalValueDialog* dialog = qobject_cast<SignalValueDialog*>(sender());
    QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(sender());
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    if (dialog) {
        emit commitData(dialog);
        emit closeEditor(dialog);
    }
    else if (spinbox) {
        emit commitData(spinbox);
        emit closeEditor(spinbox);
    }
    else if (checkbox) {
        emit commitData(spinbox);
        emit closeEditor(spinbox);
    }
}
