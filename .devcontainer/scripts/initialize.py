import os
import sys
from pathlib import Path


# In either Windows or linux, determine if the environment variable 'DEV_CACHE_DIR' is set, if not, exit(1)

def fail():
    print("ERROR: Required DEV_CACHE_DIR environment variable is not set!", file=sys.stderr)
    print("       This variable must be set to an existing directory.", file=sys.stderr)
    sys.exit(1)

try:
    dev_cache_dir = Path(os.environ['DEV_CACHE_DIR'])
    if dev_cache_dir is None:
        fail()
    if not (dev_cache_dir.exists() and dev_cache_dir.is_dir()):
        fail()
    print(f"Success: DEV_CACHE_DIR is set to '{dev_cache_dir}'.")
except:
    fail()