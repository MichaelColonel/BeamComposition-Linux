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

#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtGui>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QCheckBox>
#elif (QT_VERSION >= 0x040000 && QT_VERSION < 0x050000)
#include <QtGui>
#endif


#include "backgroundvaluedelegate.h"
#include "signalvaluedialog.h"
#include "signalvaluedelegate.h"

SignalValueDelegate::SignalValueDelegate( QObject* parent, bool charge_signal_flag)
    :
    QItemDelegate(parent),
    charge_flag(charge_signal_flag)
{
}

void
SignalValueDelegate::paint( QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    if ((index.column() > 0 && index.column() < 5) || !charge_flag) {
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
SignalValueDelegate::createEditor( QWidget* parent,
    const QStyleOptionViewItem&,
    const QModelIndex& index) const
{
    if (index.column() > 0 && index.column() < 5) {
        SignalValueDialog* dialog = new SignalValueDialog(parent);
        connect( dialog, SIGNAL(accepted()),
            this, SLOT(commitAndCloseEditor()));
        connect( dialog, SIGNAL(rejected()),
            this, SLOT(commitAndCloseEditor()));
        dialog->setModal(true);
        return dialog;
    }
    else if (index.column() == 5) {
        QDoubleSpinBox* spinbox = new QDoubleSpinBox(parent);
        spinbox->setRange( 0.10, 1.50);
        spinbox->setSingleStep(0.1);
        spinbox->setDecimals(2);
        connect( spinbox, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseEditor()));
        return spinbox;
    }
    else if (!charge_flag) {
        QDoubleSpinBox *spinbox = new QDoubleSpinBox(parent);
        spinbox->setRange( 0., 2000.0);
        spinbox->setDecimals(1);
        connect( spinbox, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseEditor()));
        return spinbox;
    }
    else {
        return nullptr;
    }
}

void
SignalValueDelegate::setEditorData( QWidget* editor,
    const QModelIndex& index) const
{
    if (index.column() > 0 && index.column() < 5) {
        QString str = index.model()->data( index, Qt::DisplayRole).toString();
        SignalValueDialog* dialog = qobject_cast<SignalValueDialog*>(editor);
        SignalPair p = parse_text(str);
        dialog->setMeanSigmaValues(p);
    }
    else if (index.column() == 5) {
        double value = index.model()->data( index, Qt::DisplayRole).toDouble();
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        spinbox->setValue(value);
    }
    else if (!charge_flag) {
        double value = index.model()->data( index, Qt::DisplayRole).toDouble();
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        spinbox->setValue(value);
    }
    else {
        QItemDelegate::setEditorData( editor, index);
    }
}

void
SignalValueDelegate::setModelData( QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() > 0 && index.column() < 5) {
        SignalValueDialog* dialog = qobject_cast<SignalValueDialog*>(editor);
        SignalPair pair = dialog->getMeanSigmaValues();
        QString str = form_text(pair);
        model->setData( index, str);
    }
    else if (index.column() == 5) {
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        double value = spinbox->value();
        model->setData( index, value);
    }
    else if (!charge_flag) {
        QDoubleSpinBox* spinbox = qobject_cast<QDoubleSpinBox*>(editor);
        double value = spinbox->value();
        model->setData( index, value);
    }
    else {
        QItemDelegate::setModelData( editor, model, index);
    }
}

void
SignalValueDelegate::commitAndCloseEditor()
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

SignalPair
SignalValueDelegate::parse_text(const QString& str)
{
    SignalPair p( 0.0, 0.0);
    QRegExp rx("\\d*\\.\\d+");    // primitive floating point matching
    int pos = 0;
    int count = 0;
    while ((pos = rx.indexIn( str, pos)) != -1) {
        QString s = rx.cap(0);

        QVariant v(s);
        if (count == 0)
            p.first = v.toDouble();
        else if (count == 1)
            p.second = v.toDouble();

        pos += rx.matchedLength();
        count++;
    }
    return p;
}

QString
SignalValueDelegate::form_text(const SignalPair& p)
{
    QString m(tr("%1").arg( p.first, 4, 'f', 2));
    QString s(tr("%1").arg( p.second, 4, 'f', 2));

    return QString(tr("%1 %2 %3").arg(m).arg(QChar(0xB1)).arg(s));
}
