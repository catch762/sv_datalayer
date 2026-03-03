#pragma once
#include <QAbstractItemModel>
#include <QVariant>

#include "sv_qtcommon.h"
#include "DataNode.h"

class DataTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(DataTreeModel)

    explicit DataTreeModel(DataNodeShared root = nullptr, QObject* parent = nullptr) : QAbstractItemModel(parent), rootItem(root) {}

    ~DataTreeModel() = default;

    //as usual for qt, returns rootItem for invalid indexes
    /*DataNode* nodePtrOfIndex(const QModelIndex& index) const
    {
        return index.isValid() ? static_cast<DataNode*>(index.internalPointer()) : rootItem.get();
    }*/

    DataNode* rawInternalPointer(const QModelIndex& index) const
    {
        return static_cast<DataNode*>(index.internalPointer());
    }

    /*
    QModelIndex is:
        - {row, column, itemPtr}
        - it means index of 'itemPtr' which exists in 'row, column' in some other parent item
    */

    // Obtain QModelIndex for item which exists at 'row, column' inside 'parent'
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
    {
        //some cheap basic checks for index existence
        if (!hasIndex(row, column, parent))
        {
            return {};
        }

        DataNode* parentItem = parent.isValid() ? rawInternalPointer(parent) : rootItem.get();

        if (auto childItem = parentItem->tryGetChild(row))
        {
            return createIndex(row, column, childItem.get());
        }

        //no such child index in parent DataNode
        return {};
    }

    QModelIndex parent(const QModelIndex &index) const override
    {
        if (!index.isValid())
        {
            return {};
        }

        auto *thisItem = rawInternalPointer(index);
        if (!thisItem)
        {
            //this means something is seriously malformed, it cant be.
            return {};
        }

        auto parentItem = thisItem->tryGetParent();
        if (!parentItem)
        {
            //this also cant really be happening, unless you fucked up your tree structure.
            //normally any 'thisItem' here must have valid parent.
            return {};
        }


        return parentItem == rootItem ? QModelIndex{} :
                                        createIndex(parentItem->rowIndexOfThisInParent(), 0, parentItem.get());
    }

    int rowCount(const QModelIndex &parent) const override
    {
        if (parent.column() > 0)
            return 0;

        DataNode* parentItem = parent.isValid() ? rawInternalPointer(parent) : rootItem.get();

        if (auto* compData = parentItem->tryGetCompositeData())
        {
            return compData->childrenCount();
        }

        //error, its leaf
        return 0;
    }

    int columnCount(const QModelIndex &parent) const override
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || role != Qt::DisplayRole)
            return {};

        auto *thisItem = rawInternalPointer(index);
        if (!thisItem)
        {
            //really shouldnt happem
            return {};
        }

        if (index.column() == 0)
        {
            return thisItem->getName();
        }
        else if (index.column() == 1)
        {
            if (auto leafValue = thisItem->tryGetLeafvalue())
            {
                return *leafValue;
            }
            else
            {
                return {};
            }
        }

        return {};
    }

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return index.isValid()
            ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
    }

    QVariant headerData(int section, Qt::Orientation orientation,
                               int role) const
    {
        return orientation == Qt::Horizontal && role == Qt::DisplayRole
            ? QString("Header %1").arg(section) : QVariant{};
    }

private:
    DataNodeShared rootItem;
};