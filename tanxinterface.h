#ifndef TANXINTERFACE_H
#define TANXINTERFACE_H

#include <QObject>
#include <QMap>
#include <QWebSocket>

enum Team {
    Blue = 0,
    Red = 1,
    Green = 2,
    Yellow = 3
};

struct Tank {
    int team;
    QString owner;
    double x, y;
    double dx, dy;
    int angle;
    int hp;
    int sp;
    bool dead;
    int killer;
    int score;
};

struct Pickable {
    enum {
        Repair,
        Damage,
        Shield
    } t;
    double r;
    double x, y;
};

struct Bullet {
    int tank;
    double x, y;
    double dx, dy;
    bool isRed;
    qint64 launched, expire;
};

struct GameData {
    QMap<QString, QString> users;
    QString mySSID;
    QMap<int, Tank> tanks;
    int myID;
    int teamScores[4];
    QMap<int, Pickable> pickables;
    QMap<int, Bullet> bullets;
};

class TanxInterface : public QObject
{
    Q_OBJECT
public:
    explicit TanxInterface(bool checkForExpiredBullets = true);
    ~TanxInterface();
    void setShooting(bool enabled = true);
    void setTarget(double angle);
    void setName(QString userName);
    void move(double dx, double dy);
    void targettedMove(double angle, double dx, double dy);
signals:
    void initialized();
    void gotUpdate();
    void endOfGame(int scoreTeam1, int scoreTeam2, int scoreTeam3, int scoreTeam4);
    void disconnected();
    void newBullet(int id, const Bullet &bullet);
    void delBullet(int id);
public slots:
    void endConnection();
private slots:
    void onConnected();
    void onDisconnected();
    void onTextReceived(QString str);
private:
    QString angleToStream(double angle);
    double streamToAngle(double sValue);
public:
    QWebSocket wSocket;
    GameData data;
    bool checkForExpiredBullets;
};

#endif // TANXINTERFACE_H
