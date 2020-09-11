#pragma once
#include <map> //Main storage
#include <vector> //For search
#include <iostream> //IO
#include <iomanip>
#include <set> //For storing pointers
#include "lib/fort.hpp" //For printing tables
#include <chrono>
#include <fstream>
#include <regex> //For checking date
#include <memory>
#include <typeinfo>
//#define NDEBUG //Forbid debugging output in the release version (explicit, but can be defined by CMake automatically)
#ifndef __linux__
#define CLS "cls"
#include <conio.h>
#define CARRIAGE_RETURN_CHAR 13 //getch() returns different keycodes for windows and linux
#define BACKSPACE_CHAR 8
using ull = unsigned long long; //The size of unsigned long on my linux distro is around ~10^25, however on my Windows OS the size of
//unsigned long long (!) is just ~10^20
using us = unsigned short;
#else
#define CLS "clear"
using ull = unsigned long; //Depends on the platform
#define CARRIAGE_RETURN_CHAR 10
#define BACKSPACE_CHAR 127
#include <termios.h>
int getch();
#endif
#define MAX_ID 1000000000000000000u //19 digits
#define MAX_ID_LENGTH 19 //I have settled on this maximum length of the id, but it can be increased and even made a string.

enum CHECK {
    LINE = 's', WORD = 'n', DATE_FUTURE = 'l', DATE_PAST = 'k', DATE = 'd', PASS = 'p', BOOL = 'b', FLOAT = 'f', ID = 'i', YEAR = 'y'
};

void ensureFileExists(const std::string& f);
void setTableProperties(fort::char_table& t, unsigned firstColored, unsigned secondColored);
void pause(); //Wait for a keypress
ull stoid(const std::string& s); //change string to an ID
std::string getPassword();
ull genID();
void cls();
unsigned getCurYear();
std::pair<bool, std::string> checkString(const std::string&,
                                         char); // Input check
bool        checkDate(const std::string& s, int is_past);
std::string format_date(const std::string& date);
std::string lowercase(const std::string&);
std::string hash(const std::string& s); //uses sha256.cpp and sha256.h for encrypting passwords, outputs hashed string
bool readString(std::istream& is, std::string& ret, char mode);
//allows for reading a line from the iostream object with input check (foolproofing)
// 's' for strings with spaces, 'n' for normal, 'd' for date, 'p' for passwords, 'i' for numbers (IDs), 'y' for years
//Not the best solution but convenient for me
class Data;
class Log;
extern Data* data;
extern Log* plog;
extern std::string path; // Path to the program folder, see main.cpp -> int main()
inline char* getCurTime() {
    auto t = std::chrono::system_clock::now();
    std::time_t ttime = std::chrono::system_clock::to_time_t(t);
    return std::ctime(&ttime);
}
//USAGE: log->put("Message part 1",someString,"message end", ...);
//NO SPACES REQUIRED

class Log {
public:
    Log() = delete;
    Log(Log const&) = delete; //No copying, no moving!
    void operator=(Log const&) = delete; //No assigning!

    static Log* init(const std::string& filename) { //Start logging and get a pointer
        static Log instance(filename);
        return &instance;
    }

    template<typename ...Args>
    void put(const Args& ...args) { //first call

        f << getCurTime() << " | ";
        _put(args...);
    }

    void flush() { //Clear the file
        f.close();
        f.open(fname, std::fstream::out | std::fstream::trunc | std::fstream::in);
        put("-------------| Log flushed |-------------");
    }
    void print() { //Print the file
        f.close();
        f.open(fname, std::fstream::out | std::fstream::in | std::fstream::app);
        f.seekg(std::fstream::beg);
        std::cout << f.rdbuf();
        f.seekg(std::fstream::end);
    }
    ~Log() {
        put("-------------| The program was closed |-------------");
    }
private:
    template<typename T>
    void _put(const T& msg) { //Last call
        f << msg << std::endl;
    }
    template<typename T, typename ...Args>
    void _put(const T& t, const Args& ...args) { //recursive calls
        f << t << " ";
        _put(args...);
    }
    explicit Log(const std::string& name) : fname(name), f(name, std::fstream::out | std::fstream::app) {
        if (!f) throw std::runtime_error("Error opening log file " + fname);
        put("-------------| Started log session |-------------"); //On start, print the message
    }
    std::string fname;
    std::fstream f;
};
