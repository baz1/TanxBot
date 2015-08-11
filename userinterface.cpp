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

UserInterface::UserInterface() : usingUI(false)
{
    struct sigaction newSig;
    newSig.sa_handler = intSignalHandler;
    sigemptyset(&newSig.sa_mask);
    sigaction(SIGINT, &newSig, &oldSig);
}

UserInterface::~UserInterface()
{
    sigaction(SIGINT, &oldSig, NULL);
}

void UserInterface::run()
{
    while (true)
    {
        hIMutex.lock();
        while (!hasInterrupt)
            hICondition.wait(&hIMutex);
        hIMutex.unlock();
        usingUI = true;
        printf("\n");
        printf("  0: Cancel\n");
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
