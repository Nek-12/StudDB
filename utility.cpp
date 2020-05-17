#include "header.h"
#include "lib/sha256.h" //Encryption
#include <random> //For genID()
#include <filesystem>

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

bool checkDate(std::string& s, int isPast) {
    std::regex reg(R"((\d{1,2})([-. /])(\d{1,2})([-. /])(\d{4}))"); //Initialize the regular expression
    std::smatch res;
//If the format is right
    if (std::regex_match(s, res, reg)) {
        time_t t = time(nullptr);
        tm* nowTm = localtime(&t);
        int day = std::stoi(res.str(1));
        int month = std::stoi(res.str(3));
        int year = std::stoi(res.str(5)); //Divide into values
        int curyear = nowTm->tm_year + 1900;
        int curm = nowTm->tm_mon + 1;
        if (res.str(2) != res.str(4) || month > 12) return false;
        if (day == 0 && month == 0 && year == 0) return true;
        if (isPast != -1) {
            bool cond = year < curyear
                        || (year == curyear && month < curm) //if year is ok, check that month is not in the past
                        || (year == curyear && month == curm && day < nowTm->tm_mday);  //if month and year are ok, check the day
            //cond = true if the date is in the future
            if (isPast) cond = !cond; //Invert the condition if we need future
            if (cond) return false; //If condition did not succeed, return
        }
        switch (month) {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                if (day > 31)
                    return false;
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                if (day > 30)
                    return false;
                break;
            case 2:
                if (year % 400 == 0 || (year % 100 != 0 && year % 4 == 0)) {
                    if (day > 29)
                        return false;
                }
                else if (day > 28)
                    return false;
                break;
            default:
                throw std::invalid_argument("Default case when parsing month");
        }
        s = res.str(1) + '.' + res.str(3) + '.' + res.str(5);
        return true;
    }
    else
        return false;
}

bool checkYear(const std::string& s) {
    if (s.size() > 4) return false;
    for (auto ch: s)
        if (!isdigit(ch))
            return false;
    return (std::stoul(s) <= getCurYear());
}

std::string checkString(std::string& s, char mode) {
    auto msgFalse = [& s](const std::string& msg) {
        return "The value " + s + " is invalid: \n" + msg + '\n';
    };
    if (s.empty())
        return msgFalse("No data?");
    size_t cnt = 0;
    switch (mode) {
        case 'p': //password
        case 'n': //No spaces
            if (s.size() < 3 || s.size() > 75) return msgFalse("too short/long for a word");
            for (auto& ch: s)
                if (!(isalnum(ch) || ch == '.' || ch == '-' || ch == '_' || ch == '\'' || ch == '#'))
                    return msgFalse("invalid characters");
            break;
        case 's': //spaces
            if (s.size() < 2) return msgFalse("too short/long for a line");
            for (auto& ch: s)
                if (!(isalnum(ch) || ispunct(ch) || ch == ' '))
                    return msgFalse("invalid characters");
            break;
        case 'k': //date past
            if (!checkDate(s, true)) return msgFalse("Wrong date");
            else return "";
        case 'l': //date future
            if (!checkDate(s, false)) return msgFalse("Wrong date");
            else return "";
        case 'd': //date any
            if (!checkDate(s, -1)) return msgFalse("Wrong date");
            else return "";
        case 'i': //integer
            if (s.size() > MAX_ID_LENGTH) return msgFalse("too long for a number");
            for (auto& ch: s)
                if (!isdigit(ch))
                    return msgFalse("invalid characters in a number");
            break;
        case 'y': //year
            if (!checkYear(s)) return msgFalse("invalid year");
            break;
        case 'f': //float
            if (s.empty() || s.size() > 7) return msgFalse("too short/long for floating-point number");
            for (auto& ch: s) {
                if (!isdigit(ch) && ch != '.')
                    return msgFalse("invalid characters in a number");
                if (ch == '.') ++cnt;
                if (cnt > 1) return msgFalse("not a number");
            }
            break;
        case 'b': //bool
            if (s.size() != 1 || (s[0] != '0' && s[0] != '1')) return msgFalse("not a boolean");
            break;
        default:
            throw std::invalid_argument("Bad argument for checkString");
    }
    return "";
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
    auto str = checkString(s, mode);
    if (str.empty()) {
        ret = s; //This guarantees that the string is NOT changed unless the input is good. I could just return bool.
        return true;
    }
    else std::cout << str;
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

void setTableProperties(fort::char_table& t, unsigned firstColored, unsigned secondColored) //Edit the given table (for uniform look)
{
    t.set_cell_text_align(fort::text_align::center);
    t.set_border_style(FT_BASIC2_STYLE);
    t.column(firstColored).set_cell_content_fg_color(fort::color::green);
    t.column(secondColored).set_cell_content_fg_color(fort::color::red);
}

void ensureFileExists(const std::string& f) {
    if (!std::filesystem::exists(path + f)) //Check if the file exists
    {//If not
        plog->put("The file", path, f, "does not exist! Creating a blank one...");
        std::ofstream file(path + f); //Create a new one
        file.close(); //Explicit
    }
}