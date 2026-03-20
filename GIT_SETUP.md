# Git Repository Setup Guide

This guide will help you initialize and manage your ROS catkin workspace as a Git repository.

## Prerequisites

### 1. Install Git

**Windows:**
- Download from: https://git-scm.com/download/win
- During installation, select:
  - "Use Git from the Windows Command Prompt" (or PowerShell)
  - "Checkout as-is, commit as LF" (important for cross-platform compatibility)
  - Install Git Credential Manager for easier authentication

**Linux/Ubuntu:**
```bash
sudo apt-get install git
```

**macOS:**
```bash
brew install git
```

Verify installation:
```bash
git --version
```

---

## Step 1: Configure Git (First Time Only)

```bash
# Set your identity
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"

# Optional: Set your default editor
git config --global core.editor "code"  # For VS Code
# or
git config --global core.editor "nano"  # For Nano
```

---

## Step 2: Initialize the Repository

Navigate to the workspace src folder and initialize git:

```bash
cd d:\ubuntu_20_RTG\catkin_ws\src
git init
```

This creates a `.git` folder that tracks the repository history.

---

## Step 3: Add All Files

Stage all files for the initial commit:

```bash
# Add all files to staging area
git add .

# View what will be committed
git status
```

Expected output should show all source files ready to be committed, with build artifacts ignored based on `.gitignore`.

---

## Step 4: Create Initial Commit

```bash
git commit -m "Initial commit: ROS catkin workspace with frontier exploration, detection nodes, and NIST arena tools"
```

---

## Step 5: (Optional) Connect to Remote Repository

If you want to push to GitHub, GitLab, or Bitbucket:

### GitHub Example:

1. Create a new repository on GitHub (https://github.com/new)
   - Name: `ros-exploration-workspace` (or your choice)
   - Do NOT initialize with README or .gitignore (we already have these)

2. Add the remote:
```bash
git remote add origin https://github.com/YOUR_USERNAME/ros-exploration-workspace.git
git branch -M main
git push -u origin main
```

3. For future commits:
```bash
git push
# or
git push origin main
```

### Using SSH (Recommended):

```bash
# Generate SSH key if you don't have one
ssh-keygen -t ed25519 -C "your.email@example.com"

# Add to GitHub SSH keys (Settings > SSH and GPG keys)
# Then use:
git remote add origin git@github.com:YOUR_USERNAME/ros-exploration-workspace.git
git push -u origin main
```

---

## Common Git Commands

### View Commit History
```bash
git log --oneline
git log --graph --all --decorate
```

### Make Changes and Commit
```bash
# Edit files...
git status                    # See what changed
git add .                     # Stage all changes
git commit -m "Description of changes"
git push                      # Push to remote
```

### Create a Branch for Development
```bash
git checkout -b feature/new-detection-algorithm
# Make changes
git commit -am "Add new detection algorithm"
git push -u origin feature/new-detection-algorithm
```

### Update from Remote
```bash
git fetch origin
git pull origin main
```

### Undo Recent Changes
```bash
git restore <filename>           # Undo unstaged changes
git restore --staged <filename>  # Unstage file
git reset --soft HEAD~1          # Undo last commit, keep changes staged
git reset --hard HEAD~1          # Undo last commit completely
```

---

## File Structure Overview

```
src/
├── .git/                           # Git repository data (auto-created)
├── .gitignore                      # Files to ignore in version control
├── .gitattributes                  # Line ending configuration
├── README.md                       # Project documentation
├── GIT_SETUP.md                    # This file
├── odev2/                          # Main exploration project
│   ├── src/
│   │   ├── explore.cpp             # Frontier exploration (optimized)
│   │   ├── object_detector.cpp     # Vision detection
│   │   ├── hazmat_detector.cpp     # Hazmat sign detection
│   │   └── ...
│   ├── scripts/
│   ├── launch/
│   └── package.xml
├── pioneer3at_demo/                # Robot simulation
├── hector_nist_arenas_gazebo/      # Arena environments
└── newModels/                      # Additional Gazebo models
```

---

## What's Tracked vs. Ignored

### 📝 Tracked (in git)
- C++ source files (`.cpp`, `.h`, `.hpp`)
- Python scripts (`.py`)
- Configuration files (`.xml`, `.yaml`, `.launch`, `.urdf`, `.xacro`)
- Documentation files (`.md`)
- Asset files (`.dae`, `.png`, `.jpg`)
- Launch files and parameter files

### ❌ Ignored (not in git)
- `build/` - CMake build output
- `devel/` - Development workspace
- `install/` - Installation output
- `*.pyc` - Python bytecode
- `.vscode/`, `.idea/` - IDE settings
- `*.bag` - ROS bag files
- `CMakeCache.txt` - CMake cache

This keeps the repository lightweight while preserving all source code.

---

## Best Practices

### 1. Commit Often
```bash
# Good - focused, atomic commits
git commit -m "Fix yaw calculation in frontier selection"
git commit -m "Add visualization dirty flag optimization"

# Avoid - vague, multi-purpose commits
git commit -m "Fixed stuff and improved things"
```

### 2. Use Descriptive Commit Messages
```
# Format:
# <type>: <subject line, max 50 chars>
# <blank line>
# <detailed explanation (wrap at 72 chars)>

Example:
fix: correct yaw calculation from robot to target

The yaw was incorrectly calculated from the map origin instead of
the robot's current position. This has been fixed to properly orient
the robot toward exploration targets.

Fixes #42
```

### 3. Push to Remote Regularly
```bash
git push origin main
```

### 4. Create Branches for Features
```bash
# For new features
git checkout -b feature/improve-clustering-algorithm

# For bug fixes
git checkout -b bugfix/fix-transform-lookup

# For experiments
git checkout -b experiment/gpu-pathfinding
```

### 5. Review Before Committing
```bash
git diff              # Review unstaged changes
git diff --staged     # Review staged changes
git status           # See what will be committed
```

---

## Troubleshooting

### "Git command not found"
- Install Git or add it to PATH
- Restart PowerShell/terminal after installation

### "Permission denied" when pushing
- Check SSH key setup: `ssh -T git@github.com`
- Or use HTTPS with Git Credential Manager

### "Line ending issues" (LF vs CRLF)
- This is already handled by `.gitattributes`
- Git will convert to LF on commit, CRLF on checkout

### Large files slowing down git
- Use `.gitignore` to exclude data/models
- Consider Git LFS for binary files > 100MB

---

## Repository Management

### Check Remote Configuration
```bash
git remote -v
```

### Change Remote URL
```bash
git remote set-url origin git@github.com:YOUR_USERNAME/new-repo.git
```

### Create a .gitignore from Template
The `.gitignore` file is already configured for ROS/catkin projects.

### Delete Sensitive Data
```bash
# If you accidentally committed sensitive data
git filter-branch --tree-filter 'rm -f secrets.txt' HEAD
git push --force-with-lease origin main
```

---

## Quick Start Summary

```bash
# 1. Install Git (download from git-scm.com)

# 2. Configure Git (first time only)
git config --global user.name "Your Name"
git config --global user.email "your@email.com"

# 3. Navigate to workspace
cd d:\ubuntu_20_RTG\catkin_ws\src

# 4. Initialize repository
git init

# 5. Add all files
git add .

# 6. Create initial commit
git commit -m "Initial commit: ROS workspace with exploration and detection nodes"

# 7. (Optional) Set up remote
git remote add origin https://github.com/YOUR_USERNAME/your-repo.git
git branch -M main
git push -u origin main

# ✅ Done! Your repository is ready.
```

---

## Resources

- Git Documentation: https://git-scm.com/doc
- GitHub Help: https://docs.github.com
- Commit Message Guidelines: https://www.conventionalcommits.org/
- ROS Developer Guide: http://wiki.ros.org/ROS/DistributedDevelopment
- ROS Git Workflow: http://wiki.ros.org/catkin/Tutorials/using_catkin_with_rosdep

---

**Happy coding! Your workspace is now ready for version control.** 🚀
