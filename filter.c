#include <windows.h>
#include <ntsecapi.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#define LOG_FILE_PATH L"C:\\PasswordFilter\\PasswordFilter.log"

#define CONFIG_FILE_PATH L"C:\\PasswordFilter\\banned_words.txt"

#define MAX_BANNED_WORDS 100
#define MAX_WORD_LEN 128

wchar_t dynamicBannedWords[MAX_BANNED_WORDS][MAX_WORD_LEN];
int bannedWordCount = 0;

// 🔧 Charger les mots interdits à partir du fichier
void LoadBannedWordsFromFile() {
    FILE* f;
    _wfopen_s(&f, CONFIG_FILE_PATH, L"r, ccs=UTF-8");
    if (!f) return;

    wchar_t line[MAX_WORD_LEN];
    while (fgetws(line, MAX_WORD_LEN, f) && bannedWordCount < MAX_BANNED_WORDS) {
        // Nettoyer les retours à la ligne
        line[wcscspn(line, L"\r\n")] = 0;
        if (wcslen(line) > 0) {
            _wcslwr_s(line, MAX_WORD_LEN);
            wcscpy_s(dynamicBannedWords[bannedWordCount++], MAX_WORD_LEN, line);
        }
    }
    fclose(f);
}

const wchar_t* bannedWords[] = {
    L"password", L"admin", L"123456", L"azerty", L"qwerty", L"letmein",
    L"motdepasse", L"welcome",
    L"abc123", L"000000", L"aaaaaa"
};

// Inverser une chaîne de caractères (Unicode)
void ReverseWString(WCHAR* dest, const WCHAR* src, size_t len) {
    for (size_t i = 0; i < len; ++i)
        dest[i] = src[len - 1 - i];
    dest[len] = L'\0';
}

// Logging
void LogFailure(PUNICODE_STRING AccountName, PUNICODE_STRING Password, const wchar_t* reason) {
    FILE* f;
    _wfopen_s(&f, LOG_FILE_PATH, L"a+, ccs=UTF-8");
    if (f) {
        fwprintf(f, L"[FAILED] User: %wZ | Password: %wZ | Reason: %s\n", AccountName, Password, reason);
        fclose(f);
    }
}

// Vérifie la répétition de caractères identiques
BOOL IsRepetitivePassword(const WCHAR* pw, size_t len) {
    for (size_t i = 1; i < len; ++i)
        if (pw[i] != pw[0]) return FALSE;
    return TRUE;
}

// Récupère le nom NetBIOS du domaine
void GetNetbiosDomainName(WCHAR* buffer, DWORD size) {
    DWORD len = size;
    GetComputerNameExW(ComputerNameNetBIOS, buffer, &len);
}

// Récupère le nom DNS du domaine (si joint)
void GetDnsDomainName(WCHAR* buffer, DWORD size) {
    DWORD len = size;
    GetComputerNameExW(ComputerNameDnsDomain, buffer, &len);
}

BOOLEAN __stdcall PasswordFilter(
    PUNICODE_STRING AccountName,
    PUNICODE_STRING FullName,
    PUNICODE_STRING Password,
    BOOLEAN SetOperation
    ) {
    LoadBannedWordsFromFile();  // ← Charger les mots dynamiques

    const WCHAR* pw = Password->Buffer;
    int len = Password->Length / sizeof(WCHAR);

    WCHAR pwLower[512] = {0};
    wcsncpy_s(pwLower, 512, pw, len);
    _wcslwr_s(pwLower, 512);

    for (int i = 0; i < sizeof(bannedWords)/sizeof(bannedWords[0]); i++) {
        if (StrStrW(pwLower, bannedWords[i]) != NULL) {
            LogFailure(AccountName, Password, L"Contains banned word");
            return FALSE;
        }

        WCHAR reversed[512];
        ReverseWString(reversed, bannedWords[i], wcslen(bannedWords[i]));
        if (StrStrW(pwLower, reversed) != NULL) {
            LogFailure(AccountName, Password, L"Contains reversed banned word");
            return FALSE;
        }
    }

    for (int i = 0; i < bannedWordCount; ++i) {
        if (StrStrW(pwLower, dynamicBannedWords[i])) {
            LogFailure(AccountName, Password, L"Password contains term from banned_words.txt");
            return FALSE;
        }

        WCHAR reversed[512];
        ReverseWString(reversed, dynamicBannedWords[i], wcslen(dynamicBannedWords[i]));
        if (StrStrW(pwLower, reversed)) {
            LogFailure(AccountName, Password, L"Password contains reversed term from banned_words.txt");
            return FALSE;
        }
    }


    if (IsRepetitivePassword(pw, len)) {
        LogFailure(AccountName, Password, L"Password is repetitive (e.g., 'aaaaaa')");
        return FALSE;
    }

    // Vérifie si le nom d’utilisateur est dans le mot de passe
    WCHAR usernameLower[512] = {0};
    wcsncpy_s(usernameLower, 512, AccountName->Buffer, AccountName->Length / sizeof(WCHAR));
    _wcslwr_s(usernameLower, 512);
    if (StrStrW(pwLower, usernameLower) != NULL) {
        LogFailure(AccountName, Password, L"Password contains username");
        return FALSE;
    }

    // Vérifie si le nom complet est dans le mot de passe
    WCHAR fullnameLower[512] = {0};
    wcsncpy_s(fullnameLower, 512, FullName->Buffer, FullName->Length / sizeof(WCHAR));
    _wcslwr_s(fullnameLower, 512);
    if (StrStrW(pwLower, fullnameLower) != NULL) {
        LogFailure(AccountName, Password, L"Password contains full name");
        return FALSE;
    }

        // Vérifie si le mot de passe contient des termes connus (NetBIOS, DNS, ou liste personnalisée)
    WCHAR netbios[256] = {0}, dnsDomain[256] = {0};
    GetNetbiosDomainName(netbios, 256);
    GetDnsDomainName(dnsDomain, 256);
    _wcslwr_s(netbios, 256);
    _wcslwr_s(dnsDomain, 256);

    if (wcslen(netbios) > 0 && StrStrW(pwLower, netbios)) {
        LogFailure(AccountName, Password, L"Password contains NetBIOS domain name");
        return FALSE;
    }

    if (wcslen(dnsDomain) > 0 && StrStrW(pwLower, dnsDomain)) {
        LogFailure(AccountName, Password, L"Password contains DNS domain name");
        return FALSE;
    }

    return TRUE;
}

// Fonctions obligatoires
BOOLEAN __stdcall InitializeChangeNotify(void) {
    return TRUE;
}

BOOLEAN __stdcall PasswordChangeNotify(
    PUNICODE_STRING UserName,
    ULONG RelativeId,
    PUNICODE_STRING NewPassword
) {
    return TRUE;
}

