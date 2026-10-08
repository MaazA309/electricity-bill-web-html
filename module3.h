// ============================================================
//  MODULE 3 — Bill Calculation Module
//  Electricity Bill Management System
//  Members: Muhammad Jawad Amir, Muhammad Sheraz Amir, Maaz Arif
//
//  OOP Concepts Used:
//    - Polymorphism  : abstract BillCalculator with virtual calculateBill()
//    - Inheritance   : Residential/Commercial/IndustrialCalculator subclasses
//    - Encapsulation : billing rates are private constants per class
//    - Abstraction   : caller only sees calculateBill(), not rate logic
// ============================================================

#include "models.h"

// ============================================================
//  BILLING RATES (adjust to match WAPDA/LESCO tariffs)
// ============================================================

// Residential slab rates (per unit)
const double RES_SLAB1_RATE  = 3.95;   // 0-100 units
const double RES_SLAB2_RATE  = 7.74;   // 101-200 units
const double RES_SLAB3_RATE  = 10.06;  // 201-300 units
const double RES_SLAB4_RATE  = 12.15;  // 301-700 units
const double RES_SLAB5_RATE  = 19.55;  // 701+ units

// Commercial & Industrial flat rates (per unit)
const double COM_RATE        = 16.00;
const double IND_RATE        = 14.00;
const double IND_DEMAND_CHARGE = 500.0; // fixed demand charge

// GST rate
const double GST_RATE        = 0.17;   // 17%

// Late fine rate (applied if overdue)
const double LATE_FINE_RATE  = 0.10;   // 10% of bill amount

// ============================================================
//  ABSTRACT BASE CLASS — BillCalculator
// ============================================================
class BillCalculator {
public:
    virtual ~BillCalculator() {}

    // Pure virtual — each subclass implements its own tariff
    virtual double calculateBill(double units) const = 0;

    virtual string getType() const = 0;

    // GST is the same for all types
    double calculateGST(double amount) const {
        return amount * GST_RATE;
    }

    // Late fine is the same for all types
    double calculateLateFine(double amount) const {
        return amount * LATE_FINE_RATE;
    }
};

// ============================================================
//  RESIDENTIAL CALCULATOR — slab-rate billing
// ============================================================
class ResidentialCalculator : public BillCalculator {
public:
    string getType() const override { return "Residential"; }

    double calculateBill(double units) const override {
        double amount = 0.0;

        if (units <= 100) {
            amount = units * RES_SLAB1_RATE;
        } else if (units <= 200) {
            amount = 100 * RES_SLAB1_RATE
                   + (units - 100) * RES_SLAB2_RATE;
        } else if (units <= 300) {
            amount = 100 * RES_SLAB1_RATE
                   + 100 * RES_SLAB2_RATE
                   + (units - 200) * RES_SLAB3_RATE;
        } else if (units <= 700) {
            amount = 100 * RES_SLAB1_RATE
                   + 100 * RES_SLAB2_RATE
                   + 100 * RES_SLAB3_RATE
                   + (units - 300) * RES_SLAB4_RATE;
        } else {
            amount = 100 * RES_SLAB1_RATE
                   + 100 * RES_SLAB2_RATE
                   + 100 * RES_SLAB3_RATE
                   + 400 * RES_SLAB4_RATE
                   + (units - 700) * RES_SLAB5_RATE;
        }
        return amount;
    }
};

// ============================================================
//  COMMERCIAL CALCULATOR — flat rate
// ============================================================
class CommercialCalculator : public BillCalculator {
public:
    string getType() const override { return "Commercial"; }

    double calculateBill(double units) const override {
        return units * COM_RATE;
    }
};

// ============================================================
//  INDUSTRIAL CALCULATOR — flat rate + fixed demand charge
// ============================================================
class IndustrialCalculator : public BillCalculator {
public:
    string getType() const override { return "Industrial"; }

    double calculateBill(double units) const override {
        return units * IND_RATE + IND_DEMAND_CHARGE;
    }
};

// ============================================================
//  FACTORY — return the right calculator for a consumer type
// ============================================================
BillCalculator* createCalculator(const string& consumerType) {
    if (consumerType == "Residential") return new ResidentialCalculator();
    if (consumerType == "Commercial")  return new CommercialCalculator();
    if (consumerType == "Industrial")  return new IndustrialCalculator();
    throw invalid_argument("Unknown consumer type: " + consumerType);
}

// ============================================================
//  HELPER — generate next bill ID  (B001, B002, ...)
// ============================================================
string generateBillId(const vector<Bill>& bills) {
    int maxNum = 0;
    for (const auto& b : bills) {
        if (b.billId.size() > 1 && b.billId[0] == 'B') {
            try {
                int num = stoi(b.billId.substr(1));
                if (num > maxNum) maxNum = num;
            } catch (...) {}
        }
    }
    ostringstream oss;
    oss << "B" << setw(3) << setfill('0') << (maxNum + 1);
    return oss.str();
}

// ============================================================
//  HELPER — compute due date (30 days after current date)
// ============================================================
string computeDueDate() {
    time_t now = time(nullptr);
    now += 30 * 24 * 3600; // add 30 days
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
    return string(buf);
}

// ============================================================
//  HELPER — is bill overdue?
// ============================================================
bool isOverdue(const string& dueDate) {
    return currentDate() > dueDate;
}

// ============================================================
//  ADMIN — generate bill for one consumer/month
// ============================================================
void adminGenerateBill() {
    vector<Consumer> consumers = loadConsumers();
    vector<Reading>  readings  = loadReadings();
    vector<Bill>     bills     = loadBills();

    cout << "  Enter Consumer ID : "; string cid;  getline(cin, cid);
    cout << "  Enter Month (YYYY-MM): "; string mon; getline(cin, mon);

    // Find consumer
    string consType;
    bool found = false;
    for (const auto& c : consumers)
        if (c.id == cid) { consType = c.type; found = true; break; }
    if (!found) { cout << "  [Error] Consumer " << cid << " not found.\n"; return; }

    // Check bill not already generated
    for (const auto& b : bills)
        if (b.consumerId == cid && b.month == mon) {
            cout << "  [Error] Bill already generated for " << cid << " / " << mon << ".\n";
            return;
        }

    // Find reading
    double units = 0;
    bool readingFound = false;
    for (const auto& r : readings)
        if (r.consumerId == cid && r.month == mon) {
            units = r.unitsConsumed;
            readingFound = true;
            break;
        }
    if (!readingFound) {
        cout << "  [Error] No meter reading found for " << cid << " / " << mon << ".\n";
        return;
    }

    // Polymorphic bill calculation
    try {
        BillCalculator* calc = createCalculator(consType);

        double amount   = calc->calculateBill(units);
        double gst      = calc->calculateGST(amount);
        double lateFine = 0.0;  // No late fine at generation time
        double total    = amount + gst + lateFine;
        string dueDate  = computeDueDate();
        string billId   = generateBillId(bills);

        delete calc;

        Bill b;
        b.billId      = billId;
        b.consumerId  = cid;
        b.month       = mon;
        b.units       = units;
        b.amount      = amount;
        b.gst         = gst;
        b.lateFine    = lateFine;
        b.totalAmount = total;
        b.status      = "Unpaid";
        b.dueDate     = dueDate;

        bills.push_back(b);
        saveBills(bills);

        cout << "\n";
        printLine('=');
        cout << "  BILL GENERATED\n";
        printLine('=');
        cout << fixed << setprecision(2);
        cout << "  Bill ID      : " << billId   << "\n";
        cout << "  Consumer     : " << cid      << " (" << consType << ")\n";
        cout << "  Month        : " << mon      << "\n";
        cout << "  Units        : " << units    << "\n";
        cout << "  Base Amount  : Rs " << amount   << "\n";
        cout << "  GST (17%)    : Rs " << gst       << "\n";
        cout << "  Total        : Rs " << total     << "\n";
        cout << "  Due Date     : " << dueDate  << "\n";
        cout << "  Status       : Unpaid\n";
        printLine('=');

    } catch (const exception& e) {
        cout << "  [Error] " << e.what() << "\n";
    }
}

// ============================================================
//  ADMIN — generate bills for ALL consumers for a given month
// ============================================================
void adminGenerateBillsAll() {
    cout << "  Enter Month (YYYY-MM): ";
    string mon; getline(cin, mon);

    vector<Consumer> consumers = loadConsumers();
    vector<Reading>  readings  = loadReadings();
    vector<Bill>     bills     = loadBills();

    int generated = 0, skipped = 0;

    for (const auto& c : consumers) {
        // Skip if bill already exists
        bool exists = false;
        for (const auto& b : bills)
            if (b.consumerId == c.id && b.month == mon) { exists = true; break; }
        if (exists) { skipped++; continue; }

        // Find reading
        double units = 0; bool found = false;
        for (const auto& r : readings)
            if (r.consumerId == c.id && r.month == mon) {
                units = r.unitsConsumed; found = true; break;
            }
        if (!found) { skipped++; continue; }

        try {
            BillCalculator* calc = createCalculator(c.type);
            double amount   = calc->calculateBill(units);
            double gst      = calc->calculateGST(amount);
            double total    = amount + gst;
            string dueDate  = computeDueDate();
            string billId   = generateBillId(bills);
            delete calc;

            Bill b;
            b.billId = billId; b.consumerId = c.id; b.month = mon;
            b.units = units; b.amount = amount; b.gst = gst;
            b.lateFine = 0; b.totalAmount = total;
            b.status = "Unpaid"; b.dueDate = dueDate;

            bills.push_back(b);
            generated++;
            cout << "  Generated: " << billId << " for " << c.id
                 << " (" << c.name << ") — Rs " << fixed << setprecision(2) << total << "\n";
        } catch (...) { skipped++; }
    }

    saveBills(bills);
    cout << "\n  Done. Generated: " << generated << "  |  Skipped: " << skipped << "\n";
}

// ============================================================
//  ADMIN — view all bills for a month or consumer
// ============================================================
void adminViewBills() {
    vector<Bill>     bills     = loadBills();
    vector<Consumer> consumers = loadConsumers();

    cout << "  Filter by Consumer ID (blank=all): "; string cid;  getline(cin, cid);
    cout << "  Filter by Month      (blank=all): "; string mon;  getline(cin, mon);

    printLine('=');
    cout << "  BILLS\n";
    printLine('=');
    cout << left << setw(8)  << "Bill ID"
                 << setw(8)  << "Cons ID"
                 << setw(10) << "Month"
                 << setw(10) << "Units"
                 << setw(14) << "Total (Rs)"
                 << setw(8)  << "Status" << "\n";
    printLine();

    int count = 0;
    for (const auto& b : bills) {
        if (!cid.empty() && b.consumerId != cid) continue;
        if (!mon.empty() && b.month      != mon) continue;
        cout << left << setw(8)  << b.billId
                     << setw(8)  << b.consumerId
                     << setw(10) << b.month
                     << fixed << setprecision(0)
                     << setw(10) << b.units
                     << fixed << setprecision(2)
                     << setw(14) << b.totalAmount
                     << setw(8)  << b.status << "\n";
        count++;
    }
    if (count == 0) cout << "  No bills found.\n";
    cout << "\n  Total: " << count << "\n";
    printLine('=');
}

// ============================================================
//  MODULE 3 — ADMIN MENU
// ============================================================
void module3AdminMenu() {
    int choice;
    do {
        cout << "\n";
        printLine('*');
        cout << "   MODULE 3 -- BILL CALCULATION\n";
        printLine('*');
        cout << "  1. Generate Bill (single consumer)\n";
        cout << "  2. Generate Bills (all consumers for a month)\n";
        cout << "  3. View Bills\n";
        cout << "  0. Back\n";
        printLine('*');
        cout << "  Enter choice: ";
        choice = safeIntInput();

        switch (choice) {
            case 1: adminGenerateBill();      break;
            case 2: adminGenerateBillsAll();  break;
            case 3: adminViewBills();         break;
            case 0: cout << "  Returning...\n"; break;
            default: cout << "  Invalid option.\n";
        }
    } while (choice != 0);
}
