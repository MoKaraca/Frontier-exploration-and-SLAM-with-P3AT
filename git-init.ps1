# Git Repository Initialization Script for ROS Workspace
# PowerShell version for Windows
# Usage: .\git-init.ps1

# Set error action preference
$ErrorActionPreference = "Stop"

Write-Host "`n=========================================="
Write-Host "ROS Catkin Workspace Git Setup"
Write-Host "==========================================`n"

# Check if git is installed
try {
    $gitVersion = git --version
    Write-Host "✓ Git is installed: $gitVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ Git is not installed. Please install it first:" -ForegroundColor Red
    Write-Host "   Download from: https://git-scm.com/download/win" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "   During installation, select:" -ForegroundColor Yellow
    Write-Host "   - Use Git from PowerShell" -ForegroundColor Yellow
    Write-Host "   - Checkout as-is, commit as LF" -ForegroundColor Yellow
    exit 1
}

Write-Host ""

# Get script directory
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Check if already a git repository
if (Test-Path "$scriptDir\.git") {
    Write-Host "⚠️  This directory is already a git repository!" -ForegroundColor Yellow
    Write-Host "Location: $scriptDir" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Current status:" -ForegroundColor Cyan
    Push-Location $scriptDir
    git status
    Pop-Location
    exit 0
}

# Configure git if needed
Write-Host "Checking git configuration..."
$gitName = git config --global user.name 2>$null
$gitEmail = git config --global user.email 2>$null

if ([string]::IsNullOrEmpty($gitName)) {
    Write-Host "⚠️  Git user name not configured." -ForegroundColor Yellow
    $gitName = Read-Host "Enter your name"
    git config --global user.name "$gitName"
}

if ([string]::IsNullOrEmpty($gitEmail)) {
    Write-Host "⚠️  Git email not configured." -ForegroundColor Yellow
    $gitEmail = Read-Host "Enter your email"
    git config --global user.email "$gitEmail"
}

Write-Host "✓ Git configured as: $gitName <$gitEmail>" -ForegroundColor Green
Write-Host ""

# Initialize repository
Write-Host "Initializing git repository..." -ForegroundColor Cyan
Push-Location $scriptDir
git init
Write-Host "✓ Repository initialized" -ForegroundColor Green
Write-Host ""

# Add all files
Write-Host "Adding files to staging area..." -ForegroundColor Cyan
git add .
Write-Host "✓ Files staged" -ForegroundColor Green
Write-Host ""

# Show status
Write-Host "Files to be committed:" -ForegroundColor Cyan
git status --short
Write-Host ""

# Create initial commit
Write-Host "Creating initial commit..." -ForegroundColor Cyan
git commit -m "Initial commit: ROS catkin workspace with frontier exploration and detection nodes"
Write-Host "✓ Initial commit created" -ForegroundColor Green
Write-Host ""

# Show final status
Write-Host "=========================================="
Write-Host "✓ Repository successfully initialized!" -ForegroundColor Green
Write-Host "==========================================`n"

Write-Host "Repository location: $scriptDir" -ForegroundColor Cyan
Write-Host "Commit history:" -ForegroundColor Cyan
git log --oneline -n 5
Write-Host ""

# Optional: Set up remote
$response = Read-Host -Prompt "Would you like to connect to a remote repository? (y/n)"
if ($response -eq "y" -or $response -eq "Y") {
    Write-Host ""
    Write-Host "Enter remote repository URL (e.g., https://github.com/username/repo.git):" -ForegroundColor Yellow
    $remoteUrl = Read-Host
    
    if ($remoteUrl) {
        git remote add origin "$remoteUrl"
        Write-Host "✓ Remote added: origin" -ForegroundColor Green
        Write-Host ""
        Write-Host "To push your code, run:" -ForegroundColor Cyan
        Write-Host "  git branch -M main" -ForegroundColor Gray
        Write-Host "  git push -u origin main" -ForegroundColor Gray
    }
}

Pop-Location  # Return to original directory

Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Make changes to your code" -ForegroundColor Green
Write-Host "2. Stage changes: git add ." -ForegroundColor Green
Write-Host "3. Commit changes: git commit -m 'Your message'" -ForegroundColor Green
Write-Host "4. Push to remote: git push" -ForegroundColor Green
Write-Host ""
Write-Host "Learn more: Read GIT_SETUP.md in the workspace root" -ForegroundColor Yellow
Write-Host ""
