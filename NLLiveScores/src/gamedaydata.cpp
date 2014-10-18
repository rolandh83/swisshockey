#include "gamedaydata.h"

#include <QDebug>

GamedayData::GamedayData(QObject *parent) : QAbstractListModel(parent) {
    QHash<int, QByteArray> roles;
    roles[HometeamRole] = "hometeam";
    roles[AwayteamRole] = "awayteam";
    roles[TotalScoreRole] = "totalscore";
    roles[PeriodsScoreRole] = "periodsscore";
    roles[GameStatus] = "gamestatus";
    setRoleNames(roles);

    this->date = "";
}

void GamedayData::updateGames(QString date, QVariantList data) {
    // Check if the we're updating the current game day or if we're given the
    // data for a new day
    if(this->date.compare(date)) {
        // Clear the whole list
        this->date = date;
        beginResetModel();
        this->games.clear();
        this->gameIndices.clear();
        endResetModel();
    } else {
        // NOP.
    }

    // Add or update each game in the list
    QListIterator<QVariant> iter(data);
    while(iter.hasNext()) {
        // Get the game...
        QVariantMap game = iter.next().toMap();
        int key = game["gameid"].toInt();

        if(this->games.contains(key)) {
            // The game is already in the list, hence, we simply update it with
            // the new data
            this->games[key]->updateGame(game);
        } else {
            // The game couldn't be found in the list so we simply add a new one
            // For that, we need to call beginInsertRows() and endInsertRows()
            // so that the ListView gets notified about the new content.
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            this->games.insert(key, new GameData(game, this));
            this->gameIndices.append(key);
            endInsertRows();
        }

        // Check if game in list has changed, if so, emit signal
        if(this->games[key]->hasChanged()) {
            QModelIndex index = createIndex(this->gameIndices.indexOf(key), 0);
            dataChanged(index, index);
        }
    }
}

// Returns the number of rows in the list
int GamedayData::rowCount(const QModelIndex &parent) const {
    return this->games.count();
}

// Returns the data requested by the view
QVariant GamedayData::data(const QModelIndex &index, int role) const {
    QVariant data;

#if 0
    qDebug() << "GamedayData::data() called. Index: " << index.row() << ", "
             << index.column();
#endif

    // Find the requested item
    int key = this->gameIndices[index.row()];

    switch(role) {
        case HometeamRole:
            data = this->games[key]->getHometeam();
            break;

        case AwayteamRole:
            data = this->games[key]->getAwayteam();
            break;

        case TotalScoreRole:
            data = this->games[key]->getTotalScore();
            break;

        case PeriodsScoreRole:
            data = this->games[key]->getPeriodsScore(1).append(", ").append(
                        this->games[key]->getPeriodsScore(2)
                    ).append(", ").append(
                        this->games[key]->getPeriodsScore(3)
                    );
            break;

        case GameStatus:
            data = this->games[key]->getGameStatus();
            break;

        default:
            break;
    }

    return data;
}

// Returns the header
QVariant GamedayData::headerData(int section, Qt::Orientation orientation, int role) const {
    qDebug() << "Header data called";
    return QVariant();
}
