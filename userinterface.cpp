#include "userinterface.h"

#include <QtCore>
#include <stdio.h>

volatile bool hasInterrupt = false;
QMutex hIMutex;
QWaitCondition hICondition;


void intSignalHandler(int sig)
{
    Q_UNUSED(sig)
    hIMutex.lock();
    hasInterrupt = true;
    hICondition.wakeOne();
    hIMutex.unlock();
}

UserInterface::UserInterface(QObject *parent) : QThread(parent), usingUI(false), isAborting(false)
{
    struct sigaction newSig;
    newSig.sa_handler = intSignalHandler;
    sigemptyset(&newSig.sa_mask);
    sigaction(SIGINT, &newSig, &oldSig);
}

UserInterface::~UserInterface()
{
    sigaction(SIGINT, &oldSig, NULL);
    hasInterrupt = false;
}

void UserInterface::abort()
{
    if (isRunning())
    {
        hIMutex.lock();
        isAborting = true;
        hasInterrupt = true;
        hICondition.wakeOne();
        hIMutex.unlock();
    }
}

void UserInterface::run()
{
    while (true)
    {
        hIMutex.lock();
        while (!hasInterrupt)
            hICondition.wait(&hIMutex);
        hIMutex.unlock();
        if (isAborting)
            return;
        usingUI = true;
        printf("\n");
        printf("  0: Cancel\n");
        printf("  1: Activate\n");
        printf("  2: Deactivate\n");
        printf("  9: Exit\n");
        int action = -1;
        while (action)
        {
            printf("Action: ");
            fflush(stdout);
            scanf("%d", &action);
            switch (action)
            {
            case 0:
                break;
            case 1:
                emit setActivated(true);
                action = 0;
                break;
            case 2:
                emit setActivated(false);
                action = 0;
                break;
            case 9:
                return;
            default:
                printf("Error: Unknown action.\n");
            }
        }
        usingUI = false;
        hasInterrupt = false;
    }
}
