#include "PlaylistModel.h"
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include "../../src/plugin_api/HippoMessages.h"

extern "C" {
	#include "src/hippo_core/native/hippo_core.h"
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

class TreeItem {
   public:
    TreeItem(const QVector<QVariant>& data, TreeItem* parent) : m_itemData(data), m_parentItem(parent) {}
    ~TreeItem() { qDeleteAll(m_childItems); }

    void appendChild(TreeItem* item) { m_childItems.append(item); }

    TreeItem* child(int row) {
        if (row < 0 || row >= m_childItems.size())
            return nullptr;
        return m_childItems.at(row);
    }

    int childCount() const { return m_childItems.count(); }
    int columnCount() const { return m_itemData.count(); }

    QVariant data(int column) const {
        if (column < 0 || column >= m_itemData.size())
            return QVariant();
        return m_itemData.at(column);
    }

    inline TreeItem* parentItem() { return m_parentItem; }

    int row() const {
        if (m_parentItem)
            return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));

        return 0;
    }

   private:
    QVector<TreeItem*> m_childItems;
    QVector<QVariant> m_itemData;
    TreeItem* m_parentItem;
};

PlaylistModel::PlaylistModel(QObject* parent) : QAbstractItemModel(parent) {
    QFile file(QStringLiteral("default.txt"));
    file.open(QIODevice::ReadOnly);
    auto t = file.readAll();
    QString data = QString::fromUtf8(t);

    QVector<QVariant> list;
    list.push_back(QStringLiteral("Title"));
    list.push_back(QStringLiteral("Summary"));

    rootItem = new TreeItem(list, nullptr);
    setupModelData(data.split(QLatin1Char('\n')), rootItem);
}

PlaylistModel::~PlaylistModel() {
    delete rootItem;
}

int PlaylistModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex& parent) const {
    qDebug() << "index " << row << ":" << column << " parent " << parent;

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem* childItem = parentItem->child(row);
    if (childItem) {
        printf("index: createIndex %p\n", childItem);
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex& index) const {
    if (!index.isValid())
        return QModelIndex();

    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    printf("parent: create index\n");

    return createIndex(parentItem->row(), 0, parentItem);
}

int PlaylistModel::rowCount(const QModelIndex& parent) const {
    TreeItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void PlaylistModel::setupModelData(const QStringList& lines, TreeItem* parent) {
    QVector<TreeItem*> parents;
    QVector<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].at(position) != QLatin1Char(' '))
                break;
            position++;
        }

        const QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            const QStringList columnStrings = lineData.split(QLatin1Char('\t'), QString::SkipEmptyParts);
            QVector<QVariant> columnData;
            columnData.reserve(columnStrings.count());
            for (const QString& columnString : columnStrings)
                columnData << columnString;

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount() - 1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new TreeItem(columnData, parents.last()));
        }
        ++number;
    }
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return QStringLiteral("Title");
            case 1:
                return QStringLiteral("Duration");
            case 2:
                return QStringLiteral("Information");
            default:
                break;
        }
    }

    return QVariant();
}

PlaylistModel::PlaylistModel(QObject* parent) : QAbstractItemModel(parent) {}

int PlaylistModel::rowCount(const QModelIndex& parent) const {
    qDebug() << "rowCount parent " << parent;
    return 1;
}

int PlaylistModel::columnCount(const QModelIndex& parent) const {
    qDebug() << "colum parent " << parent;
    return 3;
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    return QString(QStringLiteral("Row%1, Column%2")).arg(index.row() + 1).arg(index.column() + 1);
}


QModelIndex PlaylistModel::index(int row, int column, const QModelIndex& parent) const {
    printf("PlaylistModel::index -> row %d col %d\n", row, column);

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (row < 1 && column < 3) {
        printf("PlaylistModel::i create index\n");
        return createIndex(row, column, nullptr);
    }

    return QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex& index) const {
    qDebug() << ">>> parent " << index;
    return QModelIndex();
}

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PlaylistModel::PlaylistModel(HippoCore* core, QObject *parent)
    : QAbstractTableModel(parent),
    m_core(core)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return QStringLiteral("Title");
            case 1:
                return QStringLiteral("Duration");
            case 2:
                return QStringLiteral("Information");
            default:
                break;
        }
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PlaylistModel::update_index(const HippoSelectSong* select_song_msg) {
    int index = select_song_msg->playlist_index();
    auto desc = select_song_msg->description();
    int duration = desc->duration();
    QString dur;

    m_entries[index].title = QString::fromUtf8(desc->title()->c_str());
    if (duration == 0) {
        // use default duration (right now 5 min for songs without a length)
        dur = QDateTime::fromTime_t((int)m_default_length).toUTC().toString(QStringLiteral("hh:mm:ss"));
        dur.insert(0, QStringLiteral("∞ "));
    } else {
        dur = QDateTime::fromTime_t((int)duration).toUTC().toString(QStringLiteral("hh:mm:ss"));
    }

    m_entries[index].duration = dur;
    m_entries[index].description = QString::fromUtf8(desc->song_type()->c_str());

    layoutChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PlaylistModel::insert_entry(const HippoUrlEntry* select_song_msg) {
    auto desc = select_song_msg->description();
    QString duration;

    auto title = QString::fromUtf8(desc->title()->c_str());
    auto description = QString::fromUtf8(desc->song_type()->c_str());

    if (desc->duration() > 0.0) {
        duration = QDateTime::fromTime_t((int)desc->duration()).toUTC().toString(QStringLiteral("hh:mm:ss"));
    } else {
        duration = QStringLiteral("Unknown");
    }

    PlaylistEntry entry = { title, duration, description };
    m_temp_insert.push_back(entry);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PlaylistModel::begin_insert(int index, int count) {
    m_insert_index = index;
    m_temp_insert.clear();
    (void)count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PlaylistModel::end_insert() {
    m_entries.insert(m_entries.begin() + m_insert_index,
                     m_temp_insert.begin(), m_temp_insert.end());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PlaylistModel::removeRows(int row, int count, const QModelIndex&) {
    m_entries.erase(m_entries.begin() + row, m_entries.begin() + row + count);
    hippo_playlist_remove_entry(m_core, row);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PlaylistModel::rowCount(const QModelIndex&) const {
   return m_entries.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PlaylistModel::columnCount(const QModelIndex&) const {
    return 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0 : return m_entries[index.row()].title;
            case 1 : return m_entries[index.row()].duration;
            case 2 : return m_entries[index.row()].description;
        }
    }

    return QVariant();
}
