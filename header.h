#pragma once //Don't repeat the inclusion to avoid conflicts

#include <map> //Main storage
//#include <utility> //For advance and other stuff
#include <vector> //For search
#include <iostream> //IO
#include <set> //For storing pointers
#include "lib/fort.hpp" //For printing tables

//#define NDEBUG //Forbid debugging output in the release version (explicit, but can be defined by CMake automatically)
#ifndef __linux__
#include <conio.h>
#define CARRIAGE_RETURN_CHAR 13 //getch() returns different keycodes for windows and linux
#define BACKSPACE_CHAR 8
#define CLS "cls"
using ull = unsigned long long; //The size of unsigned long on my linux distro is around ~10^25, however on my Windows OS the size of
//unsigned long long (!) is just ~10^20
#else
using ull = unsigned long; //Depends on the platform
#define CARRIAGE_RETURN_CHAR 10
#define BACKSPACE_CHAR 127
#define CLS "clear"
#include <termios.h>
int getch();
#endif
#define MAX_ID 1000000000000000000u //19 digits
#define MAX_ID_LENGTH 19 //I have settled on this maximum length of the id, but it can be increased and even made a string.
extern std::string path; //Path to the program folder, see main.cpp -> int main()

class Data;
class Entry;
class Book;
class Author;
class Genre;
extern Data* data;
void setTableProperties(fort::char_table&, unsigned, unsigned); //Edit the table
void pause(); //Wait for a keypress
ull stoid(const std::string& s); //change string to an ID
std::string getPassword();
ull genID();
void cls();
unsigned getCurYear();
bool checkString(const std::string&, char); //Input check
bool checkDate(const std::string& s);
std::string lowercase(const std::string&);
std::string hash(const std::string& s); //uses sha256.cpp and sha256.h for encrypting passwords, outputs hashed string
bool readString(std::istream& is, std::string& s, char mode);
//allows for reading a line from the iostream object with input check (foolproofing)
// 's' for strings with spaces, 'n' for normal, 'd' for date, 'p' for passwords, 'i' for numbers (IDs), 'y' for years
//Not the best solution but convenient for me
class Entry {
//Base class
/*
 * Each journal entry represents some kind of table row that contains different info for each entry, but the operations are same.
 * This is some kind of merged interface and usual base class. Will be used with dynamic binding later extensively, so I tried to provide
 * A lot of virtual functions to minimize dynamic casts and RTTI which are slow
 * But the nature is such that sometimes we can't avoid casts in my project. At least I couldn't.
*/
    friend std::ostream& operator<<(std::ostream& os, const Entry& e) {//dynamic binding here
        os << e.to_string(); //each type prints its own info, virtual
        return os;
    }
    friend bool operator==(const Entry& lhs, const Entry& rhs) //For some containers and functions
    { return lhs.id() == rhs.id(); } //applicable to any entry because of dynamic binding
public:
    Entry() = delete; //No blank entries
    Entry(const Entry& e) = delete; //No copies
//    Copying an entry doesn't make sense. Why would we need several identical lines in our journal?
//    Entry& operator=(const Entry&) = delete; //No assigning
    virtual ~Entry() = default;
    [[nodiscard]] const ull& id() const //We shouldn't discard the returned value for the getter. Would make no sense. Just for safety
    { return no; }
    [[nodiscard]] std::string getName() const { return name; }
    void rename(const std::string& s) { name = s; }
    virtual bool link(Entry* e) = 0; //Pure virtuals for an abstract base class
    virtual bool unlink(Entry* e) = 0;
protected: //Constructors are protected to disallow users creating entries in the journal. It's unsafe and makes no sense. Users will use the
    [[nodiscard]] virtual std::string to_string() const = 0;
    [[nodiscard]] virtual bool check(const std::string& s) const = 0;
    //Data class instead, and the safety is going to be much better
    Entry(Entry&& e) noexcept: no(e.no), name(std::move(e.name)) {} //We can only move entries from one journal to another
    explicit Entry(std::string n, const ull& id) : no(id), name(std::move(n)) {
#ifndef NDEBUG
        std::cout << getName() << " was created \n";
#endif
    }
private:
    ull no; //unique id
    std::string name;
};

class Book : public Entry {
    friend class Genre;
    friend class Author;
    friend class Data;
public:
    ~Book() override;
    Book(Book&& b) noexcept; //Exists
    explicit Book(const std::string& t, const unsigned& y = 0, const ull& n = genID()) : Entry(t, n), year(y) {}
    [[nodiscard]] bool check(const std::string& s) const override;
    [[nodiscard]] std::string to_string() const override;
    bool link(Entry* pe) override;
    bool unlink(Entry* pe) override;
    void remGenre(const size_t& pos); //Custom functions needed for selecting one among the many
    void remAuthor(const size_t& pos);
    [[nodiscard]] size_t enumAuthors() const //Get the number of authors
    { return authors.size(); }
    [[nodiscard]] size_t enumGenres() const { return genres.size(); }
    [[nodiscard]] unsigned getYear() const { return year; }
    void setYear(unsigned int y) { year = y; }
private:
    unsigned year = 0;
    std::set<Entry*> authors; //Holds pointers to objects of authors. Thought this is a set of entries, we KNOW there are authors only
    std::set<Entry*> genres; //To avoid downcasting as much as possible
};

class Author : public Entry {//Same logic as above
    friend class Book;
    friend class Data;
public:
    ~Author() override;
    Author(Author&& a) noexcept;
    explicit Author(const std::string& n, std::string d, std::string c, const ull& id = genID()) :
            Entry(n, id), country(std::move(c)), date(std::move(d)) {}
    bool link(Entry* pe) override;
    bool unlink(Entry* pe) override;
    void remBook(const size_t& pos); //Custom
    void setCountry(const std::string& c) { country = c; }
    void setDate(const std::string& c) { date = c; }
    [[nodiscard]] size_t enumBooks() const { return books.size(); }
    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] bool check(const std::string& s) const override;
private:
    std::string country; //Where the author was born
    std::string date; //When the author was born
    std::set<Entry*> books;
};

class Genre : public Entry {
    friend class Book;
    friend class Data;
public:
    explicit Genre(std::string n, ull id = genID()) : Entry(std::move(n), id) {}
    ~Genre() override;
    Genre(Genre&& g) noexcept;
    bool link(Entry* pe) override;
    bool unlink(Entry* pe) override;
    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] bool check(const std::string& s) const override;
//    [[nodiscard]] size_t enumBooks() const
//    { return books.size(); }
private:
    std::set<Entry*> books;
};

class Data {// SINGLETON for storing all the nested structures
    friend class Book;
    friend class Genre;
    friend class Author;
public:
    Data(Data const&) = delete; //No copying, no moving!
    void operator=(Data const&) = delete; //No assigning!
    //Default destructor
    static Data* getInstance() //Returns a reference to the single static instance of Data.
    {
        static Data instance; //The instance is always one and lazy-evaluated on the first use
        return &instance; //Return a pointer to self
    }
    void load(); //Loads all the data from several files
    void save(); //Writes the data to the files (books.txt etc.)
    void printBooks(); //Print all the books
    void printAuthors(); //Authors
    void printGenres(unsigned = getCurYear()); //Special
    void printCredentials(bool isadmin);
    std::vector<Entry*> search(const std::string& s); //Search anything
    template<typename ...Args>
    //Variadic template to generate an Entry in-place without any copies
    Book* addBook(const Args& ...args) //Just forwards all the args to the constructor
    {
        auto it = mbooks.try_emplace(args...); //Forward again
        return (it.second ? &it.first->second : nullptr); //On success return the pointer
    }
    template<typename ...Args>
    //3 templates because 3 containers
    Author* addAuthor(const Args& ...args) {
        auto it = mauthors.try_emplace(args...);
        return (it.second ? &it.first->second : nullptr);
    }
    template<typename ...Args>
    Genre* addGenre(const Args& ...args) {
        auto it = mgenres.try_emplace(args...);
        return (it.second ? &it.first->second : nullptr);
    }
    bool erase(Book& e) //For every different type
    { return mbooks.erase(e.id()); }
    bool erase(Author& e) { return mbooks.erase(e.id()); }
    bool erase(Genre& e) { return mbooks.erase(e.id()); }
    //All functions operate in logariphmic time
    bool delAccount(const std::string& l, const bool& isadmin);
    bool passCheck(const std::string& l, const std::string& p, const bool& isadmin) {
        return ((isadmin ? admins.find(l) : users.find(l))->second == hash(p));
    }
    bool loginCheck(const std::string& s, const bool& isadmin) //Ensure the user exists
    { return (isadmin ? admins.find(s) != admins.end() : users.find(s) != users.end()); }
    bool addAccount(const std::string& l, const std::string& p, const bool& isadmin) {
        return (isadmin ? admins : users).try_emplace(l, hash(p)).second;
    }
    size_t enumAccounts(const bool& isadmin) //Constant time ofc
    { return (isadmin ? admins.size() : users.size()); }
    bool changePass(const std::string& l, const std::string& p, const bool& isadmin) {
        auto it = (isadmin ? admins : users).find(l);
        if (it == (isadmin ? admins : users).end())
            return false;
        it->second = hash(p);
        return true;
    }
private:
    Data() = default; //private to disallow creation
    static void ensureFileExists(const std::string& f); //Users don't need that function
    std::map<ull, Genre> mgenres;
    std::map<ull, Author> mauthors;
    std::map<ull, Book> mbooks; //Contains all the Books in the database
    std::map<std::string, std::string> users; // holds <login, password> (hashed)
    std::map<std::string, std::string> admins; //same
};
//CONSTANTS
#define LOGINPROMPT  "Enter the login or \"exit\" to exit: "
#define PASSPROMPT  "Enter the password or \"exit\" to exit: "
#define PASSCONFIRM  "Confirm the password or enter \"exit\" to exit: "
#define ADMIN_CONSOLE_ENTRIES    ":ADMIN:"\
                                 "\nSelect an option: "\
                                 "\n1 -> Manage book data "\
                                 "\n2 -> Change your password"\
                                 "\n3 -> Register an administrator"\
                                 "\n4 -> Delete users "\
                                 "\n0 -> Delete your account (careful!)"\
                                 "\nq -> Sign off"
#define USER_CONSOLE_ENTRIES    ":USER:"\
                                "\nSelect an option: "\
                                "\n1 -> Manage book data "\
                                "\n2 -> Change your password"\
                                "\n0 -> Delete your account"\
                                "\nq -> Sign off"
#define ADMIN_MANAGEMENT_ENTRIES    ":ADMIN:"\
                                    "\nSelect an option: "\
                                    "\n1 -> Search anything "\
                                    "\n2 -> Show data "\
                                    "\n3 -> Manage entries "\
                                    "\n4 -> Add a new book"\
                                    "\n5 -> Add a new author"\
                                    "\n6 -> Add a new genre "\
                                    "\nq -> Go back"
#define USER_MANAGEMENT_ENTRIES    ":USER:"\
                                   "\nSelect an option: "\
                                   "\n1 -> Search anything"\
                                   "\n2 -> Show data "\
                                   "\nq -> Go back"
#define SHOW_DATA_MENU_ENTRIES    "Select an option: "\
                                  "\n1 -> Show all books "\
                                  "\n2 -> Show all authors "\
                                  "\n3 -> Show all genres for a given year "\
                                  "\nq -> Go back"
#define WELCOME_MENU "Welcome. "\
                     "\n1 -> User sign in "\
                     "\n2 -> Admin sign in "\
                     "\nq -> Save and exit"
#define EDIT_GENRE_OPTIONS "Select an option: "\
                             "\n1 -> Rename this genre "\
                             "\n2 -> Add this genre to books "\
                             "\n3 -> Remove this genre from books"\
                             "\n4 -> Delete this genre "\
                             "\nq -> Go back"
#define EDIT_BOOK_OPTIONS   "What would you like to do? "\
                            "\n1 -> Edit title"\
                            "\n2 -> Edit publishing year"\
                            "\n3 -> Edit entries"\
                            "\n4 -> Delete this book"\
                            "\nq -> Nothing"
#define EDIT_AUTHOR_OPTIONS "What would you like to do? "\
                            "\n1 -> Edit name"\
                            "\n2 -> Edit birthdate"\
                            "\n3 -> Edit country"\
                            "\n4 -> Add books"\
                            "\n5 -> Remove books"\
                            "\n6 -> Delete this author"\
                            "\nq -> Nothing"
