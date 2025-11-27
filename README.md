# Adaptive Loan Repayment Scheduler

A C++ based **Adaptive Loan Repayment Scheduler** that helps users manage **multiple loans at once** by deciding which loan to pay first using a **dynamic, multi-factor priority model**.

This project was built as part of the **Data Structures and Algorithms** course.

---

## 🧩 Problem Statement

Modern borrowers often juggle multiple loans at the same time — education loans, car loans, personal loans, etc.  
Each loan has:

- Different interest rates  
- Different due dates  
- Different late fees and penalty structures  
- Different impact on credit score  

When available funds are **not enough to pay all EMIs**, a key question arises:

> **Which loan should be paid first so that total cost, penalties, and credit risk are minimized?**

Traditional models like static EMI calendars or simple due-date-based reminders are **too rigid and one-dimensional**, and can lead to:

- Higher total interest paid  
- Heavy penalties  
- Damage to credit score  
- Stress and confusion for the borrower  

This project attempts to solve that with a **data-structured, algorithmic scheduler**.

---

## 🎯 Proposed Solution

The **Adaptive Loan Repayment Scheduler** acts like an automated financial advisor.

It:

1. **Models each loan** as a rich data structure with all relevant attributes.
2. **Computes a weighted priority score** for each loan using multiple financial and risk factors.
3. **Ranks loans** using a max-heap (**priority queue**) so that the most critical loan is always on top.
4. **Recomputes priorities dynamically** after:
   - Any payment (full or partial)
   - Simulated passage of days

So every time the system state changes, **priorities are updated in real-time**, making the scheduler truly adaptive.

---

## 🧱 Core Data Structures & Concepts

- `struct Loan`  
  Stores all parameters for a loan:
  - `id`, `name`
  - `principal`
  - `annualRate`
  - `daysUntilDue`
  - `lateFee`
  - `creditFactor` (credit score impact)
  - `variableRate`
  - `inflationSensitivity`

- **Priority Queue (Max-Heap)**  
  Implemented using `std::priority_queue<std::pair<double, Loan>, ...>`  
  Used to always fetch the loan with the **highest priority score** in `O(1)` and update in `O(log n)`.

- **Logarithmic Urgency Function**

  ```cpp
  double computeUrgency(int days) {
      if (days <= 0) return 1.0;
      return 1.0 / (1.0 + log1p(days));
  }
