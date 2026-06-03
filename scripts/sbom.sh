#!/bin/bash
# Generate a simple SBOM for z_jail
set -euo pipefail
echo "{
  \"bomFormat\": \"Z-Jail\",
  \"dependencies\": []
}"
