#pragma once
#include "misc.h"
class Entry;
class Student;
class Event;
//TODO: Edit comments
//Base class
/*
 * Each journal entry represents some kind of table row that contains different info for each entry, but the operations are same.
 * This is some kind of merged interface and usual base class. Will be used with dynamic binding later extensively, so I tried to provide
 * A lot of virtual functions to minimize dynamic casts and RTTI which are slow
 * But the nature is such that sometimes we can't avoid casts in my project. At least I couldn't.
*/
class Entry {
    friend class Data;
    friend std::ostream& operator<<(std::ostream& os, const Entry& e) { //dynamic binding here
        os << e.to_string(); //each type prints its own info, virtual
        return os;
    }
    friend bool operator==(const Entry& lhs, const Entry& rhs) //For some containers and functions
    { plog->put("called", lhs.getName(), "==", rhs.getName()); return lhs.id() == rhs.id(); } //applicable to any entry because of dynamic binding
public:
    Entry() = delete; //No blank entries
    Entry(const Entry& e) = delete; //No copies
//    Copying an entry doesn't make sense. Why would we need several identical lines in our journal?
//    Entry& operator=(const Entry&) = delete; //No assigning
    virtual ~Entry();
    [[nodiscard]] const ull& id() const { return unique_id; }//We shouldn't discard the returned value for the getter. Would make no sense. Just for safety
    [[nodiscard]] std::string getName() const { return name; }
    void rename(const std::string& s) { name = s; }
    bool link(Entry* e);
    bool unlink(Entry* e);
    size_t enumLinks() {return links.size();}
protected: //Constructors are protected to disallow users creating entries in the journal. It's unsafe and makes no sense. Users will use the
    [[nodiscard]] virtual std::string to_string() const = 0;
    [[nodiscard]] virtual bool check(const std::string& s) const = 0;
    explicit Entry(const ull& id, std::string n) : unique_id(id), name(std::move(n)) {
       plog->put(getName(), "was created");
    }
    std::set<Entry*> links;
private:
    ull unique_id; //unique id
    std::string name;
};

class Student : public Entry {
    friend class Data;
public:
    ~Student() override = default;
    Student(Student&& b) = delete;
    [[nodiscard]] unsigned short getAge() const {
        return getCurYear() - stoi(birthdate.substr(birthdate.find_last_of('.')+1,4));
    }
    bool switchTuition() { return isTuition = !isTuition; }
    void setAvgGrade(float& avg) { avgGrade = avg;}
    void setDegree(const std::string& d) {degree = d;}
    void setBirthDate(const std::string& d) {birthdate = d;}
private:
    //CONSTRUCTOR
    explicit Student(const ull& no, const std::string& n, std::string  d,  std::string  b, bool i, float gr)
            : Entry(no, n), degree(std::move(d)), birthdate(std::move(b)), isTuition(i),avgGrade(gr) {}
    [[nodiscard]] bool check(const std::string& s) const override;
    [[nodiscard]] std::string to_string() const override;
    std::string degree;
    std::string birthdate;
    bool isTuition;
    float avgGrade;
};
class Event : public Entry {
    friend class Data;
public:
    ~Event() override = default;
    Event(Event&& b) = delete;
    void setPlace(const std::string& s) {place = s;}
    void setDate(const std::string& s) {date = s;}
private:
    //CONSTRUCTOR
    explicit Event(const ull& no, const std::string& n, std::string d, std::string pl) : Entry(no,n), place(std::move(pl)), date(std::move(d)) {}
    [[nodiscard]] std::string to_string() const override;
    [[nodiscard]] bool check(const std::string& s) const override;
    std::string place;
    std::string date;
};

class Data {  // SINGLETON for storing all the nested structures
public:
    Data(const Data &) = delete; //No copying, no moving!
    void operator=(const Data&) = delete; //No assigning!
    ~Data() = default;
    static Data* getInstance() //Returns a reference to the single static instance of Data.
    {
        static Data instance; //The instance is always one and lazy-evaluated on the first use
        return &instance; //Return a pointer to self
    }
    void load(); //Loads all the data from several files
    void save(); //Writes the data to the files (books.txt etc.)
    std::string printCredentials(bool isadmin);
    std::vector<Entry*> search(const std::string& s); //Search anything
    template<typename ...Args>
    Student* addStudent(const ull& group, const ull& id, const Args& ...args);
    template<typename ...Args>
    Event* addEvent(const ull&, const Args& ...args);
    auto addGroup(const ull& no) {
        return groups.try_emplace(no); //TODO: Is ok?
    }
    void erase(Student* s);
    bool erase(const ull& g);
    void erase(Event* e);
    std::string printEvents();
    std::string printGroups();
    std::string printStudents(const ull& gid);
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
    std::map<ull,std::vector<std::unique_ptr<Student>>> groups;
    std::vector<std::unique_ptr<Event>> events;
    std::map<std::string, std::string> users; // holds <login, password> (hashed)
    std::map<std::string, std::string> admins; //same
};

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
                                  "\n1 -> Show all groups and students "\
                                  "\n2 -> Show all events "\
                                  "\n3 -> Show all students of a group "\
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

