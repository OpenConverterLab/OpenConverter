#!/bin/bash

# OpenConverter Gatekeeper Fix Script
# This script removes the quarantine attribute from OpenConverter.app
# to bypass macOS Gatekeeper restrictions

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}    OpenConverter - Gatekeeper Fix Script${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""

# Determine the app path
if [ -f "$0" ]; then
    # Script is inside the app bundle
    SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd)"
    APP_PATH="$(dirname "$(dirname "$(dirname "$SCRIPT_PATH")")")"
else
    # Fallback to /Applications
    APP_PATH="/Applications/OpenConverter.app"
fi

echo -e "${YELLOW}App location: ${APP_PATH}${NC}"
echo ""

# Check if app exists
if [ ! -d "$APP_PATH" ]; then
    echo -e "${RED}Error: OpenConverter.app not found at ${APP_PATH}${NC}"
    echo -e "${YELLOW}Please make sure OpenConverter.app is installed in /Applications${NC}"
    exit 1
fi

# Check if quarantine attribute exists
if xattr "$APP_PATH" 2>/dev/null | grep -q "com.apple.quarantine"; then
    echo -e "${YELLOW}Quarantine attribute detected. Removing...${NC}"
    echo ""
    echo -e "${YELLOW}This will require administrator privileges.${NC}"
    echo -e "${YELLOW}Please enter your password when prompted.${NC}"
    echo ""

    # Remove quarantine attribute
    if sudo xattr -r -d com.apple.quarantine "$APP_PATH"; then
        echo ""
        echo -e "${GREEN}✓ Success! Quarantine attribute removed.${NC}"
        echo -e "${GREEN}✓ You can now open OpenConverter normally.${NC}"
        echo ""
    else
        echo ""
        echo -e "${RED}✗ Failed to remove quarantine attribute.${NC}"
        echo -e "${YELLOW}Please try running this command manually:${NC}"
        echo -e "  sudo xattr -r -d com.apple.quarantine \"$APP_PATH\""
        echo ""
        exit 1
    fi
else
    echo -e "${GREEN}✓ No quarantine attribute found.${NC}"
    echo -e "${GREEN}✓ OpenConverter should open normally.${NC}"
    echo ""
fi

echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}    Done! Enjoy using OpenConverter! 🎉${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${NC}"
echo ""
