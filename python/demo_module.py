#!/usr/bin/env python3
"""
Demo module for testing the generic Python command execution.
This module provides various functions to demonstrate different argument types and return values.
"""
import time
import math
from typing import List, Dict, Any, Union
import platform
import sys
import os

def greet(name: str, title: str = ""):
    """Simple greeting function."""
    if title:
        return f"Hello, {title} {name}!"
    return f"Hello, {name}!"


def add_numbers(a: float, b: float) -> float:
    """Add two numbers."""
    return a + b


def multiply_list(numbers: List[float]) -> float:
    """Multiply all numbers in a list."""
    result = 1.0
    for num in numbers:
        result *= num
    return result


def get_math_constants() -> Dict[str, float]:
    """Return a dictionary of math constants."""
    return {
        "pi": math.pi,
        "e": math.e,
        "tau": math.tau,
        "sqrt2": math.sqrt(2)
    }


def process_data(data: Dict[str, Any]) -> Dict[str, Any]:
    """Process a data dictionary and return statistics."""
    result = {
        "input_keys": list(data.keys()),
        "input_count": len(data),
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime()),
        "processing_info": {}
    }
    
    # Analyze different data types
    for key, value in data.items():
        if isinstance(value, (int, float)):
            result["processing_info"][key] = {
                "type": "numeric",
                "value": value,
                "squared": value * value
            }
        elif isinstance(value, str):
            result["processing_info"][key] = {
                "type": "string",
                "length": len(value),
                "uppercase": value.upper()
            }
        elif isinstance(value, list):
            result["processing_info"][key] = {
                "type": "list",
                "length": len(value),
                "elements": value[:3]  # First 3 elements
            }
        else:
            result["processing_info"][key] = {
                "type": str(type(value).__name__),
                "string_repr": str(value)
            }
    
    return result


def fibonacci(n: int) -> List[int]:
    """Generate fibonacci sequence up to n terms."""
    if n <= 0:
        return []
    elif n == 1:
        return [0]
    elif n == 2:
        return [0, 1]
    
    fib = [0, 1]
    for i in range(2, n):
        fib.append(fib[i-1] + fib[i-2])
    
    return fib


def validate_input(value: Any, expected_type: str) -> Dict[str, Any]:
    """Validate input against expected type."""
    actual_type = type(value).__name__
    
    result = {
        "value": value,
        "actual_type": actual_type,
        "expected_type": expected_type,
        "is_valid": False,
        "message": ""
    }
    
    type_map = {
        "str": str,
        "int": int,
        "float": float,
        "bool": bool,
        "list": list,
        "dict": dict
    }
    
    if expected_type in type_map:
        if isinstance(value, type_map[expected_type]):
            result["is_valid"] = True
            result["message"] = f"Value is valid {expected_type}"
        else:
            result["message"] = f"Expected {expected_type}, got {actual_type}"
    else:
        result["message"] = f"Unknown expected type: {expected_type}"
    
    return result


def execute_math_operation(operation: str, *args) -> Union[float, str]:
    """Execute various math operations with variable arguments."""
    try:
        if operation == "add":
            return sum(args)
        elif operation == "multiply":
            result = 1
            for arg in args:
                result *= arg
            return result
        elif operation == "power":
            if len(args) != 2:
                return f"Power operation requires exactly 2 arguments, got {len(args)}"
            return pow(args[0], args[1])
        elif operation == "sqrt":
            if len(args) != 1:
                return f"Square root operation requires exactly 1 argument, got {len(args)}"
            return math.sqrt(args[0])
        elif operation == "mean":
            if len(args) == 0:
                return "Mean operation requires at least 1 argument"
            return sum(args) / len(args)
        else:
            return f"Unknown operation: {operation}"
    except (ValueError, TypeError, ArithmeticError) as e:
        return f"Error in {operation}: {str(e)}"


def get_system_info() -> Dict[str, Any]:
    """Get basic system information."""

    
    return {
        "python_version": sys.version,
        "platform": platform.platform(),
        "architecture": platform.architecture(),
        "current_working_directory": os.getcwd(),
        "python_executable": sys.executable,
        "python_path": sys.path[:3]  # First 3 entries
    }


if __name__ == "__main__":
    # Test functions
    print("Testing demo functions:")
    print(f"greet('World'): {greet('World')}")
    print(f"add_numbers(5, 3): {add_numbers(5, 3)}")
    print(f"fibonacci(8): {fibonacci(8)}")
    print(f"get_math_constants(): {get_math_constants()}")
