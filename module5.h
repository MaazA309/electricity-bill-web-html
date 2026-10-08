#pragma once
// ============================================================
//  MODULE 5 — Report & File Management Module
//  (Original code — integrated; main() removed)
// ============================================================

// NOTE: models.h already included via module1.h etc.
// All structs, file I/O functions, and helpers are in models.h.
// Module 5 only needs its report/menu functions below.

// ─────────────────────────────────────────────
//  UTILITY — ensure reports/ directory exists
// ─────────────────────────────────────────────
// (ensureReportsDir already defined in models.h)

// ============================================================
//  REPORT 1 — Consumer List
// ============================================================
void reportConsumerList(const vector<Consumer>& consumers, ofstream& out) {
    cout << "\n";
    printLine('=');
    cout << "         CONSUMER LIST REPORT\n";
    cout << "  Generated: " << currentTimestamp() << "\n";
    printLine('=');
    cout << left << setw(10) << "ID" << setw(22) << "Name"
                 << setw(15) << "Type" << setw(12) << "Phone" << "\n";
    printLine();
    out << "CONSUMER LIST REPORT\n" << "Generated: " << currentTimestamp() << "\n"
        << string(65,'-') << "\n"
        << left << setw(10) << "ID" << setw(22) << "Name"
        << setw(15) << "Type" << setw(12) << "Phone" << "\n"
        << string(65,'-') << "\n";
    if (consumers.empty()) { cout << "  No consumers found.\n"; out << "No consumers found.\n"; }
    for (const auto& c : consumers) {
        cout << left << setw(10) << c.id << setw(22) << c.name
                     << setw(15) << c.type << setw(12) << c.phone << "\n";
        out  << left << setw(10) << c.id << setw(22) << c.name
                     << setw(15) << c.type << setw(12) << c.phone << "\n";
    }
    cout << "\n  Total Consumers: " << consumers.size() << "\n";
    out  << "\nTotal Consumers: "   << consumers.size() << "\n";
    printLine('=');
}

// ============================================================
//  REPORT 2 — Unpaid Bills
// ============================================================
void reportUnpaidBills(const vector<Bill>& bills, const vector<Consumer>& consumers, ofstream& out) {
    cout << "\n"; printLine('=');
    cout << "         UNPAID BILLS REPORT\n";
    cout << "  Generated: " << currentTimestamp() << "\n"; printLine('=');
    cout << left << setw(12) << "Bill ID" << setw(10) << "Cons ID"
                 << setw(18) << "Name"    << setw(10) << "Month"
                 << setw(10) << "Total (Rs)" << "\n"; printLine();
    out << "UNPAID BILLS REPORT\nGenerated: " << currentTimestamp() << "\n"
        << string(65,'-') << "\n"
        << left << setw(12) << "Bill ID" << setw(10) << "Cons ID"
        << setw(18) << "Name" << setw(10) << "Month" << setw(10) << "Total (Rs)" << "\n"
        << string(65,'-') << "\n";
    double grandTotal = 0; int count = 0;
    for (const auto& b : bills) {
        if (b.status != "Unpaid") continue;
        string name = consumerName(consumers, b.consumerId);
        cout << left << setw(12) << b.billId << setw(10) << b.consumerId
                     << setw(18) << name     << setw(10) << b.month
                     << fixed << setprecision(2) << b.totalAmount << "\n";
        out  << left << setw(12) << b.billId << setw(10) << b.consumerId
                     << setw(18) << name     << setw(10) << b.month
                     << fixed << setprecision(2) << b.totalAmount << "\n";
        grandTotal += b.totalAmount; count++;
    }
    if (count == 0) { cout << "  No unpaid bills.\n"; out << "No unpaid bills.\n"; }
    cout << "\n  Unpaid Bills: " << count << "   |   Total Pending: Rs "
         << fixed << setprecision(2) << grandTotal << "\n";
    out  << "\nUnpaid Bills: " << count << "   Total Pending: Rs "
         << fixed << setprecision(2) << grandTotal << "\n";
    printLine('=');
}

// ============================================================
//  REPORT 3 — Monthly Revenue
// ============================================================
void reportMonthlyRevenue(const vector<Bill>& bills, ofstream& out) {
    vector<pair<string,double>> revenue;
    for (const auto& b : bills) {
        if (b.status != "Paid") continue;
        bool found = false;
        for (auto& r : revenue)
            if (r.first == b.month) { r.second += b.totalAmount; found = true; break; }
        if (!found) revenue.push_back({b.month, b.totalAmount});
    }
    sort(revenue.begin(), revenue.end(),
         [](const pair<string,double>& a, const pair<string,double>& b){ return a.first < b.first; });
    cout << "\n"; printLine('=');
    cout << "         MONTHLY REVENUE REPORT\n";
    cout << "  Generated: " << currentTimestamp() << "\n"; printLine('=');
    cout << left << setw(15) << "Month" << setw(15) << "Revenue (Rs)" << "\n"; printLine();
    out << "MONTHLY REVENUE REPORT\nGenerated: " << currentTimestamp() << "\n"
        << string(65,'-') << "\n"
        << left << setw(15) << "Month" << setw(15) << "Revenue (Rs)" << "\n"
        << string(65,'-') << "\n";
    double total = 0;
    for (const auto& r : revenue) {
        cout << left << setw(15) << r.first << fixed << setprecision(2) << r.second << "\n";
        out  << left << setw(15) << r.first << fixed << setprecision(2) << r.second << "\n";
        total += r.second;
    }
    if (revenue.empty()) { cout << "  No paid bills found.\n"; out << "No paid bills found.\n"; }
    cout << "\n  Total Revenue Collected: Rs " << fixed << setprecision(2) << total << "\n";
    out  << "\nTotal Revenue Collected: Rs "    << fixed << setprecision(2) << total << "\n";
    printLine('=');
}

// ============================================================
//  REPORT 4 — Defaulters
// ============================================================
void reportDefaulters(const vector<Bill>& bills, const vector<Consumer>& consumers, ofstream& out) {
    vector<pair<string,int>> unpaidCount;
    for (const auto& b : bills) {
        if (b.status != "Unpaid") continue;
        bool found = false;
        for (auto& u : unpaidCount)
            if (u.first == b.consumerId) { u.second++; found = true; break; }
        if (!found) unpaidCount.push_back({b.consumerId, 1});
    }
    cout << "\n"; printLine('=');
    cout << "         DEFAULTERS REPORT  (2+ Unpaid Bills)\n";
    cout << "  Generated: " << currentTimestamp() << "\n"; printLine('=');
    cout << left << setw(10) << "Cons ID" << setw(22) << "Name"
                 << setw(10) << "Type"    << setw(12) << "Unpaid Bills" << "\n"; printLine();
    out << "DEFAULTERS REPORT\nGenerated: " << currentTimestamp() << "\n"
        << string(65,'-') << "\n"
        << left << setw(10) << "Cons ID" << setw(22) << "Name"
        << setw(10) << "Type" << setw(12) << "Unpaid Bills" << "\n"
        << string(65,'-') << "\n";
    int defaulterCount = 0;
    for (const auto& u : unpaidCount) {
        if (u.second < 2) continue;
        string name = consumerName(consumers, u.first);
        string type = "N/A";
        for (const auto& c : consumers) if (c.id == u.first) { type = c.type; break; }
        cout << left << setw(10) << u.first << setw(22) << name
                     << setw(10) << type    << setw(12) << u.second << "\n";
        out  << left << setw(10) << u.first << setw(22) << name
                     << setw(10) << type    << setw(12) << u.second << "\n";
        defaulterCount++;
    }
    if (defaulterCount == 0) { cout << "  No defaulters found.\n"; out << "No defaulters found.\n"; }
    cout << "\n  Total Defaulters: " << defaulterCount << "\n";
    out  << "\nTotal Defaulters: "   << defaulterCount << "\n";
    printLine('=');
}

// ============================================================
//  REPORT 5 — Consumer Bill History
// ============================================================
void reportConsumerBillHistory(const string& consumerId, const vector<Bill>& bills,
                               const vector<Consumer>& consumers, ofstream& out) {
    string name = consumerName(consumers, consumerId);
    cout << "\n"; printLine('=');
    cout << "  BILL HISTORY — Consumer: " << name << " (" << consumerId << ")\n";
    cout << "  Generated: " << currentTimestamp() << "\n"; printLine('=');
    cout << left << setw(12) << "Bill ID" << setw(10) << "Month"
                 << setw(10) << "Units"   << setw(14) << "Total (Rs)"
                 << setw(8)  << "Status"  << setw(12) << "Due Date" << "\n"; printLine();
    out << "BILL HISTORY — Consumer: " << name << " (" << consumerId << ")\n"
        << "Generated: " << currentTimestamp() << "\n" << string(65,'-') << "\n"
        << left << setw(12) << "Bill ID" << setw(10) << "Month"
        << setw(10) << "Units" << setw(14) << "Total (Rs)"
        << setw(8) << "Status" << setw(12) << "Due Date" << "\n"
        << string(65,'-') << "\n";
    int count = 0;
    for (const auto& b : bills) {
        if (b.consumerId != consumerId) continue;
        cout << left << setw(12) << b.billId << setw(10) << b.month
                     << fixed << setprecision(0) << setw(10) << b.units
                     << fixed << setprecision(2) << setw(14) << b.totalAmount
                     << setw(8) << b.status << setw(12) << b.dueDate << "\n";
        out  << left << setw(12) << b.billId << setw(10) << b.month
                     << fixed << setprecision(0) << setw(10) << b.units
                     << fixed << setprecision(2) << setw(14) << b.totalAmount
                     << setw(8) << b.status << setw(12) << b.dueDate << "\n";
        count++;
    }
    if (count == 0) { cout << "  No bills found for this consumer.\n"; out << "No bills found.\n"; }
    cout << "\n  Total Bills: " << count << "\n";
    out  << "\nTotal Bills: " << count << "\n";
    printLine('=');
}

// ============================================================
//  REPORT 6 — Payment History
// ============================================================
void reportPaymentHistory(const string& consumerId, const vector<Payment>& payments,
                          const vector<Consumer>& consumers, ofstream& out) {
    bool allConsumers = consumerId.empty();
    cout << "\n"; printLine('=');
    if (allConsumers) cout << "         FULL PAYMENT HISTORY\n";
    else cout << "  PAYMENT HISTORY — Consumer: "
              << consumerName(consumers, consumerId) << " (" << consumerId << ")\n";
    cout << "  Generated: " << currentTimestamp() << "\n"; printLine('=');
    cout << left << setw(12) << "Payment ID" << setw(12) << "Bill ID"
                 << setw(10) << "Cons ID"    << setw(14) << "Date"
                 << setw(12) << "Amount (Rs)" << "\n"; printLine();
    out << (allConsumers ? "FULL PAYMENT HISTORY\n" :
            "PAYMENT HISTORY — Consumer: " + consumerName(consumers, consumerId) + "\n")
        << "Generated: " << currentTimestamp() << "\n" << string(65,'-') << "\n"
        << left << setw(12) << "Payment ID" << setw(12) << "Bill ID"
        << setw(10) << "Cons ID" << setw(14) << "Date" << setw(12) << "Amount (Rs)" << "\n"
        << string(65,'-') << "\n";
    double total = 0; int count = 0;
    for (const auto& p : payments) {
        if (!allConsumers && p.consumerId != consumerId) continue;
        cout << left << setw(12) << p.paymentId << setw(12) << p.billId
                     << setw(10) << p.consumerId << setw(14) << p.paymentDate
                     << fixed << setprecision(2) << p.amountPaid << "\n";
        out  << left << setw(12) << p.paymentId << setw(12) << p.billId
                     << setw(10) << p.consumerId << setw(14) << p.paymentDate
                     << fixed << setprecision(2) << p.amountPaid << "\n";
        total += p.amountPaid; count++;
    }
    if (count == 0) { cout << "  No payment records found.\n"; out << "No payment records found.\n"; }
    cout << "\n  Total Payments: " << count
         << "   |   Total Amount: Rs " << fixed << setprecision(2) << total << "\n";
    out  << "\nTotal Payments: " << count
         << "   Total Amount: Rs " << fixed << setprecision(2) << total << "\n";
    printLine('=');
}

// ============================================================
//  REPORT EXPORT
// ============================================================
ofstream openReportFile(const string& reportName) {
    ensureReportsDir();
    string path = REPORTS_DIR + reportName + ".txt";
    ofstream out(path, ios::trunc);
    if (!out.is_open()) throw runtime_error("Cannot create report file: " + path);
    cout << "  [Report saved to: " << path << "]\n";
    return out;
}

// ============================================================
//  MODULE 5 — ADMIN MENU
// ============================================================
void module5AdminMenu() {
    int choice;
    do {
        cout << "\n"; printLine('*');
        cout << "   MODULE 5 -- REPORT & FILE MANAGEMENT\n"; printLine('*');
        cout << "  1. Consumer List Report\n";
        cout << "  2. Unpaid Bills Report\n";
        cout << "  3. Monthly Revenue Report\n";
        cout << "  4. Defaulters Report\n";
        cout << "  5. Consumer Bill History\n";
        cout << "  6. Full Payment History\n";
        cout << "  0. Back\n";
        printLine('*');
        cout << "  Enter choice: ";
        choice = safeIntInput();
        vector<Consumer> consumers; vector<Bill> bills; vector<Payment> payments;
        try {
            consumers = loadConsumers(); bills = loadBills(); payments = loadPayments();
        } catch (const exception& e) {
            cout << "  [File Error] " << e.what() << "\n"; continue;
        }
        switch (choice) {
            case 1: { ofstream out = openReportFile("consumer_list");
                      reportConsumerList(consumers, out); break; }
            case 2: { ofstream out = openReportFile("unpaid_bills");
                      reportUnpaidBills(bills, consumers, out); break; }
            case 3: { ofstream out = openReportFile("monthly_revenue");
                      reportMonthlyRevenue(bills, out); break; }
            case 4: { ofstream out = openReportFile("defaulters");
                      reportDefaulters(bills, consumers, out); break; }
            case 5: { cout << "  Enter Consumer ID: "; string cid; getline(cin, cid);
                      ofstream out = openReportFile("bill_history_" + cid);
                      reportConsumerBillHistory(cid, bills, consumers, out); break; }
            case 6: { ofstream out = openReportFile("payment_history_all");
                      reportPaymentHistory("", payments, consumers, out); break; }
            case 0: cout << "  Returning to main menu...\n"; break;
            default: cout << "  Invalid option.\n";
        }
    } while (choice != 0);
}

// ============================================================
//  MODULE 5 — CONSUMER MENU
// ============================================================
void module5ConsumerMenu(const string& consumerId) {
    int choice;
    do {
        cout << "\n"; printLine('*');
        cout << "   MODULE 5 -- MY REPORTS\n"; printLine('*');
        cout << "  1. View My Bill History\n";
        cout << "  2. View My Payment History\n";
        cout << "  0. Back\n";
        printLine('*');
        cout << "  Enter choice: ";
        choice = safeIntInput();
        vector<Consumer> consumers; vector<Bill> bills; vector<Payment> payments;
        try {
            consumers = loadConsumers(); bills = loadBills(); payments = loadPayments();
        } catch (const exception& e) {
            cout << "  [File Error] " << e.what() << "\n"; continue;
        }
        switch (choice) {
            case 1: { ofstream out = openReportFile("bill_history_" + consumerId);
                      reportConsumerBillHistory(consumerId, bills, consumers, out); break; }
            case 2: { ofstream out = openReportFile("payment_history_" + consumerId);
                      reportPaymentHistory(consumerId, payments, consumers, out); break; }
            case 0: cout << "  Returning to main menu...\n"; break;
            default: cout << "  Invalid option.\n";
        }
    } while (choice != 0);
}
