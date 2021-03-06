#ifndef TANXINTERFACE_H
#define TANXINTERFACE_H

#include <QObject>
#include <QMap>
#include <QWebSocket>

#define BULLET_SPEED    16.
#define TANK_SPEED      6.

#define TEAM_BLUE   0
#define TEAM_RED    1
#define TEAM_GREEN  2
#define TEAM_YELLOW 3

#define RESPAWN_REPAIR      5000
#define RESPAWN_DAMAGE      5000
#define RESPAWN_SHIELD      15000

struct Tank
{
    int id;
    int team;
    QString owner;
    double x, y;
    double dx, dy; // Relative to bullet speed (norm: TANK_SPEED / BULLET_SPEED)
    int angle;
    int hp;
    int sp;
    bool dead;
    int killer;
    int score;
};

struct Pickable
{
    enum PickableType {
        Repair = 0,
        Damage = 1,
        Shield = 2
    } t;
    double x, y;
    qint64 respawn_timestamp;
    inline Pickable() {}
    inline Pickable(PickableType t, double x, double y, qint64 now) : t(t), x(x), y(y)
    {
        init(now);
    }
    inline void init(qint64 now)
    {
        switch (t)
        {
        case Repair:
            respawn_timestamp = now + RESPAWN_REPAIR;
            return;
        case Damage:
            respawn_timestamp = now + RESPAWN_DAMAGE;
            return;
        case Shield:
            respawn_timestamp = now + RESPAWN_SHIELD;
            return;
        }
    }
};

struct Bullet
{
    int tank;
    double x, y;
    double dx, dy; // Unit length
    bool isRed;
    double duration;
    qint64 launched, expire;
};

struct GameData
{
    QMap<QString, QString> users;
    QString mySSID;
    QMap<int, Tank> tanks;
    int myID, myTeam;
    int teamScores[4];
    QMap<int, int> currentPickables;
    QMap<int, Bullet> bullets;
    Pickable pickables[9];
};

class TanxInterface : public QObject
{
    Q_OBJECT
public:
    explicit TanxInterface(QObject *parent = NULL, bool checkForExpiredBullets = true);
    ~TanxInterface();
    void setShooting(bool enabled = true);
    void setTarget(double angle);
    void setTarget(double angle, bool shootingEnabled);
    void setName(QString userName);
    void move(double dx, double dy);
    void targettedMove(double angle, double dx, double dy, bool shootingEnabled);
    bool isConnected() const;
signals:
    void initialized();
    void gotUpdate();
    void endOfGame(int scoreTeam1, int scoreTeam2, int scoreTeam3, int scoreTeam4);
    void disconnected();
    void newBullet(int id, const Bullet &bullet);
    void delBullet(int id);
    void newTank(const Tank &tank);
    void delTank(int id);
    void newUserName(QString id, QString name);
public slots:
    void endConnection();
private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onTextReceived(QString str);
private:
    QString angleToStream(double angle);
    double streamToAngle(double sValue);
public:
    QWebSocket wSocket;
    GameData data;
    bool checkForExpiredBullets;
    bool endedConnection;
    bool isShooting;
};

#endif // TANXINTERFACE_H
