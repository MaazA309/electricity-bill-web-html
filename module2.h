// ============================================================
//  MODULE 2 — Meter Reading Module
//  Electricity Bill Management System
//  Members: Muhammad Jawad Amir, Muhammad Sheraz Amir, Maaz Arif
//
//  OOP Concepts Used:
//    - Encapsulation : MeterReading class wraps all reading logic
//    - Exception Handling: invalid/negative readings throw exceptions
//    - Abstraction  : reading validation hidden inside class methods
// ============================================================

#include "models.h"

// ============================================================
//  CLASS — MeterReading
//  Encapsulates reading entry, validation, and storage
// ============================================================
class MeterReading {
private:
    string consumerId;
    string month;
    double prevReading;
    double currReading;
    double unitsConsumed;

    // Private validation — throws on invalid data
    void validate() const {
        if (currReading < 0 || prevReading < 0)
            throw invalid_argument("Meter readings cannot be negative.");
        if (currReading < prevReading)
            throw invalid_argument(
                "Current reading (" + to_string((int)currReading) +
                ") cannot be less than previous reading (" +
                to_string((int)prevReading) + ").");
    }

public:
    MeterReading() : prevReading(0), currReading(0), unitsConsumed(0) {}

    MeterReading(const string& cid, const string& mon,
                 double prev, double curr)
        : consumerId(cid), month(mon),
          prevReading(prev), currReading(curr) {
        validate();
        unitsConsumed = currReading - prevReading;
    }

    // Getters
    string getConsumerId()   const { return consumerId;   }
    string getMonth()        const { return month;        }
    double getPrevReading()  const { return prevReading;  }
    double getCurrReading()  const { return currReading;  }
    double getUnitsConsumed()const { return unitsConsumed;}

    // Convert to plain struct for file I/O
    Reading toStruct() const {
        Reading r;
        r.consumerId   = consumerId;
        r.month        = month;
        r.prevReading  = prevReading;
        r.currReading  = currReading;
        r.unitsConsumed = unitsConsumed;
        return r;
    }

    void display() const {
        cout << fixed << setprecision(2);
        cout << "  Consumer  : " << consumerId    << "\n";
        cout << "  Month     : " << month         << "\n";
        cout << "  Previous  : " << prevReading   << " units\n";
        cout << "  Current   : " << currReading   << " units\n";
        cout << "  Consumed  : " << unitsConsumed << " units\n";
    }
};

// ============================================================
//  HELPER — get last reading for a consumer (returns 0 if none)
// ============================================================
double getLastReading(const vector<Reading>& readings, const string& consumerId) {
    double last = 0.0;
    for (const auto& r : readings)
        if (r.consumerId == consumerId)
            last = r.currReading;  // keep overwriting → last one wins
    return last;
}

// ============================================================
//  HELPER — check if reading already exists for month
// ============================================================
bool readingExists(const vector<Reading>& readings,
                   const string& consumerId, const string& month) {
    for (const auto& r : readings)
        if (r.consumerId == consumerId && r.month == month) return true;
    return false;
}

// ============================================================
//  ADMIN — enter meter reading for a consumer
// ============================================================
void adminEnterReading() {
    vector<Consumer> consumers = loadConsumers();
    if (consumers.empty()) {
        cout << "  [Error] No consumers registered yet.\n"; return;
    }

    cout << "  Enter Consumer ID: ";
    string cid; getline(cin, cid);

    // Validate consumer exists
    bool found = false;
    for (const auto& c : consumers)
        if (c.id == cid) { found = true; break; }
    if (!found) { cout << "  [Error] Consumer " << cid << " not found.\n"; return; }

    cout << "  Enter Month (YYYY-MM): ";
    string month; getline(cin, month);
    if (month.size() != 7 || month[4] != '-') {
        cout << "  [Error] Invalid month format. Use YYYY-MM.\n"; return;
    }

    vector<Reading> readings = loadReadings();
    if (readingExists(readings, cid, month)) {
        cout << "  [Error] Reading for " << cid << " / " << month << " already exists.\n";
        return;
    }

    double prev = getLastReading(readings, cid);
    cout << "  Previous Reading : " << fixed << setprecision(2) << prev << " units\n";
    cout << "  Enter Current Reading: ";
    double curr;
    while (!(cin >> curr)) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "  Invalid. Enter a number: ";
    }
    cin.ignore();

    try {
        MeterReading mr(cid, month, prev, curr);
        readings.push_back(mr.toStruct());
        saveReadings(readings);
        cout << "\n  Reading recorded successfully.\n";
        mr.display();
    } catch (const exception& e) {
        cout << "  [Error] " << e.what() << "\n";
    }
}

// ============================================================
//  ADMIN — view all readings (optionally filter by consumer)
// ============================================================
void adminViewReadings() {
    vector<Reading>  readings  = loadReadings();
    vector<Consumer> consumers = loadConsumers();

    cout << "  Filter by Consumer ID (leave blank for all): ";
    string filter; getline(cin, filter);

    printLine('=');
    cout << "  METER READINGS\n";
    printLine('=');
    cout << left << setw(10) << "Cons ID"
                 << setw(20) << "Name"
                 << setw(10) << "Month"
                 << setw(12) << "Prev"
                 << setw(12) << "Curr"
                 << setw(10) << "Units" << "\n";
    printLine();

    int count = 0;
    for (const auto& r : readings) {
        if (!filter.empty() && r.consumerId != filter) continue;
        cout << left << setw(10) << r.consumerId
                     << setw(20) << consumerName(consumers, r.consumerId)
                     << setw(10) << r.month
                     << fixed << setprecision(2)
                     << setw(12) << r.prevReading
                     << setw(12) << r.currReading
                     << setw(10) << r.unitsConsumed << "\n";
        count++;
    }
    if (count == 0) cout << "  No readings found.\n";
    cout << "\n  Total records: " << count << "\n";
    printLine('=');
}

// ============================================================
//  ADMIN — update an existing reading (correction)
// ============================================================
void adminUpdateReading() {
    vector<Reading> readings = loadReadings();

    cout << "  Enter Consumer ID : "; string cid;   getline(cin, cid);
    cout << "  Enter Month (YYYY-MM): "; string mon; getline(cin, mon);

    for (auto& r : readings) {
        if (r.consumerId == cid && r.month == mon) {
            cout << "  Current reading: " << r.currReading << "\n";
            cout << "  Enter corrected current reading: ";
            double newCurr;
            while (!(cin >> newCurr)) {
                cin.clear(); cin.ignore(1000, '\n');
                cout << "  Invalid. Enter a number: ";
            }
            cin.ignore();
            try {
                MeterReading mr(cid, mon, r.prevReading, newCurr);
                r.currReading   = mr.getCurrReading();
                r.unitsConsumed = mr.getUnitsConsumed();
                saveReadings(readings);
                cout << "  Reading updated. Units consumed: "
                     << fixed << setprecision(2) << r.unitsConsumed << "\n";
            } catch (const exception& e) {
                cout << "  [Error] " << e.what() << "\n";
            }
            return;
        }
    }
    cout << "  [Error] No reading found for " << cid << " / " << mon << ".\n";
}

// ============================================================
//  MODULE 2 — ADMIN MENU
// ============================================================
void module2AdminMenu() {
    int choice;
    do {
        cout << "\n";
        printLine('*');
        cout << "   MODULE 2 -- METER READING\n";
        printLine('*');
        cout << "  1. Enter Meter Reading\n";
        cout << "  2. View Readings\n";
        cout << "  3. Update / Correct Reading\n";
        cout << "  0. Back\n";
        printLine('*');
        cout << "  Enter choice: ";
        choice = safeIntInput();

        switch (choice) {
            case 1: adminEnterReading();  break;
            case 2: adminViewReadings();  break;
            case 3: adminUpdateReading(); break;
            case 0: cout << "  Returning...\n"; break;
            default: cout << "  Invalid option.\n";
        }
    } while (choice != 0);
}
