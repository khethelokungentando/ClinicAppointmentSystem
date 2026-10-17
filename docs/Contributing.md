-Contributing to the community clinic appointment booking system
First off, thank you for considering contributing to our project 
This document outlines the coding standards, development workflow, and review processes that our team follows to ensure the system remains stable, secure, and maintainable

1. Development Workflow
We use a lightweight GitFlow branching model. You must not push directly to main.
- Step 1: Create a Branch: Always branch off main for your work.
-Features: git checkout -b feature/your-feature-name
-Bug Fixes: git checkout -b bugfix/BUG-ID
-Documentation: git checkout -b docs/topic-name
- Step 2: Make Changes: Write your code and test it locally.
- Step 3: Commit: Use the Conventional Commits format.
- Step 4: Push & Pull Request: Push your branch to GitHub and open a Pull Request against main.
- Step 5: Code Review: At least one other team member must review and approve your PR before it can be merged.
- Step 6: Merge: Once approved and all automated CI checks pass, squash and merge your PR. Delete the feature branch.

2. Coding Standards
To maintain consistency across the team, all code must adhere to the following standards.
- Naming Conventions:
-Classes/Structs: PascalCase (e.g. BookingService)
-Functions/Methods: camelCase (e.g. createBooking())
-Variables: camelCase (e.g. patientId)
-Private Member Variables: camelCase_ with a trailing underscore (e.g., databaseConnection_)
-Constants: UPPER_SNAKE_CASE
- UI Files: Layouts should be built using Qt Designer (.ui files) where possible, keeping C++ UI code clean and focused on logic.

3. Commit Message Convention
- Rules:
-Use the imperative mood (e.g. "add feature", not "added feature").
-Keep the description under 50 characters.
-Reference the GitHub Issue number if applicable

4. Pull Request Process
- Before opening a Pull Request, ensure you have completed the following:
-Your code compiles without errors or warnings.
-You have written unit tests for new functionality (using Qt Test or Google Test).
-All existing tests pass
-You have updated the documentation if your change affects setup or user behaviour.
-Your branch is up-to-date with main
- PR Template: When opening a PR, fill out the provided template completely. A reviewer will not approve a PR with an empty description or missing testing evidence.

5. Security & POPIA Compliance
Because this system handles Patient Identifiable Information (PII), security is paramount.
- Never hardcode passwords, API keys, or secrets. Use environment variables or configuration files (which are git-ignored).
- Hashing: All user passwords must be hashed using "bcrypt" or a similar standard algorithm before being stored in the database.
- Logging: Do not log sensitive patient information (names, national IDs) to the console or external files. Sanitise error messages before throwing exceptions.

6. Reporting Bugs
If you find a bug, please do not fix it directly on the main branch. 
- Search the GitHub Issues to ensure it hasn't already been reported.
- If it is a new bug, open an Issue using the Bug Report template.
- Include steps to reproduce, expected behaviour, actual behaviour, and screenshots if applicable.
- Apply the appropriate severity label (severity: critical, severity: high etc.).
