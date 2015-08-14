#include "tanxinterface.h"

#include "tanxmap.h"

#include <QtCore>

#include <stdio.h>
#include <math.h>

TanxInterface::TanxInterface(QObject *parent, bool checkForExpiredBullets)
    : QObject(parent), checkForExpiredBullets(checkForExpiredBullets), endedConnection(false), isShooting(false)
{
    connect(&wSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&wSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(&wSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&wSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextReceived(QString)));
    data.myID = -1;
    wSocket.open(QUrl("wss://tanx.playcanvas.com/socket/520/xkzs4vvy/websocket"));
}

TanxInterface::~TanxInterface()
{
    wSocket.close();
}

void TanxInterface::setShooting(bool enabled)
{
    if (enabled)
        wSocket.sendTextMessage("[\"{\\\"n\\\":\\\"shoot\\\",\\\"d\\\":true}\"]");
    else
        wSocket.sendTextMessage("[\"{\\\"n\\\":\\\"shoot\\\",\\\"d\\\":false}\"]");
    isShooting = enabled;
}

void TanxInterface::setTarget(double angle)
{
    wSocket.sendTextMessage(QStringLiteral("[\"{\\\"n\\\":\\\"target\\\",\\\"d\\\":") + angleToStream(angle) + QStringLiteral("}\"]"));
}

void TanxInterface::setTarget(double angle, bool shootingEnabled)
{
    if (shootingEnabled)
    {
        QString str;
        if (isShooting)
        {
            str = QStringLiteral("[\"{\\\"n\\\":\\\"target\\\",\\\"d\\\":") + angleToStream(angle) + QStringLiteral("}\"]");
        } else {
            str = QStringLiteral("[\"{\\\"n\\\":\\\"target\\\",\\\"d\\\":") + angleToStream(angle)
                    + QStringLiteral("}\",\"{\\\"n\\\":\\\"shoot\\\",\\\"d\\\":true}\"]");
            isShooting = true;
        }
        wSocket.sendTextMessage(str);
    } else {
        if (isShooting)
        {
            wSocket.sendTextMessage("[\"{\\\"n\\\":\\\"shoot\\\",\\\"d\\\":false}\"]");
            isShooting = false;
        }
    }
}

void TanxInterface::setName(QString userName)
{
    wSocket.sendTextMessage(QStringLiteral("[\"{\\\"n\\\":\\\"user.name\\\",\\\"d\\\":\\\"") + userName + QStringLiteral("\\\"}\"]"));
}

void TanxInterface::move(double dx, double dy)
{
    QString str = QStringLiteral("[\"{\\\"n\\\":\\\"move\\\",\\\"d\\\":[") + QString::number(dx)
            + QStringLiteral(",") + QString::number(dy) + QStringLiteral("]}\"]");
    wSocket.sendTextMessage(str);
}

void TanxInterface::targettedMove(double angle, double dx, double dy, bool shootingEnabled)
{
    QString str;
    if (shootingEnabled)
    {
        if (isShooting)
        {
            str = QStringLiteral("[\"{\\\"n\\\":\\\"move\\\",\\\"d\\\":[") + QString::number(dx)
                    + QStringLiteral(",") + QString::number(dy) + QStringLiteral("]}\",\"{\\\"n\\\":\\\"target\\\",\\\"d\\\":")
                    + angleToStream(angle) + QStringLiteral("}\"]");
        } else {
            str = QStringLiteral("[\"{\\\"n\\\":\\\"move\\\",\\\"d\\\":[") + QString::number(dx)
                    + QStringLiteral(",") + QString::number(dy) + QStringLiteral("]}\",\"{\\\"n\\\":\\\"target\\\",\\\"d\\\":")
                    + angleToStream(angle) + QStringLiteral("}\",\"{\\\"n\\\":\\\"shoot\\\",\\\"d\\\":true}\"]");
            isShooting = true;
        }
    } else {
        if (isShooting)
        {
            str = QStringLiteral("[\"{\\\"n\\\":\\\"move\\\",\\\"d\\\":[") + QString::number(dx)
                    + QStringLiteral(",") + QString::number(dy) + QStringLiteral("]}\",\"{\\\"n\\\":\\\"shoot\\\",\\\"d\\\":false}\"]");
            isShooting = false;
        } else {
            str = QStringLiteral("[\"{\\\"n\\\":\\\"move\\\",\\\"d\\\":[") + QString::number(dx)
                    + QStringLiteral(",") + QString::number(dy) + QStringLiteral("]}\"]");
        }
    }
    wSocket.sendTextMessage(str);
}

void TanxInterface::onConnected()
{
    printf("Info: Connected.\n");
    fflush(stdout);
}

void TanxInterface::onDisconnected()
{
    if (!endedConnection)
        fprintf(stderr, "Error: Unexpected disconnection.\n");
    emit disconnected();
    //QCoreApplication::instance()->quit();
}

void TanxInterface::endConnection()
{
    endedConnection = true;
    wSocket.close();
}

void TanxInterface::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    fprintf(stderr, "QWebSocket error: %s\n", qPrintable(wSocket.errorString()));
}

void TanxInterface::onTextReceived(QString str)
{
    int index;
    QString name;
    QJsonDocument doc;
    QJsonObject obj;
    QJsonValue val, val2;
    QJsonArray array;
    Tank tank;
    Pickable pickable;
    Bullet bullet;
    if (str.startsWith("a[\"") && str.endsWith("\"]"))
    {
        str = str.mid(3, str.length() - 5);
        str.replace("\\\"", "\"");
        doc = QJsonDocument::fromJson(str.toLocal8Bit());
        if (doc.isNull())
            goto unknown_msg;
        obj = doc.object();
        val = obj.value("n");
        if (!val.isString())
            goto unknown_msg;
        name = val.toString();
        val = obj.value("d");
        if (val.isUndefined())
            goto unknown_msg;
        if (name == "update")
        {
            if (!val.isObject())
                goto unknown_msg;
            obj = val.toObject();
            val = obj.value("tanks");
            if (!val.isUndefined())
            {
                if (!val.isArray())
                    goto unknown_msg;
                array = val.toArray();
                for (index = array.count(); index > 0;)
                {
                    val = array.at(--index);
                    if (!val.isObject())
                        goto unknown_msg;
                    val2 = val.toObject().value("id");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    if (!data.tanks.contains(val2.toInt()))
                    {
                        fprintf(stderr, "Error: Unknown tank (id:%d)\n", val2.toInt());
                        continue;
                    }
                    Tank &rtank = data.tanks[val2.toInt()];
                    val2 = val.toObject().value("x");
                    if (!val2.isUndefined())
                    {
                        if (val2.isNull())
                        {
                            rtank.dx = -rtank.x;
                            rtank.x = 0;
                        } else if (val2.isDouble())
                        {
                            rtank.dx = val2.toDouble() - rtank.x;
                            rtank.x = val2.toDouble();
                        } else {
                            goto unknown_msg;
                        }
                    }
                    val2 = val.toObject().value("y");
                    if (!val2.isUndefined())
                    {
                        if (val2.isNull())
                        {
                            rtank.dy = -rtank.y;
                            rtank.y = 0;
                        } else if (val2.isDouble())
                        {
                            rtank.dy = val2.toDouble() - rtank.y;
                            rtank.y = val2.toDouble();
                        } else {
                            goto unknown_msg;
                        }
                    }
                    if ((tank.dx != 0) || (tank.dy != 0))
                    {
                        double distinv = (TANK_SPEED / BULLET_SPEED) / sqrt(tank.dx * tank.dx + tank.dy * tank.dy);
                        tank.dx *= distinv;
                        tank.dy *= distinv;
                    }
                    val2 = val.toObject().value("a");
                    if (!val2.isUndefined())
                    {
                        if (!val2.isDouble())
                            goto unknown_msg;
                        rtank.angle = streamToAngle(val2.toDouble());
                    }
                    val2 = val.toObject().value("hp");
                    if (!val2.isUndefined())
                    {
                        if (!val2.isDouble())
                            goto unknown_msg;
                        rtank.hp = val2.toInt();
                    }
                    val2 = val.toObject().value("sp");
                    if (!val2.isUndefined())
                    {
                        if (!val2.isDouble())
                            goto unknown_msg;
                        rtank.sp = val2.toInt();
                    }
                    val2 = val.toObject().value("s");
                    if (!val2.isUndefined())
                    {
                        if (!val2.isDouble())
                            goto unknown_msg;
                        rtank.sp = val2.toInt();
                    }
                    val2 = val.toObject().value("dead");
                    if (!val2.isUndefined())
                    {
                        if (!val2.isBool())
                            goto unknown_msg;
                        rtank.dead = val2.toBool();
                        val2 = val.toObject().value("killer");
                        if (!val2.isUndefined())
                        {
                            if (val2.isNull())
                            {
                                rtank.killer = -1;
                            } else {
                                if (!val2.isDouble())
                                    goto unknown_msg;
                                rtank.killer = val2.toInt();
                            }
                        }
                    } else {
                        rtank.dead = false;
                    }
                }
            }
            obj.remove("tanks");
            val = obj.value("pickable");
            if (!val.isUndefined())
            {
                if (!val.isArray())
                    goto unknown_msg;
                array = val.toArray();
                for (index = array.count(); index > 0;)
                {
                    val = array.at(--index);
                    if (!val.isObject())
                        goto unknown_msg;
                    val2 = val.toObject().value("t");
                    if (!val2.isString())
                        goto unknown_msg;
                    if (val2.toString() == "damage")
                    {
                        pickable.t = Pickable::Damage;
                    } else if (val2.toString() == "repair")
                    {
                        pickable.t = Pickable::Repair;
                    } else if (val2.toString() == "shield")
                    {
                        pickable.t = Pickable::Shield;
                    } else {
                        goto unknown_msg;
                    }
                    val2 = val.toObject().value("r");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    pickable.r = val2.toDouble();
                    val2 = val.toObject().value("x");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    pickable.x = val2.toDouble();
                    val2 = val.toObject().value("y");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    pickable.y = val2.toDouble();
                    val2 = val.toObject().value("id");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    data.pickables[val2.toInt()] = pickable;
                }
            }
            obj.remove("pickable");
            val = obj.value("pickableDelete");
            if (!val.isUndefined())
            {
                if (!val.isArray())
                    goto unknown_msg;
                array = val.toArray();
                for (index = array.count(); index > 0;)
                {
                    val = array.at(--index);
                    if (!val.isObject())
                        goto unknown_msg;
                    val2 = val.toObject().value("id");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    data.pickables.remove(val2.toInt());
                }
            }
            obj.remove("pickableDelete");
            val = obj.value("bullets");
            if (!val.isUndefined())
            {
                if (!val.isArray())
                    goto unknown_msg;
                array = val.toArray();
                for (index = array.count(); index > 0;)
                {
                    val = array.at(--index);
                    if (!val.isObject())
                        goto unknown_msg;
                    val2 = val.toObject().value("tank");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    bullet.tank = val2.toInt();
                    if (data.tanks[bullet.tank].team == data.myTeam)
                        continue;
                    val2 = val.toObject().value("x");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    bullet.x = val2.toDouble();
                    val2 = val.toObject().value("y");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    bullet.y = val2.toDouble();
                    val2 = val.toObject().value("tx");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    bullet.dx = val2.toDouble();
                    bullet.dx -= bullet.x;
                    val2 = val.toObject().value("ty");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    bullet.dy = val2.toDouble();
                    bullet.dy -= bullet.y;
                    val2 = val.toObject().value("s");
                    if (val2.isUndefined())
                    {
                        bullet.isRed = false;
                    } else if (val2.isBool() && val2.toBool())
                    {
                        bullet.isRed = true;
                    } else {
                        goto unknown_msg;
                    }
                    val2 = val.toObject().value("id");
                    if (!val2.isDouble())
                        goto unknown_msg;
                    double len = sqrt(bullet.dx * bullet.dx + bullet.dy * bullet.dy);
                    bullet.dx /= len;
                    bullet.dy /= len;
                    bullet.duration = TanxMap::getDuration(bullet.x, bullet.y, bullet.dx, bullet.dy);
                    bullet.launched = QDateTime::currentMSecsSinceEpoch();
                    bullet.expire = bullet.launched + (qint64) (bullet.duration * 1000. / BULLET_SPEED);
                    emit newBullet(val2.toInt(), data.bullets[val2.toInt()] = bullet);
                }
            }
            obj.remove("bullets");
            val = obj.value("bulletsDelete");
            if (!val.isUndefined())
            {
                if (!val.isArray())
                    goto unknown_msg;
                array = val.toArray();
                for (index = array.count(); index > 0;)
                {
                    val = array.at(--index);
                    if (!val.isObject())
                        goto unknown_msg;
                    val = val.toObject().value("id");
                    if (!val.isDouble())
                        goto unknown_msg;
                    data.bullets.remove(val.toInt());
                }
            }
            obj.remove("bulletsDelete");
            if (checkForExpiredBullets)
            {
                QMap<int, Bullet>::iterator bulletIter = data.bullets.begin();
                qint64 now = QDateTime::currentMSecsSinceEpoch();
                while (bulletIter != data.bullets.end())
                {
                    if (now >= bulletIter.value().expire)
                    {
                        emit delBullet(bulletIter.key());
                        bulletIter = data.bullets.erase(bulletIter);
                    } else {
                        ++bulletIter;
                    }
                }
            }
            val = obj.value("teams");
            if (!val.isUndefined())
            {
                if (!val.isArray())
                    goto unknown_msg;
                array = val.toArray();
                if (array.count() != 4)
                    goto unknown_msg;
                val = array.at(0);
                if (!val.isDouble())
                    goto unknown_msg;
                data.teamScores[0] = val.toInt();
                val = array.at(1);
                if (!val.isDouble())
                    goto unknown_msg;
                data.teamScores[1] = val.toInt();
                val = array.at(2);
                if (!val.isDouble())
                    goto unknown_msg;
                data.teamScores[2] = val.toInt();
                val = array.at(3);
                if (!val.isDouble())
                    goto unknown_msg;
                data.teamScores[3] = val.toInt();
            }
            obj.remove("teams");
            val = obj.value("winner");
            if (!val.isUndefined())
            {
                if (!val.isObject())
                    goto unknown_msg;
                val = val.toObject().value("scores");
                if (!val.isArray())
                    goto unknown_msg;
                array = val.toArray();
                if (array.count() != 4)
                    goto unknown_msg;
                emit endOfGame(array.at(0).toInt(0), array.at(1).toInt(0), array.at(2).toInt(0), array.at(3).toInt(0));
            }
            obj.remove("winner");
            if (!obj.isEmpty())
                goto unknown_msg;
            if (data.myID >= 0)
                emit gotUpdate();
        } else if (name == "tank.new")
        {
            if (!val.isObject())
                goto unknown_msg;
            val2 = val.toObject().value("team");
            if (!val2.isDouble())
                goto unknown_msg;
            tank.team = val2.toInt();
            val2 = val.toObject().value("owner");
            if (!val2.isString())
                goto unknown_msg;
            tank.owner = val2.toString();
            val2 = val.toObject().value("pos");
            if (!val2.isArray())
                goto unknown_msg;
            array = val2.toArray();
            if (array.count() != 2)
                goto unknown_msg;
            val2 = array.at(0);
            if (!val2.isDouble())
                goto unknown_msg;
            tank.x = val2.toDouble();
            val2 = array.at(1);
            if (!val2.isDouble())
                goto unknown_msg;
            tank.y = val2.toDouble();
            val2 = val.toObject().value("hp");
            if (!val2.isDouble())
                goto unknown_msg;
            tank.hp = val2.toInt();
            val2 = val.toObject().value("shield");
            if (!val2.isDouble())
                goto unknown_msg;
            tank.sp = val2.toInt();
            val2 = val.toObject().value("dead");
            if (!val2.isBool())
                goto unknown_msg;
            tank.dead = val2.toBool();
            tank.killer = -1;
            val2 = val.toObject().value("score");
            if (!val2.isDouble())
                goto unknown_msg;
            tank.score = val2.toInt();
            val2 = val.toObject().value("id");
            if (!val2.isDouble())
                goto unknown_msg;
            tank.id = val2.toInt();
            data.tanks[tank.id] = tank;
            if (tank.owner == data.mySSID)
            {
                printf("Initialization complete.\n");
                fflush(stdout);
                data.myID = tank.id;
                data.myTeam = tank.team;
                emit initialized();
            }
        } else if (name == "tank.delete")
        {
            if (!val.isObject())
                goto unknown_msg;
            val = val.toObject().value("id");
            if (!val.isDouble())
                goto unknown_msg;
            data.tanks.remove(val.toInt());
        } else if ((name == "user.add") || (name == "user.name"))
        {
            if (!val.isObject())
                goto unknown_msg;
            val2 = val.toObject().value("id");
            if (!val2.isString())
                goto unknown_msg;
            val = val.toObject().value("name");
            if (!val.isString())
                goto unknown_msg;
            data.users[val2.toString()] = val.toString();
        } else if (name == "user.remove")
        {
            if (!val.isObject())
                goto unknown_msg;
            val = val.toObject().value("id");
            if (!val.isString())
                goto unknown_msg;
            data.users.remove(val.toString());
        } else if (name == "user.sync")
        {
            if (!val.isArray())
                goto unknown_msg;
            data.users.clear();
            array = val.toArray();
            for (index = array.count(); index > 0;)
            {
                val = array.at(--index);
                if (!val.isObject())
                    goto unknown_msg;
                val2 = val.toObject().value("id");
                if (!val2.isString())
                    goto unknown_msg;
                val = val.toObject().value("name");
                if (!val.isString())
                    goto unknown_msg;
                data.users[val2.toString()] = val.toString();
            }
        } else if (name == "init")
        {
            if (!val.isObject())
                goto unknown_msg;
            val = val.toObject().value("id");
            if (!val.isString())
                goto unknown_msg;
            data.mySSID = val.toString();
        } else {
            goto unknown_msg;
        }
    } else if ((str == "o") || (str == "h"))
    {
        qt_noop();
    } else {
        goto unknown_msg;
    }
    return;
unknown_msg:
    printf("Received: %s\n", qPrintable(str));
    fflush(stdout);
}

QString TanxInterface::angleToStream(double angle)
{
    angle = fmod(angle, 6.283185307179586476925);
    angle *= 57.2957795130823208768;
    int iAngle = (int) lround(angle);
    if (iAngle > 180)
        iAngle -= 360;
    if (iAngle <= -180)
        iAngle += 360;
    return QString::number(iAngle);
}

double TanxInterface::streamToAngle(double sValue)
{
    return sValue * 0.01745329251994329577;
}
