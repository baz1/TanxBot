#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <QThread>

#include <signal.h>

class UserInterface : public QThread
{
public:
    UserInterface(QObject *parent = NULL);
    ~UserInterface();
    void abort();
signals:
    void setActivated(bool enabled);
protected:
    void run();
public:
    bool usingUI;
private:
    struct sigaction oldSig;
    bool isAborting;
};

#endif // USERINTERFACE_H
