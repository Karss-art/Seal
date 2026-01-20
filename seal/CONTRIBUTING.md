# Contributing to Seal Shell

Thank you for your interest in contributing to Seal Shell!

## How to Contribute

### Reporting Bugs
- Use the GitHub Issues tracker
- Include detailed reproduction steps
- Specify your OS and compiler version

### Suggesting Features
- Open an issue with the `enhancement` label
- Describe the feature and its use case
- Explain why it would be useful

### Code Contributions

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes** following the coding style
4. **Test thoroughly**: `make test`
5. **Commit**: `git commit -m 'Add amazing feature'`
6. **Push**: `git push origin feature/amazing-feature`
7. **Open a Pull Request**

## Coding Standards

- Follow existing code style
- Use meaningful variable names
- Comment complex logic
- No memory leaks (verify with `valgrind`)
- No file descriptor leaks
- Proper error handling

## Testing

Before submitting:
```bash
make clean
make
make test
valgrind --leak-check=full ./seal
```

## Code of Conduct

- Be respectful and constructive
- Focus on the code, not the person
- Help newcomers learn

## Questions?

Open an issue with the `question` label.

Thank you for contributing! 🎉
