#include "header.h"

Entry::~Entry() {
    plog->put("called destructor on", getName());
    for (const auto & el : links)
        unlink(el);
}

bool Entry::checkLinks(const std::string& str) {
    for (const auto & el : links)
        if (lowercase(el->getName()).find(lowercase(str)) != std::string::npos)
            return true;
    return false;
}

bool Entry::link(Entry* e) {
    if (typeid(*this) == typeid(*e))
        return false;
    plog->put("called link on ", getName());
    return (links.insert(e).second && e->links.insert(this).second);
}
bool Entry::unlink(Entry* e) {
    if (e == nullptr) {
        plog->put(getName(), "got nullptr on their unlink!");
        return false;
    }
    plog->put("called unlink on ", getName(), "and", e->getName());
    if (typeid(*this) == typeid(*e))
        return false;
    return (links.erase(e) && e->links.erase(this));
}

void Entry::unlink(const size_t& pos) {
    if (pos >= links.size())
        throw std::invalid_argument("Deleting genre past the end of book " +
                                    getName()); // Should never happen though
    auto it = links.begin();
    std::advance(it, pos); // Set doesn't have a random-access iterator, so this
                           // will suffice.
    links.erase(it); // Remove it
}

////STUDENT
std::string Student::to_string() const {
    fort::char_table t; // Make a nice table
    t << fort::header << "Name"
      << "Events"
      << "Degree"
      << "Birthdate"
      << "Age"
      << "GPA"
      << "Tuition"
      << "ID" << fort::endr;
    std::string       delim;
    std::stringstream l;
    size_t            i = 0;
    for (const auto& el : links) {
        i++;
        l << delim << i << "."
          << el->getName(); // Add numbers to select something later
        delim = "\n";
    }
    t << getName() << l.str() << degree << birthdate << getAge() << avgGrade
      << (isTuition ? "Yes" : "No") << id() << fort::endr;
    setTableProperties(t, 0, 7);
    return t.to_string();
}

bool Student::check(const std::string& s) const {
    std::string ls = lowercase(s); // Ignore the case
    return (lowercase(getName()).find(ls) != std::string::npos ||
            lowercase(degree).find(ls) != std::string::npos ||
            birthdate.find(s) != std::string::npos ||
            s == std::to_string(avgGrade) || s == std::to_string(id())) ||
           ls == (isTuition ? "tuition" : "scholarship");
}

std::string Event::to_string() const {
    fort::char_table t; // Make a nice table
    t << fort::header << "Name"
      << "Participants"
      << "Date"
      << "Place"
      << "ID" << fort::endr;
    std::string       delim;
    std::stringstream p;
    size_t            i = 0;
    for (const auto& el : links) {
        i++;
        p << delim << i << "."
          << el->getName(); // Add numbers to select something later
        delim = "\n";
    }
    t << getName() << p.str() << date << place << id() << fort::endr;
    setTableProperties(t, 0, 4);
    return t.to_string();
}
bool Event::check(const std::string& s) const {
    std::string ls = lowercase(s); // Ignore the case
    return (lowercase(getName()).find(ls) != std::string::npos ||
            lowercase(place).find(ls) != std::string::npos ||
            date.find(s) != std::string::npos || s == std::to_string(id()));
}
