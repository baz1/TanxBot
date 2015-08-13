#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include <QThread>

#include <signal.h>

class UserInterface : public QThread
{
public:
    UserInterface(QObject *parent = NULL);
    ~UserInterface();
protected:
    void run();
public:
    bool usingUI;
private:
    struct sigaction oldSig;
};

#endif // USERINTERFACE_H
