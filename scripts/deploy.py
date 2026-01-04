#!/usr/bin/env python3
"""Deployment script for creating and pushing version tags."""

import subprocess
import sys
import re

GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
NC = "\033[0m"

def run_git(cmd):
    result = subprocess.run(f"git {cmd}", shell=True, capture_output=True, text=True)
    return result.returncode, result.stdout.strip()


def exit_error(message):
    print(f"{RED}[ERROR]{NC} {message}")
    sys.exit(1)

def print_info(message):
    print(f"{GREEN}[INFO]{NC} {message}")

def parse_semver(tag):
    match = re.match(r"^v?(\d+)\.(\d+)\.(\d+)$", tag)
    return tuple(map(int, match.groups())) if match else None


def validate_increment(last_tag, new_tag):
    new_ver = parse_semver(new_tag)
    if not new_ver:
        exit_error(f"Invalid version format: {new_tag}. Expected: v1.2.3")

    last_ver = parse_semver(last_tag)
    if not last_ver:
        return

    if new_ver <= last_ver:
        exit_error(f"New version {new_tag} must be greater than {last_tag}")

    total_diff = sum(n - l for n, l in zip(new_ver, last_ver))
    if total_diff != 1:
        l_major, l_minor, l_patch = last_ver
        exit_error(
            f"Invalid increment from {last_tag} to {new_tag}.\n"
            f"Valid options:\n"
            f"  - Major: v{l_major + 1}.0.0\n"
            f"  - Minor: v{l_major}.{l_minor + 1}.0\n"
            f"  - Patch: v{l_major}.{l_minor}.{l_patch + 1}"
        )


def check_uncommitted():
    code, output = run_git("status --porcelain")
    if output:
        exit_error("Uncommitted changes found. Commit or stash before deploying.")


def check_commit_pushed():
    code, output = run_git("log @{u}..")
    if code != 0:
        exit_error("No tracking branch found. Push your commits first.")
    if output:
        exit_error("Current commit not pushed. Push your commits before deploying.")


def get_current_tag():
    code, current_tag = run_git("describe --exact-match --tags 2>/dev/null")
    return current_tag if code == 0 else None


def get_last_tag():
    code, last_tag = run_git("describe --tags --abbrev=0 2>/dev/null")
    if code == 0:
        print(f"Last tag: {last_tag}")
        return last_tag
    else:
        print("No tags found.")
        return None


def prompt_and_validate_tag(last_tag):
    print("Current commit has no tag.")
    new_tag = input("\nEnter new tag (e.g., v1.0.0): ").strip()
    if not new_tag:
        exit_error("No tag provided.")

    if not new_tag.startswith("v"):
        new_tag = f"v{new_tag}"
        print(f"Normalized to: {new_tag}")

    validate_increment(last_tag, new_tag)
    return new_tag


def create_tag(tag):
    print(f"Creating tag {tag}...")
    run_git(f"tag {tag}")


def push_tag(tag):
    print(f"Pushing tag {tag}...")
    run_git(f"push origin {tag}")
    print_info("Deployment triggered successfully")


def main():
    print_info("Checking repository status...")

    check_uncommitted()
    check_commit_pushed()

    current_tag = get_current_tag()
    if current_tag:
        print(f"Current commit is already tagged as {current_tag}")
        push_tag(current_tag)
        return

    last_tag = get_last_tag()
    new_tag = prompt_and_validate_tag(last_tag)
    create_tag(new_tag)
    push_tag(new_tag)


if __name__ == "__main__":
    main()
