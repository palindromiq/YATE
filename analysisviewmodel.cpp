#include "analysisviewmodel.h"
#include "analysisviewitem.h"

#include <QStringList>
#include "huntinfo.h"

namespace Yate {

AnalysisViewModel::AnalysisViewModel(HuntInfo *hunt, QObject *parent)
    : QAbstractItemModel(parent), hunt_(hunt)
{
    rootItem_ = new AnalysisViewItem({tr("Title"), tr("Summary")});
    //setupModelData(data.split('\n'), rootItem_);
}



AnalysisViewModel::~AnalysisViewModel()
{
    delete rootItem_;
}



int AnalysisViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<AnalysisViewItem*>(parent.internalPointer())->columnCount();
    return rootItem_->columnCount();
}


QVariant AnalysisViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    AnalysisViewItem *item = static_cast<AnalysisViewItem*>(index.internalPointer());

    return item->data(index.column());
}


Qt::ItemFlags AnalysisViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}


QVariant AnalysisViewModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem_->data(section);

    return QVariant();
}


QModelIndex AnalysisViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    AnalysisViewItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem_;
    else
        parentItem = static_cast<AnalysisViewItem*>(parent.internalPointer());

    AnalysisViewItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}


QModelIndex AnalysisViewModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AnalysisViewItem *childItem = static_cast<AnalysisViewItem*>(index.internalPointer());
    AnalysisViewItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem_)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int AnalysisViewModel::rowCount(const QModelIndex &parent) const
{
    AnalysisViewItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem_;
    else
        parentItem = static_cast<AnalysisViewItem*>(parent.internalPointer());

    return parentItem->childCount();
}


void AnalysisViewModel::setupModelData(const QStringList &lines, AnalysisViewItem *parent)
{
    QVector<AnalysisViewItem*> parents;
    QVector<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].at(position) != ' ')
                break;
            position++;
        }

        const QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            const QStringList columnStrings =
                lineData.split(QLatin1Char('\t'), Qt::SkipEmptyParts);
            QVector<QVariant> columnData;
            columnData.reserve(columnStrings.count());
            for (const QString &columnString : columnStrings)
                columnData << columnString;

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new AnalysisViewItem(columnData, parents.last()));
        }
        ++number;
    }
}

}
