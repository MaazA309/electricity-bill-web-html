// ============================================================
//  MODULE 1 — User & Consumer Management
//  Electricity Bill Management System
//  Members: Muhammad Jawad Amir, Muhammad Sheraz Amir, Maaz Arif
//
//  OOP Concepts Used:
//    - Abstraction  : abstract base class Person
//    - Inheritance  : Admin, ResidentialConsumer, CommercialConsumer,
//                     IndustrialConsumer all inherit from Person
//    - Encapsulation: private data, public getters/setters
//    - Polymorphism : virtual getType() overridden in each subclass
// ============================================================

#include "models.h"

// ============================================================
//  ABSTRACT BASE CLASS — Person
// ============================================================
class Person {
protected:
    string id;
    string name;
    string password;

public:
    Person() {}
    Person(const string& id, const string& name, const string& password)
        : id(id), name(name), password(password) {}

    virtual ~Person() {}

    // Pure virtual — forces every subclass to declare its type
    virtual string getType() const = 0;

    // Getters
    string getId()       const { return id; }
    string getName()     const { return name; }
    string getPassword() const { return password; }

    // Setters
    void setName(const string& n)     { name     = n; }
    void setPassword(const string& p) { password = p; }

    virtual void display() const {
        cout << "  ID   : " << id   << "\n";
        cout << "  Name : " << name << "\n";
        cout << "  Type : " << getType() << "\n";
    }
};

// ============================================================
//  ADMIN CLASS
// ============================================================
class AdminUser : public Person {
public:
    AdminUser() {}
    AdminUser(const string& username, const string& password)
        : Person(username, username, password) {}

    string getType() const override { return "Admin"; }

    void display() const override {
        cout << "  Username : " << id   << "\n";
        cout << "  Role     : Admin\n";
    }
};

// ============================================================
//  CONSUMER BASE CLASS
// ============================================================
class ConsumerUser : public Person {
protected:
    string address;
    string phone;

public:
    ConsumerUser() {}
    ConsumerUser(const string& id, const string& name, const string& password,
                 const string& address, const string& phone)
        : Person(id, name, password), address(address), phone(phone) {}

    string getAddress() const { return address; }
    string getPhone()   const { return phone;   }
    void setAddress(const string& a) { address = a; }
    void setPhone(const string& p)   { phone   = p; }

    // Convert to plain struct for file I/O (matches Module 5's format)
    Consumer toStruct() const {
        Consumer c;
        c.id       = id;
        c.name     = name;
        c.type     = getType();
        c.address  = address;
        c.phone    = phone;
        c.password = password;
        return c;
    }

    void display() const override {
        cout << "  ID      : " << id      << "\n";
        cout << "  Name    : " << name    << "\n";
        cout << "  Type    : " << getType() << "\n";
        cout << "  Address : " << address << "\n";
        cout << "  Phone   : " << phone   << "\n";
    }
};

// ============================================================
//  CONSUMER SUBCLASSES (Polymorphism via getType())
// ============================================================
class ResidentialConsumer : public ConsumerUser {
public:
    ResidentialConsumer() {}
    ResidentialConsumer(const string& id, const string& name, const string& password,
                        const string& address, const string& phone)
        : ConsumerUser(id, name, password, address, phone) {}

    string getType() const override { return "Residential"; }
};

class CommercialConsumer : public ConsumerUser {
public:
    CommercialConsumer() {}
    CommercialConsumer(const string& id, const string& name, const string& password,
                       const string& address, const string& phone)
        : ConsumerUser(id, name, password, address, phone) {}

    string getType() const override { return "Commercial"; }
};

class IndustrialConsumer : public ConsumerUser {
public:
    IndustrialConsumer() {}
    IndustrialConsumer(const string& id, const string& name, const string& password,
                       const string& address, const string& phone)
        : ConsumerUser(id, name, password, address, phone) {}

    string getType() const override { return "Industrial"; }
};

// ============================================================
//  HELPER — generate next consumer ID  (C001, C002, ...)
// ============================================================
string generateConsumerId(const vector<Consumer>& existing) {
    int maxNum = 0;
    for (const auto& c : existing) {
        if (c.id.size() > 1 && c.id[0] == 'C') {
            try {
                int num = stoi(c.id.substr(1));
                if (num > maxNum) maxNum = num;
            } catch (...) {}
        }
    }
    ostringstream oss;
    oss << "C" << setw(3) << setfill('0') << (maxNum + 1);
    return oss.str();
}

// ============================================================
//  ADMIN FUNCTIONS
// ============================================================
bool adminLogin() {
    vector<Admin> admins = loadAdmins();

    // Seed a default admin if file is empty
    if (admins.empty()) {
        Admin def; def.username = "admin"; def.password = "admin123";
        admins.push_back(def);
        saveAdmins(admins);
    }

    cout << "  Admin Username: ";
    string user; getline(cin, user);
    cout << "  Password      : ";
    string pass; getline(cin, pass);

    for (const auto& a : admins)
        if (a.username == user && a.password == pass) return true;

    cout << "  [Error] Invalid credentials.\n";
    return false;
}

void adminAddConsumer() {
    vector<Consumer> consumers = loadConsumers();

    string name, type, address, phone, password;
    cout << "  Name    : "; getline(cin, name);
    cout << "  Type (1=Residential, 2=Commercial, 3=Industrial): ";
    int t = safeIntInput();
    if (t == 1) type = "Residential";
    else if (t == 2) type = "Commercial";
    else if (t == 3) type = "Industrial";
    else { cout << "  Invalid type.\n"; return; }

    cout << "  Address : "; getline(cin, address);
    cout << "  Phone   : "; getline(cin, phone);
    cout << "  Password: "; getline(cin, password);

    string newId = generateConsumerId(consumers);

    Consumer c;
    c.id       = newId;
    c.name     = name;
    c.type     = type;
    c.address  = address;
    c.phone    = phone;
    c.password = password;
    consumers.push_back(c);
    saveConsumers(consumers);

    cout << "  Consumer added successfully. ID: " << newId << "\n";
}

void adminUpdateConsumer() {
    vector<Consumer> consumers = loadConsumers();

    cout << "  Enter Consumer ID to update: ";
    string cid; getline(cin, cid);

    for (auto& c : consumers) {
        if (c.id == cid) {
            cout << "  New Name (leave blank to keep '" << c.name << "'): ";
            string name; getline(cin, name);
            if (!name.empty()) c.name = name;

            cout << "  New Address (leave blank to keep): ";
            string addr; getline(cin, addr);
            if (!addr.empty()) c.address = addr;

            cout << "  New Phone (leave blank to keep): ";
            string phone; getline(cin, phone);
            if (!phone.empty()) c.phone = phone;

            saveConsumers(consumers);
            cout << "  Consumer " << cid << " updated.\n";
            return;
        }
    }
    cout << "  [Error] Consumer ID " << cid << " not found.\n";
}

void adminDeleteConsumer() {
    vector<Consumer> consumers = loadConsumers();

    cout << "  Enter Consumer ID to delete: ";
    string cid; getline(cin, cid);

    auto it = remove_if(consumers.begin(), consumers.end(),
                        [&](const Consumer& c){ return c.id == cid; });
    if (it == consumers.end()) {
        cout << "  [Error] Consumer ID " << cid << " not found.\n";
        return;
    }
    consumers.erase(it, consumers.end());
    saveConsumers(consumers);
    cout << "  Consumer " << cid << " deleted.\n";
}

void adminViewAllConsumers() {
    vector<Consumer> consumers = loadConsumers();
    printLine('=');
    cout << "  ALL CONSUMERS\n";
    printLine('=');
    cout << left << setw(8) << "ID"
                 << setw(20) << "Name"
                 << setw(14) << "Type"
                 << setw(13) << "Phone" << "\n";
    printLine();
    for (const auto& c : consumers) {
        cout << left << setw(8)  << c.id
                     << setw(20) << c.name
                     << setw(14) << c.type
                     << setw(13) << c.phone << "\n";
    }
    cout << "\n  Total: " << consumers.size() << "\n";
    printLine('=');
}

// ============================================================
//  CONSUMER LOGIN
// ============================================================
// Returns the consumer's ID on success, empty string on failure
string consumerLogin() {
    vector<Consumer> consumers = loadConsumers();

    cout << "  Consumer ID : "; string cid; getline(cin, cid);
    cout << "  Password    : "; string pass; getline(cin, pass);

    for (const auto& c : consumers)
        if (c.id == cid && c.password == pass) return cid;

    cout << "  [Error] Invalid ID or password.\n";
    return "";
}

// ============================================================
//  MODULE 1 — ADMIN MENU
// ============================================================
void module1AdminMenu() {
    int choice;
    do {
        cout << "\n";
        printLine('*');
        cout << "   MODULE 1 -- USER & CONSUMER MANAGEMENT\n";
        printLine('*');
        cout << "  1. Add Consumer\n";
        cout << "  2. Update Consumer\n";
        cout << "  3. Delete Consumer\n";
        cout << "  4. View All Consumers\n";
        cout << "  0. Back\n";
        printLine('*');
        cout << "  Enter choice: ";
        choice = safeIntInput();

        switch (choice) {
            case 1: adminAddConsumer();        break;
            case 2: adminUpdateConsumer();     break;
            case 3: adminDeleteConsumer();     break;
            case 4: adminViewAllConsumers();   break;
            case 0: cout << "  Returning...\n"; break;
            default: cout << "  Invalid option.\n";
        }
    } while (choice != 0);
}
