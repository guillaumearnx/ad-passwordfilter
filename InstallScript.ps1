# Installer le filtre de mot de passe sécurisé
$filterName = "PasswordFilter"
$sourceDir = "C:\PasswordFilter"
$sourceDll = "$sourceDir\PasswordFilter.dll"
$targetDll = "C:\Windows\System32\$filterName.dll"
$regPath = "HKLM:\SYSTEM\CurrentControlSet\Control\Lsa"
$githubURL = "https://github.com/guillaumearnx/ad-passwordfilter/releases/latest/download/PasswordFilter.dll"

if (-Not (Test-Path $sourceDir)) {
    New-Item -Path $sourceDir -ItemType Directory | Out-Null
    Write-Host "📁 Created directory $sourceDir"
}

# 🔽 Download DLL from GitHub release
Write-Host "🌐 Downloading DLL from GitHub..."
Invoke-WebRequest -Uri $githubDllUrl -OutFile $sourceDll -UseBasicParsing
Write-Host "✅ Downloaded DLL to $sourceDll"

Write-Host "🔧 Copie du DLL dans System32..."

if (-Not (Test-Path "$targetDll")) {
    Copy-Item -Path $sourceDll -Destination $targetDll -Force
} else {
    Write-Host "DLL already present in System32"
}

Write-Host "📝 Vérification / ajout dans le registre..."
$current = Get-ItemProperty -Path $regPath -Name "Notification Packages"
$packages = $current."Notification Packages"

if ($packages -notcontains $filterName) {
    $packages += $filterName
    Set-ItemProperty -Path $regPath -Name "Notification Packages" -Value $packages
    Write-Host "✅ '$filterName' ajouté dans Notification Packages"
} else {
    Write-Host "ℹ️ '$filterName' déjà présent dans le registre"
}

if (-Not (Test-Path "$sourceDir\PasswordFilter.log")) {
    New-Item -Path "$sourceDir\PasswordPolicy.log" -ItemType File | Out-Null
    Write-Host "📁 Création du dossier de logs"
}

if (-Not (Test-Path "$sourceDir\banned_words.txt")) {
    New-Item -Path "$sourceDir\banned_words.txt" -ItemType File | Out-Null
    Write-Host "📁 Création du fichier banned_words.txt"
}

Write-Host "📁 Veuillez personnaliser le fichier banned_words.txt"
Write-Host "✅ Installation terminée."
Write-Host "🚨 Redémarrez le contrôleur de domaine pour appliquer les changements."

