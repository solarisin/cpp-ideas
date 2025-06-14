#!/usr/bin/env python3
"""
Diagnostic script to understand why math import fails at module level
in embedded Python but works at function level.
"""

import sys
import json

def diagnostic_info():
    """Get diagnostic information about the Python environment"""
    return {
        "python_version": sys.version,
        "sys_path": sys.path,
        "builtin_module_names": sys.builtin_module_names,
        "modules_loaded": list(sys.modules.keys())[:20],  # First 20 modules
    }

# Try importing at module level with error handling
math_import_error = None
math_available_at_module_level = False

try:
    import math
    math_available_at_module_level = True
    math_sqrt_test = math.sqrt(4)  # Test if it actually works
except Exception as e:
    math_import_error = str(e)

def test_function_level_import():
    """Test importing math at function level"""
    try:
        import math
        return {"success": True, "sqrt_4": math.sqrt(4)}
    except Exception as e:
        return {"success": False, "error": str(e)}

def process_json(json_str):
    """Main function for testing"""
    try:
        data = json.loads(json_str)
        
        result = {
            "diagnostic_info": diagnostic_info(),
            "math_at_module_level": {
                "available": math_available_at_module_level,
                "error": math_import_error
            },
            "math_at_function_level": test_function_level_import(),
            "request_data": data
        }
        
        return json.dumps(result, indent=2)
        
    except Exception as e:
        return json.dumps({
            "success": False,
            "error": str(e),
            "diagnostic_info": diagnostic_info()
        })

if __name__ == "__main__":
    test_input = '{"action": "diagnose"}'
    print(process_json(test_input))
