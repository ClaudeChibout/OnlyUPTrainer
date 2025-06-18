#include "backend.h"

int nbPos = 0;
backend::POS posList[500];


backend::backend() {
}


int addPosition(std::string name, float xPos, float yPos, float zPos){
    posList[nbPos].nom = name;
    posList[nbPos].xPos = xPos;
    posList[nbPos].yPos = yPos;
    posList[nbPos].zPos = zPos;
    nbPos += 1;

    return 0;
}

void loadDefaultPosition(){
    addPosition("Debut", 5.5463, 6.56133, -5.19284);
    addPosition("voiture", 5.16819, 6.3269, -5.29905);
    addPosition("ascenseur", 4.81652, 5.47318, -5.24727);
    addPosition("tuyaux", 4.97328, 5.86563, 5.2307);
    addPosition("apres tuyaux", 5.32687, 6.36775, 5.84643);
    addPosition("rail", 5.57467, 6.40678, 6.11992);
    addPosition("petrol bed", 5.23776, 6.15151, 7.03154);
    addPosition("eolienne bed", -3.64278, 5.74427, 7.27035);
    addPosition("next bed", -3.91256, 6.26011, 7.44539);
    addPosition("bed to autoroute", 3.7882, 6.10143, 7.52186);
    addPosition("trampo place", -5.61598, 6.46126, 7.66371);
    addPosition("lingo", 4.08054, 5.94214, 7.72025);
    addPosition("win", 4.95325, 5.83284, 7.79837);
}

void readPosition(DWORD targetProcessPid, DWORD_PTR xPosAdress, DWORD_PTR yPosAdress, DWORD_PTR zPosAdress, std::string name){
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS,
                                  FALSE, targetProcessPid);

    float xPos;
    float yPos;
    float zPos;

    ReadProcessMemory(hProcess, (LPCVOID)zPosAdress, (LPVOID)&zPos, sizeof(zPos), NULL);
    ReadProcessMemory(hProcess, (LPCVOID)xPosAdress, (LPVOID)&xPos, sizeof(xPos), NULL);
    ReadProcessMemory(hProcess, (LPCVOID)yPosAdress, (LPVOID)&yPos, sizeof(yPos), NULL);

    qDebug() << "pos: "<< name << xPos << yPos << zPos <<Qt::endl;
    addPosition(name,  xPos,  yPos,  zPos);
    CloseHandle(hProcess);
}


void setPos(DWORD targetProcessPid, DWORD_PTR xPosAdress, DWORD_PTR yPosAdress, DWORD_PTR zPosAdress, float x, float y, float z){
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS,
                                  FALSE, targetProcessPid);

    float xPos = x;
    float yPos = y;
    float zPos = z;
    WriteProcessMemory(hProcess, (LPVOID)zPosAdress, (LPCVOID)&zPos, sizeof(zPos), NULL);
    WriteProcessMemory(hProcess, (LPVOID)xPosAdress, (LPCVOID)&xPos, sizeof(xPos), NULL);
    WriteProcessMemory(hProcess, (LPVOID)yPosAdress, (LPCVOID)&yPos, sizeof(yPos), NULL);
    CloseHandle(hProcess);


}
