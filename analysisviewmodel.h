#ifndef ANALYSISVIEWMODEL_H
#define ANALYSISVIEWMODEL_H

#include <QAbstractItemModel>




namespace Yate {

class AnalysisViewItem;
class HuntInfo;

class AnalysisViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit AnalysisViewModel(HuntInfo *hunt, QObject *parent = nullptr);
    ~AnalysisViewModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void setupModelData(const QStringList &lines, AnalysisViewItem *parent);

    AnalysisViewItem *rootItem_;
    HuntInfo *hunt_;
};

}

#endif // ANALYSISVIEWMODEL_H
