#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <memory>
#include <stdexcept>
#include <functional>
#include <queue>
#include <stack>
#include <list>
#include <numeric>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <regex>

using namespace std;
using namespace chrono;

// Forward declarations
class Book;
class Member;
class Librarian;
class Library;
class Transaction;
class Notification;
class ReportGenerator;

// Enums for various states
enum class BookStatus { AVAILABLE, BORROWED, RESERVED, LOST, DAMAGED, IN_REPAIR };
enum class MemberStatus { ACTIVE, SUSPENDED, EXPIRED, BANNED };
enum class TransactionType { BORROW, RETURN, RENEW, RESERVE, CANCEL_RESERVATION };
enum class NotificationType { DUE_REMINDER, OVERDUE, RESERVATION_AVAILABLE, FINE_ASSESSED };
enum class BookCategory { FICTION, NON_FICTION, SCIENCE, TECHNOLOGY, HISTORY, ART, MUSIC, CHILDREN, REFERENCE };
enum class FineStatus { UNPAID, PAID, WAIVED };

// Date class for handling dates
class Date {
private:
    int day, month, year;
    
    bool isLeapYear(int y) const {
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }
    
    int daysInMonth(int m, int y) const {
        static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (m == 2 && isLeapYear(y)) return 29;
        return days[m - 1];
    }
    
    void normalize() {
        while (month > 12) {
            month -= 12;
            year++;
        }
        while (month < 1) {
            month += 12;
            year--;
        }
        while (day > daysInMonth(month, year)) {
            day -= daysInMonth(month, year);
            month++;
            if (month > 12) {
                month = 1;
                year++;
            }
        }
    }

public:
    Date(int d = 1, int m = 1, int y = 2000) : day(d), month(m), year(y) {
        normalize();
    }
    
    static Date currentDate() {
        auto now = system_clock::now();
        time_t tt = system_clock::to_time_t(now);
        tm local_tm = *localtime(&tt);
        return Date(local_tm.tm_mday, local_tm.tm_mon + 1, local_tm.tm_year + 1900);
    }
    
    Date operator+(int days) const {
        Date result = *this;
        result.day += days;
        result.normalize();
        return result;
    }
    
    int operator-(const Date& other) const {
        // Simplified day difference calculation
        // In real implementation, this would be more accurate
        int days1 = year * 365 + month * 30 + day;
        int days2 = other.year * 365 + other.month * 30 + other.day;
        return days1 - days2;
    }
    
    bool operator<(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }
    
    bool operator<=(const Date& other) const {
        return !(other < *this);
    }
    
    string toString() const {
        ostringstream oss;
        oss << setfill('0') << setw(2) << day << "/"
            << setw(2) << month << "/" << year;
        return oss.str();
    }
    
    int getDay() const { return day; }
    int getMonth() const { return month; }
    int getYear() const { return year; }
};

// Exception classes
class LibraryException : public runtime_error {
public:
    LibraryException(const string& msg) : runtime_error(msg) {}
};

class BookNotFoundException : public LibraryException {
public:
    BookNotFoundException(const string& isbn) 
        : LibraryException("Book with ISBN " + isbn + " not found") {}
};

class MemberNotFoundException : public LibraryException {
public:
    MemberNotFoundException(int id) 
        : LibraryException("Member with ID " + to_string(id) + " not found") {}
};

class BookNotAvailableException : public LibraryException {
public:
    BookNotAvailableException(const string& title) 
        : LibraryException("Book '" + title + "' is not available for borrowing") {}
};

class MemberSuspendedException : public LibraryException {
public:
    MemberSuspendedException(const string& name) 
        : LibraryException("Member '" + name + "' is suspended") {}
};

// Book class
class Book {
private:
    string isbn;
    string title;
    string author;
    string publisher;
    int publicationYear;
    BookCategory category;
    BookStatus status;
    string location;
    double replacementCost;
    int borrowCount;
    vector<string> keywords;
    string edition;
    int totalCopies;
    int availableCopies;
    
public:
    Book(const string& isbn, const string& title, const string& author, 
         const string& publisher, int year, BookCategory cat)
        : isbn(isbn), title(title), author(author), publisher(publisher),
          publicationYear(year), category(cat), status(BookStatus::AVAILABLE),
          replacementCost(50.0), borrowCount(0), edition("First"),
          totalCopies(1), availableCopies(1) {
        generateKeywords();
    }
    
    void generateKeywords() {
        // Extract keywords from title and author
        istringstream iss(title);
        string word;
        while (iss >> word) {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            if (word.length() > 2) {
                keywords.push_back(word);
            }
        }
    }
    
    // Getters
    string getISBN() const { return isbn; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    int getPublicationYear() const { return publicationYear; }
    BookCategory getCategory() const { return category; }
    BookStatus getStatus() const { return status; }
    string getLocation() const { return location; }
    double getReplacementCost() const { return replacementCost; }
    int getBorrowCount() const { return borrowCount; }
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }
    string getEdition() const { return edition; }
    
    // Setters
    void setStatus(BookStatus newStatus) { status = newStatus; }
    void setLocation(const string& loc) { location = loc; }
    void setReplacementCost(double cost) { replacementCost = cost; }
    void setEdition(const string& ed) { edition = ed; }
    
    void addCopy() {
        totalCopies++;
        availableCopies++;
        if (status == BookStatus::AVAILABLE) {
            availableCopies++;
        }
    }
    
    bool removeCopy() {
        if (totalCopies > 1) {
            totalCopies--;
            if (status == BookStatus::AVAILABLE) {
                availableCopies--;
            }
            return true;
        }
        return false;
    }
    
    bool borrow() {
        if (availableCopies > 0) {
            availableCopies--;
            borrowCount++;
            if (availableCopies == 0) {
                status = BookStatus::BORROWED;
            }
            return true;
        }
        return false;
    }
    
    void return_() {
        availableCopies++;
        if (availableCopies > 0) {
            status = BookStatus::AVAILABLE;
        }
    }
    
    bool matchesKeyword(const string& keyword) const {
        string lowerKeyword = keyword;
        transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);
        return any_of(keywords.begin(), keywords.end(),
                     [&](const string& kw) { return kw.find(lowerKeyword) != string::npos; });
    }
    
    void displayInfo() const {
        cout << "ISBN: " << isbn << endl;
        cout << "Title: " << title << endl;
        cout << "Author: " << author << endl;
        cout << "Publisher: " << publisher << endl;
        cout << "Year: " << publicationYear << endl;
        cout << "Category: " << static_cast<int>(category) << endl;
        cout << "Status: " << static_cast<int>(status) << endl;
        cout << "Available Copies: " << availableCopies << "/" << totalCopies << endl;
        cout << "Borrow Count: " << borrowCount << endl;
    }
};

// Fine class
class Fine {
private:
    int memberId;
    double amount;
    Date assessedDate;
    Date dueDate;
    FineStatus status;
    string reason;
    
public:
    Fine(int memberId, double amount, const Date& assessed, const Date& due, const string& reason)
        : memberId(memberId), amount(amount), assessedDate(assessed), dueDate(due), 
          status(FineStatus::UNPAID), reason(reason) {}
    
    double getAmount() const { return amount; }
    FineStatus getStatus() const { return status; }
    Date getAssessedDate() const { return assessedDate; }
    Date getDueDate() const { return dueDate; }
    int getMemberId() const { return memberId; }
    
    void pay() { status = FineStatus::PAID; }
    void waive() { status = FineStatus::WAIVED; }
    
    double calculateLateFee(const Date& currentDate) const {
        if (status != FineStatus::UNPAID) return 0;
        if (currentDate <= dueDate) return 0;
        
        int daysLate = currentDate - dueDate;
        return amount + (daysLate * 0.50); // 50 cents per day late fee
    }
};

// Person base class
class Person {
protected:
    string name;
    string address;
    string phone;
    string email;
    Date dateOfBirth;
    
public:
    Person(const string& name, const string& addr, const string& phone, 
           const string& email, const Date& dob)
        : name(name), address(addr), phone(phone), email(email), dateOfBirth(dob) {}
    
    virtual ~Person() = default;
    
    string getName() const { return name; }
    string getAddress() const { return address; }
    string getPhone() const { return phone; }
    string getEmail() const { return email; }
    Date getDateOfBirth() const { return dateOfBirth; }
    
    void setAddress(const string& addr) { address = addr; }
    void setPhone(const string& p) { phone = p; }
    void setEmail(const string& e) { email = e; }
    
    virtual void displayInfo() const = 0;
};

// Member class
class Member : public Person, public enable_shared_from_this<Member> {
private:
    static int nextMemberId;
    int memberId;
    MemberStatus status;
    Date registrationDate;
    Date expiryDate;
    vector<string> borrowedBooks;
    map<string, Date> reservedBooks;
    vector<Fine> fines;
    int borrowingLimit;
    int totalBorrowed;
    string membershipType;
    
public:
    Member(const string& name, const string& addr, const string& phone, 
           const string& email, const Date& dob, const string& type = "Standard")
        : Person(name, addr, phone, email, dob), memberId(nextMemberId++),
          status(MemberStatus::ACTIVE), registrationDate(Date::currentDate()),
          expiryDate(registrationDate + 365), borrowingLimit(5), totalBorrowed(0),
          membershipType(type) {}
    
    int getMemberId() const { return memberId; }
    MemberStatus getStatus() const { return status; }
    Date getRegistrationDate() const { return registrationDate; }
    Date getExpiryDate() const { return expiryDate; }
    int getBorrowingLimit() const { return borrowingLimit; }
    int getTotalBorrowed() const { return totalBorrowed; }
    string getMembershipType() const { return membershipType; }
    
    bool canBorrow() const {
        return status == MemberStatus::ACTIVE && 
               Date::currentDate() <= expiryDate &&
               borrowedBooks.size() < borrowingLimit &&
               getTotalUnpaidFines() == 0;
    }
    
    double getTotalUnpaidFines() const {
        return accumulate(fines.begin(), fines.end(), 0.0,
                         [](double sum, const Fine& fine) {
                             return sum + (fine.getStatus() == FineStatus::UNPAID ? fine.getAmount() : 0);
                         });
    }
    
    bool borrowBook(const string& isbn) {
        if (!canBorrow()) return false;
        
        borrowedBooks.push_back(isbn);
        totalBorrowed++;
        return true;
    }
    
    bool returnBook(const string& isbn) {
        auto it = find(borrowedBooks.begin(), borrowedBooks.end(), isbn);
        if (it != borrowedBooks.end()) {
            borrowedBooks.erase(it);
            return true;
        }
        return false;
    }
    
    void reserveBook(const string& isbn, const Date& date) {
        reservedBooks[isbn] = date;
    }
    
    bool cancelReservation(const string& isbn) {
        return reservedBooks.erase(isbn) > 0;
    }
    
    void addFine(double amount, const Date& due, const string& reason) {
        fines.emplace_back(memberId, amount, Date::currentDate(), due, reason);
    }
    
    bool payFine(int fineIndex) {
        if (fineIndex >= 0 && fineIndex < fines.size()) {
            fines[fineIndex].pay();
            return true;
        }
        return false;
    }
    
    void suspend() { status = MemberStatus::SUSPENDED; }
    void activate() { status = MemberStatus::ACTIVE; }
    void expire() { status = MemberStatus::EXPIRED; }
    
    void displayInfo() const override {
        cout << "Member ID: " << memberId << endl;
        cout << "Name: " << name << endl;
        cout << "Status: " << static_cast<int>(status) << endl;
        cout << "Membership Type: " << membershipType << endl;
        cout << "Books Borrowed: " << borrowedBooks.size() << "/" << borrowingLimit << endl;
        cout << "Total Fines: $" << getTotalUnpaidFines() << endl;
        cout << "Expiry Date: " << expiryDate.toString() << endl;
    }
    
    vector<string> getBorrowedBooks() const { return borrowedBooks; }
    vector<Fine> getFines() const { return fines; }
};

int Member::nextMemberId = 1000;

// Transaction class
class Transaction {
private:
    static int nextTransactionId;
    int transactionId;
    TransactionType type;
    int memberId;
    string bookIsbn;
    Date transactionDate;
    Date dueDate;
    Date returnDate;
    double fineAmount;
    string notes;
    
public:
    Transaction(TransactionType t, int memberId, const string& isbn, const Date& due = Date::currentDate())
        : transactionId(nextTransactionId++), type(t), memberId(memberId), bookIsbn(isbn),
          transactionDate(Date::currentDate()), dueDate(due), returnDate(),
          fineAmount(0.0) {}
    
    int getTransactionId() const { return transactionId; }
    TransactionType getType() const { return type; }
    int getMemberId() const { return memberId; }
    string getBookIsbn() const { return bookIsbn; }
    Date getTransactionDate() const { return transactionDate; }
    Date getDueDate() const { return dueDate; }
    
    void setReturnDate(const Date& date) { 
        returnDate = date;
        calculateFine();
    }
    
    void calculateFine() {
        if (returnDate.getDay() > 0 && returnDate < dueDate) {
            int daysLate = dueDate - returnDate;
            fineAmount = daysLate * 0.50; // 50 cents per day
        }
    }
    
    void addNote(const string& note) { notes = note; }
    
    void displayInfo() const {
        cout << "Transaction ID: " << transactionId << endl;
        cout << "Type: " << static_cast<int>(type) << endl;
        cout << "Member ID: " << memberId << endl;
        cout << "Book ISBN: " << bookIsbn << endl;
        cout << "Date: " << transactionDate.toString() << endl;
        if (dueDate.getDay() > 0) {
            cout << "Due Date: " << dueDate.toString() << endl;
        }
        if (fineAmount > 0) {
            cout << "Fine: $" << fixed << setprecision(2) << fineAmount << endl;
        }
    }
};

int Transaction::nextTransactionId = 5000;

// Notification class
class Notification {
private:
    static int nextNotificationId;
    int notificationId;
    int memberId;
    NotificationType type;
    string message;
    Date sentDate;
    bool isRead;
    bool isDelivered;
    
public:
    Notification(int memberId, NotificationType type, const string& msg)
        : notificationId(nextNotificationId++), memberId(memberId), type(type),
          message(msg), sentDate(Date::currentDate()), isRead(false), isDelivered(false) {}
    
    void markAsRead() { isRead = true; }
    void markAsDelivered() { isDelivered = true; }
    
    bool wasRead() const { return isRead; }
    bool wasDelivered() const { return isDelivered; }
    int getMemberId() const { return memberId; }
    string getMessage() const { return message; }
    Date getSentDate() const { return sentDate; }
    
    void displayInfo() const {
        cout << "Notification #" << notificationId << endl;
        cout << "Type: " << static_cast<int>(type) << endl;
        cout << "Message: " << message << endl;
        cout << "Sent: " << sentDate.toString() << endl;
        cout << "Status: " << (isRead ? "Read" : "Unread") << endl;
    }
};

int Notification::nextNotificationId = 1;

// Librarian class
class Librarian : public Person {
private:
    int employeeId;
    string department;
    Date hireDate;
    string qualification;
    double salary;
    vector<string> permissions;
    
public:
    Librarian(const string& name, const string& addr, const string& phone,
              const string& email, const Date& dob, int empId, const string& dept)
        : Person(name, addr, phone, email, dob), employeeId(empId), department(dept),
          hireDate(Date::currentDate()), salary(45000.0) {
        initializePermissions();
    }
    
    void initializePermissions() {
        permissions = {"ADD_BOOK", "REMOVE_BOOK", "UPDATE_BOOK", "ADD_MEMBER", 
                       "SUSPEND_MEMBER", "PROCESS_FINES", "GENERATE_REPORTS"};
    }
    
    bool hasPermission(const string& perm) const {
        return find(permissions.begin(), permissions.end(), perm) != permissions.end();
    }
    
    void addPermission(const string& perm) {
        permissions.push_back(perm);
    }
    
    void setSalary(double newSalary) { salary = newSalary; }
    double getSalary() const { return salary; }
    int getEmployeeId() const { return employeeId; }
    string getDepartment() const { return department; }
    
    void displayInfo() const override {
        cout << "Employee ID: " << employeeId << endl;
        cout << "Name: " << name << endl;
        cout << "Department: " << department << endl;
        cout << "Hire Date: " << hireDate.toString() << endl;
        cout << "Permissions: " << permissions.size() << endl;
    }
};

// Search strategy interface
class SearchStrategy {
public:
    virtual vector<shared_ptr<Book>> search(const vector<shared_ptr<Book>>& books, const string& query) = 0;
    virtual ~SearchStrategy() = default;
};

class TitleSearch : public SearchStrategy {
public:
    vector<shared_ptr<Book>> search(const vector<shared_ptr<Book>>& books, const string& query) override {
        vector<shared_ptr<Book>> results;
        string lowerQuery = query;
        transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        
        for (const auto& book : books) {
            string title = book->getTitle();
            transform(title.begin(), title.end(), title.begin(), ::tolower);
            if (title.find(lowerQuery) != string::npos) {
                results.push_back(book);
            }
        }
        return results;
    }
};

class AuthorSearch : public SearchStrategy {
public:
    vector<shared_ptr<Book>> search(const vector<shared_ptr<Book>>& books, const string& query) override {
        vector<shared_ptr<Book>> results;
        string lowerQuery = query;
        transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        
        for (const auto& book : books) {
            string author = book->getAuthor();
            transform(author.begin(), author.end(), author.begin(), ::tolower);
            if (author.find(lowerQuery) != string::npos) {
                results.push_back(book);
            }
        }
        return results;
    }
};

class ISBNSearch : public SearchStrategy {
public:
    vector<shared_ptr<Book>> search(const vector<shared_ptr<Book>>& books, const string& query) override {
        vector<shared_ptr<Book>> results;
        
        for (const auto& book : books) {
            if (book->getISBN() == query) {
                results.push_back(book);
            }
        }
        return results;
    }
};

// ReportGenerator class
class ReportGenerator {
private:
    ofstream reportFile;
    string reportPath;
    
public:
    ReportGenerator(const string& path) : reportPath(path) {}
    
    void generateMemberReport(const vector<shared_ptr<Member>>& members) {
        reportFile.open(reportPath + "/member_report_" + Date::currentDate().toString() + ".txt");
        if (!reportFile.is_open()) return;
        
        reportFile << "LIBRARY MEMBER REPORT" << endl;
        reportFile << "Generated: " << Date::currentDate().toString() << endl;
        reportFile << string(50, '=') << endl << endl;
        
        int activeCount = 0, suspendedCount = 0, expiredCount = 0;
        
        for (const auto& member : members) {
            member->displayInfo();
            reportFile << string(30, '-') << endl;
            
            switch(member->getStatus()) {
                case MemberStatus::ACTIVE: activeCount++; break;
                case MemberStatus::SUSPENDED: suspendedCount++; break;
                case MemberStatus::EXPIRED: expiredCount++; break;
                default: break;
            }
        }
        
        reportFile << "\nSUMMARY" << endl;
        reportFile << "Total Members: " << members.size() << endl;
        reportFile << "Active: " << activeCount << endl;
        reportFile << "Suspended: " << suspendedCount << endl;
        reportFile << "Expired: " << expiredCount << endl;
        
        reportFile.close();
    }
    
    void generateBookReport(const vector<shared_ptr<Book>>& books) {
        reportFile.open(reportPath + "/book_report_" + Date::currentDate().toString() + ".txt");
        if (!reportFile.is_open()) return;
        
        reportFile << "LIBRARY BOOK REPORT" << endl;
        reportFile << "Generated: " << Date::currentDate().toString() << endl;
        reportFile << string(50, '=') << endl << endl;
        
        int totalBorrows = 0;
        int availableCount = 0, borrowedCount = 0, lostCount = 0;
        
        for (const auto& book : books) {
            book->displayInfo();
            reportFile << string(30, '-') << endl;
            
            totalBorrows += book->getBorrowCount();
            
            switch(book->getStatus()) {
                case BookStatus::AVAILABLE: availableCount++; break;
                case BookStatus::BORROWED: borrowedCount++; break;
                case BookStatus::LOST: lostCount++; break;
                default: break;
            }
        }
        
        reportFile << "\nSUMMARY" << endl;
        reportFile << "Total Books: " << books.size() << endl;
        reportFile << "Available: " << availableCount << endl;
        reportFile << "Borrowed: " << borrowedCount << endl;
        reportFile << "Lost: " << lostCount << endl;
        reportFile << "Total Borrows: " << totalBorrows << endl;
        
        reportFile.close();
    }
    
    void generateTransactionReport(const vector<Transaction>& transactions) {
        reportFile.open(reportPath + "/transaction_report_" + Date::currentDate().toString() + ".txt");
        if (!reportFile.is_open()) return;
        
        reportFile << "LIBRARY TRANSACTION REPORT" << endl;
        reportFile << "Generated: " << Date::currentDate().toString() << endl;
        reportFile << string(50, '=') << endl << endl;
        
        map<TransactionType, int> typeCount;
        
        for (const auto& trans : transactions) {
            trans.displayInfo();
            reportFile << string(30, '-') << endl;
            typeCount[trans.getType()]++;
        }
        
        reportFile << "\nSUMMARY" << endl;
        reportFile << "Total Transactions: " << transactions.size() << endl;
        for (const auto& pair : typeCount) {
            reportFile << "Type " << static_cast<int>(pair.first) << ": " << pair.second << endl;
        }
        
        reportFile.close();
    }
};

// Library class - main system
class Library {
private:
    string name;
    string address;
    string phone;
    string email;
    vector<shared_ptr<Book>> books;
    vector<shared_ptr<Member>> members;
    vector<shared_ptr<Librarian>> librarians;
    vector<Transaction> transactions;
    queue<Notification> notificationQueue;
    stack<Transaction> transactionHistory;
    map<string, queue<string>> reservationQueue; // ISBN -> queue of member IDs
    unique_ptr<SearchStrategy> searchStrategy;
    ReportGenerator reportGen;
    mutable mutex libraryMutex;
    
public:
    Library(const string& name, const string& addr, const string& phone, const string& email)
        : name(name), address(addr), phone(phone), email(email), 
          reportGen("./reports") {
        initializeDefaultData();
    }
    
    void initializeDefaultData() {
        // Add some default books
        addBook(make_shared<Book>("978-0-13-110362-7", "The C++ Programming Language", "Bjarne Stroustrup", 
                                  "Addison-Wesley", 2013, BookCategory::TECHNOLOGY));
        addBook(make_shared<Book>("978-0-596-52068-7", "Head First Design Patterns", "Eric Freeman", 
                                  "O'Reilly Media", 2004, BookCategory::TECHNOLOGY));
        addBook(make_shared<Book>("978-0-201-63361-0", "Design Patterns", "Erich Gamma", 
                                  "Addison-Wesley", 1994, BookCategory::TECHNOLOGY));
        
        // Add some default members
        Date dob(15, 6, 1990);
        addMember(make_shared<Member>("John Doe", "123 Main St", "555-0101", "john@email.com", dob, "Premium"));
        
        // Add default librarian
        Date libDob(10, 3, 1985);
        librarians.push_back(make_shared<Librarian>("Jane Smith", "456 Oak Ave", "555-0102", 
                                                    "jane@library.com", libDob, 1001, "Circulation"));
    }
    
    void setSearchStrategy(unique_ptr<SearchStrategy> strategy) {
        searchStrategy = move(strategy);
    }
    
    vector<shared_ptr<Book>> searchBooks(const string& query) {
        lock_guard<mutex> lock(libraryMutex);
        if (searchStrategy) {
            return searchStrategy->search(books, query);
        }
        return vector<shared_ptr<Book>>();
    }
    
    void addBook(shared_ptr<Book> book) {
        lock_guard<mutex> lock(libraryMutex);
        books.push_back(book);
    }
    
    bool removeBook(const string& isbn) {
        lock_guard<mutex> lock(libraryMutex);
        auto it = find_if(books.begin(), books.end(),
                         [&](const auto& book) { return book->getISBN() == isbn; });
        
        if (it != books.end()) {
            books.erase(it);
            return true;
        }
        return false;
    }
    
    shared_ptr<Book> findBookByISBN(const string& isbn) {
        lock_guard<mutex> lock(libraryMutex);
        auto it = find_if(books.begin(), books.end(),
                         [&](const auto& book) { return book->getISBN() == isbn; });
        
        if (it != books.end()) {
            return *it;
        }
        return nullptr;
    }
    
    void addMember(shared_ptr<Member> member) {
        lock_guard<mutex> lock(libraryMutex);
        members.push_back(member);
    }
    
    bool removeMember(int memberId) {
        lock_guard<mutex> lock(libraryMutex);
        auto it = find_if(members.begin(), members.end(),
                         [&](const auto& member) { return member->getMemberId() == memberId; });
        
        if (it != members.end()) {
            members.erase(it);
            return true;
        }
        return false;
    }
    
    shared_ptr<Member> findMemberById(int memberId) {
        lock_guard<mutex> lock(libraryMutex);
        auto it = find_if(members.begin(), members.end(),
                         [&](const auto& member) { return member->getMemberId() == memberId; });
        
        if (it != members.end()) {
            return *it;
        }
        return nullptr;
    }
    
    bool borrowBook(int memberId, const string& isbn) {
        lock_guard<mutex> lock(libraryMutex);
        
        auto member = findMemberById(memberId);
        if (!member) {
            throw MemberNotFoundException(memberId);
        }
        
        if (!member->canBorrow()) {
            throw MemberSuspendedException(member->getName());
        }
        
        auto book = findBookByISBN(isbn);
        if (!book) {
            throw BookNotFoundException(isbn);
        }
        
        if (book->getAvailableCopies() <= 0) {
            // Add to reservation queue
            reservationQueue[isbn].push(to_string(memberId));
            throw BookNotAvailableException(book->getTitle());
        }
        
        if (book->borrow() && member->borrowBook(isbn)) {
            Date dueDate = Date::currentDate() + 14; // 14 days borrowing period
            Transaction trans(TransactionType::BORROW, memberId, isbn, dueDate);
            transactions.push_back(trans);
            transactionHistory.push(trans);
            
            // Send notification
            string msg = "You have borrowed '" + book->getTitle() + "'. Due date: " + dueDate.toString();
            notificationQueue.emplace(memberId, NotificationType::DUE_REMINDER, msg);
            
            return true;
        }
        
        return false;
    }
    
    bool returnBook(int memberId, const string& isbn) {
        lock_guard<mutex> lock(libraryMutex);
        
        auto member = findMemberById(memberId);
        if (!member) {
            throw MemberNotFoundException(memberId);
        }
        
        auto book = findBookByISBN(isbn);
        if (!book) {
            throw BookNotFoundException(isbn);
        }
        
        if (member->returnBook(isbn)) {
            book->return_();
            
            Transaction trans(TransactionType::RETURN, memberId, isbn);
            trans.setReturnDate(Date::currentDate());
            transactions.push_back(trans);
            transactionHistory.push(trans);
            
            // Check reservation queue
            auto& queue = reservationQueue[isbn];
            if (!queue.empty()) {
                int nextMemberId = stoi(queue.front());
                queue.pop();
                
                auto nextMember = findMemberById(nextMemberId);
                if (nextMember) {
                    string msg = "Book '" + book->getTitle() + "' is now available for you.";
                    notificationQueue.emplace(nextMemberId, NotificationType::RESERVATION_AVAILABLE, msg);
                }
            }
            
            return true;
        }
        
        return false;
    }
    
    void reserveBook(int memberId, const string& isbn) {
        lock_guard<mutex> lock(libraryMutex);
        
        auto member = findMemberById(memberId);
        if (!member) {
            throw MemberNotFoundException(memberId);
        }
        
        auto book = findBookByISBN(isbn);
        if (!book) {
            throw BookNotFoundException(isbn);
        }
        
        member->reserveBook(isbn, Date::currentDate());
        reservationQueue[isbn].push(to_string(memberId));
        
        Transaction trans(TransactionType::RESERVE, memberId, isbn);
        transactions.push_back(trans);
    }
    
    void processOverdueBooks() {
        lock_guard<mutex> lock(libraryMutex);
        Date today = Date::currentDate();
        
        for (const auto& trans : transactions) {
            if (trans.getType() == TransactionType::BORROW && today < trans.getDueDate()) {
                int memberId = trans.getMemberId();
                auto member = findMemberById(memberId);
                
                if (member) {
                    int daysOverdue = trans.getDueDate() - today;
                    double fine = daysOverdue * 0.50;
                    
                    member->addFine(fine, today + 30, "Overdue book fine");
                    
                    string msg = "You have an overdue fine of $" + to_string(fine) + 
                                " for book. Please pay within 30 days.";
                    notificationQueue.emplace(memberId, NotificationType::OVERDUE, msg);
                }
            }
        }
    }
    
    void sendNotifications() {
        lock_guard<mutex> lock(libraryMutex);
        
        while (!notificationQueue.empty()) {
            auto& notification = notificationQueue.front();
            
            auto member = findMemberById(notification.getMemberId());
            if (member) {
                cout << "Sending notification to " << member->getName() << ": ";
                cout << notification.getMessage() << endl;
                notification.markAsDelivered();
            }
            
            notificationQueue.pop();
        }
    }
    
    void generateReports() {
        lock_guard<mutex> lock(libraryMutex);
        
        reportGen.generateMemberReport(members);
        reportGen.generateBookReport(books);
        reportGen.generateTransactionReport(transactions);
        
        cout << "Reports generated successfully!" << endl;
    }
    
    void displayLibraryInfo() const {
        cout << "\n=== Library Information ===" << endl;
        cout << "Name: " << name << endl;
        cout << "Address: " << address << endl;
        cout << "Phone: " << phone << endl;
        cout << "Email: " << email << endl;
        cout << "Total Books: " << books.size() << endl;
        cout << "Total Members: " << members.size() << endl;
        cout << "Total Librarians: " << librarians.size() << endl;
        cout << "Total Transactions: " << transactions.size() << endl;
        cout << "Pending Notifications: " << notificationQueue.size() << endl;
    }
    
    // Async operations
    future<bool> borrowBookAsync(int memberId, const string& isbn) {
        return async(launch::async, [this, memberId, isbn]() {
            try {
                return this->borrowBook(memberId, isbn);
            } catch (const exception& e) {
                cout << "Error in async borrow: " << e.what() << endl;
                return false;
            }
        });
    }
    
    future<bool> returnBookAsync(int memberId, const string& isbn) {
        return async(launch::async, [this, memberId, isbn]() {
            try {
                return this->returnBook(memberId, isbn);
            } catch (const exception& e) {
                cout << "Error in async return: " << e.what() << endl;
                return false;
            }
        });
    }
    
    // Statistics
    void displayStatistics() {
        lock_guard<mutex> lock(libraryMutex);
        
        cout << "\n=== Library Statistics ===" << endl;
        
        // Book statistics
        int totalBooks = books.size();
        int availableBooks = count_if(books.begin(), books.end(),
                                      [](const auto& b) { return b->getAvailableCopies() > 0; });
        int borrowedBooks = count_if(books.begin(), books.end(),
                                     [](const auto& b) { return b->getStatus() == BookStatus::BORROWED; });
        
        cout << "\nBook Statistics:" << endl;
        cout << "  Total Books: " << totalBooks << endl;
        cout << "  Available: " << availableBooks << endl;
        cout << "  Borrowed: " << borrowedBooks << endl;
        
        // Member statistics
        int totalMembers = members.size();
        int activeMembers = count_if(members.begin(), members.end(),
                                     [](const auto& m) { return m->getStatus() == MemberStatus::ACTIVE; });
        int suspendedMembers = count_if(members.begin(), members.end(),
                                        [](const auto& m) { return m->getStatus() == MemberStatus::SUSPENDED; });
        
        cout << "\nMember Statistics:" << endl;
        cout << "  Total Members: " << totalMembers << endl;
        cout << "  Active: " << activeMembers << endl;
        cout << "  Suspended: " << suspendedMembers << endl;
        
        // Transaction statistics
        cout << "\nTransaction Statistics:" << endl;
        cout << "  Total Transactions: " << transactions.size() << endl;
        
        double totalFines = 0;
        for (const auto& member : members) {
            totalFines += member->getTotalUnpaidFines();
        }
        cout << "  Total Unpaid Fines: $" << fixed << setprecision(2) << totalFines << endl;
    }
};

// Console Menu System
class LibraryConsole {
private:
    Library library;
    
public:
    LibraryConsole() : library("City Central Library", "123 Library St", "555-LIBRARY", "info@citylibrary.com") {
        initializeTestData();
    }
    
    void initializeTestData() {
        // Add more test books
        auto book1 = make_shared<Book>("978-0-13-235088-4", "Clean Code", "Robert Martin", 
                                       "Prentice Hall", 2008, BookCategory::TECHNOLOGY);
        library.addBook(book1);
        
        auto book2 = make_shared<Book>("978-0-321-35668-0", "Effective Modern C++", "Scott Meyers", 
                                       "O'Reilly Media", 2014, BookCategory::TECHNOLOGY);
        library.addBook(book2);
        
        // Add more test members
        Date dob1(20, 8, 1988);
        auto member1 = make_shared<Member>("Alice Johnson", "789 Pine St", "555-0103", 
                                          "alice@email.com", dob1, "Standard");
        library.addMember(member1);
        
        Date dob2(5, 3, 1995);
        auto member2 = make_shared<Member>("Bob Wilson", "321 Elm St", "555-0104", 
                                          "bob@email.com", dob2, "Student");
        library.addMember(member2);
    }
    
    void run() {
        int choice = 0;
        
        do {
            displayMainMenu();
            cin >> choice;
            cin.ignore();
            
            handleMenuChoice(choice);
            
        } while (choice != 0);
    }
    
    void displayMainMenu() {
        cout << "\n=== Library Management System ===" << endl;
        cout << "1. Book Operations" << endl;
        cout << "2. Member Operations" << endl;
        cout << "3. Borrow/Return Operations" << endl;
        cout << "4. Search Books" << endl;
        cout << "5. Display Statistics" << endl;
        cout << "6. Generate Reports" << endl;
        cout << "7. Process Notifications" << endl;
        cout << "8. Process Overdue Books" << endl;
        cout << "9. Display Library Info" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter your choice: ";
    }
    
    void handleMenuChoice(int choice) {
        switch (choice) {
            case 1:
                bookOperationsMenu();
                break;
            case 2:
                memberOperationsMenu();
                break;
            case 3:
                borrowReturnOperations();
                break;
            case 4:
                searchBooksMenu();
                break;
            case 5:
                library.displayStatistics();
                break;
            case 6:
                library.generateReports();
                break;
            case 7:
                library.sendNotifications();
                break;
            case 8:
                library.processOverdueBooks();
                break;
            case 9:
                library.displayLibraryInfo();
                break;
            case 0:
                cout << "Thank you for using the Library Management System!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    void bookOperationsMenu() {
        int choice;
        cout << "\n=== Book Operations ===" << endl;
        cout << "1. Add New Book" << endl;
        cout << "2. Remove Book" << endl;
        cout << "3. Find Book by ISBN" << endl;
        cout << "4. Display All Books" << endl;
        cout << "5. Update Book Information" << endl;
        cout << "6. Add Copy of Book" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1:
                addBookMenu();
                break;
            case 2:
                removeBookMenu();
                break;
            case 3:
                findBookMenu();
                break;
            case 4:
                displayAllBooks();
                break;
            case 5:
                updateBookMenu();
                break;
            case 6:
                addBookCopyMenu();
                break;
            default:
                cout << "Invalid choice." << endl;
        }
    }
    
    void addBookMenu() {
        string isbn, title, author, publisher;
        int year, catInt;
        
        cout << "\nEnter ISBN: ";
        getline(cin, isbn);
        cout << "Enter Title: ";
        getline(cin, title);
        cout << "Enter Author: ";
        getline(cin, author);
        cout << "Enter Publisher: ";
        getline(cin, publisher);
        cout << "Enter Publication Year: ";
        cin >> year;
        cout << "Enter Category (0-8): ";
        cin >> catInt;
        cin.ignore();
        
        BookCategory category = static_cast<BookCategory>(catInt);
        
        auto book = make_shared<Book>(isbn, title, author, publisher, year, category);
        library.addBook(book);
        
        cout << "Book added successfully!" << endl;
    }
    
    void removeBookMenu() {
        string isbn;
        cout << "Enter ISBN of book to remove: ";
        getline(cin, isbn);
        
        if (library.removeBook(isbn)) {
            cout << "Book removed successfully!" << endl;
        } else {
            cout << "Book not found." << endl;
        }
    }
    
    void findBookMenu() {
        string isbn;
        cout << "Enter ISBN: ";
        getline(cin, isbn);
        
        auto book = library.findBookByISBN(isbn);
        if (book) {
            cout << "\nBook Found:" << endl;
            book->displayInfo();
        } else {
            cout << "Book not found." << endl;
        }
    }
    
    void displayAllBooks() {
        cout << "\n=== All Books in Library ===" << endl;
        
        // This would need a method to get all books from library
        // For now, we'll just use search with empty string
        library.setSearchStrategy(make_unique<TitleSearch>());
        auto results = library.searchBooks("");
        
        if (results.empty()) {
            cout << "No books in library." << endl;
        } else {
            for (const auto& book : results) {
                book->displayInfo();
                cout << string(40, '-') << endl;
            }
        }
    }
    
    void updateBookMenu() {
        string isbn;
        cout << "Enter ISBN of book to update: ";
        getline(cin, isbn);
        
        auto book = library.findBookByISBN(isbn);
        if (!book) {
            cout << "Book not found." << endl;
            return;
        }
        
        cout << "\nUpdating Book: " << book->getTitle() << endl;
        cout << "1. Update Location" << endl;
        cout << "2. Update Replacement Cost" << endl;
        cout << "3. Update Edition" << endl;
        cout << "Enter choice: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: {
                string location;
                cout << "Enter new location: ";
                getline(cin, location);
                book->setLocation(location);
                cout << "Location updated." << endl;
                break;
            }
            case 2: {
                double cost;
                cout << "Enter new replacement cost: $";
                cin >> cost;
                book->setReplacementCost(cost);
                cout << "Cost updated." << endl;
                break;
            }
            case 3: {
                string edition;
                cout << "Enter new edition: ";
                getline(cin, edition);
                book->setEdition(edition);
                cout << "Edition updated." << endl;
                break;
            }
            default:
                cout << "Invalid choice." << endl;
        }
    }
    
    void addBookCopyMenu() {
        string isbn;
        cout << "Enter ISBN of book to add copy: ";
        getline(cin, isbn);
        
        auto book = library.findBookByISBN(isbn);
        if (book) {
            book->addCopy();
            cout << "Copy added successfully! Total copies: " << book->getTotalCopies() << endl;
        } else {
            cout << "Book not found." << endl;
        }
    }
    
    void memberOperationsMenu() {
        int choice;
        cout << "\n=== Member Operations ===" << endl;
        cout << "1. Add New Member" << endl;
        cout << "2. Remove Member" << endl;
        cout << "3. Find Member by ID" << endl;
        cout << "4. Display All Members" << endl;
        cout << "5. Update Member Information" << endl;
        cout << "6. Suspend Member" << endl;
        cout << "7. Activate Member" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1:
                addMemberMenu();
                break;
            case 2:
                removeMemberMenu();
                break;
            case 3:
                findMemberMenu();
                break;
            case 4:
                displayAllMembers();
                break;
            case 5:
                updateMemberMenu();
                break;
            case 6:
                suspendMemberMenu();
                break;
            case 7:
                activateMemberMenu();
                break;
            default:
                cout << "Invalid choice." << endl;
        }
    }
    
    void addMemberMenu() {
        string name, address, phone, email, membershipType;
        int day, month, year;
        
        cout << "\nEnter Member Name: ";
        getline(cin, name);
        cout << "Enter Address: ";
        getline(cin, address);
        cout << "Enter Phone: ";
        getline(cin, phone);
        cout << "Enter Email: ";
        getline(cin, email);
        cout << "Enter Birth Date (day month year): ";
        cin >> day >> month >> year;
        cin.ignore();
        cout << "Enter Membership Type (Standard/Premium/Student): ";
        getline(cin, membershipType);
        
        Date dob(day, month, year);
        auto member = make_shared<Member>(name, address, phone, email, dob, membershipType);
        library.addMember(member);
        
        cout << "Member added successfully with ID: " << member->getMemberId() << endl;
    }
    
    void removeMemberMenu() {
        int memberId;
        cout << "Enter Member ID to remove: ";
        cin >> memberId;
        cin.ignore();
        
        if (library.removeMember(memberId)) {
            cout << "Member removed successfully!" << endl;
        } else {
            cout << "Member not found." << endl;
        }
    }
    
    void findMemberMenu() {
        int memberId;
        cout << "Enter Member ID: ";
        cin >> memberId;
        cin.ignore();
        
        auto member = library.findMemberById(memberId);
        if (member) {
            cout << "\nMember Found:" << endl;
            member->displayInfo();
        } else {
            cout << "Member not found." << endl;
        }
    }
    
    void displayAllMembers() {
        cout << "\n=== All Members ===" << endl;
        
        // Similar to displayAllBooks, we'd need a method to get all members
        cout << "Feature not fully implemented." << endl;
    }
    
    void updateMemberMenu() {
        int memberId;
        cout << "Enter Member ID to update: ";
        cin >> memberId;
        cin.ignore();
        
        auto member = library.findMemberById(memberId);
        if (!member) {
            cout << "Member not found." << endl;
            return;
        }
        
        cout << "\nUpdating Member: " << member->getName() << endl;
        cout << "1. Update Address" << endl;
        cout << "2. Update Phone" << endl;
        cout << "3. Update Email" << endl;
        cout << "Enter choice: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: {
                string address;
                cout << "Enter new address: ";
                getline(cin, address);
                member->setAddress(address);
                cout << "Address updated." << endl;
                break;
            }
            case 2: {
                string phone;
                cout << "Enter new phone: ";
                getline(cin, phone);
                member->setPhone(phone);
                cout << "Phone updated." << endl;
                break;
            }
            case 3: {
                string email;
                cout << "Enter new email: ";
                getline(cin, email);
                member->setEmail(email);
                cout << "Email updated." << endl;
                break;
            }
            default:
                cout << "Invalid choice." << endl;
        }
    }
    
    void suspendMemberMenu() {
        int memberId;
        cout << "Enter Member ID to suspend: ";
        cin >> memberId;
        cin.ignore();
        
        auto member = library.findMemberById(memberId);
        if (member) {
            member->suspend();
            cout << "Member suspended." << endl;
        } else {
            cout << "Member not found." << endl;
        }
    }
    
    void activateMemberMenu() {
        int memberId;
        cout << "Enter Member ID to activate: ";
        cin >> memberId;
        cin.ignore();
        
        auto member = library.findMemberById(memberId);
        if (member) {
            member->activate();
            cout << "Member activated." << endl;
        } else {
            cout << "Member not found." << endl;
        }
    }
    
    void borrowReturnOperations() {
        int choice;
        cout << "\n=== Borrow/Return Operations ===" << endl;
        cout << "1. Borrow Book" << endl;
        cout << "2. Return Book" << endl;
        cout << "3. Reserve Book" << endl;
        cout << "4. View Member's Borrowed Books" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1:
                borrowBookMenu();
                break;
            case 2:
                returnBookMenu();
                break;
            case 3:
                reserveBookMenu();
                break;
            case 4:
                viewMemberBorrowedBooks();
                break;
            default:
                cout << "Invalid choice." << endl;
        }
    }
    
    void borrowBookMenu() {
        int memberId;
        string isbn;
        
        cout << "Enter Member ID: ";
        cin >> memberId;
        cin.ignore();
        cout << "Enter Book ISBN: ";
        getline(cin, isbn);
        
        try {
            if (library.borrowBook(memberId, isbn)) {
                cout << "Book borrowed successfully!" << endl;
            } else {
                cout << "Unable to borrow book." << endl;
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
    void returnBookMenu() {
        int memberId;
        string isbn;
        
        cout << "Enter Member ID: ";
        cin >> memberId;
        cin.ignore();
        cout << "Enter Book ISBN: ";
        getline(cin, isbn);
        
        try {
            if (library.returnBook(memberId, isbn)) {
                cout << "Book returned successfully!" << endl;
            } else {
                cout << "Unable to return book." << endl;
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
    void reserveBookMenu() {
        int memberId;
        string isbn;
        
        cout << "Enter Member ID: ";
        cin >> memberId;
        cin.ignore();
        cout << "Enter Book ISBN: ";
        getline(cin, isbn);
        
        try {
            library.reserveBook(memberId, isbn);
            cout << "Book reserved successfully!" << endl;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
    
    void viewMemberBorrowedBooks() {
        int memberId;
        cout << "Enter Member ID: ";
        cin >> memberId;
        cin.ignore();
        
        auto member = library.findMemberById(memberId);
        if (!member) {
            cout << "Member not found." << endl;
            return;
        }
        
        auto books = member->getBorrowedBooks();
        cout << "\nBooks borrowed by " << member->getName() << ":" << endl;
        
        if (books.empty()) {
            cout << "No books currently borrowed." << endl;
        } else {
            for (const auto& isbn : books) {
                auto book = library.findBookByISBN(isbn);
                if (book) {
                    cout << "  - " << book->getTitle() << " (ISBN: " << isbn << ")" << endl;
                }
            }
        }
    }
    
    void searchBooksMenu() {
        int choice;
        cout << "\n=== Search Books ===" << endl;
        cout << "1. Search by Title" << endl;
        cout << "2. Search by Author" << endl;
        cout << "3. Search by ISBN" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore();
        
        string query;
        cout << "Enter search query: ";
        getline(cin, query);
        
        vector<shared_ptr<Book>> results;
        
        switch (choice) {
            case 1:
                library.setSearchStrategy(make_unique<TitleSearch>());
                break;
            case 2:
                library.setSearchStrategy(make_unique<AuthorSearch>());
                break;
            case 3:
                library.setSearchStrategy(make_unique<ISBNSearch>());
                break;
            default:
                cout << "Invalid choice." << endl;
                return;
        }
        
        results = library.searchBooks(query);
        
        cout << "\nSearch Results:" << endl;
        if (results.empty()) {
            cout << "No books found." << endl;
        } else {
            for (const auto& book : results) {
                book->displayInfo();
                cout << string(30, '-') << endl;
            }
            cout << "Found " << results.size() << " book(s)." << endl;
        }
    }
};

// Main function
int main() {
    try {
        cout << "Starting Library Management System..." << endl;
        
        LibraryConsole console;
        console.run();
        
        return 0;
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
}