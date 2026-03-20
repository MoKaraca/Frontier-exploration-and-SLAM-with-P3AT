# Git Quick Start - For Your ROS Workspace

Your workspace is ready to be added to Git! Follow these simple steps.

---

## ⚡ 30-Second Setup (Windows PowerShell)

```powershell
# 1. Install Git from: https://git-scm.com/download/win
#    (Restart PowerShell after installation)

# 2. Navigate to your workspace
cd d:\ubuntu_20_RTG\catkin_ws\src

# 3. Run one of these scripts:
.\git-init.ps1        # PowerShell version (recommended)
# OR
.\git-init.bat        # Command Prompt version
```

Done! Your repository is initialized with all files tracked.

---

## Alternative: Manual Setup

If you prefer to do it manually:

```powershell
cd d:\ubuntu_20_RTG\catkin_ws\src

# 1. Configure Git (first time only)
git config --global user.name "Your Name"
git config --global user.email "your@email.com"

# 2. Initialize repo
git init

# 3. Add all files
git add .

# 4. Create first commit
git commit -m "Initial commit: ROS catkin workspace"

# 5. (Optional) Add remote repository
git remote add origin https://github.com/YOUR_USERNAME/your-repo.git
git branch -M main
git push -u origin main
```

---

## What Files Are Already Configured?

✅ **`.gitignore`** - Tells git what to ignore (build files, Python cache, etc.)
✅ **`.gitattributes`** - Handles line endings correctly (CRLF ↔ LF)
✅ **`LICENSE`** - MIT License for your project
✅ **`GIT_SETUP.md`** - Comprehensive setup guide
✅ **`CONTRIBUTING.md`** - Guidelines for contributors
✅ **Initialization scripts** - Automated setup for Linux/macOS/Windows

---

## After You Initialize Git

### Track Your Changes
```powershell
# See what changed
git status

# Stage changes
git add .

# Commit with message
git commit -m "Fixed frontier clustering algorithm"

# View history
git log --oneline
```

### Push to GitHub

1. **Create repo on GitHub** at https://github.com/new (don't initialize it)
2. **Connect your local repo:**
   ```powershell
   git remote add origin https://github.com/YOUR_USERNAME/your-repo-name.git
   git branch -M main
   git push -u origin main
   ```

3. **Future pushes are simple:**
   ```powershell
   git push origin main
   ```

---

## Common Commands Cheat Sheet

| Command | Purpose |
|---------|---------|
| `git status` | See what changed |
| `git add .` | Stage all changes |
| `git commit -m "msg"` | Create a commit |
| `git log` | View history |
| `git push` | Send to remote |
| `git pull` | Get latest from remote |
| `git checkout -b feature` | Create new branch |
| `git diff` | See detailed changes |

---

## Key Points

✅ **All source code is tracked** (`.cpp`, `.py`, `.xml`, `.yaml`, etc.)
✅ **Build artifacts are ignored** (`build/`, `devel/`, `*.pyc`, etc.)
✅ **Perfect for collaboration** - Easy to manage contributions
✅ **Full history** - Track every change with commit messages
✅ **Remote backup** - Push to GitHub for safe storage

---

## File Structure After Init

```
src/
├── .git/                 ← Git repository (created by git init)
├── .gitignore           ← Files to ignore (already configured)
├── .gitattributes       ← Line ending config (already configured)
├── LICENSE              ← MIT License
├── README.md            ← Project documentation
├── GIT_SETUP.md         ← Full setup guide
├── GIT_QUICK_START.md   ← This file
├── CONTRIBUTING.md      ← Contributor guidelines
├── git-init.sh          ← Init script for Linux/macOS
├── git-init.ps1         ← Init script for PowerShell
├── git-init.bat         ← Init script for Command Prompt
├── odev2/               ← Your main project (all tracked)
├── pioneer3at_demo/     ← Robot simulation (all tracked)
├── hector_nist_arenas_gazebo/  ← Arena tools (all tracked)
└── newModels/           ← Models (all tracked)
```

---

## Troubleshooting

### "Git is not installed"
- Download from: https://git-scm.com/download/win
- Select "Use Git from PowerShell" during installation
- Restart PowerShell after installation

### "Permission denied" when pushing
```powershell
# Use one of these:

# Option 1: HTTPS with token (easier)
git remote set-url origin https://github.com/USERNAME/REPO.git

# Option 2: SSH (more secure)
# First generate SSH key (requires Git Bash):
ssh-keygen -t ed25519 -C "your@email.com"
# Then add public key to GitHub settings
# Then use:
git remote set-url origin git@github.com:USERNAME/REPO.git
```

### "Line ending issues"
- Already handled by `.gitattributes` file ✅
- Git will auto-convert between Windows (CRLF) and Unix (LF)

---

## Example Workflow

```powershell
# Day 1: Initial setup
cd d:\ubuntu_20_RTG\catkin_ws\src
.\git-init.ps1

# Day 2: Make changes
# ... edit explore.cpp ...
git add .
git commit -m "Optimize frontier clustering"
git push

# Day 3: More changes
# ... edit object_detector.cpp ...
git add odev2/src/object_detector.cpp
git commit -m "Fix barrel detection accuracy"
git push

# View history
git log --oneline --all
```

---

## Useful Git Resources

- **Git Documentation**: https://git-scm.com/doc
- **GitHub Help**: https://docs.github.com
- **Interactive Tutorial**: https://learngitbranching.js.org/
- **Commit Message Guide**: https://www.conventionalcommits.org/
- **ROS Development**: http://wiki.ros.org/ROS/DistributedDevelopment

---

## Next Steps

1. ✅ Install Git (if not already done)
2. ✅ Run `git-init.ps1` or `git-init.bat`
3. ✅ Start making commits: `git add .` → `git commit -m "msg"` → `git push`
4. 📖 Read `GIT_SETUP.md` for advanced topics
5. 🚀 Share your repo on GitHub!

---

**Your workspace is now ready for professional version control!** 🎉

Questions? Check:
- [GIT_SETUP.md](GIT_SETUP.md) - Comprehensive guide
- [CONTRIBUTING.md](CONTRIBUTING.md) - Collaboration guidelines
- Git documentation: https://git-scm.com/doc

Happy coding! 🚀
