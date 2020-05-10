#include "header.h"
#include <fstream>
#include <filesystem>

void setTableProperties(fort::char_table& t, unsigned firstColored, unsigned secondColored) //Edit the given table (for uniform look)
{
    t.set_cell_text_align(fort::text_align::center);
    t.set_border_style(FT_BASIC2_STYLE);
    t.column(firstColored).set_cell_content_fg_color(fort::color::green);
    t.column(secondColored).set_cell_content_fg_color(fort::color::red);
}

void Data::printBooks() {
    fort::char_table t; //Create a table
    t << fort::header << "Title" << "Genres" << "Authors" << "Year" << "ID" << fort::endr; //Add a header
    for (const auto& book: mbooks) //For every book in the DB
    {
        std::stringstream authors, genres;
        std::string delim; //Set to empty
        for (const auto& g: book.second.genres) //For every genre of this book
        {
            genres << delim << g->getName(); //Print them line by line
            delim = "\n"; //Set the delim to a newline after the first
        }
        delim.clear(); //No need to separate the 1st
        for (const auto& a: book.second.authors) //For every genre of the book
        {
            authors << delim << a->getName(); //merge them
            delim = "\n";
        }
        t << book.second.getName() << genres.str() << authors.str() << book.second.year << book.second.id()
          << fort::endr; //Print everything
    }
    setTableProperties(t, 0, 4); //Change the table
    std::cout << t.to_string() << std::endl; //Output
}
void Data::printAuthors() //Same logic, see above
{
    fort::char_table t;
    t << fort::header << "Name" << "Books" << "Birthdate" << "Country" << "ID" << fort::endr;
    for (const auto& author: mauthors) {
        std::stringstream books;
        std::string delim;
        delim.clear();
        for (const auto& a: author.second.books) {
            books << delim << a->getName();
            delim = "\n";
        }
        t << author.second.getName() << books.str() << author.second.date << author.second.country << author.second.id() << fort::endr;
    }
    setTableProperties(t, 0, 4);
    std::cout << t.to_string() << std::endl;
}
void Data::printGenres(unsigned years) //See above, specialized per supervisor's request
{
    unsigned diff = getCurYear() - years; //Get difference
    fort::char_table t;
    std::cout << "Books grouped by genres for the past " << years << " years" << std::endl;
    t << fort::header << "Name" << "Quantity" << "Books" << "Years" << "ID" << fort::endr;
    for (const auto& genre: mgenres) {
        unsigned long cnt = 0; //For counting
        std::stringstream books, syear;
        std::string delim;
        for (const auto& b: genre.second.books) {
            auto pb = static_cast<Book*>(b); //Safe since we know there are books
            if (pb->year != 0 && pb->year < diff) continue; //We print 0 years because those are "unknown"
            books << delim << pb->getName(); //Append the title
            syear << delim << pb->getYear(); //Append the year
            delim = "\n";
            ++cnt; //Increase the book counter
        }
        t << genre.second.getName() << cnt << books.str() << syear.str() << genre.second.id() << fort::endr; //Output
    }
    setTableProperties(t, 0, 4);
    std::cout << t.to_string() << std::endl;//Print
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
    return true;
}
void Data::printCredentials(bool isAdmin) {
    std::cout << (isAdmin ? "Admin" : "User") << " credentials: " << std::endl;
    for (const auto& el: (isAdmin ? admins : users))
        std::cout << el.first << "\n"; //Don't print the password
    std::cout << std::endl;
}
void Data::ensureFileExists(const std::string& f) {
    if (!std::filesystem::exists(path + f)) //Check if the file exists
    {//If not
        std::cerr << "Warning! The file " << path << f << " does not exist! Creating a blank one..." << std::endl;
        std::ofstream file(path + f); //Create a new one
        file.close(); //Explicit
    }
}

std::vector<Entry*> Data::search(const std::string& s) //Search anything
{
    std::vector<Entry*> ret; //Create the result
    for (auto& b : mbooks)
        if (b.second.check(s)) ret.push_back(&b.second); //Check every entry linearly
    for (auto& a : mauthors)
        if (a.second.check(s)) ret.push_back(&a.second); //Without the DB no better choice
    for (auto& g : mgenres)
        if (g.second.check(s)) ret.push_back(&g.second);
    return ret;
}

void Data::load() try //Try-catch function block
{
    std::string tempA, tempB, tempC, tempD, name = "user.txt"; //Reusable
    ensureFileExists(name); //Make sure
    std::ifstream f(path + name); //Open using the path
    auto eof = [&f]() //Lambda to check for the end of file.
    { return f.peek() == std::ifstream::traits_type::eof(); };
    while (f) //While file f is good (converted to bool)
    { //USER
        if (eof()) break; //If the file has ended break out
        if (!readString(f, tempA, 'n')) throw std::invalid_argument("File: " + name + " couldn't read login"); //Must be right!
        if (!readString(f, tempB, 'n')) throw std::invalid_argument("File: " + name + " couldn't read password");
        users.try_emplace(tempA, tempB); //Create a new entry
        //Notice that we read already hashed values, so we mustn't use our addAccount() function.
        if (!std::getline(f, tempC)) break; //Empty line (can be at the end of file)
        if (!tempC.empty() && tempC != " ")
            throw std::invalid_argument("File " + name + " read error, check delimiters."); //Separate by a blank line or a space
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
        if (!readString(f, tempA, 'n')) throw std::invalid_argument("File: " + name + " couldn't read login");
        if (!readString(f, tempB, 'n')) throw std::invalid_argument("File: " + name + " couldn't read password");
        admins.try_emplace(tempA, tempB);
        if (!std::getline(f, tempC)) break;
        if (!tempC.empty() && tempC != " ")
            throw std::invalid_argument("File " + name + " read error, check delimiters.");
    }
    std::cout << "Successfully read admins" << std::endl;
    if (admins.empty()) //If there are no admins
    {
        std::cerr << "Warning! We  couldn't find any valid administrator accounts. \n" //We MUST have at least one.
                     "Created a new one: admin | admin" << std::endl;
        addAccount("admin", "admin", true); //here we use hashing
    }
    name = "genres.txt";
    f.close(); //Same
    ensureFileExists(name);
    f.open(path + name);
    while (f) {
        if (eof()) break;
        if (!readString(f, tempA, 'i')) throw std::invalid_argument("File: " + name + " couldn't read ID");
        if (!readString(f, tempB, 's')) throw std::invalid_argument("File: " + name + " couldn't read name");
        addGenre(stoid(tempA), tempB, stoid(tempA)); //Add genre in-place to get the performance boost and avoid copies
        if (!std::getline(f, tempC)) break;
        if (!tempC.empty() && tempC != " ")
            throw std::invalid_argument("File " + name + " read error, check delimiters.");
    }
    std::cout << "Successfully read genres" << std::endl;
    name = "authors.txt";
    f.close();
    ensureFileExists(name);
    f.open(path + name);
    while (f) { //id, name, date, country <- for variable names. Confusing but no copies
        if (eof()) break;
        if (!readString(f, tempA, 'i')) throw std::invalid_argument("File: " + name + " couldn't read id");
        if (!readString(f, tempB, 's')) throw std::invalid_argument("File: " + name + " couldn't read name");
        if (!readString(f, tempC, 'd')) throw std::invalid_argument("File: " + name + " couldn't read date");
        if (!readString(f, tempD, 's')) throw std::invalid_argument("File: " + name + " couldn't read country");
        addAuthor(stoid(tempA), tempB, tempC, tempD, stoid(tempA));
        if (!std::getline(f, tempC)) break;
        if (!tempC.empty() && tempC != " ")
            throw std::invalid_argument("File " + name + " read error, check delimiters.");
    }
    std::cout << "Successfully read authors" << std::endl;
    f.close();
    name = "books.txt";
    ensureFileExists(name);
    f.open(path + name);
#ifndef NDEBUG //For debugging, this is the hardest part.
    std::cout << "Current books state: " << std::endl;
    for (auto& el: mbooks)
        std::cout << "key: " << el.first << "\n" << el.second << std::endl;
    std::cout << "Current genres state: " << std::endl;
    for (auto& el: mgenres)
        std::cout << "key: " << el.first << "\n" << el.second << std::endl;
    std::cout << "Current authors state: " << std::endl;
    for (auto& el: mauthors)
        std::cout << "key: " << el.first << "\n" << el.second << std::endl;
#endif
    while (f) { //id, title, year, temp, entry
        if (eof()) break;
#ifndef NDEBUG
        std::cout << "Starting to parse a new book " << std::endl;
#endif
        if (!readString(f, tempA, 'i')) throw std::invalid_argument("File: " + name + " couldn't read id");
        if (!readString(f, tempB, 's')) throw std::invalid_argument("File: " + name + " couldn't read title");
        if (!readString(f, tempC, 'y')) throw std::invalid_argument("File: " + name + " couldn't read year"); //Read basic info
#ifndef NDEBUG
        std::cout << "emplace_back " << tempA << ' ' << tempB << ' ' << tempC << std::endl;
#endif
        auto curbook = addBook(stoid(tempA), tempB, stoid(tempC), stoid(tempA)); //Create and save the pointer to a new book
#ifndef NDEBUG
        std::cout << "emplace_back finished\n";
#endif
        if (!curbook) throw std::runtime_error("Duplicate on emplace book"); //No duplicates!
        //Place genres
        if (!readString(f, tempA, 's')) throw std::invalid_argument("File: " + name + " couldn't read book's genres");
        std::stringstream ss(tempA); //tempA - line with genres, tempD - genre;
        while (getline(ss, tempD, ',')) //For every word (separated by comma) in the genres list
        {
            if (!checkString(tempD, 'i')) throw std::invalid_argument("File: " + name + " couldn't read book's genre ID"); //If ok
            auto sought = mgenres.find(stoid(tempD)); //Search for that ID and save
#ifndef NDEBUG
            std::cout << "sought.first is: " << (sought != mgenres.end() ? std::to_string(sought->first) : "NULL") << std::endl;
#endif
            if (sought == mgenres.end()) //If we didn't find anything
            {
#ifndef NDEBUG
                std::cout << "Executing new genre creation" << std::endl;
#endif
                addGenre(stoid(tempD), "Unknown genre", stoid(tempD))->link(curbook); //Handle the error, create a blank entry
            }   //create a new genre and bind it to the book
            else {
#ifndef NDEBUG
                std::cout << "Executing adding pointer" << std::endl;
#endif
                sought->second.link(curbook); //Else it's fine to link the book
            }
            tempD.clear(); //We will need it later
        }
#ifndef NDEBUG
        std::cout << "Successfully linked book with genres\n";
#endif
        //Place authors, same logic as abve
        if (!readString(f, tempA, 's')) throw std::invalid_argument("File: " + name + " couldn't read book's authors");
        ss.clear();
        ss.str(tempA); //author's line, tempD is author's ID;
        while (getline(ss, tempD, ',')) {
            if (!checkString(tempD, 'i')) throw std::invalid_argument("File: " + name + " couldn't read book's author ID ");
            auto author = mauthors.find(stoid(tempD));
#ifndef NDEBUG
            std::cout << "sought.first is: " << (author != mauthors.end() ? std::to_string(author->first) : "NULL") << std::endl;
#endif
            if (author == mauthors.end()) {
#ifndef NDEBUG
                std::cout << "Executing new author creation" << std::endl;
#endif
                addAuthor(stoid(tempD), "Unknown author", "0.0.0000", "Unknown", stoid(tempD))->link(curbook);
            }
            else {
#ifndef NDEBUG
                std::cout << "Executing adding pointer" << std::endl;
#endif
                author->second.link(curbook);
            }
            tempD.clear();
        }
        if (!std::getline(f, tempC)) break; //Ifnore one line
        if (!tempC.empty() && tempC != " ")
            throw std::invalid_argument("File " + name + " read error, check delimiters.");
#ifndef NDEBUG
        std::cout << "Linked book " << tempB << std::endl;
#endif
    } //If the book was read correctly, continue
    std::cout << "Successfully read books" << std::endl;
} //Finished linking
catch (...) {
    std::cerr << "Error while reading files. The program cannot continue." << std::endl;
    throw; //Rethrow, main() will deal
}
void Data::save() {
    std::cout << "Saving..." << std::endl;
    std::ofstream f(path + "user.txt"); //Open a file
#ifndef NDEBUG
    printBooks();
    printCredentials(true);
    printCredentials(false);
#endif
    for (auto& el: users)
        f << el.first << "\n" << el.second << "\n" << std::endl; //Write
    f.close(); //Reopen for a new file
    f.open(path + "admin.txt");
    for (auto& el: admins)
        f << el.first << "\n" << el.second << "\n" << std::endl;
    f.close();
    f.open(path + "genres.txt");
    f << std::setfill('0'); //For numbers
    for (auto& el: mgenres) //Set the width according to ID
        f << std::setw(MAX_ID_LENGTH) << el.first << '\n' << el.second.getName() << '\n' << std::endl;
    f.close();
    f.open(path + "authors.txt");
    f << std::setfill('0'); //New stream object
    for (auto& el: mauthors)
        f << std::setw(MAX_ID_LENGTH) << el.first << '\n' << el.second.getName() << '\n'
          << el.second.date << '\n' << el.second.country << '\n' << std::endl;
    f.close();
    f.open(path + "books.txt");
    f << std::setfill('0');
    for (auto& b: mbooks) {
        std::string delim;
        f << std::setw(MAX_ID_LENGTH) << b.first << "\n" << b.second.getName() << "\n" << std::setw(4) << b.second.year << "\n";
        //Place genres
        if (!b.second.genres.empty()) //If there is something
        {
            for (auto& g: b.second.genres) //Write it
            {
                f << delim << std::setw(MAX_ID_LENGTH) << g->id();
                delim = ',';
            }
        }
        else //If nothing, then this is bad
        {
            std::cerr << "Warning! The book \n" << b.second << "\n Has zero genres! The data will be generated automatically!" << std::endl;
            f << genID(); //Add a mock entry, will be dealt with on startup
        }
        f << "\n"; //Blank lines
        delim.clear();
        if (!b.second.authors.empty()) //Same logic
        {
            for (auto& a: b.second.authors) {
                f << delim << std::setw(MAX_ID_LENGTH) << a->id();
                delim = ',';
            }
        }
        else {
            std::cerr << "Warning! The book \n" << b.second << "\n Has zero authors! The data will be generated automatically!"
                      << std::endl;
            f << genID();
        }
        f << "\n" << std::endl;
    }
    f << std::setfill(' ');
}