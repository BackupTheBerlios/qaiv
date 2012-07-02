/******************************************************************************
** This file is part of qadvanceditemviews.
**
** Copyright (c) 2011-2012 Martin Hoppe martin@2x2hoppe.de
**
** qadvanceditemviews is free software: you can redistribute it
** and/or modify it under the terms of the GNU Lesser General
** Public License as published by the Free Software Foundation,
** either version 3 of the License, or (at your option) any
** later version.
**
** qadvanceditemviews is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with qadvanceditemviews.
** If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "stdafx.h"
#include "qadvancedheaderview.h"

#include "qabstractfiltermodel.h"
#include "qfilterview.h"

QAdvancedHeaderView::QAdvancedHeaderView( Qt::Orientation orientation, QWidget* parent )
    : QHeaderView(orientation, parent)
{
    setClickable(true);
}

QAdvancedHeaderView::~QAdvancedHeaderView()
{
}

void QAdvancedHeaderView::addFilterSetActionTriggered()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (a){
        model()->insertRows(a->data().toInt(), 1);
    } else {
        model()->insertRows(model()->rowCount(), 1);
    }
}

void QAdvancedHeaderView::contextMenuEvent(QContextMenuEvent* event)
{
    QAction* a;
    QMenu menu;
    QAbstractFilterModel* filterModel = qobject_cast<QAbstractFilterModel*>(model());
    if (orientation() == Qt::Horizontal){
        int column = logicalIndexAt(event->pos());
        if (column > -1){
            a = new QAction(QIcon(":/qadvancedtableviews/column.hide"), QString(tr("Hide Column '%1'")).arg(model()->headerData(column, Qt::Horizontal).toString()), &menu);
            connect(a, SIGNAL(triggered()), this, SLOT(hideSectionActionTriggered()));
            a->setData(column);
            menu.addAction(a);
        }
        menu.addAction(tr("Show All Columns"), this, SLOT(showAllColumns()));
        QVector<QAction*> actions(model()->columnCount() < 11?model()->columnCount():10);
        for (int iColumn = 0; iColumn < model()->columnCount() && iColumn < 10; iColumn++){
            a = new QAction(model()->headerData(iColumn, Qt::Horizontal).toString(), &menu);
            a->setCheckable(true);
            a->setChecked(!isSectionHidden(iColumn));
            a->setData(iColumn);
            actions[visualIndex(iColumn)] = a;
            connect(a, SIGNAL(toggled(bool)), this, SLOT(sectionToggled(bool)));
        }
        menu.addSeparator();
        QMenu* columnsMenu = menu.addMenu(QIcon(":/qadvancedtableviews/columns.select"), tr("Columns"));
        for (int iAction = 0; iAction < actions.size(); iAction++){
            columnsMenu->addAction(actions.at(iAction));
        }
        if (model()->columnCount() > 10){
            columnsMenu->addSeparator();
            columnsMenu->addAction(tr("More Columns..."), this, SLOT(moreColumnsActionTriggered()));
        }
        if (filterModel){
            menu.addSeparator();
            QMenu* modeMenu = menu.addMenu(tr("Mode"));
            a = modeMenu->addAction(tr("Filter"), this, SLOT(filterModeActionTriggered()));
            a->setCheckable(true);
            a->setChecked(filterModel->mode() == QAdvancedItemViews::FilterMode);

            a = modeMenu->addAction(tr("Highlight"), this, SLOT(highlightModeActionTriggered()));
            a->setCheckable(true);
            a->setChecked(filterModel->mode() == QAdvancedItemViews::HighlightMode);

            modeMenu->addSeparator();
            a = modeMenu->addAction(tr("Inverted"), this, SLOT(invertedMatchModeActionTriggered()));
            a->setCheckable(true);
            a->setChecked(filterModel->matchMode() == QAdvancedItemViews::MatchInverted);
        }
        QFilterView* mvew = qobject_cast<QFilterView*>(parentWidget());
        if (mvew != 0){
            menu.addSeparator();
            if (mvew->filterVisible()){
                menu.addAction(tr("Hide Filter"), this, SLOT(hideFilterActionTriggered()));
            } else {
                menu.addAction(tr("Show Filter"), this, SLOT(showFilterActionTriggered()));
            }
        }
    } else if (orientation() == Qt::Vertical){
        int logicalIndex = logicalIndexAt(event->pos());

        a = menu.addAction(QIcon(":/qaiv/filter/add"), tr("Add Filter Set"), this, SLOT(addFilterSetActionTriggered()));
        a->setData(logicalIndex);
        a = menu.addAction(QIcon(":/qaiv/filter/rename"), tr("Rename Filter Set"), this, SLOT(renameFilterActionTriggered()));
        a->setData(logicalIndex);
        //
        a = menu.addAction(QIcon(":/qaiv/filter/delete"), tr("Remove Filter Set"), this, SLOT(removeFilterSetActionTriggered()));
        a->setData(logicalIndex);
        a->setEnabled(model()->rowCount() > 1);
    }
    event->accept();
    menu.exec(mapToGlobal(event->pos()));
}

void QAdvancedHeaderView::filterModeActionTriggered()
{
    QAbstractFilterModel* m = qobject_cast<QAbstractFilterModel*>(model());
    if (m){
        m->setMode(QAdvancedItemViews::FilterMode);
    }
}

void QAdvancedHeaderView::hideFilterActionTriggered()
{
    QFilterView* v = qobject_cast<QFilterView*>(parentWidget());
    if (v){
        v->setFilterVisible(false);
    }
}
void QAdvancedHeaderView::hideSectionActionTriggered()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (a){
        setSectionHidden(a->data().toInt(), true);
    }
}

void QAdvancedHeaderView::highlightModeActionTriggered()
{
    QAbstractFilterModel* m = qobject_cast<QAbstractFilterModel*>(model());
    if (m){
        m->setMode(QAdvancedItemViews::HighlightMode);
    }
}

void QAdvancedHeaderView::invertedMatchModeActionTriggered()
{
    QAbstractFilterModel* m = qobject_cast<QAbstractFilterModel*>(model());
    if (m){
        if (m->matchMode() == QAdvancedItemViews::MatchNormal){
            m->setMatchMode(QAdvancedItemViews::MatchInverted);
        } else {
            m->setMatchMode(QAdvancedItemViews::MatchNormal);
        }
    }
}

void QAdvancedHeaderView::moreColumnsActionTriggered()
{
    QDialog* dlg = new QDialog(this,Qt::Widget);
    dlg->setWindowTitle(tr("More columns..."));
    QVBoxLayout* l = new QVBoxLayout(dlg);

    QTableView* v = new QTableView(dlg);
    l->addWidget(v);

    QDialogButtonBox* b = new QDialogButtonBox(dlg);
    b->setOrientation(Qt::Horizontal);
    b->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    l->addWidget(b);

    QObject::connect(b, SIGNAL(accepted()), dlg, SLOT(accept()));
    QObject::connect(b, SIGNAL(rejected()), dlg, SLOT(reject()));

    QAbstractItemModel* m = model();
    QAbstractFilterModel* hp;
    QSortFilterProxyModel* p = qobject_cast<QSortFilterProxyModel*>(model());
    if (p){
        m = p->sourceModel();
        hp = qobject_cast<QAbstractFilterModel*>(p);
    }
    QStandardItemModel* cm = new QStandardItemModel(dlg);
    cm->setColumnCount(1);
    cm->setRowCount(m->columnCount());

    v->setModel(cm);
    v->horizontalHeader()->setStretchLastSection(true);
    v->horizontalHeader()->setVisible(false);
    v->verticalHeader()->setVisible(false);

    QStandardItem* item;
    for (int iColumn = 0; iColumn < m->columnCount(); iColumn++){
        item = new QStandardItem(m->headerData(iColumn, Qt::Horizontal).toString());
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        item->setData(isSectionHidden(iColumn)?Qt::Unchecked:Qt::Checked, Qt::CheckStateRole);
        cm->setItem(iColumn, 0, item);
    }
    v->resizeRowsToContents();
    if (dlg->exec()){
        for (int iRow = 0; iRow < cm->rowCount(); iRow++){
            setSectionHidden(iRow, cm->index(iRow, 0).data(Qt::CheckStateRole).toInt() == Qt::Unchecked);
        }
    }
    delete dlg;
}

void QAdvancedHeaderView::removeFilterSetActionTriggered()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (a){
        if (QMessageBox::question(this, tr("Remove Filter"), QString(tr("Please confirm to delete filter '%1'.")).arg(model()->headerData(a->data().toInt(), Qt::Vertical, Qt::DisplayRole).toString()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes){
            model()->removeRows(a->data().toInt(), 1);
        }
    }
}

void QAdvancedHeaderView::renameFilterActionTriggered()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (a){
        QString name = QInputDialog::getText(this, tr("Rename Filter Set"), tr("Name"), QLineEdit::Normal, model()->headerData(a->data().toInt(), Qt::Vertical, Qt::DisplayRole).toString());
        if (!name.isEmpty()){
            model()->setHeaderData(a->data().toInt(), Qt::Vertical, name);
        }
    }
}

void QAdvancedHeaderView::sectionToggled( bool on )
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (a){
        setSectionHidden(a->data().toInt(), !on);
    }
}

void QAdvancedHeaderView::showAllColumns()
{
    for (int iSection = 0; iSection < count(); iSection++){
        showSection(iSection);
    }
}

void QAdvancedHeaderView::showFilterActionTriggered()
{
    QFilterView* v = qobject_cast<QFilterView*>(parentWidget());
    if (v){
        v->setFilterVisible(true);
    }
}

int QAdvancedHeaderView::sizeHintForRow(int row) const
{
    return 25;
}

void QAdvancedHeaderView::toggleSortOrderActionTriggered()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (a){
        Qt::SortOrder sortOrder = sortIndicatorOrder();
        if (sortOrder == Qt::AscendingOrder){
            model()->sort(sortIndicatorSection(), Qt::DescendingOrder);
        } else {
            model()->sort(sortIndicatorSection(), Qt::AscendingOrder);
        }
    }
}
