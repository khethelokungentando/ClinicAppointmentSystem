# Community Clinic Appointment Management System

## 1. Project Overview

The Community Clinic Appointment Management System is an academic software prototype designed to simplify appointment scheduling and management in a community healthcare environment.

The system provides role-specific dashboards for patients, doctors, nurses and administrators. It allows patients to request appointments, doctors to review appointment requests, and patients to receive in-app notifications about appointment decisions.

This project demonstrates the integration of a web-based user interface, a C++ backend and an SQLite database.

## 2. Project Objectives

- Develop a digital appointment management prototype.
- Allow patients to register and request clinic appointments.
- Provide separate dashboards for different clinic roles.
- Allow doctors to approve or cancel appointment requests.
- Store appointment information using SQLite.
- Display appointment status updates and in-app notifications.
- Demonstrate integration between frontend, backend and database components.

## 3. Technologies Used

| Technology | Purpose |
|---|---|
| C++ | Backend server and application logic |
| SQLite | Relational database |
| HTML5 | Web page structure |
| CSS3 | User interface styling |
| JavaScript | Frontend interaction |
| Git and GitHub | Version control and source code management |
| Visual Studio Code | Development environment |
| MSYS2 UCRT64 | C++ compilation environment on Windows |

## 4. System Users

### Patient
Patients can register, sign in, request appointments, view current and previous bookings, and receive in-app appointment notifications.

### Doctor
Doctors can view appointment requests and approve or cancel pending bookings.

### Nurse
Nurses have access to an appointment-related dashboard and a patient check-in workflow. Some advanced nursing features remain under development.

### Administrator
The system includes an administrator dashboard for administrative access. Additional management features may require further development.

## 5. System Architecture

The application follows a three-layer architecture.

**Presentation Layer:** HTML, CSS and JavaScript provide the interface used by patients and clinic staff.

**Application Layer:** A C++ server processes application requests and manages the appointment workflow.

**Data Layer:** SQLite stores the data used by the application.

The general communication flow is:

`Web Browser → C++ Backend → SQLite Database`

The backend returns information to the frontend, where it is displayed to the user.

## 6. Project Structure

```text
ClinicAppointmentSystem/
├── app/
│   ├── public/
│   │   ├── assets/
│   │   │   └── clinic.svg
│   │   ├── app.js
│   │   ├── index.html
│   │   └── style.css
│   ├── server.cpp
│   └── Launch-Clinic.bat
├── backend/          # Earlier development files
├── frontend/         # Earlier development files
├── docs/             # Project documentation
├── .gitignore
└── README.md
```

The current working application is located in the `app` directory.

## 7. Installation and Execution

### Requirements

- Windows computer
- MSYS2 UCRT64
- C++ compiler supporting C++17
- SQLite3 development libraries
- OpenSSL development libraries
- Web browser

### Compile the Backend

Open MSYS2 UCRT64 and navigate to the `app` directory.

Compile the C++ server:

```bash
g++ -std=c++17 -O2 server.cpp -o clinic.exe -lsqlite3 -lcrypto -lws2_32
```

If compilation is successful, the `clinic.exe` executable will be created.

### Run the Application

Run:

```bash
./clinic.exe
```

Open the following address in your browser:

http://127.0.0.1:8765

On a properly configured Windows computer, the `Launch-Clinic.bat` file can also be used to start the compiled application.

The application initializes its local SQLite database when required.

## 8. Appointment Workflow

1. A patient registers and signs in.
2. The patient submits an appointment request.
3. The appointment is recorded with a pending status.
4. The doctor reviews the appointment request.
5. The doctor approves or cancels the booking.
6. The patient dashboard displays the updated appointment status.
7. An in-app notification informs the patient of the appointment decision when supported by the implemented workflow.

## 9. Testing Strategy

The project uses functional, integration and end-to-end testing approaches.

Testing activities include:

- Checking application startup.
- Verifying login and dashboard navigation.
- Submitting appointment requests.
- Reviewing appointments through the doctor dashboard.
- Checking appointment status updates.
- Checking in-app notifications.

The appointment approval and patient notification workflow has been demonstrated in the working prototype.

Further testing is required to establish comprehensive reliability, performance and security results.

## 10. Known Limitations

- The prototype currently runs locally.
- Some advanced nursing and administrative features are incomplete.
- SMS delivery is not included in the current demonstration version.
- In-app notifications are used for appointment updates.
- Additional authentication, authorization, privacy and security testing would be required for real clinical deployment.
- The system has not been validated for use with real patient records.

## 11. Future Improvements

Potential improvements include:

- Online deployment.
- Appointment rescheduling.
- Additional cancellation and reminder notifications.
- More complete nurse and administrator functionality.
- Automated software testing.
- Enhanced security and database backup procedures.
- Accessibility and usability improvements.

## 12. Academic Purpose

This project was developed as a group academic prototype to demonstrate software engineering principles, system integration, database design and software testing.

**Disclaimer:** This system is intended for educational demonstration only. It should not be used to store or process real patient medical information without appropriate security, privacy, compliance and operational safeguards.

## 13. Project Team

Developed by the Community Clinic Appointment Management System project group.

University of KwaZulu-Natal (UKZN)

Group members: Add the names of all group members here.