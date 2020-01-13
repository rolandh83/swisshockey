/*
 * Copyright 2014-2017 Roland Hostettler
 *
 * This file is part of swisshockey.
 *
 * swisshockey is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * swisshockey is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * swisshockey. If not, see http://www.gnu.org/licenses/.
 */

#include "gamedata.h"
#include "logger.h"

// The list of game status texts
QStringList GameData::mGameStatusTexts = QStringList()
    << QString("Not Started")    // 0
    << QString("First Period")
    << QString("End of First")
    << QString("Second Period")
    << QString("End of Second")
    << QString("Third Period")   // 5
    << QString("End of Third")
    << QString("Overtime")
    << QString("Shootout")
    << QString("Final (unofficial)")
    << QString("Final (unofficial, overtime)") // 10
    << QString("Final (unofficial, shootout)")
    << QString("Final");

// Initialize the game
GameData::GameData(QString gameId, QObject *parent) : QAbstractListModel(parent) {
    // Store game ID
    mGameId = gameId;

    // Set the data roles
    // TODO: Consider renaming the roles at some point
    mDataRoleNames[TeamRole] = "eventteam";
    mDataRoleNames[TimeRole] = "eventtime";
    mDataRoleNames[PlayerRole] = "eventplayer";
    mDataRoleNames[AdditionalInfoRole] = "eventinfo";
    mDataRoleNames[EventRole] = "eventtext";
    mDataRoleNames[EventSubtextRole] = "eventsubtext";
}

// Update the game score/summary
void GameData::updateSummary(QVariantMap data) {
    // Update the score and trigger a signal if it changed
    QMap<QString, QString> oldScore = mScore;
    QVariantMap newScore = data["score"].toMap();
    mScore["first"] = newScore.value("first", "-:-").toString();
    mScore["second"] = newScore.value("second", "-:-").toString();
    mScore["third"] =  newScore.value("third", "-:-").toString();
    if(newScore.contains("overtime")) {
        mScore["overtime"] =  newScore.value("overtime", "-:-").toString();
    }
    mScore["total"] = newScore["total"].toString();
    if(mScore["total"] != oldScore["total"] && oldScore["total"] != "-:-") {
        emit scoreChanged();
    }

    // Update the game status and trigger a signal if it changed
    int oldStatus = mGameStatus;
    mGameStatus = data["status"].toInt();
    if(this->mGameStatus != oldStatus) {
        emit statusChanged();
    }
}

void GameData::updateEvents(QList<GameEvent *> events) {
    Logger& logger = Logger::getInstance();
    logger.log(Logger::DEBUG, QString(Q_FUNC_INFO).append(": Updating game events."));

    // Clear all events
    beginResetModel();
    mGameEvents.clear();
    endResetModel();

    // Update the list of events (if there are any)
    QListIterator<GameEvent *> iter(events);
    while(iter.hasNext()) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        mGameEvents.append(iter.next());
        endInsertRows();
    }

    // Sort the events
    // TODO: Consider sorting upside down when the game is in progress and increasing when the game is finished
    layoutAboutToBeChanged();
    qSort(mGameEvents.begin(), mGameEvents.end(), GameEvent::greaterThan);
    layoutChanged();
    logger.log(Logger::DEBUG, QString(Q_FUNC_INFO).append(": Added " + QString::number(mGameEvents.size()) + " events."));
}

#if 0
// Parses the player list and adds them to the local list of players where we
// have a player license <=> player name map
void GameData::updateRosters(QList<Player *> players) {
    Logger& logger = Logger::getInstance();
    logger.log(Logger::DEBUG, QString(Q_FUNC_INFO).append(": Updating rosters."));

    mRoster.clear();
    QListIterator<Player *> iter(players);
    while(iter.hasNext()) {
        Player *player = iter.next();
        mRoster.insert(player->getPlayerId(), player);
    }

    logger.log(Logger::DEBUG, QString(Q_FUNC_INFO).append(": Added " + QString::number(mRoster.size()) + " players to roster."));
}
#endif

QString GameData::getGameId(void){
    return mGameId;
}

void GameData::setLeague(QString leagueId) {
    mLeagueId = leagueId;
}

QString GameData::getLeague() {
    return this->mLeagueId;
}

void GameData::setDateTime(QString time) {
    mStartTime = time;
}

void GameData::setHometeam(QString id, QString name) {
    mHometeamId = id.toLongLong();
    mHometeamName = name;
}

QString GameData::getHometeam() {
    return mHometeamName;
}

QString GameData::getHometeamId() {
    return QString::number(this->mHometeamId);
}

void GameData::setAwayteam(QString id, QString name) {
    mAwayteamId = id.toLongLong();
    mAwayteamName = name;
}

QString GameData::getAwayteam() {
    return this->mAwayteamName;
}

QString GameData::getAwayteamId() {
    return QString::number(this->mAwayteamId);
}

void GameData::setScore(QMap<QString, QString> score) {
    // Update the score and trigger a signal if it changed
    QMap<QString, QString> oldScore = mScore;
    mScore = score;
    if(mScore["total"] != oldScore["total"] && oldScore["total"] != "-:-") {
        emit scoreChanged();
    }
}

QString GameData::getTotalScore() {
    return this->mScore["total"];
}

QString GameData::getPeriodsScore() {
    QString score = mScore.value("first") + ", " + mScore.value("second") + ", " + mScore.value("third");

    if(mScore.contains("overtime")) {
        score.append(", " + this->mScore.value("overtime"));
    }

    return score;
}

void GameData::setStatus(int status) {
    // Update the game status and trigger a signal if it changed
    int oldStatus = mGameStatus;
    mGameStatus = status;
    if(mGameStatus != oldStatus) {
        emit statusChanged();
    }
}

int GameData::getGameStatus() {
    return this->mGameStatus;
}

QString GameData::getGameStatusText() {
    QString text;
    if(this->mGameStatus == 0) {
        text = "Starts " + this->mStartTime;
    } else {
        text = GameData::mGameStatusTexts.value(this->mGameStatus, "Unknown status");
    }

    return text;
}

QMap<quint32, Player *> *GameData::getRoster() {
    return &mRoster;
}


// Implementation of the QAbstractListModel methods
// Returns the number of rows in the list
int GameData::rowCount(const QModelIndex &parent) const {
    return this->mGameEvents.count();
}

// Returns the data requested by the view
QVariant GameData::data(const QModelIndex &index, int role) const {
    QVariant data;
    int key = index.row();
    GameEvent *event = this->mGameEvents[key];

    switch(role) {
        case TeamRole:
            data = event->getTeam();
            break;

        case TimeRole: {
                float tmp = event->getTime();
                quint32 minutes = (int)tmp/60;
                quint32 seconds = (int)(tmp-minutes*60);
                QString str;
                str.sprintf("%02u:%02u", minutes, seconds);
                data = str;
            }
            break;

        case PlayerRole: {
                quint32 id = event->getPlayer();
                Player *player = mRoster.value(id, NULL);
//                Player *player
                if(player != NULL) {
                    data = player->getName();
                } else {
                    data = "";
                }
            }
            break;

        case AdditionalInfoRole: {
                int type = event->getType();
                switch(type) {
                    case GameEvent::GOAL: {
                            QString text = "";
                            Player *player = mRoster.value(event->getPlayer(GameEvent::FIRST_ASSIST), NULL);
                            if(player != NULL) {
                                text.append(player->getName());
                            }
                            player = mRoster.value(event->getPlayer(GameEvent::SECOND_ASSIST), NULL);
                            if(player != NULL) {
                                text.append(", " + player->getName());
                            }
                            data = text;
                        }
                        break;

                    case GameEvent::PENALTY:
                        data = GameEvent::getPenaltyText(event->getPenalty());
                        break;

                    case GameEvent::GOALKEEPER_IN:
                        data = "Goalkeeper in";
                        break;

                    case GameEvent::GOALKEEPER_OUT:
                        data = "Goalkeeper out";
                        break;

                    case GameEvent::PENALTY_SHOT:
                        data = "Goalkeeper: "; //+ this->players.value(event->getPlayer(GameEvent::GOALKEEPER), "");
                        break;

                    default:
                        break;
                }
            }
            break;

        case EventRole:
            data = event->getEvent();
            break;

        case EventSubtextRole:
            data = event->getEventSubtext();
            break;

        default:
            break;
    }

    return data;
}

// Returns the header
QVariant GameData::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}

// Role names for QML
QHash<int, QByteArray> GameData::roleNames() const {
    return this->mDataRoleNames;
}
