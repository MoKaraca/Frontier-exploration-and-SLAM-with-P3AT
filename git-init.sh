#!/bin/bash
# Git Repository Initialization Script for ROS Workspace
# This script initializes a git repository for the catkin workspace

set -e  # Exit on error

echo "=========================================="
echo "ROS Catkin Workspace Git Setup"
echo "=========================================="
echo ""

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo "❌ Git is not installed. Please install git first:"
    echo "   Linux/Ubuntu: sudo apt-get install git"
    echo "   macOS: brew install git"
    echo "   Windows: Download from https://git-scm.com/download/win"
    exit 1
fi

echo "✓ Git is installed: $(git --version)"
echo ""

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if already a git repository
if [ -d "$SCRIPT_DIR/.git" ]; then
    echo "⚠️  This directory is already a git repository!"
    echo "Location: $SCRIPT_DIR"
    echo ""
    echo "Current status:"
    cd "$SCRIPT_DIR"
    git status
    exit 0
fi

# Configure git if needed
echo "Checking git configuration..."
if [ -z "$(git config --global user.name)" ]; then
    echo ""
    echo "⚠️  Git user name not configured."
    read -p "Enter your name: " git_name
    git config --global user.name "$git_name"
fi

if [ -z "$(git config --global user.email)" ]; then
    echo "⚠️  Git email not configured."
    read -p "Enter your email: " git_email
    git config --global user.email "$git_email"
fi

echo "✓ Git configured as: $(git config --global user.name) <$(git config --global user.email)>"
echo ""

# Initialize repository
echo "Initializing git repository..."
cd "$SCRIPT_DIR"
git init
echo "✓ Repository initialized"
echo ""

# Add all files
echo "Adding files to staging area..."
git add .
echo "✓ Files staged"
echo ""

# Show status
echo "Files to be committed:"
git status --short
echo ""

# Create initial commit
echo "Creating initial commit..."
git commit -m "Initial commit: ROS catkin workspace with frontier exploration and detection nodes"
echo "✓ Initial commit created"
echo ""

# Show final status
echo "=========================================="
echo "✓ Repository successfully initialized!"
echo "=========================================="
echo ""
echo "Repository location: $SCRIPT_DIR"
echo "Commit history:"
git log --oneline -n 5
echo ""

# Optional: Set up remote
read -p "Would you like to connect to a remote repository? (y/n): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Enter remote repository URL (e.g., https://github.com/username/repo.git):"
    read remote_url
    
    if [ ! -z "$remote_url" ]; then
        git remote add origin "$remote_url"
        echo "✓ Remote added: origin"
        echo ""
        echo "To push your code, run:"
        echo "  git branch -M main"
        echo "  git push -u origin main"
    fi
fi

echo ""
echo "Next steps:"
echo "1. Make changes to your code"
echo "2. Stage changes: git add ."
echo "3. Commit changes: git commit -m 'Your message'"
echo "4. Push to remote: git push"
echo ""
echo "Learn more: Read GIT_SETUP.md in the workspace root"
echo ""
