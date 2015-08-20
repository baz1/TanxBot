#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <QThread>

#include <signal.h>

class UserInterface : public QThread
{
    Q_OBJECT
public:
    UserInterface(QObject *parent = NULL);
    ~UserInterface();
    void abort();
signals:
    void setActivated(bool enabled);
    void setMyName(QString name);
    void setFollowName(QString name);
    void setTargetName(QString name);
protected:
    void run() Q_DECL_OVERRIDE;
public:
    bool usingUI;
private:
    struct sigaction oldSig;
    bool isAborting;
};

#endif // USERINTERFACE_H
