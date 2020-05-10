//NOTE: READ THIS FILE IN REVERSE DIRECTION (FROM THE END)
#include "header.h"

std::string path; //extern global string
Data* data = nullptr;
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
inline void cls() //This function depends on platform
{ system(CLS); }
//TODO: Fix: When adding/editing books, there is no check that GENRE or AUTHOR with that ID is present and vice versa
bool yesNo(const std::string& msg);
ull inputID();
ull select(const ull& limit);
std::vector<Entry*> search();
Entry* selectEntry();
void addEntries(Entry* pe);
Author* newAuthor();
Book* newBook();
Genre* newGenre();
void editBookEntries(Book* pb);
void manageBook(Book* pb);
void manageAuthor(Author* pa);
void manageGenre(Genre* pg);
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
        std::cout << "#" << it + 1 - sought.begin() << ":\n" << **it; //There are genres, authors, books, up to user.
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

Author* newAuthor() {//Add a new author and, if needed, provide a recursion to add something else.
    cls();
    std::string n, d, c;
    ull id = inputID();
    std::cout << "Enter author's credentials" << std::endl;
    while (!readString(std::cin, n, 's'));
    std::cout << "Enter the author's birthdate" << std::endl;
    while (!readString(std::cin, d, 'd'));
    std::cout << "Enter the author's country" << std::endl;
    while (!readString(std::cin, c, 's'));
    Author* added = data->addAuthor(id, n, d, c, id);
    if (!added) {
        std::cout << "Such author already exists" << std::endl;
        return nullptr;
    }
    addEntries(added);
    std::cout << "Successfully added author " << n << std::endl;
    pause();
    return added;
}

Book* newBook() {//The same logic as in the newGenre() see below
    cls();
    std::string n, a, y;
    ull id = inputID();
    std::cout << "Enter the title of the book" << std::endl;
    while (!readString(std::cin, n, 's'));
    std::cout << "Enter the year the book was published" << std::endl;
    while (!readString(std::cin, y, 'y'));
    Book* added = data->addBook(id, n, stoid(y), id);
    if (!added) {
        std::cerr << "Such book already exists." << std::endl;
        return nullptr;
    }
    addEntries(added);
    std::cout << "Successfully added your book" << std::endl;
    pause();
    return added;
}

Genre* newGenre() {
    std::string temp;
    std::cout << "Enter the new genre's name" << std::endl;
    while (!readString(std::cin, temp, 's')); //Once we read the name
    ull id = inputID();
    Genre* added = data->addGenre(id, temp, id); //Add new genre to the Data
    if (!added) //If wasn't added
    {
        std::cerr << "Such genre already exists." << std::endl;
        pause();
        return nullptr; //Return nothing
    }
    addEntries(added);
    std::cout << "Successfully added your genre" << std::endl;
    pause();
    return added;
}

void editBookEntries(Book* pb)  {//Books are more complicated, so special menu
    if (pb == nullptr) return;
    while (true) {
        cls();
        std::cout << *pb << std::endl;
        std::cout << "Select an option for book " << pb->getName() <<
                  "\n1 -> Add entries to the book "
                  "\n2 -> Remove authors from the book "
                  "\n3 -> Remove genres from the book "
                  "\nq -> Go back" << std::endl;
        switch (getch()) {
            case '1':
                addEntries(pb); //Dynamic binding
                break;
            case '2':
                std::cout << "Select an author's #: " << std::endl;
                pb->remAuthor(select(pb->enumAuthors()) - 1);
                std::cout << "Removed successfully" << std::endl;
                pause();
                break;
            case '3':
                std::cout << "Select a genre's #: " << std::endl;
                pb->remGenre(select(pb->enumGenres()) - 1);
                std::cout << "Removed successfully" << std::endl;
                pause();
                break;
            case 'q':
                return;
            default:
                break;
        }
    }
}

void manageBook(Book* pb) {
    std::string temp;
    while (true) {
        cls();
        std::cout << *pb << std::endl;
        std::cout << EDIT_BOOK_OPTIONS << std::endl;
        switch (getch()) {
            case '1':
                std::cout << "Enter the new title of the book: " << std::endl;
                while (!readString(std::cin, temp, 's'));
                pb->rename(temp);
                std::cout << "Changed successfully." << std::endl;
                break;
            case '2':
                std::cout << "Enter the new publishing year of the book: " << std::endl;
                while (!readString(std::cin, temp, 'y'));
                pb->setYear(stoid(temp)); //safe to stoid
                std::cout << "Changed successfully." << std::endl;
                break;
            case '3':
                editBookEntries(pb); //To add and remove
                return;
            case '4':
                if (yesNo("Delete this record?")) {
                    if (data->erase(*pb))
                        std::cout << "Erased this book and removed all references." << std::endl;
                    else std::cout << "Couldn't erase this entry!" << std::endl;
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
void manageAuthor(Author* pa) {
    std::string temp;
    while (true) {
        cls();
        std::cout << *pa << std::endl;
        std::cout << EDIT_AUTHOR_OPTIONS << std::endl;
        switch (getch()) {
            case '1':
                std::cout << "Enter the new name for the author: " << std::endl;
                while (!readString(std::cin, temp, 's'));
                pa->rename(temp);
                std::cout << "Changed successfully." << std::endl;
                break;
            case '2':
                std::cout << "Enter the new birthdate: " << std::endl;
                while (!readString(std::cin, temp, 'd'));
                pa->setDate(temp); //We can accept the 0.0.0000 as Tnown date.
                std::cout << "Changed successfully." << std::endl;
                break;
            case '3':
                std::cout << "Enter the new country: " << std::endl;
                while (!readString(std::cin, temp, 's'));
                pa->setCountry(temp); //There could be countries that use ' or - or spaces in their names
                std::cout << "Changed successfully." << std::endl;
                break;
            case '4':
                addEntries(pa); //We can add entries to anything
                break;
            case '5':
                std::cout << "Select a book's #: " << std::endl; //Once printed, there are numbers in our table
                pa->remBook(select(pa->enumBooks()) - 1); //Select the number
                std::cout << "Removed successfully" << std::endl;
                pause();
                break;
            case '6':
                if (yesNo("Delete this record?")) {
                    data->erase(*pa);
                    std::cout << "Erased this author and removed all references." << std::endl;
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

void manageGenre(Genre* pg) {//Specialized actions for every entry
    std::string temp;
    Entry* pe;
    while (true) {
        cls();
        std::cout << *pg << std::endl; //Print what we are editing
        std::cout << EDIT_GENRE_OPTIONS << std::endl;
        switch (getch()) {
            case '1':
                std::cout << "Enter the new genre's title" << std::endl;
                while (!readString(std::cin, temp, 's'));
                pg->rename(temp);
                std::cout << "Renamed to " << temp << std::endl;
                pause();
                return;
            case '2':
                addEntries(pg);
                return;
            case '3': //For genres, there can be thousands of books, so we have to search for the one we want to remove.
                pe = selectEntry();
                if (typeid(*pe) == typeid(Book)) //If we selected a book
                {
                    if (yesNo("Remove book " + pe->getName() + " from genre " + pg->getName() + "?")) {
                        if (pg->unlink(static_cast<Book*>(pe))) //Unlink If exists (just to be sure)
                            std::cout << "Removed successfully" << std::endl;
                        else std::cout << "Couldn't remove this entry!" << std::endl;
                        pause();
                    }
                    break;
                }
                else {
                    std::cout << "Please select a book! " << std::endl; //Else get outta here
                    break;
                }
            case '4':
                if (yesNo("Delete this record?")) {
                    std::cout << "Erased this genre and removed all references." << std::endl;
                    data->erase(*pg);
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
    if (typeid(*pe) == typeid(Genre))
        manageGenre(static_cast<Genre*>(pe));
    else if (typeid(*pe) == typeid(Author))
        manageAuthor(static_cast<Author*>(pe)); //Static cast is SAFE since we got the typeid match
    else if (typeid(*pe) == typeid(Book))
        manageBook(static_cast<Book*>(pe));
}

void showData() {
    std::string temp;
    unsigned y;
    cls();
    std::cout << SHOW_DATA_MENU_ENTRIES << std::endl;
    while (true) {
        switch (getch()) {
            case '1':
                cls();
                data->printBooks(); //Print tables
                pause();
                return;
            case '2':
                cls();
                data->printAuthors();
                pause();
                return;
            case '3':
                cls();
                std::cout << "Enter the time period in years: " << std::endl; //Custom behavior according to the supervisor's request
                while (!readString(std::cin, temp, 'y'));
                y = stoid(temp);
                data->printGenres(y);
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
                if (isadmin) newBook();
                break;
            case '5':
                if (isadmin) newAuthor();
                break;
            case '6':
                if (isadmin) newGenre();
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
        data->printCredentials(false); //To see which users to delete
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
#ifndef NDEBUG
    data->printCredentials(isadmin);
#endif
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
    std::cout << "Successfully created account " << l << " ! Forwarding..." << std::endl;
#ifndef NDEBUG
    data->printCredentials(isadmin);
#endif
    pause();
    if (!isadmin) console(l, false); //If the acc was created for user we can log him in instantly
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
                if (isadmin) createAccPrompt(isadmin); //Conditional statements
                else continue;
                break;
            case '4':
                if (isadmin) manageUsr();
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
//Try function block for convenience. Argc is unused, argv is an array of char arrays, each with an argument, first is path
    data = Data::getInstance(); //Assign to our global pointer. For exception safety
    path = argv[0];
    path.erase(path.find_last_of('\\') + 1); //Makes 'path' be the path to the app folder, removing program name
    data->load(); //Loads ALL the data
#ifndef NDEBUG //For debugging
    data->printBooks();
    data->printAuthors();
    data->printGenres();
    std::cout << std::endl;
    std::cout << path << std::endl;
    data->printCredentials(false);
    std::cout << std::endl;
    data->printCredentials(true);
    std::cout << std::endl;
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
                if (yesNo("Have you got an account?")) login(false); //Ask for an account and if yes (true) log the user in
                else createAccPrompt(false); //false means NOT admin
                break;
            case '2':
                login(true); //log the admin in
                break;
            default:
                break;
        }
    }
    data->save(); //Saving only before exiting only to avoid corrupting the database
    return EXIT_SUCCESS;
}
catch (std::exception& e) {//If an exception is thrown, the program 100% can't continue. RIP.
    std::cerr << "Critical Error: " << e.what() << "\n The program cannot continue. Press any key to exit..." << std::endl;
    getch();
    return (EXIT_FAILURE);
}
catch (...) { //Sometimes we can get something completely random. In this case we just exit
    std::cerr << "Undefined Error. \n The program cannot continue. Press any key to exit..." << std::endl;
    getch();
    return (EXIT_FAILURE);
}