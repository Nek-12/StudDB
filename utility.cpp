#include "header.h"
#include "lib/sha256.h" //Encryption
#include <regex> //For checking date
#include <random> //For genID()

void pause() {
    std::cout << "Press any key to continue..." << std::endl;
    getch();
}

ull genID() { //Static to not diminish randomness
    static std::default_random_engine e(std::random_device{}()); //Initialize a random engine from system random device
    static std::uniform_int_distribution<ull> rng(0, MAX_ID); //Generate numbers for IDs from 0 to a large system value
    return rng(e); //use random engine
}

std::string hash(const std::string& s) {
    SHA256 sha256;
    return sha256(s); //returns hashed string, you can now never decrypt the password back
}

unsigned getCurYear() {
    time_t t = time(nullptr); //get system time
    tm* nowTm = localtime(&t); //format it according to the region
    return (unsigned) nowTm->tm_year + 1900; //return the year.
}

bool checkDate(const std::string& s) {
    std::regex reg(R"((\d{1,2})([-. /])(\d{1,2})([-. /])(\d{4}))"); //Initialize the regular expression
    std::smatch res;
    auto msgFalse = [& s](const std::string& what) { //lambda
        std::cerr << "The date " << s << " is invalid: " << what << std::endl;
        return false;
    };
//If the format is right
    if (std::regex_match(s, res, reg)) {
        time_t t = time(nullptr);
        tm* nowTm = localtime(&t);
        int day = std::stoi(res.str(1));
        int month = std::stoi(res.str(3));
        int year = std::stoi(res.str(5)); //Divide into values
        if (res.str(2) != res.str(4)) return msgFalse("Divisors don't match: " + res.str(2) + " < =/= > " + res.str(4));
        if (day == 0 && month == 0 && year == 0)
            return true; //We are allowed to input 0.0.0000 and it's considered unknown date
        //Start checking year,month,day
        if (year > (nowTm->tm_year + 1900))
            return msgFalse("This year is in the future: " + std::to_string(year));
        else if ((year == nowTm->tm_year + 1900) && month > nowTm->tm_mon + 1)
            return msgFalse("This month is in the future: " + std::to_string(month));
        if (month > 12)
            return msgFalse("More than 12 months");
        switch (month) {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                if (day > 31)
                    return msgFalse("More than 31 days");
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                if (day > 30)
                    return msgFalse("More than 30 days");
                break;
            case 2:
                if (year % 400 == 0 || (year % 100 != 0 && year % 4 == 0)) {
                    if (day > 29)
                        return msgFalse("More than 29 days");
                }
                else if (day > 28)
                    return msgFalse("More than 28 days");
                break;
            default:
                throw std::invalid_argument("Default case when parsing month");
        }
        return true;
    }
    else
        return msgFalse("Wrong date formatting");
}

bool checkYear(const std::string& s) {
    if (s.size() > 4) return false;
    for (auto ch: s)
        if (!isdigit(ch))
            return false;
    return (std::stoul(s) <= getCurYear());
}

bool checkString(const std::string& s, char mode) {
    auto msgFalse = [& s](const std::string& msg) {
        std::cerr << "The value " << s << " is invalid: \n" << msg << std::endl;
        return false;
    };
    if (s.empty())
        return msgFalse("No data?");
    switch (mode) {
        case 'p': //Password contains the same chars as a normal string (word)
        case 'n': //No spaces allowed
            if (s.size() < 3 || s.size() > 75) return msgFalse("too short/long for a word");
            for (auto ch: s)
                if (!(isalnum(ch) || ch == '.' || ch == '-' || ch == '_' || ch == '\''))
                    return msgFalse("invalid characters");
            break;
        case 's': //Line (s) has less strict restrictions and can contain spaces
            if (s.size() < 2) return msgFalse("too short/long for a line");
            for (auto ch: s)
                if (!(isalnum(ch) || ispunct(ch) || ch == ' '))
                    return msgFalse("invalid characters");
            break;
        case 'd': //Delegates
            return (checkDate(s));
        case 'i': //I stands for integer, or ID only digits, no negatives.
            if (s.size() > MAX_ID_LENGTH) return msgFalse("too long for a number");
            for (auto ch: s)
                if (!isdigit(ch))
                    return msgFalse("invalid characters in a number");
            break;
        case 'y': //Delegates
            if (!checkYear(s)) return msgFalse("invalid year");
            break;
        default:
            throw std::invalid_argument("Bad argument for checkString");
    }
    return true;
}

std::string getPassword() { //Input password, hide it with *'s
    std::string password;
    int a;
    while ((a = getch()) != CARRIAGE_RETURN_CHAR) //Differs on linux and windows
    { //While ENTER is not pressed
        if (a == BACKSPACE_CHAR) //same
        { //If Backspace
            if (password.empty()) continue;
            password.pop_back(); //remove char
            std::cout << '\b' << ' ' << '\b'; //replace a star with a space
        }
        else {
            password += (char) a; //Add this char
            std::cout << '*'; //But output the star
        }
    }
    std::cout << std::endl;
    return password; //Then we input check this string
}

bool readString(std::istream& is, std::string& ret, char mode = 'n') {
// 's' for strings with spaces, 'n' for normal, 'd' for date, 'p' for password
    std::string s;
    if (mode == 'p')
        s = getPassword(); //Display stars
    else if (!std::getline(is, s)) return false; //Display chars

    if (checkString(s, mode)) {
        ret = s; //This guarantees that the string is NOT changed unless the input is good. I could just return bool.
        return true;
    }
    return false;
}

std::string lowercase(const std::string& s) {
    std::string ret = s;
    for (auto& ch: ret)
        ch = tolower(ch);
    return ret;
}

ull stoid(const std::string& s) {
#ifndef __linux__
    return std::stoull(s);
#else //This function depends on the platform. See header.h for details
    return std::stoul(s);
#endif
}