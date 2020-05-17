#include "header.h"

void Data::erase(Student* s) {
    plog->put("Called erase on", s->getName());
    for (auto& g: groups)
        for (auto it = g.second.begin(); it != g.second.end(); ++it)
            if (s->id() == (*it)->id()) {
                g.second.erase(it); //changes the underlying vector but never continues looking through
                return;
            }
}
void Data::erase(Event* e) {
    plog->put("Called erase on", e->getName());
    for (auto it = events.begin(); it != events.end(); ++it)
        if (e->id() == (*it)->id()) {
            events.erase(it);
            return;
        }
}

bool Data::erase(const ull& g) {
    plog->put("Called erase on", g);
    auto res = groups.find(g);
    if (res == groups.end())
        return false;
    else {
        groups.erase(res);
        return true;
    }
}

std::string Data::printGroups() {
    std::stringstream ss;
    for (auto& el : groups) {
        ss << "-| Group # " << el.first << " |-" << std::endl;
        ss << printStudents(el.first) << "\n";
    }
    return ss.str();
}
std::string Data::printStudents(const ull& gid) {
    fort::char_table t;
    const auto& g = groups.find(gid);
    if (g == groups.cend()) return "No such group #" + std::to_string(gid) + " exists\n";
    t << fort::header << "Name" << "Events" << "Degree" << "Birthdate" << "Age" << "GPA" << "Tuition" << "ID" << fort::endr;
    for (const auto& s: g->second) {
        std::stringstream ev;
        std::string delim;
        size_t i = 0;
        for (const auto& l: s->links) {
            i++;
            ev << delim << i << '.' << l->getName(); //Add numbers to select something later
            delim = '\n';
        }
        t << s->getName() << ev.str() << s->degree << s->birthdate << s->getAge() << s->avgGrade << (s->isTuition ? "Yes" : "No") << s->id()
          << fort::endr;
    }
    setTableProperties(t, 0, 7);
    return t.to_string();
}
std::string Data::printEvents() {
    fort::char_table t;
    t << fort::header << "Name" << "Participants" << "Place" << "Date" << "ID" << fort::endr;
    for (const auto& e: events) {
        std::string delim;
        size_t cnt = 0;
        std::stringstream ss;
        for (const auto& b: e->links) {
            ++cnt;
            ss << delim << cnt << '.' << b->getName(); //Append the title
            delim = '\n';

        }
        t << e->getName() << ss.str() << e->place << e->date << e->id() << fort::endr; //Output
    }
    setTableProperties(t, 0, 4);
    return t.to_string();
}
bool Data::delAccount(const std::string& l, const bool& isadmin) {
    if (isadmin) //For admins and users
    {
        auto sought = admins.find(l);
        if (sought == admins.end() || admins.size() < 2) return false; //Can't delete the last account
        admins.erase(sought);
    }
    else {
        auto sought = users.find(l);
        if (sought == users.end()) return false; //Can delete all
        users.erase(sought);
    }
    plog->put("deleted account", l);
    return true;
}
std::string Data::printCredentials(bool isAdmin) {
    std::stringstream ss;
    ss << (isAdmin ? "Admin" : "User") << " credentials: " << std::endl;
    for (const auto& el: (isAdmin ? admins : users))
        ss << el.first << "\n"; //Don't print the password
    ss << std::endl;
    return ss.str();
}

std::vector<Entry*> Data::search(std::string& str) //Search anything
{
    std::vector<Entry*> ret; //Create the result
    plog->put("Started data.search()");
    for (auto& e : events)
        if (e->check(str)) ret.push_back(e.get());
    if (checkString(str, 'i').empty()) {
        ull no = stoid(str);
        auto gr = groups.find(no);
        if (gr != groups.end())
            for (auto& el: gr->second)
                ret.push_back(el.get());
    }
    else
        for (auto& g : groups)
            for (auto& s: g.second)
                if (s->check(str)) ret.push_back(s.get());
    return ret;
}

std::vector<Entry*> Data::sieve(ull gid,std::string& str) //Search anything
{
    std::vector<Entry*> ret; //Create the result
    plog->put("Started data.sieve()");
    auto gr = groups.find(gid);
    if (gr != groups.end()) {
        for (auto& el: gr->second)
            if (el->checkLinks(str)) ret.push_back(el.get());
    }
    else plog->put("Couldn't find group #", gid);
    return ret;
}

//LOAD

void Data::load() try //Try-catch function block
{
    plog->put("Called data.load()");
    std::string tempA, tempB, name = "user.txt"; //Reusable
    auto fexc = [& name](const std::string& what) { throw std::invalid_argument("File: " + name + " couldn't read " + what); };
    ensureFileExists(name); //Make sure
    std::ifstream f(path + name); //Open using the path
    auto eof = [&f]() //Lambda to check for the end of file.
    { return f.peek() == EOF; };
    while (f) //While file f is good (converted to bool)
    { //USER
        if (eof()) break; //If the file has ended break out
        if (!readString(f, tempA, 'n')) fexc("login");
        if (!readString(f, tempB, 'n')) fexc("password");
        users.try_emplace(tempA, tempB); //Create a new entry
        //continues to read if f is good;
    }
    std::cout << "Successfully read users" << std::endl; //Little debugging
    name = "admin.txt";
    f.close();
    ensureFileExists(name);
    f.open(path + name); //Open another file
    while (f) //Starts parsing the file. Paragraphs are divided by a blank line
    { //ADMIN
        if (eof()) break; //See above
        if (f.eof()) break;
        if (!readString(f, tempA, 'n')) fexc("login");
        if (!readString(f, tempB, 'n')) fexc("password");
        admins.try_emplace(tempA, tempB);
    }
    std::cout << "Successfully read admins" << std::endl;
    if (admins.empty()) //If there are no admins
    {
        std::cerr << "Warning! We  couldn't find any valid administrator accounts. \n" //We MUST have at least one.
                     "Created a new one: admin | admin" << std::endl;
        addAccount("admin", "admin", true); //here we use hashing
    }
    name = "Events.txt";
    f.close(); //Same
    ensureFileExists(name);
    f.open(path + name);
    std::string tempC, tempD;
    while (f) { //id, name, date, place
        if (eof()) break;
        if (!readString(f, tempA, 'i')) fexc("ID");
        if (!readString(f, tempB, 's')) fexc("name");
        if (!readString(f, tempC, CHECK::DATE)) fexc("date");
        if (!readString(f, tempD, 's')) fexc("place");
        addEvent(stoid(tempA), tempB, tempC, tempD);
    }
    std::cout << "Successfully read events" << std::endl;
    f.close();
    name = "Students.txt";
    ensureFileExists(name);
    f.open(path + name);
    std::string tempE, tempF, tempG;
    std::string gid;
    std::pair<decltype(groups.begin()), bool> group = std::make_pair(groups.end(), 0);
    while (f) {  //id, name, degree, date, tuition, grade
        plog->put("Starting to parse a new group");
        while (!eof()) {
            std::getline(f, tempA);
            if (tempA.empty()) fexc("ID");
            if (tempA[0] == '#') {
                plog->put("Found gid:", tempA);
                tempA.erase(0, 1);
                if (!checkString(tempA, 'i').empty()) fexc("Group ID");
                gid = tempA;
                group = addGroup(stoid(gid));
                plog->put((group.second ? "Added group" : "Couldn't add group"), gid);
                break;
            }
            if (!checkString(tempA, 'i').empty()) fexc("ID");
            if (!readString(f, tempB, 's')) fexc("name");
            if (!readString(f, tempC, 's')) fexc("events");
            if (!readString(f, tempD, 's')) fexc("degree");
            if (!readString(f, tempE, CHECK::DATE)) fexc("date");
            if (!readString(f, tempF, 'b')) fexc("tuition");
            if (!readString(f, tempG, 'f')) fexc("grade");
            Student* ps = addStudent(stoid(gid), stoid(tempA), tempB, tempD, tempE, std::stoi(tempF), std::stof(tempG));
            std::stringstream ss(tempC);
            while (getline(ss, tempC, ',')) { //May change it
                if (!checkString(tempC, 'i').empty()) fexc("student's event ID");
                ull sid = stoid(tempC);
                auto sought = std::find_if(events.begin(), events.end(),
                                           [&sid](std::unique_ptr<Event>& pev) { return sid == pev->id(); }); //Search for that ID and save
                plog->put("sought.first is: ", (sought != events.end() ? (*sought)->getName() : "NULL"));
                if (sought == events.end()) //If we didn't find anything
                {
                    plog->put("Creating new unknown event and linking with", ps->getName());
                    addEvent(stoid(tempD), "Unknown event", "0.0.0000", "Unknown")->link(ps); //Handle the error, create a blank entry
                }
                else {
                    plog->put("Linking", (*sought)->getName(), "+", ps->getName());
                    (*sought)->link(ps); //Else it's fine to link
                }
                tempC.clear(); //We will need it later
            }
        }
    }
} //Finished linking
catch (...) {
    std::cerr << "Error while reading files. The program cannot continue." << std::endl;
    throw; //Rethrow, main() will deal
}
void Data::save() {
    std::cout << "Saving..." << std::endl;
    std::ofstream f(path + "user.txt"); //Open a file
    for (auto& el: users)
        f << el.first << "\n" << el.second << '\n'; //Write
    f.close(); //Reopen for a new file
    f.open(path + "admin.txt");
    for (auto& el: admins)
        f << el.first << "\n" << el.second << '\n';
    f.close();
    f.open(path + "Events.txt");
    f << std::setfill('0'); //For numbers
    for (auto& e : events)
        f << std::setw(MAX_ID_LENGTH) << e->id() << '\n'
          << e->getName() << '\n'
          << e->date << '\n'
          << e->place
          << std::endl;
    f.close();
    f.open(path + "Students.txt");
    for (auto& g : groups) {
        if (g.second.empty()) continue; //Skip empty groups
        f << "#" << g.first << std::endl;
        for (auto& s : g.second) {
            f << std::setw(MAX_ID_LENGTH) << s->id() << '\n'
              << s->getName() << '\n';
            std::string delim;
            for (auto& l: s->links) {
                f << delim << std::setw(MAX_ID_LENGTH) << l->id();
                delim = ',';
            }
            f << '\n'
              << s->degree << '\n'
              << s->birthdate << '\n'
              << s->isTuition << '\n'
              << s->avgGrade << '\n';
        }
    }
    f << std::setfill(' ');
}