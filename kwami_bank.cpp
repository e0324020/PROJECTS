#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <ctime>
#include <cstring>
#include <cstdlib>
using namespace std;


#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    int _getch() {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
    #define getch _getch
#endif

struct TransactionRecord {
    unsigned long long accNo;
    char type[20];
    int amount;
    int updatedBalance;
    char timestamp[30];
};

class Bank {
private:
    unsigned long long account_number;
    char holder_name[51];
    int deposit;
    char type;
    char dream[100];
    int dreamCost;
    int pin;

public:
    Bank() {
        account_number = 0;
        memset(holder_name, 0, sizeof(holder_name));
        deposit = 0;
        type = 'S';
        memset(dream, 0, sizeof(dream));
        dreamCost = 0;
        pin = 0;
    }

    void Get_Data();
    void Write_Data();
    void Show_account() const;
    void Show_account_customer() const;
    void showDreamProgress() const;

    void dep(int x) { deposit += x; }
    void draw(int x) { deposit -= x; }

    unsigned long long retacno()  const { return account_number; }
    int                retdeposit() const { return deposit; }
    char               rettype()  const { return type; }
    int                retpin()   const { return pin; }
    string             retname()  const { return holder_name; }
    string             retDream() const { return dream; }
    int                retDreamCost() const { return dreamCost; }
    int dreamProgress() const {
        return dreamCost > 0 ? (deposit * 100) / dreamCost : 0;
    }
    void setPin(int newPin) { pin = newPin; }
};

void Record_Transaction(unsigned long long acc, const char* type, int amt, int bal);
bool AdminLogin();
bool CustomerLogin(unsigned long long &loggedInAccNo);
void Deposit_Withdraw(unsigned long long n, int option);
void Transfer_Funds();
void Display_All_Accounts();
void Search_Account();
void Delete_Account();
void Display_Transaction_History();
void Generate_Web_Dashboard();
void CustomerMenu(unsigned long long accNo);
void Customer_Deposit(unsigned long long accNo);
void Customer_Withdraw(unsigned long long accNo);
void Customer_Transaction_History(unsigned long long accNo);


void Bank::showDreamProgress() const {
    if (dreamCost > 0) {
        int progress = dreamProgress();
        cout << "\n Dream Goal    : " << dream;
        cout << "\n Estimated Cost: $" << dreamCost;
        cout << "\n Progress      : " << progress << "%";
        if (progress >= 100)
            cout << "\n Congratulations! You've achieved your dream goal!";
        else
            cout << "\n Keep saving! You're " << (100 - progress) << "% away.";
    }
}

void Bank::Get_Data() {
    cout << "\n\n=========== CREATE KWAMI BANK ACCOUNT ===========";

   
    cout << "\nEnter 12-digit Account Number: ";
    cin >> account_number;
    while (account_number < 100000000000ULL || account_number > 999999999999ULL) {
        cout << "Invalid! Must be exactly 12 digits: ";
        cin >> account_number;
    }

    cout << "Enter Account Holder Name: ";
    cin.ignore();
    cin.getline(holder_name, 51);

  
    cout << "Enter Type (S for Saving / C for Current): ";
    cin >> type;
    type = toupper(type);
    while (type != 'S' && type != 'C') {
        cout << "Invalid! Enter S or C: ";
        cin >> type;
        type = toupper(type);
    }

  
    cout << "Enter Initial Deposit (Min $500): ";
    cin >> deposit;
    while (deposit < 500) {
        cout << "Minimum deposit is $500. Enter again: ";
        cin >> deposit;
    }

    cout << "Set 4-digit Customer PIN: ";
    string pinStr = "";
    char ch;
    while ((ch = _getch()) != 13) {
        if (ch == 8) {
            if (!pinStr.empty()) { pinStr.pop_back(); cout << "\b \b"; }
        } else if (pinStr.size() < 4 && ch >= '0' && ch <= '9') {
            pinStr += ch;
            cout << '*';
        }
    }
    cout << endl;
    pin = stoi(pinStr.empty() ? "0" : pinStr);
    while (pin < 1000 || pin > 9999) {
        cout << "PIN must be exactly 4 digits. Set again: ";
        pinStr = "";
        while ((ch = _getch()) != 13) {
            if (ch == 8) {
                if (!pinStr.empty()) { pinStr.pop_back(); cout << "\b \b"; }
            } else if (pinStr.size() < 4 && ch >= '0' && ch <= '9') {
                pinStr += ch;
                cout << '*';
            }
        }
        cout << endl;
        pin = stoi(pinStr.empty() ? "0" : pinStr);
    }

    cout << "Enter your Dream Goal (e.g., Trip to Paris): ";
    cin.ignore();
    cin.getline(dream, 100);

    cout << "Estimated Cost for this Dream ($): ";
    cin >> dreamCost;
    while (dreamCost <= 0) {
        cout << "Cost must be positive. Enter again: ";
        cin >> dreamCost;
    }

    cout << "\n Account Created Successfully!";
    cout << "\n Please remember your PIN for customer login.";

    Record_Transaction(account_number, "Account Open", deposit, deposit);
    showDreamProgress();
}

void Bank::Write_Data() {
    ofstream outFile("account.dat", ios::binary | ios::app);
    Get_Data();
    outFile.write((char*)this, sizeof(*this));
    outFile.close();
}

void Bank::Show_account() const {
    cout << "\n Acc No  : " << account_number;
    cout << "\n Name    : " << holder_name;
    cout << "\n Type    : " << (type == 'S' ? "Saving" : "Current");
    cout << "\n Balance : $" << deposit;
    cout << "\n PIN     : " << pin;   // Admin view — PIN visible
    showDreamProgress();
}

void Bank::Show_account_customer() const {
    cout << "\n Acc No  : " << account_number;
    cout << "\n Name    : " << holder_name;
    cout << "\n Type    : " << (type == 'S' ? "Saving" : "Current");
    cout << "\n Balance : $" << deposit;
    showDreamProgress();
}

void Record_Transaction(unsigned long long acc, const char* type, int amt, int bal) {
    TransactionRecord tr;
    tr.accNo = acc;
    strcpy(tr.type, type);
    tr.amount = amt;
    tr.updatedBalance = bal;
    time_t now = time(0);
    strftime(tr.timestamp, 30, "%Y-%m-%d %H:%M", localtime(&now));
    ofstream trxFile("transactions.dat", ios::binary | ios::app);
    trxFile.write((char*)&tr, sizeof(TransactionRecord));
    trxFile.close();
}

bool AdminLogin() {
    string pass = "";
    char ch;
    cout << "\n\n=========== KWAMI BANK ADMIN LOGIN ===========";
    cout << "\nEnter Admin Password: ";
    while ((ch = _getch()) != 13) {
        if (ch == 8) {
            if (!pass.empty()) { pass.pop_back(); cout << "\b \b"; }
        } else {
            pass += ch;
            cout << '*';
        }
    }
    cout << endl;
    return pass == "admin123";
}

bool CustomerLogin(unsigned long long &loggedInAccNo) {
    unsigned long long accNo;
    int enteredPin;
    Bank obj;
    bool found = false;

    cout << "\n\n=========== KWAMI BANK CUSTOMER LOGIN ===========";
    cout << "\nEnter Account Number: ";
    cin >> accNo;
    cout << "Enter 4-digit PIN: ";
    cin >> enteredPin;

    ifstream inFile("account.dat", ios::binary);
    if (!inFile) {
        cout << "\n No account records found!";
        return false;
    }
    while (inFile.read((char*)&obj, sizeof(Bank))) {
        if (obj.retacno() == accNo && obj.retpin() == enteredPin) {
            found = true;
            loggedInAccNo = accNo;
            cout << "\n Login Successful!";
            cout << "\n Welcome, " << obj.retname() << "!";
            break;
        }
    }
    inFile.close();
    if (!found) cout << "\n Invalid Account Number or PIN!";
    return found;
}

void Customer_Deposit(unsigned long long accNo) {
    int amt;
    Bank obj;
    bool found = false;

    fstream File("account.dat", ios::binary | ios::in | ios::out);
    if (!File) { cout << "\n System error!"; return; }

    while (File.read((char*)&obj, sizeof(Bank))) {
        if (obj.retacno() == accNo) {
            cout << "\n=========== DEPOSIT MONEY ===========";
            cout << "\n Current Balance: $" << obj.retdeposit();
            cout << "\n Enter Deposit Amount: $";
            cin >> amt;
            while (amt <= 0) {
                cout << " Amount must be positive! Enter again: $";
                cin >> amt;
            }
            obj.dep(amt);
            Record_Transaction(accNo, "Deposit", amt, obj.retdeposit());

            int pos = (-1) * (int)sizeof(Bank);
            File.seekp(pos, ios::cur);
            File.write((char*)&obj, sizeof(Bank));

            cout << "\n Deposited : $" << amt;
            cout << "\n New Balance: $" << obj.retdeposit();
            obj.showDreamProgress();
            found = true;
            break;
        }
    }
    File.close();
    if (!found) cout << "\n Account error!";
}

void Customer_Withdraw(unsigned long long accNo) {
    int amt;
    Bank obj;
    bool found = false;

    fstream File("account.dat", ios::binary | ios::in | ios::out);
    if (!File) { cout << "\n System error!"; return; }

    while (File.read((char*)&obj, sizeof(Bank))) {
        if (obj.retacno() == accNo) {
            found = true;
            cout << "\n=========== WITHDRAW MONEY ===========";
            cout << "\n Current Balance: $" << obj.retdeposit();
            cout << "\n Enter Withdrawal Amount: $";
            cin >> amt;
            while (amt <= 0) {
                cout << " Amount must be positive! Enter again: $";
                cin >> amt;
            }
            if (amt > obj.retdeposit()) {
              
                cout << "\n Insufficient Balance!";
                cout << "\n Available Balance: $" << obj.retdeposit();
                break;
            }
            obj.draw(amt);
            Record_Transaction(accNo, "Withdraw", amt, obj.retdeposit());

            int pos = (-1) * (int)sizeof(Bank);
            File.seekp(pos, ios::cur);
            File.write((char*)&obj, sizeof(Bank));

            cout << "\n Withdrawn  : $" << amt;
            cout << "\n New Balance: $" << obj.retdeposit();
            obj.showDreamProgress();
            break;
        }
    }
    File.close();   
    if (!found) cout << "\n Account error!";
}

void Customer_Transaction_History(unsigned long long accNo) {
    TransactionRecord tr;
    bool found = false;
    ifstream trxFile("transactions.dat", ios::binary);

    cout << "\n=========== YOUR TRANSACTION HISTORY ===========";
    cout << "\n Account: " << accNo;
    cout << "\n" << string(70, '=');
    cout << "\nType            Amount      Balance     Timestamp";
    cout << "\n" << string(70, '=') << "\n";

    while (trxFile.read((char*)&tr, sizeof(TransactionRecord))) {
        if (tr.accNo == accNo) {
            cout << left << setw(16) << tr.type
                 << "$" << setw(12) << tr.amount
                 << "$" << setw(12) << tr.updatedBalance
                 << tr.timestamp << "\n";
            found = true;
        }
    }
    trxFile.close();
    if (!found) cout << "\n No transactions found!";
}

void CustomerMenu(unsigned long long accNo) {
    Bank obj;
    char choice;

    do {
        system("cls");
        cout << "\n\n\t\t+================================================+";
        cout << "\n\t\t|      KWAMI BANK - CUSTOMER PORTAL              |";
        cout << "\n\t\t+================================================+";
        cout << "\n\t\t|                                                |";
        cout << "\n\t\t|   1. View My Account Details                   |";
        cout << "\n\t\t|   2. Deposit Money                             |";
        cout << "\n\t\t|   3. Withdraw Money                            |";
        cout << "\n\t\t|   4. View My Transaction History               |";
        cout << "\n\t\t|   5. View My Dream Goal Progress               |";
        cout << "\n\t\t|   0. Logout                                    |";
        cout << "\n\t\t|                                                |";
        cout << "\n\t\t+================================================+";
        cout << "\n\t\t   Enter Your Choice: ";
        cin >> choice;

        switch (choice) {
            case '1': {
                ifstream inFile("account.dat", ios::binary);
                bool found = false;
                while (inFile.read((char*)&obj, sizeof(Bank))) {
                    if (obj.retacno() == accNo) { obj.Show_account_customer(); found = true; break; }
                }
                inFile.close();
                if (!found) cout << "\n Account not found!";
                break;
            }
            case '2': Customer_Deposit(accNo);            break;
            case '3': Customer_Withdraw(accNo);           break;
            case '4': Customer_Transaction_History(accNo); break;
            case '5': {
                ifstream inFile("account.dat", ios::binary);
                bool found = false;
                while (inFile.read((char*)&obj, sizeof(Bank))) {
                    if (obj.retacno() == accNo) {
                        cout << "\n=========== YOUR DREAM GOAL PROGRESS ===========";
                        obj.showDreamProgress();
                        cout << "\n================================================";
                        found = true;
                        break;
                    }
                }
                inFile.close();
                if (!found) cout << "\n Account not found!";
                break;
            }
            case '0':
                cout << "\n Logging out...";
                cout << "\n Thank you for using Kwami Bank!";
                break;
            default:
                cout << "\n Invalid choice! Please try again.";
        }

        if (choice != '0') {
            cout << "\n\nPress any key to continue...";
            _getch();
        }
    } while (choice != '0');
}

void Deposit_Withdraw(unsigned long long n, int option) {
    int amt;
    Bank obj;
    bool found = false;
    fstream File("account.dat", ios::binary | ios::in | ios::out);

    while (File.read((char*)&obj, sizeof(Bank)) && !found) {
        if (obj.retacno() == n) {
            if (option == 1) {
                cout << "\nEnter Deposit Amount: $";
                cin >> amt;
               
                while (amt <= 0) { cout << "Must be positive: $"; cin >> amt; }
                obj.dep(amt);
                Record_Transaction(n, "Deposit", amt, obj.retdeposit());
            } else {
                cout << "\nEnter Withdrawal Amount: $";
                cin >> amt;
                while (amt <= 0) { cout << "Must be positive: $"; cin >> amt; }
                if (amt > obj.retdeposit()) {
                    cout << "\n Insufficient Balance! Available: $" << obj.retdeposit();
                    File.close();
                    return;
                }
                obj.draw(amt);
                Record_Transaction(n, "Withdraw", amt, obj.retdeposit());
            }
            int pos = (-1) * (int)sizeof(Bank);
            File.seekp(pos, ios::cur);
            File.write((char*)&obj, sizeof(Bank));
            cout << "\n Transaction Completed Successfully.";
            obj.showDreamProgress();
            found = true;
        }
    }
    File.close();
    if (!found) cout << "\n Account not found!";
}

void Transfer_Funds() {
    unsigned long long sAcc, rAcc;
    int amt;
    Bank sender, receiver;
    bool sFound = false, rFound = false;
    long sPos = 0, rPos = 0;

    cout << "\nEnter Sender Account Number  : "; cin >> sAcc;
    cout << "Enter Receiver Account Number: "; cin >> rAcc;

    // FIX #4: block self-transfer
    if (sAcc == rAcc) {
        cout << "\n Cannot transfer to the same account!";
        return;
    }

    cout << "Enter Amount to Transfer     : $"; cin >> amt;
    while (amt <= 0) { cout << "Must be positive: $"; cin >> amt; }

    fstream File("account.dat", ios::binary | ios::in | ios::out);
    if (!File) { cout << "\n System error!"; return; }

    while (File.read((char*)&sender, sizeof(Bank))) {
        if (sender.retacno() == sAcc) { sPos = (long)File.tellg() - sizeof(Bank); sFound = true; }
        if (sender.retacno() == rAcc) { rPos = (long)File.tellg() - sizeof(Bank); rFound = true; }
    }

    if (sFound && rFound) {
        File.seekg(sPos, ios::beg);
        File.read((char*)&sender, sizeof(Bank));
        if (sender.retdeposit() < amt) {
            cout << "\n Sender has insufficient funds! Available: $" << sender.retdeposit();
        } else {
            sender.draw(amt);
            File.seekp(sPos, ios::beg);
            File.write((char*)&sender, sizeof(Bank));

            File.seekg(rPos, ios::beg);
            File.read((char*)&receiver, sizeof(Bank));
            receiver.dep(amt);
            File.seekp(rPos, ios::beg);
            File.write((char*)&receiver, sizeof(Bank));

            Record_Transaction(sAcc, "Transfer Out", amt, sender.retdeposit());
            Record_Transaction(rAcc, "Transfer In",  amt, receiver.retdeposit());

            cout << "\n Transfer Successful!";
            cout << "\n Sender's Progress  :"; sender.showDreamProgress();
            cout << "\n Receiver's Progress:"; receiver.showDreamProgress();
        }
    } else {
        if (!sFound) cout << "\n Sender account not found!";
        if (!rFound) cout << "\n Receiver account not found!";
    }
    File.close();
}

void Display_All_Accounts() {
    Bank obj;
    ifstream inFile("account.dat", ios::binary);
    if (!inFile) { cout << "\n No accounts found!"; return; }

    int count = 0;
    cout << "\n=========== ALL ACCOUNTS ===========";
    while (inFile.read((char*)&obj, sizeof(Bank))) {
        cout << "\n Account #" << ++count;
        obj.Show_account();
        cout << "\n" << string(40, '-');
    }
    inFile.close();
    if (count == 0) cout << "\n No accounts to display!";
}

void Search_Account() {
    unsigned long long n;
    Bank obj;
    bool found = false;
    ifstream inFile("account.dat", ios::binary);
    cout << "\nEnter Account Number: "; cin >> n;
    while (inFile.read((char*)&obj, sizeof(Bank))) {
        if (obj.retacno() == n) { obj.Show_account(); found = true; break; }
    }
    inFile.close();
    if (!found) cout << "\n Account not found!";
}

void Delete_Account() {
    unsigned long long n;
    Bank obj;
    // FIX #3: track whether the account was actually found
    bool found = false;

    ifstream inFile("account.dat", ios::binary);
    ofstream outFile("temp.dat", ios::binary);

    cout << "\nEnter Account Number to Delete: "; cin >> n;

    while (inFile.read((char*)&obj, sizeof(Bank))) {
        if (obj.retacno() == n) {
            found = true;
            Record_Transaction(n, "Account Closed", 0, 0);
         
        } else {
            outFile.write((char*)&obj, sizeof(Bank));
        }
    }
    inFile.close();
    outFile.close();

    remove("account.dat");
    rename("temp.dat", "account.dat");

    if (found)
        cout << "\n Account " << n << " deleted successfully!";
    else
        cout << "\n Account not found!";
}

void Display_Transaction_History() {
    TransactionRecord tr;
    bool found = false;
    ifstream inFile("transactions.dat", ios::binary);
    if (!inFile) { cout << "\n No transaction records found!"; return; }

    cout << "\n=========== ALL TRANSACTION HISTORY ===========";
    cout << "\n" << string(80, '=');
    cout << "\nAcc No          Type            Amount      Balance     Timestamp";
    cout << "\n" << string(80, '=') << "\n";

    while (inFile.read((char*)&tr, sizeof(TransactionRecord))) {
        cout << left
             << setw(16) << tr.accNo
             << setw(16) << tr.type
             << "$" << setw(12) << tr.amount
             << "$" << setw(12) << tr.updatedBalance
             << tr.timestamp << "\n";
        found = true;
    }
    inFile.close();
    if (!found) cout << "\n No transactions found!";
}

void Generate_Web_Dashboard() {
    Bank obj;
    TransactionRecord tr;
    ifstream inFile("account.dat", ios::binary);
    ifstream trxFile("transactions.dat", ios::binary);

    if (!inFile) {
        cout << "\n No account data found! Please create accounts first.";
        return;
    }


    double totalBalance = 0;
    int accountCount = 0;
    while (inFile.read((char*)&obj, sizeof(Bank))) {
        totalBalance += obj.retdeposit();
        accountCount++;
    }

    ofstream html("dashboard.html");

    html << "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>"
         << "<title>Kwami Bank Dashboard</title><style>"
         << "body{font-family:'Segoe UI',Arial,sans-serif;background:#f0f2f5;margin:0;padding:20px;}"
         << ".container{max-width:1100px;margin:0 auto;}"
         << ".card{background:#fff;padding:25px;border-radius:12px;"
         <<       "box-shadow:0 4px 15px rgba(0,0,0,0.1);margin-bottom:24px;}"
         << "h1{color:#1a2a4a;margin:0 0 6px 0;}"
         << "h3{color:#333;margin-top:0;}"
         << ".stats{display:flex;gap:20px;flex-wrap:wrap;margin-bottom:8px;}"
         << ".stat{flex:1;min-width:150px;background:#f8f9fb;border-radius:10px;"
         <<       "padding:16px;text-align:center;border-left:4px solid #d35400;}"
         << ".stat-num{font-size:26px;font-weight:bold;color:#d35400;}"
         << "table{width:100%;border-collapse:collapse;margin-top:12px;}"
         << "th{background:#1a2a4a;color:#fff;padding:11px;font-size:13px;}"
         << "td{padding:9px;border-bottom:1px solid #eee;font-size:13px;text-align:center;}"
         << "tr:hover td{background:#fdf6ec;}"
         << ".bar-wrap{background:#e0e0e0;border-radius:8px;overflow:hidden;}"
         << ".bar-fill{background:#d35400;color:#fff;font-size:11px;text-align:center;"
         <<           "border-radius:8px;min-width:20px;}"
         << ".dep{color:green;font-weight:bold;}"
         << ".out{color:#c0392b;font-weight:bold;}"
         << ".footer{text-align:center;background:#1a2a4a;color:#aaa;border-radius:10px;padding:14px;}"
         << "</style></head><body><div class='container'>"

        
         << "<div class='card'><h1>Kwami Bank — Admin Dashboard</h1>"
         << "<p style='color:#666;'>Generated: " << __DATE__ << " at " << __TIME__ << "</p>"
         << "<div class='stats'>"
         << "<div class='stat'><div class='stat-num'>" << accountCount << "</div>Total Accounts</div>"
         << "<div class='stat'><div class='stat-num'>$" << (int)totalBalance << "</div>Total Deposits</div>"
         << "<div class='stat'><div class='stat-num'>$"
         << (accountCount > 0 ? (int)(totalBalance / accountCount) : 0)
         << "</div>Average Balance</div>"
         << "</div></div>";

    html << "<div class='card'><h3>Active Accounts</h3><table>"
         << "<tr><th>Acc No</th><th>Name</th><th>Type</th>"
         << "<th>Balance</th><th>Dream Goal</th><th>Cost</th><th>Progress</th></tr>";

    inFile.clear();
    inFile.seekg(0, ios::beg);
    while (inFile.read((char*)&obj, sizeof(Bank))) {
        int p = obj.dreamProgress();
        html << "<tr>"
             << "<td>" << obj.retacno() << "</td>"
             << "<td>" << obj.retname() << "</td>"
             << "<td>" << (obj.rettype() == 'S' ? "Saving" : "Current") << "</td>"
             << "<td>$" << obj.retdeposit() << "</td>"
             << "<td>" << obj.retDream() << "</td>"
             << "<td>$" << obj.retDreamCost() << "</td>"
             << "<td><div class='bar-wrap'><div class='bar-fill' style='width:"
             << (p > 100 ? 100 : p) << "%'>" << p << "%</div></div></td>"
             << "</tr>";
    }
    inFile.close();
    html << "</table></div>";

    
    html << "<div class='card'><h3>Transaction History</h3><table>"
         << "<tr><th>Acc No</th><th>Type</th><th>Amount</th><th>Balance After</th><th>Timestamp</th></tr>";

    int txCount = 0;
    while (trxFile.read((char*)&tr, sizeof(TransactionRecord))) {
        bool isOut = (strcmp(tr.type, "Withdraw") == 0 || strcmp(tr.type, "Transfer Out") == 0);
        html << "<tr>"
             << "<td>" << tr.accNo << "</td>"
             << "<td>" << tr.type << "</td>"
             << "<td class='" << (isOut ? "out" : "dep") << "'>$" << tr.amount << "</td>"
             << "<td>$" << tr.updatedBalance << "</td>"
             << "<td>" << tr.timestamp << "</td>"
             << "</tr>";
        txCount++;
    }
    if (txCount == 0)
        html << "<tr><td colspan='5'>No transactions found.</td></tr>";

    trxFile.close();
    html << "</table></div>"
         << "<div class='footer'>Kwami Bank System &mdash; Secure Banking</div>"
         << "</div></body></html>";
    html.close();

    cout << "\n Web Dashboard generated as dashboard.html";
    cout << "\n Opening in browser...";
    system("start dashboard.html");
}


int main() {
    char userType;
    unsigned long long loggedInAccNo = 0;

    system("cls");
    cout << "\n\t\t==========================================";
    cout << "\n\t\t|                                        |";
    cout << "\n\t\t|         KWAMI BANK SYSTEM              |";
    cout << "\n\t\t|    A Complete C++ Banking Solution     |";
    cout << "\n\t\t|                                        |";
    cout << "\n\t\t==========================================\n";

    cout << "\n\t\t  Select Login Type:";
    cout << "\n\t\t  1. Admin Login";
    cout << "\n\t\t  2. Customer Login";
    cout << "\n\t\t  0. Exit";
    cout << "\n\t\t  Enter Choice: ";
    cin >> userType;

    if (userType == '0') {
        cout << "\n\t\tThank you! Goodbye!\n";
        return 0;
    }
    else if (userType == '1') {
        if (!AdminLogin()) {
            cout << "\n Invalid Password! Exiting...";
            _getch();
            return 0;
        }
        cout << "\n Admin Login Successful! Press any key to continue...";
        _getch();

        char choice;
        do {
            system("cls");
            cout << "\n\n\t\t+================================================+";
            cout << "\n\t\t|        KWAMI BANK - ADMIN PORTAL               |";
            cout << "\n\t\t+================================================+";
            cout << "\n\t\t|                                                |";
            cout << "\n\t\t|   1. Create New Account                        |";
            cout << "\n\t\t|   2. Deposit Amount (Any Account)              |";
            cout << "\n\t\t|   3. Withdraw Amount (Any Account)             |";
            cout << "\n\t\t|   4. Transfer Funds                            |";
            cout << "\n\t\t|   5. Display All Accounts                      |";
            cout << "\n\t\t|   6. Search Account                            |";
            cout << "\n\t\t|   7. Delete Account                            |";
            cout << "\n\t\t|   8. Transaction History (All)                 |";
            cout << "\n\t\t|   9. Generate Web Dashboard                    |";
            cout << "\n\t\t|   0. Logout                                    |";
            cout << "\n\t\t|                                                |";
            cout << "\n\t\t+================================================+";
            cout << "\n\t\t  Enter Your Choice: ";
            cin >> choice;

            switch (choice) {
                case '1': { Bank obj; obj.Write_Data(); _getch(); break; }
                case '2': {
                    unsigned long long acc;
                    cout << "\nEnter Account Number: "; cin >> acc;
                    Deposit_Withdraw(acc, 1); _getch(); break;
                }
                case '3': {
                    unsigned long long acc;
                    cout << "\nEnter Account Number: "; cin >> acc;
                    Deposit_Withdraw(acc, 2); _getch(); break;
                }
                case '4': { Transfer_Funds(); _getch(); break; }
                case '5': { Display_All_Accounts(); _getch(); break; }
                case '6': { Search_Account(); _getch(); break; }
                case '7': { Delete_Account(); _getch(); break; }
                case '8': { Display_Transaction_History(); _getch(); break; }
                case '9': { Generate_Web_Dashboard(); _getch(); break; }
                case '0': { cout << "\n Logging out from Admin Portal..."; break; }
                default:  { cout << "\n Invalid choice!"; _getch(); }
            }
        } while (choice != '0');
    }
    else if (userType == '2') {
        if (CustomerLogin(loggedInAccNo)) {
            cout << "\n Press any key to continue...";
            _getch();
            CustomerMenu(loggedInAccNo);
        } else {
            cout << "\n Press any key to exit...";
            _getch();
        }
    }
    else {
        cout << "\n Invalid choice!";
        _getch();
    }

    return 0;
}
