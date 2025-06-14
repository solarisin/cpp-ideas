#!/usr/bin/env python3
"""
Python script to process JSON data and return results.
This script demonstrates various JSON processing operations.
"""

import json
import math


def process_json(json_string: str) -> str:
    """
    Process JSON input and return a result based on the request type.
    
    Args:
        json_string: JSON string containing the request
        
    Returns:
        JSON string containing the result
    """
    try:
        data = json.loads(json_string)

        if not isinstance(data, dict):
            return json.dumps({
                "success": False,
                "error": "Input must be a JSON object",
                "timestamp": "2025-06-14T00:00:00"
            })

        request_type = data.get("type", "unknown")
        
        if request_type == "math":
            return handle_math_request(data)
        elif request_type == "text":
            return handle_text_request(data)
        elif request_type == "data":
            return handle_data_request(data)
        elif request_type == "echo":
            return handle_echo_request(data)
        else:
            return json.dumps({
                "success": False,
                "error": f"Unknown request type: {request_type}",
                "available_types": ["math", "text", "data", "echo"],
                "timestamp": "2025-06-14T00:00:00"
            })
            
    except json.JSONDecodeError as e:
        return json.dumps({
            "success": False,
            "error": f"Invalid JSON: {str(e)}",
            "timestamp": "2025-06-14T00:00:00"
        })
    except Exception as e:
        return json.dumps({
            "success": False,
            "error": f"Processing error: {str(e)}",
            "timestamp": "2025-06-14T00:00:00"
        })


def handle_math_request(data) -> str:
    """Handle mathematical operations."""
    operation = data.get("operation", "")
    numbers = data.get("numbers", [])
    
    if not isinstance(numbers, list) or not numbers:
        return json.dumps({
            "success": False,
            "error": "Numbers array is required for math operations",
            "timestamp": "2025-06-14T00:00:00"
        })

    try:
        
        if operation == "add":
            result = sum(numbers)
        elif operation == "multiply":
            result = 1
            for num in numbers:
                result *= num
        elif operation == "mean":
            result = sum(numbers) / len(numbers)
        elif operation == "sqrt":
            if len(numbers) != 1:
                raise ValueError("sqrt operation requires exactly one number")
            result = math.sqrt(numbers[0])
        elif operation == "power":
            if len(numbers) != 2:
                raise ValueError("power operation requires exactly two numbers")
            result = math.pow(numbers[0], numbers[1])
        else:
            return json.dumps({
                "success": False,
                "error": f"Unknown math operation: {operation}",
                "available_operations": ["add", "multiply", "mean", "sqrt", "power"],
                "timestamp": "2025-06-14T00:00:00"
            })
        
        fact = math.factorial(5)

        return json.dumps({
            "success": True,
            "result": result,
            "operation": operation,
            "input_numbers": numbers,
            "timestamp": "2025-06-14T00:00:00",
            "path": "/workspaces/cpp-ideas"
        })
        
    except Exception as e:
        return json.dumps({
            "success": False,
            "error": f"Math operation failed: {str(e)}",
            "timestamp": "2025-06-14T00:00:00"
        })


def handle_text_request(data) -> str:
    """Handle text processing operations."""
    operation = data.get("operation", "")
    text = data.get("text", "")
    
    if not isinstance(text, str):
        return json.dumps({
            "success": False,
            "error": "Text field is required for text operations",
            "timestamp": "2025-06-14T00:00:00"
        })
    
    try:
        if operation == "uppercase":
            result = text.upper()
        elif operation == "lowercase":
            result = text.lower()
        elif operation == "reverse":
            result = text[::-1]
        elif operation == "word_count":
            result = len(text.split())
        elif operation == "char_count":
            result = len(text)
        elif operation == "capitalize":
            result = text.capitalize()
        else:
            return json.dumps({
                "success": False,
                "error": f"Unknown text operation: {operation}",
                "available_operations": ["uppercase", "lowercase", "reverse", "word_count", "char_count", "capitalize"],
                "timestamp": "2025-06-14T00:00:00"
            })
        
        return json.dumps({
            "success": True,
            "result": result,
            "operation": operation,
            "input_text": text,
            "timestamp": "2025-06-14T00:00:00"
        })
        
    except Exception as e:
        return json.dumps({
            "success": False,
            "error": f"Text operation failed: {str(e)}",
            "timestamp": "2025-06-14T00:00:00"
        })


def handle_data_request(data) -> str:
    """Handle data analysis operations."""
    operation = data.get("operation", "")
    dataset = data.get("dataset", [])
    
    if not isinstance(dataset, list):
        return json.dumps({
            "success": False,
            "error": "Dataset array is required for data operations",
            "timestamp": "2025-06-14T00:00:00"
        })
    
    try:
        if operation == "stats":
            if not dataset:
                raise ValueError("Dataset cannot be empty")
            
            numeric_data = [x for x in dataset if isinstance(x, (int, float))]
            if not numeric_data:
                raise ValueError("Dataset must contain numeric values")
            
            result = {
                "count": len(numeric_data),
                "sum": sum(numeric_data),
                "mean": sum(numeric_data) / len(numeric_data),
                "min": min(numeric_data),
                "max": max(numeric_data),
                "range": max(numeric_data) - min(numeric_data)
            }
        elif operation == "sort":
            result = sorted(dataset)
        elif operation == "unique":
            result = list(set(dataset))
        elif operation == "filter_numbers":
            result = [x for x in dataset if isinstance(x, (int, float))]
        else:
            return json.dumps({
                "success": False,
                "error": f"Unknown data operation: {operation}",
                "available_operations": ["stats", "sort", "unique", "filter_numbers"],
                "timestamp": "2025-06-14T00:00:00"
            })
        
        return json.dumps({
            "success": True,
            "result": result,
            "operation": operation,
            "input_dataset": dataset,
            "timestamp": "2025-06-14T00:00:00"
        })
        
    except Exception as e:
        return json.dumps({
            "success": False,
            "error": f"Data operation failed: {str(e)}",
            "timestamp": "2025-06-14T00:00:00"
        })


def handle_echo_request(data) -> str:
    """Echo the input data back with timestamp."""
    return json.dumps({
        "success": True,
        "result": "Echo successful",
        "echoed_data": data,
        "timestamp": "2025-06-14T00:00:00"
    })


if __name__ == "__main__":
    # Test the processor with sample data
    test_requests = [
        '{"type": "math", "operation": "add", "numbers": [1, 2, 3, 4, 5]}',
        '{"type": "text", "operation": "uppercase", "text": "hello world"}',
        '{"type": "data", "operation": "stats", "dataset": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]}',
        '{"type": "echo", "message": "This is a test"}'
    ]
    
    for i, request in enumerate(test_requests, 1):
        print(f"\n--- Test {i} ---")
        print(f"Input: {request}")
        result = process_json(request)
        print(f"Output: {result}")
