#include "header.h"
#include "lib/sha256.h" //Encryption
#include <filesystem>
#include <random> //For genID()
//!Don't reinitialize the regex every time, it's slow
const std::regex date_regex(R"((\d{1,2})([-. /])(\d{1,2})([-. /])(\d{4}))"); //NOLINT

void pause() {
    std::cout << "Press any key to continue..." << std::endl;
    getch();
}

ull genID() { // Static to not diminish randomness
    static std::default_random_engine e(
        std::random_device{}()); // Initialize a random engine from system
                                 // random device
    static std::uniform_int_distribution<ull> rng(
        0, MAX_ID); // Generate numbers for IDs from 0 to a large system value
    return rng(e);  // use random engine
}

std::string hash(const std::string& s) {
    SHA256 sha256;
    return sha256(s); // returns hashed string, you can now never decrypt the
                      // password back
}

unsigned getCurYear() {
    time_t t     = time(nullptr); // get system time
    tm*    nowTm = localtime(&t); // format it according to the region
    return static_cast<unsigned>(nowTm->tm_year) + 1900; // return the year.
}

//isPast can be 1,0 or -1 which means no check;
bool checkDate(const std::string& s, int is_past) {
    std::smatch res;
    // If the format is right
    if (std::regex_match(s, res, date_regex)) {
        time_t t     = time(nullptr);
        tm*    nowTm = localtime(&t);
        int    day   = std::stoi(res.str(1));
        int    month = std::stoi(res.str(3));
        int year = std::stoi(res.str(5)); // Divide into values
        int curyear = nowTm->tm_year + 1900;
        int curm    = nowTm->tm_mon + 1;
        if (res.str(2) != res.str(4) || month > 12)
            return false;
        if (day == 0 && month == 0 && year == 0)
            return false;
        if (is_past != -1) { // the user requested sanity check
            bool cond =
                year < curyear ||
                (year == curyear &&
                 month <
                     curm) // if year is ok, check that month is not in the past
                ||
                (year == curyear && month == curm &&
                 day <
                     nowTm->tm_mday); // if month and year are ok, check the day
            // cond = true if the date is in the future
            if (is_past)
                cond = !cond; // Invert the condition if we need future
            if (cond)
                return false; // If condition did not succeed, return
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
            } else if (day > 28)
                return false;
            break;
        default: throw std::invalid_argument("Default case when parsing month");
        } //switch
        return true;
    }
    return false; //if regex didn't match
}

std::string format_date(const std::string& date) {
    std::smatch res;
    std::regex_match(date, res, date_regex);
    return res.str(1) + '.' + res.str(3) + '.' + res.str(5);
}

bool check_year_past(const std::string& s) {
    if (s.size() > 4)
        return false;
    for (auto ch : s)
        if (!isdigit(ch))
            return false;
    return (std::stoul(s) <= getCurYear());
}

//@returns a pair, where left is whether the check succeeded and right is an error message (if there was no error then the second member is empty)
auto checkString(const std::string& s, char mode)
    -> std::pair<bool, std::string> {
    auto msgFalse = [&s](const std::string& msg) {
        return std::make_pair(false, "The value " + s + " is invalid: \n" +
                                         msg + '\n');
    };
    if (s.empty())
        return msgFalse("No data?");
    size_t                       cnt = 0;
    std::pair<bool, std::string> p;
    switch (mode) {
    case 'p': // password
    case 'n': // No spaces
        if (s.size() < 3 || s.size() > 75)
            return msgFalse("Too short/long for a word");
        for (const auto & ch : s)
            if (!(isalnum(ch) || ch == '.' || ch == '-' || ch == '_' ||
                  ch == '\'' || ch == '#'))
                return msgFalse("Invalid characters");
        break;
    case 's': // spaces
        if (s.size() < 2)
            return msgFalse("Too short/long for a line");
        for (const auto & ch : s)
            if (!(isalnum(ch) || ispunct(ch) || ch == ' '))
                return msgFalse("Invalid characters");
        break;
    case 'k': // date past
        if (!checkDate(s,1))
            return msgFalse("Wrong date");
        else
            return std::make_pair(true,"");
    case 'l': // date future
        if (!checkDate(s, false))
            return msgFalse("Wrong date");
        else
            return std::make_pair(true, "");
    case 'd': // date any
        if (!checkDate(s, -1))
            return msgFalse("Wrong date");
        else
            return std::make_pair(true,"");
    case 'i': // integer
        if (s.size() > MAX_ID_LENGTH)
            return msgFalse("too long for a number");
        for (const auto& ch : s)
            if (!isdigit(ch))
                return msgFalse("invalid characters in a number");
        break;
    case 'y': // year
        if (!check_year_past(s))
            return msgFalse("invalid year");
        break;
    case 'f': // float
        if (s.empty() || s.size() > 7)
            return msgFalse("too short/long for floating-point number");
        for (const auto& ch : s) {
            if (!isdigit(ch) && ch != '.')
                return msgFalse("invalid characters in a number");
            if (ch == '.')
                ++cnt;
            if (cnt > 1)
                return msgFalse("not a number");
        }
        break;
    case 'b': // bool
        if (s.size() != 1 || (s[0] != '0' && s[0] != '1'))
            return msgFalse("not a boolean");
        break;
    default: throw std::invalid_argument("Bad argument for checkString");
    }
    return std::make_pair(true, "");
}

std::string getPassword() { // Input password, hide it with *'s
    std::string password;
    int         a = 0;
    while ((a = getch()) !=
           CARRIAGE_RETURN_CHAR) // Differs on linux and windows
    {                            // While ENTER is not pressed
        if (a == BACKSPACE_CHAR) // same
        {                        // If Backspace
            if (password.empty())
                continue;
            password.pop_back();              // remove char
            std::cout << '\b' << ' ' << '\b'; // replace a star with a space
        } else {
            password += static_cast<char>(a); // Add this char
            std::cout << '*';    // But output the star
        }
    }
    std::cout << std::endl;
    return password; // Then we input check this string
}

// 's' for strings with spaces, 'n' for normal, 'd' for date, 'p' for password
bool readString(std::istream& is, std::string& ret, char mode = 'n') {
    std::string s;
    if (mode == 'p')
        s = getPassword(); // Display stars
    else if (!std::getline(is, s))
        return false; // Display chars
    std::pair<bool,std::string> pair = checkString(s, mode);
    if (pair.first) {
        ret = s; // This guarantees that the string is NOT changed unless the
                 // input is good. I could just return bool.
        return true;
    }
    std::cout << pair.second;
    return false;
}

std::string lowercase(const std::string& s) {
    std::string ret = s;
    for (auto& ch : ret)
        ch = tolower(ch);
    return ret;
}

ull stoid(const std::string& s) {
#ifndef __linux__
    return std::stoull(s);
#else // This function depends on the platform. See header.h for details
    return std::stoul(s);
#endif
}

void setTableProperties(
    fort::char_table& t, unsigned firstColored,
    unsigned secondColored) // Edit the given table (for uniform look)
{
    t.set_cell_text_align(fort::text_align::center);
    t.set_border_style(FT_BASIC2_STYLE);
    t.column(firstColored).set_cell_content_fg_color(fort::color::green);
    t.column(secondColored).set_cell_content_fg_color(fort::color::red);
}

void ensureFileExists(const std::string& f) {
    if (!std::filesystem::exists(path + f)) // Check if the file exists
    {                                       // If not
        plog->put("The file", path, f,
                  "does not exist! Creating a blank one...");
        std::ofstream file(path + f); // Create a new one
        file.close();                 // Explicit
    }
}
