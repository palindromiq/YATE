#include "analysisviewitem.h"

namespace Yate {

AnalysisViewItem::AnalysisViewItem(const QVector<QVariant> &data, AnalysisViewItem *parent)
    : itemData_(data), parentItem_(parent)
{}


AnalysisViewItem::~AnalysisViewItem()
{
    qDeleteAll(childItems_);
}


void AnalysisViewItem::appendChild(AnalysisViewItem *item)
{
    childItems_.append(item);
}


AnalysisViewItem *AnalysisViewItem::child(int row)
{
    if (row < 0 || row >= childItems_.size())
        return nullptr;
    return childItems_.at(row);
}


int AnalysisViewItem::childCount() const
{
    return childItems_.count();
}


int AnalysisViewItem::columnCount() const
{
    return itemData_.count();
}


QVariant AnalysisViewItem::data(int column) const
{
    if (column < 0 || column >= itemData_.size())
        return QVariant();
    return itemData_.at(column);
}


AnalysisViewItem *AnalysisViewItem::parentItem()
{
    return parentItem_;
}


int AnalysisViewItem::row() const
{
    if (parentItem_)
        return parentItem_->childItems_.indexOf(const_cast<AnalysisViewItem*>(this));

    return 0;
}
}
