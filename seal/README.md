# Seal Shell 🐚

<div align="center">

**A Unix-like shell implementation in C with full job control and I/O redirection**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Standard: GNU C11](https://img.shields.io/badge/Standard-GNU%20C11-green.svg)](https://gcc.gnu.org/)

[Features](#features) • [Installation](#installation) • [Usage](#usage) • [Documentation](#documentation) • [Contributing](#contributing)

</div>

---

## 🎯 About

Seal Shell is a lightweight, educational shell implementation that demonstrates core Unix concepts including process groups, terminal control, and signal handling. Built from scratch in C without using `system()` or `popen()`.

**Key Stats:**
- 📝 1,339 lines of clean C code
- 💾 37KB compiled binary
- ⚡ Zero external dependencies
- 🎓 Educational and well-documented

## ✨ Features

### 🔧 Job Control
- **Background execution** (`&`) - Run processes in the background
- **Job listing** (`jobs`) - View all active jobs
- **Foreground** (`fg`) - Bring background jobs to foreground
- **Background** (`bg`) - Resume stopped jobs in background
- **Process groups** - Proper process group management
- **Terminal control** - Correct TTY foreground/background handling
- **Signal handling** - Ctrl-C and Ctrl-Z work as expected

### 📂 I/O Redirection
- **Input** (`<`) - Redirect input from file
- **Output** (`>`) - Redirect output to file (truncate)
- **Append** (`>>`) - Redirect output to file (append)
- **Stderr** (`2>`) - Redirect stderr to file
- **Combine** (`2>&1`) - Redirect stderr to stdout
- **Pipes** (`|`) - Connect commands in pipelines

### 🛠️ Built-in Commands
- `cd [dir]` - Change directory
- `exit [status]` - Exit the shell
- `jobs` - List active jobs
- `fg [job_id]` - Move job to foreground
- `bg [job_id]` - Move job to background
- `help` - Display help information
- `export VAR=value` - Set environment variables

## 🚀 Installation

### Prerequisites
- GCC compiler
- GNU Make
- Linux operating system

### Build from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/seal-shell.git
cd seal-shell

# Compile
make

# Optional: Run tests
make test

# Optional: Install system-wide (requires sudo)
sudo make install
```

## 📖 Usage

### Interactive Mode

```bash
./seal
seal> pwd
/home/user/seal
seal> ls | wc -l
15
seal> exit
```

### Non-Interactive Mode

```bash
echo "ls -la | grep seal" | ./seal
```

### Examples

**Simple redirection:**
```bash
seal> echo "Hello World" > output.txt
seal> cat output.txt
Hello World
```

**Pipelines:**
```bash
seal> cat /etc/passwd | cut -d: -f1 | sort | head -5
bin
daemon
games
mail
man
```

**Job control:**
```bash
seal> sleep 100 &
[1] 12345
seal> jobs
[1]  Running		sleep 100 &
seal> fg 1
# Press Ctrl-Z
^Z
[1]+ Stopped sleep 100
seal> bg 1
[1]+ sleep 100 &
```

## 📚 Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Quick start guide
- **[EXAMPLES.md](EXAMPLES.md)** - Detailed usage examples
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Contribution guidelines

## 🏗️ Architecture

```
┌─────────────────────────────────────────┐
│           REPL Loop (main.c)            │
│  • Read user input                      │
│  • Parse and tokenize                   │
│  • Execute commands                     │
└──────────┬──────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────┐
│      Lexer & Parser                     │
│  • lexer.c: Tokenization                │
│  • parser.c: AST construction           │
└──────────┬──────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────┐
│        Executor                         │
│  • pipeline.c: Pipeline execution       │
│  • redirect.c: I/O redirection          │
└──────────┬──────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────┐
│     Job Control & Signals               │
│  • jobs.c: Job management               │
│  • signals.c: Signal handling           │
└─────────────────────────────────────────┘
```

## 🔬 Technical Details

### Process Groups
Each pipeline creates its own process group using `setpgid()`. The first process in the pipeline becomes the group leader.

### Terminal Control
- `tcsetpgrp()` gives terminal control to foreground jobs
- Shell always regains control after foreground jobs complete
- Background jobs run without terminal access

### Signal Handling
- **SIGCHLD**: Handles child process state changes
- **SIGINT/SIGTSTP**: Ignored by shell, forwarded to foreground jobs
- **SIGTTIN/SIGTTOU**: Handled to prevent shell suspension

### Memory Management
- All file descriptors properly closed
- No memory leaks (verified with valgrind)
- Proper cleanup on exit

## 🧪 Testing

Run the test suite:
```bash
make test
```

Check for memory leaks:
```bash
valgrind --leak-check=full ./seal
```

Verify file descriptor leaks:
```bash
ls -la /proc/$(pgrep seal)/fd
```

## 🤝 Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by Unix shell design principles
- Built following POSIX standards
- Educational resource for systems programming

## ⚠️ Known Limitations

- No wildcard expansion (`*`, `?`)
- No variable expansion (`$VAR`)
- No command history
- No tab completion
- No script file execution
- Background jobs don't persist after shell exit

## 🚧 Future Improvements

- [ ] Add wildcard support
- [ ] Implement variable expansion
- [ ] Add readline integration for history
- [ ] Implement tab completion
- [ ] Add script file support
- [ ] Improve error messages
- [ ] Add subshell support `()`

---

<div align="center">

**Made with ❤️ for systems programming education**

[Report Bug](https://github.com/yourusername/seal-shell/issues) • [Request Feature](https://github.com/yourusername/seal-shell/issues)

</div>
