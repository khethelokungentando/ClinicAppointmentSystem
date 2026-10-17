This project is for a community clinic appointment system used to help community clinics manage patient appointments more efficiently. The system allows patients to book appointments with healthcare providers while also allowing clinic staff/admin to manage appointments, patients, and available slots reducing double booking and long wait periods.

Installation and Setup guide: The system is developed and tested using Windows.

Step 1: Install and/or download the following software on your computer Git from the official Git website Visual Studio Code with C++ and CMake Tools extensions C++ compiler suitable for your operating system SQLite and ensure SQLite command-line tools are available

Step 2: Clone the repository Use command-line: https://github.com/khethelokungentando/ClinicAppointmentSystem.git Navigate into the project directory

Step 3: Open project in Visual Studio Code Select File then Open Folder Select the project folder

Step 4: Set up the database Navigate to the database directory : cd database Initialize database using SQL schema : .read database.sql

Step 5: Configure C++ backend Navigate to the backend directory : cd backend Ensure C++ source files and SQLite libraries are correctly configured Run the backend After successfully compiling backend, start the backend application using the generated executable Run the frontend Navigate to frontend directory and open main file using web browser

Step 6: Verify installation After completing setup, ensure that the frontend loads correctly C++ backend starts without errors SQLite database can be accessed Frontend can communicate with backend The main system functionality can be accessed

Troubleshooting Git command not recognised - ensure Git is installed and added to the system path Backend fails to compile - ensure a supported C++ compiler is installed - all required source and header files are present - the required SQLite development files/libraries are available Database cannot be opened - check that database file exists or can be created - ensure the application is using the correct database path

The repository's .gitignore file is configured to prevent common generated, temporary and local files from being commited. The .gitattributes file provides consistent handling of text and binary files across different development environments
