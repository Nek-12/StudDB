//NOTE: READ THIS FILE IN REVERSE DIRECTION (FROM THE END)
#include "header.h"

std::string path; //extern global string
Data* data = nullptr;
Log* plog = nullptr;
#ifdef __linux__
int getch() //Getch for linux
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(0, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(0, TCSANOW, &oldattr);
    return (ch);
}
#endif
//TODO: Edit comments
inline void cls() //This function depends on platform
{ system(CLS); }
bool yesNo(const std::string& msg);
ull inputID();
ull select(const ull& limit);
std::vector<Entry*> search();
Entry* selectEntry();
void addEntries(Entry* pe);
void manageEntry();
void showData();
void management(bool isadmin);
bool passConfirm(std::string& p);
bool passChange(const std::string& l, bool isadmin);
void manageUsr();
void createAccPrompt(bool isadmin);
bool delDialog(const std::string& l, bool isadmin);
void console(const std::string& usr, bool);
void login(bool isadmin);

//Not all declarations are needed because I arranged the functions properly. However this is a bad practice
bool yesNo(const std::string& msg) { //Asks for confirmation
    std::cout << msg << " y/n" << std::endl;
    while (true) {
        switch (tolower(getch())) {
            case 'y':
                return true;
            case 'n':
                return false;
            default :
                break;
        }
    }
}

ull inputID() { //You can choose if you want a new ID or you already know one
    std::string id;
    if (!yesNo("Generate an ID?")) {
        std::cout << "Enter the ID: ";
        while (!readString(std::cin, id, 'i'));
    }
    else
        return genID();
    return stoid(id);
}

ull select(const ull& limit) { //Select from some kind of range, used in search primarily
    while (true) {
        std::string s;
        while (!readString(std::cin, s, 'i'));
        ull ret = stoid(s);
        if (ret > limit || ret == 0) {
            std::cerr << "You have selected a bad value. Try again: " << std::endl;
            continue;
        }
        else return ret;
    }
}

void sortResults(std::vector <Entry*>& v) {
    std::cout << SORT_RESULTS_OPTIONS << std::endl;
    while(true) {
        switch (getch()) {
            case '1':
                std::sort(v.begin(),v.end(),[](Entry* left, Entry* right){ return left->getName() < right->getName();});
                return;
            case '2':
                std::sort(v.begin(),v.end(),[](Entry* left, Entry* right){ return left->getName() > right->getName();});
                return;
            case '3':
                std::sort(v.begin(),v.end(),[](Entry* left, Entry* right){ return left->id() < right->id();});
                return;
            case 'q':
                return;
            default:
                break;
        }
    }
}

std::vector<Entry*> search() {
    std::vector<Entry*> sought; //There can be several results depending on our query.
    while (true) {
        cls();
        std::string s;
        std::cout << "Enter any property of anything to search (case ignored): " << std::endl;
        while (!readString(std::cin, s, 's'));
        sought = data->search(s); //All kinds of entries there
        if (sought.empty()) {
            std::cerr << "Nothing found." << std::endl;
        }
        else {
            if (sought.size() >= 3) sortResults(sought);
            std::cout << "Found: " << std::endl;
            for (auto el: sought)
                std::cout << *el << std::endl; //Print
        }
        if (yesNo("Try again?")) continue;
        break;
    }
    return sought;
}

Entry* selectEntry() {
    auto sought = search(); //First search
    if (sought.empty()) return nullptr; //If nothing go back
    else if (sought.size() == 1) //If one entry select it immediately
        return sought[0]; //TODO: Maybe we should ask if we want to use the entry found?
    cls();
    for (auto it = sought.begin(); it != sought.end(); ++it) //If more print them and select just one
        std::cout << "#" << it + 1 - sought.begin() << ":\n" << **it;
    std::cout << "Select the entry's #: " << std::endl;
    return sought[select(sought.size()) - 1]; //select one
}

void addEntries(Entry* pe) {
    while (true) {
        if (!yesNo("Add an entry to " + pe->getName() + " ?")) break;
        Entry* sought = selectEntry();
        if (!sought) break; //If we found nothing
        if (pe->link(sought)) //We don't care, there are overridden functions which are going to take care of that for us.
            std::cout << "Successfully linked " << sought->getName() << " and " << pe->getName() << std::endl;
        else
            std::cout << "Please select a valid entry to link with " << pe->getName() << " !" << std::endl;
    }
}

Event* newEvent() {
    cls();
    std::string n, d, p;
    ull id = inputID();
    std::cout << "Enter the event's title" << std::endl;
    while (!readString(std::cin, n, 's'));
    std::cout << "Enter the place " << std::endl;
    while (!readString(std::cin, p, 's'));
    std::cout << "Enter the date" << std::endl;
    while (!readString(std::cin, d, CHECK::DATE_FUTURE));
    Event* added = data->addEvent(id, n, d, p);
    if (!added) {
        std::cout << "Such event already exists" << std::endl;
        return nullptr;
    }
    addEntries(added);
    std::cout << "Successfully added the event " << n << std::endl;
    pause();
    return added;
}

Student* newStudent() {
    std::string temp, name, degree, date;
    float grade;
    ull id, gid;
    std::cout << "Enter the group's #" << std::endl;
    while (!readString(std::cin, temp, 'i')); //Once we read the group no
    gid = stoid(temp);
    if(!data->findGroup(gid) ) {
        if (yesNo("Group not found. Add a new one?")) data->addGroup(gid);
        else return nullptr;
    }
    id = inputID();
    std::cout << "Enter the new student's name" << std::endl;
    while (!readString(std::cin, name, 's'));
    std::cout << "Enter the new student's degree" << std::endl;
    while (!readString(std::cin, degree, 's'));
    std::cout << "Enter the new student's birthdate" << std::endl;
    while (!readString(std::cin, date, CHECK::DATE_PAST));
    bool tuition = yesNo("Are they on tuition?");
    std::cout << "Enter the new student's GPA" << std::endl;
    while (!readString(std::cin, temp, 'f'));
    grade = stof(temp);
    Student* added = data->addStudent(gid, id, name, degree, date,tuition,grade);
    if (!added) //If wasn't added
    {
        std::cerr << "Such student already exists." << std::endl;
        pause();
        return nullptr; //Return nothing
    }
    addEntries(added);
    std::cout << "Successfully added "<< added->getName() << " to group #" << gid << std::endl;
    pause();
    return added;
}

void manageStudent(Student* ps) {
    std::string temp;
    auto suc = []() {std::cout << "Changed successfully." << std::endl; };
    while (true) {
        cls();
        std::cout << *ps << std::endl;
        std::cout << EDIT_STUDENT_OPTIONS << std::endl;
        switch (getch()) {
            case '1':
                std::cout << "Enter the new name for the student: " << std::endl;
                while (!readString(std::cin, temp, 's'));
                ps->rename(temp);
                suc();
                break;
            case '2':
                std::cout << "Enter the new degree: " << std::endl;
                while (!readString(std::cin, temp, 's'));
                ps->setDegree(temp);
                suc();
                break;
            case '3':
                std::cout << "Enter the new birthdate: " << std::endl;
                while (!readString(std::cin, temp, CHECK::DATE_PAST));
                ps->setBirthDate(temp);
                suc();
                break;
            case '4':
                if (yesNo("Switch tuition?")) ps->switchTuition();
                break;
            case '5':
                std::cout << "Enter the new average grade: " << std::endl;
                while (!readString(std::cin, temp, 'f'));
                ps->setAvgGrade(std::stof(temp));
                suc();
                break;
            case '6':
                addEntries(ps); //We can add entries to anything
                break;
            case '7':
                std::cout << "Select an event's #: " << std::endl; //Once printed, there are numbers in our table
                ps->unlink(select(ps->enumLinks() ) - 1);
                suc();
                pause();
                break;
            case '8':
                if (yesNo("Delete this record?")) {
                    data->erase(ps);
                    std::cout << "Erased this record and removed all references." << std::endl;
                    pause();
                    return;
                }
                else return;
            case 'q':
                return;
            default :
                break;
        }
    }
}

void manageEvent(Event* pev) {//Specialized actions for every entry
    std::string temp;
    while (true) {
        cls();
        std::cout << *pev << std::endl; //Print what we are editing
        std::cout << EDIT_EVENT_OPTIONS << std::endl;
        switch (getch()) {
            case '1':
                std::cout << "Enter the new event's title" << std::endl;
                while (!readString(std::cin, temp, 's'));
                pev->rename(temp);
                std::cout << "Renamed to " << temp << std::endl;
                pause();
                break;
            case '2':
                std::cout << "Enter the new event's place" << std::endl;
                while (!readString(std::cin, temp, 's'));
                pev->setPlace(temp);
                std::cout << "Changed the place to " << temp << std::endl;
                pause();
                break;
            case '3':
                std::cout << "Enter the new event's date" << std::endl;
                while (!readString(std::cin, temp, CHECK::DATE_FUTURE));
                pev->setDate(temp);
                std::cout << "Changed the date to " << temp << std::endl;
                pause();
                break;
            case '4':
                addEntries(pev);
                break;
            case '5':
                std::cout << "Select a participant's #: " << std::endl; //Once printed, there are numbers in our table
                pev->unlink(select(pev->enumLinks() ) - 1); //Select the number
                std::cout << "Removed successfully" << std::endl;
                pause();
                break;
            case '6':
                if (yesNo("Delete this record?")) {
                    std::cout << "Erased this event and removed all references." << std::endl;
                    data->erase(pev);
                    pause();
                    return;
                }
                else return;
            case 'q':
                return;
            default:
                break;
        }
    }
}

void manageEntry() {//Uses RTTI to know which entry we are editing.
    Entry* pe = selectEntry(); //Find an entry to edit and select it
    if (!pe) return;
    if (typeid(*pe) == typeid(Student))
        manageStudent(static_cast<Student*>(pe));
    else if (typeid(*pe) == typeid(Event))
        manageEvent(static_cast<Event*>(pe)); //Static cast is SAFE since we got the typeid match
}

void showData() {
    std::string temp;
    ull gid;
    cls();
    std::cout << SHOW_DATA_MENU_ENTRIES << std::endl;
    while (true) {
        switch (getch()) {
            case '1':
                cls();
                std::cout << data->printGroups(); //Print tables
                pause();
                return;
            case '2':
                cls();
                std::cout << data->printEvents();
                pause();
                return;
            case '3':
                cls();
                std::cout << "Enter the group #: " << std::endl;
                while (!readString(std::cin, temp, 'i'));
                gid = stoid(temp);
                std::cout << data->printStudents(gid);
                pause();
                return;
            case '4':
                cls();
                std::cout << "Enter the group #: " << std::endl;
                while (!readString(std::cin, temp, 'i'));
                gid = stoid(temp);
                if (data->findGroup(gid)) {
                    std::cout << "Enter the selection property: " << std::endl;
                    while (!readString(std::cin, temp, 's'));
                    auto res = data->sieve(gid, temp);
                    if (!res.empty()) {
                        std::cout << "Found: " << std::endl;
                        for (auto el: res)
                            std::cout << *el << std::endl; //Print
                    }
                    else std::cout << "Nothing found" << std::endl;
                } else
                    std::cout << "No such group exists" << std::endl;
                pause();
                return;
            case 'q':
                return;
            default:
                break;
        }
    }
}

void management(bool isadmin) {//Differentiates between right levels
    while (true) {
        cls();
        std::cout << (isadmin ? ADMIN_MANAGEMENT_ENTRIES : USER_MANAGEMENT_ENTRIES) << std::endl;
        switch (getch()) {
            case 'q':
                return;
            case '1':
                search(); //Search anything
                break;
            case '2':
                showData(); //Show tables
                break;
            case '3':
                if (isadmin) manageEntry(); //For admins: create and manage entries
                break;
            case '4':
                if (isadmin) newStudent();
                break;
            case '5':
                if (isadmin) newEvent();
                break;
            default:
                break;
        }
    }
}

bool passConfirm(std::string& p) {
    std::string tempA, tempB;
    while (true) {
        cls();
        std::cout << PASSPROMPT << std::endl;
        while (!readString(std::cin, tempA, 'p'));
        if (tempA == "exit") return false;

        std::cout << PASSCONFIRM << std::endl;
        while (!readString(std::cin, tempB, 'p'));
        if (tempB == "exit") return false;
        if (tempA == tempB)
            break;
        std::cerr << "Your passwords don't match." << std::endl;
    }
    p = tempA; //Changes the password it was given if they match
    return true;
}

bool passChange(const std::string& l, bool isadmin) {
    std::string p;
    if (!passConfirm(p)) return false; //It changes the string it was given on success
    data->changePass(l, p, isadmin);
    std::cout << "Your password was changed successfully." << std::endl;
    pause();
    return true;
}

void manageUsr() //Admins can delete users, but not admins (except own)
{
    std::string l, p;
    while (true) {
        cls();
        std::cout << data->printCredentials(false); //To see which users to delete
        std::cout << LOGINPROMPT << std::endl;
        while (!readString(std::cin, l, 'n'));
        if (l == "exit") return;
        if (!data->loginCheck(l, false)) //Check if the user exists before deleting
            std::cout << "User not found." << std::endl;
        else {
            data->delAccount(l, false);
            std::cout << "Deleted account " << l << std::endl;
            if (data->enumAccounts(false) == 0) {
                std::cout << "No accounts left. Exiting." << std::endl;
                pause();
                return;
            }
        }
        if (yesNo("Delete another one?")) continue;
        else return;
    }
}

void createAccPrompt(bool isadmin) {
    std::string l, p, temp;
    cls();
    std::cout << LOGINPROMPT << std::endl;
    while (!readString(std::cin, l, 'n'));
    if (l == "exit") return;
    if (data->loginCheck(l, isadmin) || data->loginCheck(l, !isadmin)) //We can't have both admin and user with the same username.
    {
        std::cerr << "Such account already exists!" << std::endl;
        pause();
        return;
    }
    if (!passConfirm(p)) return;
    data->addAccount(l, p, isadmin);
    std::cout << "Successfully created account " << l << " ! Going back..." << std::endl;
    pause();
}

bool delDialog(const std::string& l, bool isadmin) {
    std::string p;
    while (true) {
        cls();
        std::cout << "THIS WILL DELETE YOUR ACCOUNT AND YOU WILL BE LOGGED OFF."
                  << "\nTYPE YOUR PASSWORD, " << l << ", TO PROCEED OR \"exit\" TO CANCEL." << std::endl;
        while (!readString(std::cin, p, 'p'));
        if (p == "exit") return false;
        if (data->passCheck(l, p, isadmin))
            break;
        else
            std::cerr << "Wrong password." << std::endl;
    }
    if (!data->delAccount(l, isadmin)) {
        std::cerr << "You can't delete the last account!" << std::endl;
        //Only valid for admins, you have to remove the generated admin | admin acc. Feel free to remove all users.
        pause();
        return false;
    }
    std::cout << "Deleted account  " << l << std::endl;
    pause();
    return true; //Means was deleted
}

void console(const std::string& usr, bool isadmin) {
    while (true) {
        cls();
        std::cout << (isadmin ? ADMIN_CONSOLE_ENTRIES : USER_CONSOLE_ENTRIES) << std::endl; //Different options for every1
        switch (getch()) {
            case '1':
                management(isadmin);
                break;
            case '2':
                passChange(usr, isadmin);
                break;
            case '3':
                if (isadmin) createAccPrompt(true); //Conditional statements
                else continue;
                break;
            case '4':
                if (isadmin) manageUsr();
                else continue;
                break;
            case '5':
                if (isadmin) createAccPrompt(false);
                else continue;
                break;
            case '0':
                if (delDialog(usr, isadmin)) return;
                else break;
            case 'q':
                return;
            default:
                continue;
        }
    }
}

void login(bool isadmin) {
    std::string usr, pass;
    while (true) //While the user did not enter his username
    {
        cls();
        std::cout << LOGINPROMPT << std::endl; //Ask for the username
        if (!readString(std::cin, usr, 'n')) //Read in normal mode
        {
            pause(); //On error start over
            continue;
        }
        if (usr == "exit") return;
        if (data->loginCheck(usr, isadmin)) //If user exists
            break; //Move to the next step
        else
            std::cout << "User not found." << std::endl;
        pause();
    }
    while (true) //While the user did not enter his password
    {
        cls();
        std::cout << PASSPROMPT << std::endl; //Ask
        if (!readString(std::cin, pass, 'p')) //Read in password mode
        {
            pause();
            continue;
        }
        if (pass == "exit") return;
        if (data->passCheck(usr, pass, isadmin)) //If the login and password match
            break; //Summon the console
        else
            std::cout << "Wrong password.\n";
        pause();
    }
    std::cout << "Success. Redirecting..." << std::endl;
    pause();
    console(usr, isadmin);
}
int main(int, char* argv[]) try {
    path = argv[0];
    path.erase(path.find_last_of('\\') + 1); //Makes 'path' be the path to the app folder, removing program name
//Try function block for convenience. Argc is unused, argv is an array of char arrays, each with an argument, first is path
    plog = Log::init(path + "log.txt");
    data = Data::getInstance(); //Assign to our global pointer. For exception safety
        data->load(); //Loads ALL the data
#ifndef NDEBUG //For debugging
    std::cout << data->printGroups() << data->printEvents() << '\n' << path << '\n'
    << data->printCredentials(false) << '\n' << data->printCredentials(true) << std::endl;
    getch();
#endif
    bool workin = true, first = true; //The first time we don't clear the screen to show the user the info from data->load()
    while (workin) //While the user didn't quit
    {
        if (!first) cls(); //clear the screen
        first = false; //not first anymore
        std::cout << WELCOME_MENU << std::endl; //Draw a menu
        switch (tolower(getch())) //Switch the input from a user
        {
            case 'q': //Exit
                workin = false;
                break;
            case '1':
                login(false);
                break;
            case '2':
                login(true); //log the admin in
                break;
            default:
                break;
        }
    }
    data->save(); //Saving only before exiting only to avoid corrupting the database
    plog->put("Reached end of the program");
    return EXIT_SUCCESS;
}
catch (std::exception& e) {//If an exception is thrown, the program 100% can't continue. RIP.
    std::cerr << "Critical Error: " << e.what() << "\n The program cannot continue. Press any key to exit..." << std::endl;
    plog->put("Exception", e.what());
    getch();
    return (EXIT_FAILURE);
}
catch (...) { //Sometimes we can get something completely random. In this case we just exit
    std::cerr << "Undefined Error. \n The program cannot continue. Press any key to exit..." << std::endl;
    plog->put("Unhandled exception");
    getch();
    return (EXIT_FAILURE);
}