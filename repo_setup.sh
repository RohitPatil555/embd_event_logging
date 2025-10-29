# SPDX-License-Identifier: MIT | Author: Rohit Patil
#!/bin/bash

# remove all unwanted hook and force to use pre-commit hook
rm -rf .git/hooks/*

pre-commit install --install-hooks
pre-commit install -t commit-msg
git config --local commit.template .github/commit-template.txt
echo "✅ Git hooks installed using pre-commit"
