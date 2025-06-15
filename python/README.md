# Python Scripts

This directory contains the Python modules that are embedded and called from the C++ Qt application via Boost.Python.

## Files

### `processor.py`
The main Python processor that handles JSON requests from the C++ application. Supports:
- **Math operations**: add, subtract, multiply, divide
- **Text operations**: uppercase, lowercase, reverse
- **Data operations**: statistics (count, sum, mean, min, max, range)
- **Echo operations**: simple echo for testing

### `__init__.py`
Package initialization file that makes this directory a proper Python package.

## Usage

### From C++ Application
The C++ application automatically loads `processor.py` and calls the `process_json()` function with JSON data.

### Standalone Testing
You can test the Python scripts directly:

```bash
# Activate the virtual environment
source .venv/bin/activate

# Run the main processor
python python/processor.py

# Run diagnostics
python python/diagnostic_processor.py
```

### Debugging
Use the VS Code debug configuration "Debug Python Processor (via venv)" to debug the Python scripts with breakpoints and step-through debugging.

## Development

The Python code follows these standards:
- **Black** for code formatting
- **isort** for import organization  
- **flake8** for style checking
- **mypy** for type checking
- **pylint** for code analysis

All tools are automatically configured via `pyproject.toml` and `setup.cfg` in the project root.
