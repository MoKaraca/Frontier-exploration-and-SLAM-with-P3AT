# Contributing Guidelines

Thank you for considering contributing to the ROS Exploration Workspace! This document provides guidelines and instructions for contributing.

## Code of Conduct

- Be respectful and constructive in all interactions
- Report issues professionally
- Welcome newcomers and help them get started

## Getting Started

1. **Fork the repository** (if applicable)
2. **Clone your fork:**
   ```bash
   git clone git@github.com:YOUR_USERNAME/ros-exploration-workspace.git
   cd ros-exploration-workspace/src
   ```

3. **Create a new branch for your work:**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b bugfix/issue-number
   ```

4. **Make your changes and commit:**
   ```bash
   git add .
   git commit -m "Clear description of changes"
   ```

5. **Push to your fork:**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create a Pull Request** with a clear description

## Coding Standards

### C++
- Follow ROS C++ Style Guide
- Use meaningful variable names
- Add comments for complex logic
- Compile without warnings: `catkin_make -- -Wall -Wextra`
- Use const-correctness and move semantics where appropriate

### Python
- Follow PEP 8 style guide
- Use type hints where possible
- Document functions with docstrings

### ROS Configuration
- Use descriptive parameter names in YAML files
- Comment all custom parameters
- Keep launch files organized logically
- Use namespaces to avoid conflicts

## Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat:` - New feature
- `fix:` - Bug fix
- `perf:` - Performance improvement
- `refactor:` - Code refactor
- `test:` - Test additions
- `docs:` - Documentation
- `style:` - Formatting (no logic change)
- `chore:` - Build, dependencies, etc.

**Example:**
```
feat(explore): add frontier information gain weighting

Replace simple distance-based frontier selection with multi-objective
scoring that considers information gain, proximity, and cluster size.

Improves exploration efficiency by 30% on typical environments.

Fixes #123
```

## Testing

Before submitting:

1. **Build successfully:**
   ```bash
   cd ~/catkin_ws
   catkin_make
   source devel/setup.bash
   ```

2. **Test your changes:**
   - For exploration: `roslaunch odev2 exploration.launch gui:=true`
   - Verify frontiers are detected correctly
   - Check RViz visualization
   - Monitor performance with `rqt_graph`

3. **Check for regressions:**
   - Run existing tests if available
   - Test on multiple map sizes

## Documentation

- Add/update docstrings for new functions
- Update README if adding features
- Add comments for non-obvious code
- Include usage examples for new modules

## Issues and Discussions

### Reporting Bugs

Include:
- Description of the problem
- Steps to reproduce
- Expected vs actual behavior
- ROS version, Ubuntu version
- Relevant error messages/logs

### Feature Requests

Include:
- Clear description of desired feature
- Why it's useful
- Implementation ideas (optional)
- Example use cases

## Pull Request Process

1. Ensure all tests pass
2. Update documentation
3. Add descriptive PR title and description
4. Reference related issues with #123
5. Respond to review comments
6. Ensure code is properly formatted

## Development Setup

```bash
# Clone and setup workspace
git clone <your-fork-url>
cd ros-exploration-workspace/src
cd ..

# Full workspace build
catkin_make -j4

# Source the workspace
source devel/setup.bash

# Run tests
catkin_make test

# Clean build
catkin_make clean
catkin_make -j4
```

## Performance Considerations

- Profile code before and after changes on realistic maps (500x500, 1000x1000)
- Use ROS diagnostic tools: `rqt_graph`, `rqt_plot`, `roswtf`
- Monitor memory usage and CPU
- Add timing measurements for critical sections
- Avoid blocking operations in callbacks

## File Structure

Keep the repository organized:
- Source code in `odev2/src/`
- Launch files in `odev2/launch/`
- Configuration in `odev2/nav_params/`
- Documentation in root and package folders
- Tests in `odev2/test/` (if applicable)

## Documentation Standards

- Use Markdown for documentation
- Include code examples
- Add diagrams where helpful
- Link to external resources
- Keep README.md up to date

## License

By contributing, you agree that your contributions follow the same license as the project (MIT License).

## Questions?

- Open an issue for discussion
- Email the maintainers
- Check existing issues for similar questions

---

**Thank you for contributing!** 🙏
