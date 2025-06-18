#ifndef BACKEND_H
#define BACKEND_H
#include <string>
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <memory.h>
#include <QDebug>

class backend
{
public:
    backend();

    struct POS
    {
        std::string nom;
        float xPos;
        float yPos;
        float zPos;
    }onlyup_position;


};

extern int nbPos;
extern backend::POS posList[500];

void loadDefaultPosition();
int addPosition(std::string name, float xPos, float yPos, float zPos);
void readPosition(DWORD targetProcessPid, DWORD_PTR xPosAdress, DWORD_PTR yPosAdress, DWORD_PTR zPosAdress, std::string name);
void setPos(DWORD targetProcessPid, DWORD_PTR xPosAdress, DWORD_PTR yPosAdress, DWORD_PTR zPosAdress, float x, float y, float z);

#endif // BACKEND_H
