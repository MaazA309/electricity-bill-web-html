// ============================================================
//  MODULE 4 — Payment & History Module
//  Electricity Bill Management System
//  Members: Muhammad Jawad Amir, Muhammad Sheraz Amir, Maaz Arif
//
//  OOP Concepts Used:
//    - Encapsulation : PaymentProcessor class owns all payment logic
//    - Exception Handling: duplicate payments, paying a paid bill, etc.
//    - Abstraction  : late-fine recalculation hidden inside processor
// ============================================================

#include "models.h"

// ============================================================
//  CLASS — PaymentProcessor
//  Encapsulates payment validation, late-fine application,
//  status update, and record keeping.
// ============================================================
class PaymentProcessor {
private:
    // Late fine rate (must match Module 3's constant)
    static const double LATE_FINE_RATE; // defined below class

    // Generate next payment ID  (P001, P002, ...)
    string generatePaymentId(const vector<Payment>& payments) const {
        int maxNum = 0;
        for (const auto& p : payments) {
            if (p.paymentId.size() > 1 && p.paymentId[0] == 'P') {
                try {
                    int num = stoi(p.paymentId.substr(1));
                    if (num > maxNum) maxNum = num;
                } catch (...) {}
            }
        }
        ostringstream oss;
        oss << "P" << setw(3) << setfill('0') << (maxNum + 1);
        return oss.str();
    }

    // Check if today is past the due date
    bool isPastDue(const string& dueDate) const {
        return currentDate() > dueDate;
    }

public:
    // ─────────────────────────────────────────
    //  Process payment for a given bill ID
    //  Returns the Payment record on success
    //  Throws on any error
    // ─────────────────────────────────────────
    Payment processPayment(const string& billId,
                           vector<Bill>& bills,
                           vector<Payment>& payments) const {
        // Find the bill
        Bill* targetBill = nullptr;
        for (auto& b : bills)
            if (b.billId == billId) { targetBill = &b; break; }

        if (!targetBill)
            throw runtime_error("Bill ID " + billId + " not found.");

        if (targetBill->status == "Paid")
            throw runtime_error("Bill " + billId + " is already paid.");

        // Apply late fine if overdue
        if (isPastDue(targetBill->dueDate) && targetBill->lateFine == 0.0) {
            targetBill->lateFine    = targetBill->amount * LATE_FINE_RATE;
            targetBill->totalAmount = targetBill->amount
                                    + targetBill->gst
                                    + targetBill->lateFine;
            cout << "  [Notice] Late fine applied: Rs "
                 << fixed << setprecision(2) << targetBill->lateFine << "\n";
        }

        // Build payment record
        Payment pmt;
        pmt.paymentId   = generatePaymentId(payments);
        pmt.billId      = billId;
        pmt.consumerId  = targetBill->consumerId;
        pmt.paymentDate = currentDate();
        pmt.amountPaid  = targetBill->totalAmount;

        // Update bill status
        targetBill->status = "Paid";

        return pmt;
    }
};

const double PaymentProcessor::LATE_FINE_RATE = 0.10;

// ============================================================
//  CONSUMER — pay a bill
// ============================================================
void consumerPayBill(const string& consumerId) {
    vector<Bill>    bills    = loadBills();
    vector<Payment> payments = loadPayments();

    // Show unpaid bills for this consumer
    cout << "\n";
    printLine('=');
    cout << "  YOUR UNPAID BILLS\n";
    printLine('=');
    cout << left << setw(10) << "Bill ID"
                 << setw(10) << "Month"
                 << setw(12) << "Total (Rs)"
                 << setw(12) << "Due Date" << "\n";
    printLine();

    bool any = false;
    for (const auto& b : bills) {
        if (b.consumerId == consumerId && b.status == "Unpaid") {
            cout << left << setw(10) << b.billId
                         << setw(10) << b.month
                         << fixed << setprecision(2)
                         << setw(12) << b.totalAmount
                         << setw(12) << b.dueDate << "\n";
            any = true;
        }
    }
    if (!any) { cout << "  No unpaid bills.\n"; printLine('='); return; }
    printLine('=');

    cout << "  Enter Bill ID to pay: ";
    string bid; getline(cin, bid);

    // Validate the bill belongs to this consumer
    bool belongs = false;
    for (const auto& b : bills)
        if (b.billId == bid && b.consumerId == consumerId) { belongs = true; break; }
    if (!belongs) {
        cout << "  [Error] Bill " << bid << " does not belong to your account.\n";
        return;
    }

    try {
        PaymentProcessor processor;
        Payment pmt = processor.processPayment(bid, bills, payments);
        payments.push_back(pmt);
        saveBills(bills);
        savePayments(payments);

        cout << "\n";
        printLine('=');
        cout << "  PAYMENT SUCCESSFUL\n";
        printLine('=');
        cout << "  Payment ID   : " << pmt.paymentId   << "\n";
        cout << "  Bill ID      : " << pmt.billId      << "\n";
        cout << "  Date         : " << pmt.paymentDate << "\n";
        cout << "  Amount Paid  : Rs " << fixed << setprecision(2) << pmt.amountPaid << "\n";
        printLine('=');

    } catch (const exception& e) {
        cout << "  [Error] " << e.what() << "\n";
    }
}

// ============================================================
//  ADMIN — pay a bill on behalf of consumer
// ============================================================
void adminProcessPayment() {
    vector<Bill>    bills    = loadBills();
    vector<Payment> payments = loadPayments();

    cout << "  Enter Bill ID to mark as paid: ";
    string bid; getline(cin, bid);

    try {
        PaymentProcessor processor;
        Payment pmt = processor.processPayment(bid, bills, payments);
        payments.push_back(pmt);
        saveBills(bills);
        savePayments(payments);

        cout << "  Payment recorded. ID: " << pmt.paymentId
             << "  Amount: Rs " << fixed << setprecision(2) << pmt.amountPaid << "\n";
    } catch (const exception& e) {
        cout << "  [Error] " << e.what() << "\n";
    }
}

// ============================================================
//  ADMIN — apply late fines to all overdue unpaid bills
// ============================================================
void adminApplyLateFines() {
    vector<Bill> bills = loadBills();
    int count = 0;

    for (auto& b : bills) {
        if (b.status == "Unpaid" && b.lateFine == 0.0 && currentDate() > b.dueDate) {
            b.lateFine     = b.amount * 0.10;
            b.totalAmount  = b.amount + b.gst + b.lateFine;
            count++;
            cout << "  Late fine applied to " << b.billId
                 << " — Rs " << fixed << setprecision(2) << b.lateFine << "\n";
        }
    }
    saveBills(bills);
    cout << "  Done. Late fines applied to " << count << " bill(s).\n";
}

// ============================================================
//  ADMIN — view all payment records
// ============================================================
void adminViewPayments() {
    vector<Payment>  payments  = loadPayments();
    vector<Consumer> consumers = loadConsumers();

    cout << "  Filter by Consumer ID (blank=all): ";
    string filter; getline(cin, filter);

    printLine('=');
    cout << "  PAYMENT HISTORY\n";
    printLine('=');
    cout << left << setw(10) << "Pay ID"
                 << setw(10) << "Bill ID"
                 << setw(10) << "Cons ID"
                 << setw(20) << "Name"
                 << setw(12) << "Date"
                 << setw(12) << "Amount (Rs)" << "\n";
    printLine();

    double total = 0; int count = 0;
    for (const auto& p : payments) {
        if (!filter.empty() && p.consumerId != filter) continue;
        cout << left << setw(10) << p.paymentId
                     << setw(10) << p.billId
                     << setw(10) << p.consumerId
                     << setw(20) << consumerName(consumers, p.consumerId)
                     << setw(12) << p.paymentDate
                     << fixed << setprecision(2) << p.amountPaid << "\n";
        total += p.amountPaid; count++;
    }
    if (count == 0) cout << "  No payment records.\n";
    cout << "\n  Records: " << count
         << "   |   Total: Rs " << fixed << setprecision(2) << total << "\n";
    printLine('=');
}

// ============================================================
//  CONSUMER — view own bill & payment history
// ============================================================
void consumerViewBillHistory(const string& consumerId) {
    vector<Bill>     bills     = loadBills();
    vector<Consumer> consumers = loadConsumers();

    string name = consumerName(consumers, consumerId);
    printLine('=');
    cout << "  BILL HISTORY — " << name << " (" << consumerId << ")\n";
    printLine('=');
    cout << left << setw(10) << "Bill ID"
                 << setw(10) << "Month"
                 << setw(10) << "Units"
                 << setw(14) << "Total (Rs)"
                 << setw(8)  << "Status"
                 << setw(12) << "Due Date" << "\n";
    printLine();

    int count = 0;
    for (const auto& b : bills) {
        if (b.consumerId != consumerId) continue;
        cout << left << setw(10) << b.billId
                     << setw(10) << b.month
                     << fixed << setprecision(0)
                     << setw(10) << b.units
                     << fixed << setprecision(2)
                     << setw(14) << b.totalAmount
                     << setw(8)  << b.status
                     << setw(12) << b.dueDate << "\n";
        count++;
    }
    if (count == 0) cout << "  No bills found.\n";
    cout << "\n  Total Bills: " << count << "\n";
    printLine('=');
}

void consumerViewPaymentHistory(const string& consumerId) {
    vector<Payment>  payments  = loadPayments();
    vector<Consumer> consumers = loadConsumers();

    string name = consumerName(consumers, consumerId);
    printLine('=');
    cout << "  PAYMENT HISTORY — " << name << " (" << consumerId << ")\n";
    printLine('=');
    cout << left << setw(10) << "Pay ID"
                 << setw(10) << "Bill ID"
                 << setw(14) << "Date"
                 << setw(12) << "Amount (Rs)" << "\n";
    printLine();

    double total = 0; int count = 0;
    for (const auto& p : payments) {
        if (p.consumerId != consumerId) continue;
        cout << left << setw(10) << p.paymentId
                     << setw(10) << p.billId
                     << setw(14) << p.paymentDate
                     << fixed << setprecision(2) << p.amountPaid << "\n";
        total += p.amountPaid; count++;
    }
    if (count == 0) cout << "  No payment records.\n";
    cout << "\n  Total Paid: Rs " << fixed << setprecision(2) << total << "\n";
    printLine('=');
}

// ============================================================
//  MODULE 4 — ADMIN MENU
// ============================================================
void module4AdminMenu() {
    int choice;
    do {
        cout << "\n";
        printLine('*');
        cout << "   MODULE 4 -- PAYMENT & HISTORY\n";
        printLine('*');
        cout << "  1. Process Payment (by Bill ID)\n";
        cout << "  2. Apply Late Fines to All Overdue Bills\n";
        cout << "  3. View Payment Records\n";
        cout << "  0. Back\n";
        printLine('*');
        cout << "  Enter choice: ";
        choice = safeIntInput();

        switch (choice) {
            case 1: adminProcessPayment(); break;
            case 2: adminApplyLateFines(); break;
            case 3: adminViewPayments();   break;
            case 0: cout << "  Returning...\n"; break;
            default: cout << "  Invalid option.\n";
        }
    } while (choice != 0);
}

// ============================================================
//  MODULE 4 — CONSUMER MENU
// ============================================================
void module4ConsumerMenu(const string& consumerId) {
    int choice;
    do {
        cout << "\n";
        printLine('*');
        cout << "   MODULE 4 -- MY BILLS & PAYMENTS\n";
        printLine('*');
        cout << "  1. Pay a Bill\n";
        cout << "  2. View Bill History\n";
        cout << "  3. View Payment History\n";
        cout << "  0. Back\n";
        printLine('*');
        cout << "  Enter choice: ";
        choice = safeIntInput();

        switch (choice) {
            case 1: consumerPayBill(consumerId);           break;
            case 2: consumerViewBillHistory(consumerId);   break;
            case 3: consumerViewPaymentHistory(consumerId);break;
            case 0: cout << "  Returning...\n"; break;
            default: cout << "  Invalid option.\n";
        }
    } while (choice != 0);
}
