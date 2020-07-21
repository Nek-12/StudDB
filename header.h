#pragma once
#include "misc.h"
class Entry;
class Student;
class Event;

//Base class
/*
 * Each journal entry represents some kind of table row that contains different info for each entry, but the operations are same.
 * This is some kind of merged interface and usual base class. Will be used with dynamic binding later extensively, so I tried to provide
 * A lot of virtual functions to minimize dynamic casts and RTTI which are slow
*/
class Entry {
    friend class Data;
    friend std::ostream& operator<<(std::ostream& os, const Entry& e) { //dynamic binding here
        os << e.to_string(); //Each type prints its own info, virtual
        return os;
    }
    friend bool operator==(const Entry& lhs, const Entry& rhs) //For some containers and functions
    { plog->put("called", lhs.getName(), "==", rhs.getName()); return lhs.id() == rhs.id(); } //applicable to any entry because of dynamic binding
public:
    Entry() = delete; //No blank entries
    Entry(const Entry& e) = delete; //No copies
//    Copying an entry doesn't make sense. Why would we need several identical lines in our journal?
    virtual ~Entry();
    [[nodiscard]] const ull& id() const { return unique_id; }//We shouldn't discard the returned value for the getter. Would make no sense. Just for safety
    [[nodiscard]] std::string getName() const { return name; }
    void rename(const std::string& s) { name = s; }
    bool link(Entry* e);
    bool unlink(Entry* e);
    void unlink(const size_t& pos);
    bool checkLinks(const std::string& str);
    size_t enumLinks() {return links.size();}
protected: //Constructors are protected to disallow users creating entries in the journal. It's unsafe and makes no sense. Users will use the
    //Appropriate functions from Data
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
        auto age = getCurYear() - stoi(birthdate.substr(birthdate.find_last_of('.')+1,4));
        return (age == getCurYear()? 0 : age);
    }
    bool switchTuition() { return isTuition = !isTuition; }
    void setAvgGrade(const float& avg) { avgGrade = avg;}
    void setDegree(const std::string& d) { degree = d; }
    void setBirthDate(const std::string& d) { birthdate = d; }
private:
    //CONSTRUCTOR
    explicit Student(const ull& no, const std::string& n, std::string d, std::string b, bool i, float gr)
            : Entry(no, n), degree(std::move(d)), birthdate(std::move(b)), isTuition(i), avgGrade(gr) {}
    [[nodiscard]] bool check(const std::string& s) const override;
    [[nodiscard]] std::string to_string() const override;
    std::string degree;
    std::string birthdate;
    bool isTuition; //Are they paying for the education?
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
    ~Data() {
        events.clear(); //To ensure we don't call a pure virtual, we need a strict order.
        groups.clear();
    }
    static Data* getInstance() //Returns a reference to the single static instance of Data.
    {
        static Data instance; //The instance is always one and lazy-evaluated on the first use
        return &instance; //Return a pointer to self
    }
    void load(); //Loads all the data from several files
    void save(); //Writes the data to the files
    std::string printCredentials(bool isadmin);
    std::vector<Entry*> search(std::string& s); //Search anything
    template<typename... Args>
    Student* addStudent(const ull& gid, const ull& id, const Args& ... args) {
        plog->put("Called add with args", id, args...);
        if (groups.find(gid) == groups.end()) return nullptr;
        for (auto& el: groups[gid])
            if (el->id() == id)
                return nullptr; //Nullptr if we don't have any entries
        return groups[gid].emplace_back(new Student(id, args...)).get(); // TODO: Don't return raw pointers
    }
    template<typename... Args>
    Event* addEvent(const ull& id, const Args& ... args) {
        plog->put("Called add with args", id, args...);
        for (auto& el: events)
            if (el->id() == id) return nullptr;
        return events.emplace_back(new Event(id, args...)).get();
    }
    std::vector<Entry*> sieve(ull, const std::string& str); //Checks the events in the group
    auto addGroup(const ull& no) { return groups.try_emplace(no); }
    bool findGroup(const ull g) { return groups.find(g) != groups.end(); }
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
    size_t enumAccounts(const bool& isadmin)
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
    //Groups are stored in a map where the key is the group #, and each group contains a vector of pointers to Students, allocated on the heap
    std::vector<std::unique_ptr<Event>> events;
    std::map<std::string, std::string> users; // holds <login, password> (hashed)
    std::map<std::string, std::string> admins; //same
};

#define LOGINPROMPT  "Enter the login or \"exit\" to exit: "
#define PASSPROMPT  "Enter the password or \"exit\" to exit: "
#define PASSCONFIRM  "Confirm the password or enter \"exit\" to exit: "
#define ADMIN_CONSOLE_ENTRIES    ":ADMIN:"\
                                 "\nSelect an option: "\
                                 "\n1 -> Manage entry data "\
                                 "\n2 -> Change your password"\
                                 "\n3 -> Register an administrator"\
                                 "\n4 -> Delete users "\
                                 "\n5 -> Add users"\
                                 "\n0 -> Delete your account (careful!)"\
                                 "\nq -> Sign off"
#define USER_CONSOLE_ENTRIES    ":USER:"\
                                "\nSelect an option: "\
                                "\n1 -> Manage entry data "\
                                "\n2 -> Change your password"\
                                "\n0 -> Delete your account"\
                                "\nq -> Sign off"
#define ADMIN_MANAGEMENT_ENTRIES    ":ADMIN:"\
                                    "\nSelect an option: "\
                                    "\n1 -> Search anything "\
                                    "\n2 -> Show data "\
                                    "\n3 -> Manage entries "\
                                    "\n4 -> Add a new student"\
                                    "\n5 -> Add a new event"\
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
                                  "\n4 -> Show all students of a group for a given event name"\
                                  "\nq -> Go back"
#define WELCOME_MENU "Welcome. "\
                     "\n1 -> User sign in "\
                     "\n2 -> Admin sign in "\
                     "\nq -> Save and exit"
#define EDIT_EVENT_OPTIONS "Select an option: "\
                             "\n1 -> Rename "\
                             "\n2 -> Change the location "\
                             "\n3 -> Change the date "\
                             "\n4 -> Assign students "\
                             "\n5 -> Remove students "\
                             "\n6 -> Delete"\
                             "\nq -> Go back"
#define EDIT_STUDENT_OPTIONS "What would you like to do? "\
                            "\n1 -> Edit name"\
                            "\n2 -> Edit degree"\
                            "\n3 -> Edit birthdate"\
                            "\n4 -> Switch the education model"\
                            "\n5 -> Edit the average grade"\
                            "\n6 -> Assign to events"\
                            "\n7 -> Remove from events"\
                            "\n8 -> Delete"\
                            "\nq -> Nothing"
#define SORT_RESULTS_OPTIONS "How would you like to sort the result?"\
                             "\n1 -> By Name Ascending"\
                             "\n2 -> By Name Descending"\
                             "\n3 -> By ID"\
                             "\nq -> Don't care"
