#pragma once


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include <algorithm>
#include <sys/stat.h>

#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(p) _mkdir(p)
#else
  #define MKDIR(p) mkdir(p, 0755)
#endif

using namespace std;

// ─────────────────────────────────────────────
//  FILE PATHS  (shared across all modules)
// ─────────────────────────────────────────────
const string FILE_CONSUMERS = "consumers.txt";
const string FILE_READINGS  = "readings.txt";
const string FILE_BILLS     = "bills.txt";
const string FILE_PAYMENTS  = "payments.txt";
const string FILE_ADMINS    = "admins.txt";
const string REPORTS_DIR    = "reports/";

// ─────────────────────────────────────────────
//  DATA STRUCTURES  (plain structs, pipe-delimited)
// ─────────────────────────────────────────────
struct Consumer {
    string id;
    string name;
    string type;       // Residential / Commercial / Industrial
    string address;
    string phone;
    string password;
};

struct Admin {
    string username;
    string password;
};

struct Reading {
    string consumerId;
    string month;          // e.g. "2025-04"
    double prevReading;
    double currReading;
    double unitsConsumed;
};

struct Bill {
    string billId;
    string consumerId;
    string month;
    double units;
    double amount;
    double gst;
    double lateFine;
    double totalAmount;
    string status;         // "Paid" / "Unpaid"
    string dueDate;
};

struct Payment {
    string paymentId;
    string billId;
    string consumerId;
    string paymentDate;
    double amountPaid;
};

// ─────────────────────────────────────────────
//  UTILITY
// ─────────────────────────────────────────────
inline string currentTimestamp() {
    time_t now = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

inline string currentDate() {
    time_t now = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
    return string(buf);
}

inline void printLine(char ch = '-', int len = 65) {
    cout << string(len, ch) << "\n";
}

inline void ensureReportsDir() {
    if (MKDIR(REPORTS_DIR.c_str()) != 0 && errno != EEXIST)
        throw runtime_error("Cannot create reports directory.");
}

// ─────────────────────────────────────────────
//  FILE I/O — loaders
// ─────────────────────────────────────────────
inline vector<Consumer> loadConsumers() {
    vector<Consumer> list;
    ifstream file(FILE_CONSUMERS);
    if (!file.is_open()) return list; // return empty if file not yet created
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        Consumer c;
        getline(ss, c.id,       '|');
        getline(ss, c.name,     '|');
        getline(ss, c.type,     '|');
        getline(ss, c.address,  '|');
        getline(ss, c.phone,    '|');
        getline(ss, c.password, '|');
        if (!c.id.empty()) list.push_back(c);
    }
    return list;
}

inline void saveConsumers(const vector<Consumer>& list) {
    ofstream file(FILE_CONSUMERS, ios::trunc);
    if (!file.is_open()) throw runtime_error("Cannot write to " + FILE_CONSUMERS);
    for (const auto& c : list)
        file << c.id      << "|" << c.name    << "|" << c.type << "|"
             << c.address << "|" << c.phone   << "|" << c.password << "\n";
}

inline vector<Admin> loadAdmins() {
    vector<Admin> list;
    ifstream file(FILE_ADMINS);
    if (!file.is_open()) return list;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        Admin a;
        getline(ss, a.username, '|');
        getline(ss, a.password, '|');
        if (!a.username.empty()) list.push_back(a);
    }
    return list;
}

inline void saveAdmins(const vector<Admin>& list) {
    ofstream file(FILE_ADMINS, ios::trunc);
    if (!file.is_open()) throw runtime_error("Cannot write to " + FILE_ADMINS);
    for (const auto& a : list)
        file << a.username << "|" << a.password << "\n";
}

inline vector<Reading> loadReadings() {
    vector<Reading> list;
    ifstream file(FILE_READINGS);
    if (!file.is_open()) return list;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        Reading r;
        string prev, curr, units;
        getline(ss, r.consumerId, '|');
        getline(ss, r.month,      '|');
        getline(ss, prev,         '|');
        getline(ss, curr,         '|');
        getline(ss, units,        '|');
        try {
            r.prevReading   = stod(prev);
            r.currReading   = stod(curr);
            r.unitsConsumed = stod(units);
        } catch (...) { continue; }
        list.push_back(r);
    }
    return list;
}

inline void saveReadings(const vector<Reading>& list) {
    ofstream file(FILE_READINGS, ios::trunc);
    if (!file.is_open()) throw runtime_error("Cannot write to " + FILE_READINGS);
    for (const auto& r : list)
        file << r.consumerId   << "|" << r.month       << "|"
             << fixed << setprecision(2)
             << r.prevReading  << "|" << r.currReading << "|"
             << r.unitsConsumed << "\n";
}

inline vector<Bill> loadBills() {
    vector<Bill> list;
    ifstream file(FILE_BILLS);
    if (!file.is_open()) return list;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        Bill b;
        string units, amount, gst, late, total;
        getline(ss, b.billId,     '|');
        getline(ss, b.consumerId, '|');
        getline(ss, b.month,      '|');
        getline(ss, units,        '|');
        getline(ss, amount,       '|');
        getline(ss, gst,          '|');
        getline(ss, late,         '|');
        getline(ss, total,        '|');
        getline(ss, b.status,     '|');
        getline(ss, b.dueDate,    '|');
        try {
            b.units       = stod(units);
            b.amount      = stod(amount);
            b.gst         = stod(gst);
            b.lateFine    = stod(late);
            b.totalAmount = stod(total);
        } catch (...) { continue; }
        list.push_back(b);
    }
    return list;
}

inline void saveBills(const vector<Bill>& list) {
    ofstream file(FILE_BILLS, ios::trunc);
    if (!file.is_open()) throw runtime_error("Cannot write to " + FILE_BILLS);
    for (const auto& b : list)
        file << b.billId      << "|" << b.consumerId  << "|" << b.month << "|"
             << fixed << setprecision(2)
             << b.units       << "|" << b.amount      << "|" << b.gst   << "|"
             << b.lateFine    << "|" << b.totalAmount << "|"
             << b.status      << "|" << b.dueDate     << "\n";
}

inline vector<Payment> loadPayments() {
    vector<Payment> list;
    ifstream file(FILE_PAYMENTS);
    if (!file.is_open()) return list;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream ss(line);
        Payment p;
        string amt;
        getline(ss, p.paymentId,   '|');
        getline(ss, p.billId,      '|');
        getline(ss, p.consumerId,  '|');
        getline(ss, p.paymentDate, '|');
        getline(ss, amt,           '|');
        try { p.amountPaid = stod(amt); } catch (...) { continue; }
        list.push_back(p);
    }
    return list;
}

inline void savePayments(const vector<Payment>& list) {
    ofstream file(FILE_PAYMENTS, ios::trunc);
    if (!file.is_open()) throw runtime_error("Cannot write to " + FILE_PAYMENTS);
    for (const auto& p : list)
        file << p.paymentId  << "|" << p.billId     << "|"
             << p.consumerId << "|" << p.paymentDate << "|"
             << fixed << setprecision(2) << p.amountPaid << "\n";
}

// ─────────────────────────────────────────────
//  HELPER
// ─────────────────────────────────────────────
inline string consumerName(const vector<Consumer>& consumers, const string& id) {
    for (const auto& c : consumers)
        if (c.id == id) return c.name;
    return "Unknown";
}

inline int safeIntInput() {
    int val;
    while (!(cin >> val)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "  Invalid input. Enter a number: ";
    }
    cin.ignore();
    return val;
}
