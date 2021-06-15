#ifndef ANALYSISVIEWITEM_H
#define ANALYSISVIEWITEM_H

#include <QVector>
#include <QVariant>

namespace Yate {
class AnalysisViewItem
{
public:
    explicit AnalysisViewItem(const QVector<QVariant> &data, AnalysisViewItem *parentItem = nullptr);
    ~AnalysisViewItem();

    void appendChild(AnalysisViewItem *child);

    AnalysisViewItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    AnalysisViewItem *parentItem();

private:
    QVector<AnalysisViewItem*> childItems_;
    QVector<QVariant> itemData_;
    AnalysisViewItem *parentItem_;
};
}

#endif // ANALYSISVIEWITEM_H
