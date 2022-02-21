#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Global.h"

#include <chrono>
#include <thread>
#include <utility>

extern string rand_from = "";
extern string rand_to = "";
extern string echo_from = "";
extern string echo_to = "";
extern string time_from = "";
extern string time_to = "";

string tmp_rand_from = "";
string tmp_rand_to = "";
string tmp_echo_from = "";
string tmp_echo_to = "";
string tmp_time_from = "";
string tmp_time_to = "";

int* getHoursAndMinute(string str);
const string currentDateTim();

template<class Func, class ...Args>
void run(int msec, Func func, Args &&...args) {
    auto endless = [=](Args &&...args) {
        while (true) {
            func(args...);
            std::this_thread::sleep_for(std::chrono::milliseconds(msec));
        }
    };

    std::thread thread(endless, args...);

    thread.detach();
}

void foo() {
    std::cout << "Schedule updated";
    rand_from = tmp_rand_from;
    rand_to = tmp_rand_to;
    echo_from = tmp_echo_from;
    echo_to = tmp_echo_to;
    time_from = tmp_time_from;
    time_to = tmp_time_to;
}

DWORD WINAPI ServiceWorkTime(LPVOID pPrm) {
    cout << "ServiceWorkTime started\n" << endl;
    DWORD rc = 0;
    try {

        HANDLE mail;
        char swt_name[50];
        char readbuf[50];
        DWORD nBytesRead;
        string mailname = "\\\\.\\mailslot\\box";
        string last_time = currentDateTim();

        if ((mail = CreateMailslot(mailname.c_str(), 300, 200000, NULL)) == INVALID_HANDLE_VALUE) {
            throw SetErrorMsgText("Create:", GetLastError());
        }

        run(60000, foo);

        while (*((TalkersCmd*)pPrm) != Exit) {
            if (ReadFile(mail, readbuf, sizeof(readbuf), &nBytesRead, NULL)) {
                string str = readbuf;
                if (rand_from == "") {
                    rand_from = str.substr(0, 5);
                    rand_to = str.substr(6, 5);
                    echo_from = str.substr(12, 5);
                    echo_to = str.substr(18, 5);
                    time_from = str.substr(24, 5);
                    time_to = str.substr(30, 5);
                }
                tmp_rand_from = str.substr(0, 5);
                tmp_rand_to = str.substr(6, 5);
                tmp_echo_from = str.substr(12, 5);
                tmp_echo_to = str.substr(18, 5);
                tmp_time_from = str.substr(24, 5);
                tmp_time_to = str.substr(30, 5);
                
            }
            else {
                throw "ReadFile: Ошибка ";
            }
            cout << endl;
        }



        CloseHandle(mail);
    }
    catch (string errorMsgText) {
        std::cout << errorMsgText << endl;
    }
    catch (...) {
        cout << "Error ServiceWork" << endl;
    }
    cout << "ServiceWorkTime остановлен\n" << endl;
    ExitThread(rc);
}

int* getHoursAndMinute(string str) {
    string str_1, str_2;
    if (str[2] == ':') {
        str_1 += str.substr(0, 2);
        str_2 += str.substr(3, 2);
        int ihh = atoi(str_1.c_str());
        int imm = atoi(str_2.c_str());
        return new int[2]{ ihh, imm };
    }
    return NULL;
}

const string currentDateTim() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%H:%M", &tstruct);






    return buf;
}