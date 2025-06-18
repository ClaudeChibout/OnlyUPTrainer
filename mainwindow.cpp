#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "backend.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <memory.h>

#include <QListWidgetItem>
#include <QPushButton>
#include <QAbstractButton>

#include <QThread>



DWORD targetProcessPid = 0;
DWORD_PTR baseAddress;
DWORD_PTR xPosAdress;
DWORD_PTR yPosAdress;
DWORD_PTR zPosAdress;

backend::POS selected;


QPushButton * loadGameButton = nullptr;


class HotkeyThread : public QThread
{
    Q_OBJECT
public:
    explicit HotkeyThread(QObject *parent = nullptr) : QThread(parent) {}

    void run() override {
        // 1. Enregistrer le raccourci global
        if(!RegisterHotKey(
                NULL,             // Pas de fenêtre associée
                1,                // ID du hotkey
                MOD_ALT | MOD_NOREPEAT, // Ctrl
                0x45))            // 'E' (0x45 est le code virtuel)
        {
            qDebug() << "Erreur d'enregistrement:" << GetLastError();
            return;
        }

        qDebug() << "Raccourci Ctrl+E enregistré";

        // 2. Boucle de messages
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) != 0) {
            if (msg.message == WM_HOTKEY) {
                // Envoyer un signal à l'application principale
                emit hotkeyPressed();
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

signals:
    void hotkeyPressed();
};


int getGamePID(TCHAR * gameName){
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    int PID = 0;

    // récupère la liste des identifiers des processus
    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded)){
        return 1;
    }

    cProcesses = cbNeeded /sizeof(DWORD);

    DWORD cb;
    HANDLE process;
    HMODULE hmodule;
    TCHAR ProcessName[MAX_PATH] = TEXT("<unknown>");
    for (i = 0; i<cProcesses; i++){
        process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcesses[i]);

        EnumProcessModules(process, &hmodule, sizeof(hmodule), NULL);

        GetModuleBaseName(process, hmodule, ProcessName, sizeof(ProcessName)/sizeof(TCHAR));

        CloseHandle(process);
        if (_tcscmp(ProcessName, gameName) == 0){
            printf("%s\t%d\n", ProcessName, aProcesses[i]);
            if (PID < aProcesses[i]){
                PID = aProcesses[i];
            }
        }
    }

    return PID;
}

int FindPID(){
    TCHAR gameName[] = TEXT("OnlyUP.exe");
    int PID = getGamePID(gameName);

    if (PID==0){
        printf("GameProcessus not found ");
    }else{
        printf("Le PID du processus du jeu %s est %d", gameName, PID);
    }

    targetProcessPid = PID;
    return 0;
}

int FindBaseAdress(){
    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS,
                                  FALSE, targetProcessPid);


    // on cherche la base adresse

    HMODULE hModule;

    DWORD cbNeeded;

    // Obtenir le premier module (exécutable principal)
    if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded)) {
        MODULEINFO moduleInfo;

        // Récupérer les infos du module
        if (GetModuleInformation(hProcess, hModule, &moduleInfo, sizeof(moduleInfo))) {
            printf("Base Address: 0x%p\n", moduleInfo.lpBaseOfDll);
            baseAddress = (DWORD_PTR)moduleInfo.lpBaseOfDll;
            zPosAdress = baseAddress + 0x0F8F1A08;
            xPosAdress = baseAddress + 0x0F5A9500;
            yPosAdress = baseAddress + 0x0FA7A140;
        } else {
            printf("Erreur GetModuleInformation: %lu\n", GetLastError());
        }
    } else {
        printf("Erreur EnumProcessModules: %lu\n", GetLastError());
    }


    printf("Real Base Address: 0x%p\n",baseAddress);

    DWORD_PTR buffer = 0;
    SIZE_T bytesRead;
    int zOffsets[] = {0xA70, 0x50, 0x60, 0x6C8, 0x8, 0x78, 0x274};
    int xOffsets[] = {0x108, 0xB30, 0x368, 0x108, 0x220, 0x180, 0x834};
    int yOffsets[] = {0x30, 0x2F0, 0x180, 0x280, 0xA8, 0xB0, 0x26C};
    for (int i = 0; i<7; i++){
        ReadProcessMemory(hProcess, (LPCVOID)zPosAdress, &buffer, sizeof(buffer), &bytesRead);
        zPosAdress = buffer+zOffsets[i];
        ReadProcessMemory(hProcess, (LPCVOID)xPosAdress, &buffer, sizeof(buffer), &bytesRead);
        xPosAdress = buffer+xOffsets[i];
        ReadProcessMemory(hProcess, (LPCVOID)yPosAdress, &buffer, sizeof(buffer), &bytesRead);
        yPosAdress = buffer+yOffsets[i];
    }
    CloseHandle(hProcess);
    return 0;
}

void MainWindow::processLoaded(){
    FindPID();

    if (targetProcessPid == 0){
        if (loadGameButton == nullptr){
            loadGameButton = new QPushButton("Link Game Application", ui->centralwidget);
            ui->pushButton_save->hide();
            ui->pushButton_tp->hide();
            ui->listWidget->hide();
            ui->textEdit->hide();
            loadGameButton->setGeometry(100,20,200,50);
            loadGameButton->show();
            QObject::connect(loadGameButton, &QPushButton::clicked, this, &MainWindow::processLoaded);
        }

    }
    else{
        delete loadGameButton;
        loadGameButton = nullptr;
        FindBaseAdress();
        ui->pushButton_save->show();
        ui->pushButton_tp->show();
        ui->listWidget->show();
        ui->textEdit->show();
    }
}

HotkeyThread hotkeyThread;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "nomSelected: "<< selected.nom <<Qt::endl;
    ui->setupUi(this);

    // Créer un raccourci global
    // Créer et démarrer le thread
    QObject::connect(&hotkeyThread, &HotkeyThread::hotkeyPressed, [this]() {
        MainWindow::on_tp();
    });

    hotkeyThread.start();

    processLoaded();
    loadDefaultPosition();
    for (int i = 0; i<nbPos; i++){
        backend::POS tmp = posList[i];
        ui->listWidget->addItem(QString::fromStdString(tmp.nom));
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::refreshList(){
    ui->listWidget->clear();
    for (int i = 0; i<nbPos; i++){
        if (selected.nom == posList[i].nom){
            ui->listWidget->addItem(QString::fromStdString(selected.nom + " [selected]"));
        }else{
            ui->listWidget->addItem(QString::fromStdString(posList[i].nom));
        }
    }
}


void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    // si on clique sur la position selectionné -> rien faire
    if (item->text().contains("[selected]")) return;

    std::string itemName = item->text().toStdString();

    // sinon on update la position selectionné et la listWidget
    ui->listWidget->clear();
    for (int i = 0; i<nbPos; i++){
        if (itemName == posList[i].nom){
            selected = posList[i];
            ui->listWidget->addItem(QString::fromStdString(selected.nom + " [selected]"));
        }else{
            ui->listWidget->addItem(QString::fromStdString(posList[i].nom));
        }
    }
}



void MainWindow::on_tp(){
    if (selected.nom != ""){
        setPos(targetProcessPid, xPosAdress, yPosAdress, zPosAdress, selected.xPos, selected.yPos,selected.zPos);
    }
}

void MainWindow::on_pushButton_save_clicked()
{
    if (ui->textEdit->toPlainText().toStdString() != ""){
        readPosition(targetProcessPid, xPosAdress, yPosAdress, zPosAdress, ui->textEdit->toPlainText().toStdString());
        ui->textEdit->clear();
        refreshList();
    }
}

void MainWindow::on_pushButton_tp_clicked()
{
    MainWindow::on_tp();
}


#include "mainwindow.moc"
