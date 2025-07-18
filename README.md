# ad-passwordfilter
DLL Implementation of Password filter for Active Directory

# 🔒 Secure Password Filter for Windows Domain Controllers

This project implements a custom password filter DLL for Windows Server to enhance password security on domain controllers. It checks passwords against static and dynamic banned word lists, prevents inclusion of usernames, full names, domain names, and more.

## ✨ Features

- Prevents passwords containing:
  - Common weak terms (e.g., `password`, `admin`, `123456`)
  - Custom banned words (from `banned_words.txt`)
  - Reversed banned words
  - Repetitive characters (e.g., `aaaaaa`, `111111`)
  - Username or full name
  - NetBIOS or DNS domain name
- Dynamic configuration via `banned_words.txt`
- Logging of failed password attempts to `PasswordFilter.log`

## 📁 Folder Structure

```
C:\PasswordFilter
│
├── PasswordFilter.dll # Compiled DLL to copy into System32
├── PasswordFilter.log # Auto-generated log file for failed attempts
├── banned_words.txt # Custom list of banned words (editable)
```

## ⚙️ Installation

1. Download the [InstallScript.ps1](https://github.com/guillaumearnx/ad-passwordfilter/releases/latest/download/InstallScript.ps1) and execute it on any domain controler of your forest.²

2. The script will:
   - Download and copy `PasswordFilter.dll` to `C:\Windows\System32`
   - Add the Password Filter to the registry
   - Create a `banned_words.txt` file in `C:\PasswordFilter`
   - Set up the log file `PasswordFilter.log` in the same directory

3. **Important**: Ensure the `banned_words.txt` file is populated with your custom banned words. The script will create an empty file if it doesn't exist.
Exemple content for `banned_words.txt`:
   ```
   password
   admin
   123456
   ```

4. Restart the domain controller to apply the changes.

## 📝 Usage

After installation, the password filter will automatically check new passwords against the criteria defined in the DLL and the `banned_words.txt` file. If a password fails validation, it will be rejected, and an entry will be logged in `PasswordFilter.log`.

From client-side, password change request is rejected with error message :

<img width="459" height="425" alt="image" src="https://github.com/user-attachments/assets/86275696-9d0d-4e8b-9198-fec13fed84b2" />


Example log entry:
```
[FAILED] User: jdoe | Password: admin123 | Reason: Contains banned word
```

## 📜 License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.

## 🛠️ Development

To compile the DLL from the source code, you can use the provided `filter.c` file. The project is designed to be compiled with a C compiler that supports Windows DLL creation.

Example compilation command using `gcc`:

```bash
x86_64-w64-mingw32-gcc -shared -o filter.dll filter.c -lshlwapi -municode
```
