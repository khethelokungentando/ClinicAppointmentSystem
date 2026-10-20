#include <iostream>
#include <string>
#include <memory>
#include "httplib.h"
#include "json.hpp"
#include "sqlite3.h" 

using json = nlohmann::json;

void initDatabase() {
    sqlite3* db;
    int rc = sqlite3_open("clinic.db", &db);
    
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql = 
        "CREATE TABLE IF NOT EXISTS Patients ("
        "ID TEXT PRIMARY KEY, "
        "FirstName TEXT NOT NULL);"
        
        "CREATE TABLE IF NOT EXISTS Appointments ("
        "ApptID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "PatientID TEXT, "
        "Status TEXT, "
        "Date TEXT, "
        "Time TEXT, "
        "Type TEXT, "
        "Doctor TEXT, "
        "Room TEXT, "
        "FOREIGN KEY(PatientID) REFERENCES Patients(ID));"

        "INSERT OR IGNORE INTO Patients (ID, FirstName) VALUES ('224057231', 'Khethelo');"
        "INSERT OR IGNORE INTO Appointments (PatientID, Status, Date, Time, Type, Doctor, Room) "
        "VALUES ('224057231', 'Confirmed', 'Tue, 22 Sep', '10:30 - 11:00', 'General medicine', 'Dr. Mashazi', 'Room 3');";

    char* errMsg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "SUCCESS: SQLite Database and Tables initialized!" << std::endl;
    }
    sqlite3_close(db);
}

int main() {
    initDatabase(); 

    httplib::Server svr;
    svr.set_mount_point("/", "../frontend");

    // Real SQLite Database Fetch
    svr.Get(R"(/api/patient/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string studentId = req.matches[1];
        json dashboardData;
        
        sqlite3* db;
        if (sqlite3_open("clinic.db", &db) != SQLITE_OK) {
            res.status = 500;
            res.set_content("Database connection failed", "text/plain");
            return;
        }

        std::string sql = "SELECT Patients.FirstName, Appointments.Status, Appointments.Date, "
                          "Appointments.Time, Appointments.Type, Appointments.Doctor, Appointments.Room "
                          "FROM Patients JOIN Appointments ON Patients.ID = Appointments.PatientID "
                          "WHERE Patients.ID = '" + studentId + "' LIMIT 1;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                dashboardData["patientId"] = studentId;
                dashboardData["firstName"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                dashboardData["appointment"]["status"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                dashboardData["appointment"]["date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                dashboardData["appointment"]["time"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                dashboardData["appointment"]["type"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                dashboardData["appointment"]["doctor"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                dashboardData["appointment"]["room"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            } else {
                dashboardData["error"] = "Patient ID not found in database.";
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);

        res.set_content(dashboardData.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    std::cout << "Starting Clinic Server on port 8080..." << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}