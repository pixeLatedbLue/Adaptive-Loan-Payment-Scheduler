#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <algorithm>
using namespace std;

// ==============================
// Loan Structure
// ==============================
struct Loan {
    int id;
    string name;
    double principal;        // current outstanding
    double annualRate;       // %
    int daysUntilDue;        // days left to EMI due
    double lateFee;          // flat late fee
    double creditFactor;     // 0–1 impact on credit
    bool variableRate;
    double inflationSensitivity; // 0–1 multiplier for variable rate loans

    Loan(int id, string name, double principal, double rate, int days, double lateFee,
         double creditFactor = 0.0, bool variableRate = false, double inflationSensitivity = 0.0)
        : id(id), name(move(name)), principal(principal), annualRate(rate),
          daysUntilDue(days), lateFee(lateFee), creditFactor(creditFactor),
          variableRate(variableRate), inflationSensitivity(inflationSensitivity) {}
};

// ==============================
// Helper Functions
// ==============================
static inline double clamp(double v, double lo, double hi) {
    return max(lo, min(v, hi));
}

// Urgency score – closer due date = higher urgency
double computeUrgency(int days) {
    if (days <= 0) return 1.0;                // overdue = maximum urgency
    return 1.0 / (1.0 + log1p(days));         // smooth decay
}

double computePriority(const Loan& L, double inflationRate) {
    if (L.principal <= 1e-6) return -1e15;    // paid off loans drop to bottom

    const double urgency = computeUrgency(L.daysUntilDue);
    const double interestImpact = (L.annualRate / 100.0) * (L.principal / 1000.0);

    // Normalized penalty term
    double perRupeePenalty = L.lateFee / max(1.0, L.principal);
    perRupeePenalty = clamp(perRupeePenalty, 0.0, 5e3);
    const double penaltyWeight = perRupeePenalty * 10000.0 * urgency;

    const double creditImpact = L.creditFactor * 100.0;

    double inflationAdj = 0.0;
    if (L.variableRate)
        inflationAdj = -inflationRate * L.inflationSensitivity * (L.principal / 1000.0);

    // Weighted priority components
    double priority = (interestImpact * 1.5)
                    + (penaltyWeight * 0.8)
                    + (creditImpact * 0.8)
                    + (urgency * 5000.0)
                    + inflationAdj;

    if (L.daysUntilDue <= 5)
        priority *= 1.25; // short-term boost

    return priority;
}

// Comparator for heap (max-heap)
struct Compare {
    bool operator()(const pair<double, Loan>& a, const pair<double, Loan>& b) const {
        return a.first < b.first;
    }
};

// ==============================
// Adaptive Scheduler Class
// ==============================
class AdaptiveScheduler {
    vector<Loan> loans;
    double inflationRate;
    priority_queue<pair<double, Loan>, vector<pair<double, Loan>>, Compare> pq;

    void rebuildHeap() {
        while (!pq.empty()) pq.pop();
        for (const auto& L : loans) {
            double score = computePriority(L, inflationRate);
            pq.push({score, L});
        }
    }

public:
    explicit AdaptiveScheduler(double inflationRate = 0.05)
        : inflationRate(inflationRate) {}

    void addLoan(const Loan& L) {
        loans.push_back(L);
    }

    // Stable & accurate display directly from heap
    void displayPriorities() {
        if (loans.empty()) {
            cout << "\n⚠️  No loans to display.\n";
            return;
        }

        rebuildHeap();

        cout << "\n--- 📊 Current Loan Priorities ---\n";
        cout << "[DEBUG] pushing from heap in order of scores\n";

        cout << left << setw(22) << "Loan Name"
             << setw(18) << "Priority Score"
             << setw(15) << "Principal"
             << setw(12) << "Days Left" << "\n";
        cout << string(70, '-') << "\n";

        auto temp = pq;
        bool anyShown = false;
        cout << fixed << setprecision(2);

        while (!temp.empty()) {
            auto [score, L] = temp.top();
            temp.pop();
            if (L.principal <= 1e-6) continue;

            anyShown = true;

            cout << left << setw(22) << L.name
                 << setw(18) << score
                 << setw(15) << L.principal
                 << setw(12) << L.daysUntilDue << "\n";
        }

        if (!anyShown)
            cout << "✅ All loans repaid or inactive.\n";
    }

    void allocatePayment(double amount) {
        if (loans.empty()) {
            cout << "\n⚠️  No loans available for repayment.\n";
            return;
        }

        if (amount <= 0) {
            cout << "\n⚠️  Invalid payment amount.\n";
            return;
        }

        rebuildHeap();
        cout << "\n💸 Allocating Payment of ₹" << fixed << setprecision(2) << amount << " ---\n";

        while (amount > 0.0 && !pq.empty()) {
            auto [score, L] = pq.top();
            pq.pop();

            double pay = min(amount, L.principal);
            amount -= pay;
            L.principal -= pay;

            cout << "✅ Paid ₹" << pay
                 << " to " << L.name
                 << " | Remaining Principal: ₹" << L.principal << "\n";

            // Update in master list
            for (auto& loan : loans) {
                if (loan.id == L.id) {
                    loan.principal = L.principal;
                    break;
                }
            }

            rebuildHeap();   // dynamically refresh priorities
        }

        if (amount > 0.0)
            cout << "💰 Leftover cash: ₹" << fixed << setprecision(2) << amount << "\n";

        displayPriorities();
    }

    void simulateDays(int days) {
        if (days == 0) {
            cout << "\n⚠️  No days simulated.\n";
            return;
        }

        for (auto& L : loans)
            L.daysUntilDue -= days;

        cout << "\n⏳ Simulated " << days << " days. Deadlines updated.\n";
        rebuildHeap();
        displayPriorities();
    }
};

// ==============================
// Main Function
// ==============================
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    AdaptiveScheduler scheduler(0.05); // inflation = 5%
    int choice, id = 1;

    cout << "=== Adaptive Loan Repayment Scheduler ===\n";

    while (true) {
        cout << "\n========= MENU =========\n"
             << "1. Add a Loan\n"
             << "2. View Loan Priorities\n"
             << "3. Allocate Payment\n"
             << "4. Simulate Passing Days\n"
             << "5. Exit\n"
             << "========================\n"
             << "Enter choice: ";

        if (!(cin >> choice)) return 0;

        if (choice == 1) {
            string name;
            double principal, rate, fee, credit;
            int days;
            char varRate;

            cout << "Enter Loan Name: ";
            cin >> ws;
            getline(cin, name);

            cout << "Enter Principal Amount: ₹";
            cin >> principal;

            cout << "Enter Annual Interest Rate (%): ";
            cin >> rate;

            cout << "Enter Days Until Due: ";
            cin >> days;

            cout << "Enter Late Fee (₹): ";
            cin >> fee;

            cout << "Enter Credit Impact Factor (0–1): ";
            cin >> credit;

            cout << "Variable Rate (y/n)? ";
            cin >> varRate;

            scheduler.addLoan(
                Loan(id++, name, principal, rate, days, fee, credit, (varRate == 'y' || varRate == 'Y'))
            );

            cout << "✅ Loan added successfully!\n";
        }

        else if (choice == 2) {
            scheduler.displayPriorities();
        }

        else if (choice == 3) {
            double amt;
            cout << "Enter total payment amount: ₹";
            cin >> amt;
            scheduler.allocatePayment(amt);
        }

        else if (choice == 4) {
            int days;
            cout << "Enter number of days to simulate: ";
            cin >> days;
            scheduler.simulateDays(days);
        }

        else if (choice == 5) {
            cout << "\n=== ✅ Exiting Adaptive Scheduler ===\n";
            break;
        }

        else {
            cout << "❌ Invalid choice. Try again.\n";
        }
    }

    return 0;
}
