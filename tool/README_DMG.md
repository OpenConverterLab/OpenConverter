# DMG Creation - Simple Approach

## Overview

We use a **simple shell script** with `hdiutil` to create clean macOS DMG installers. No Node.js dependencies, no hidden files, just the app and Applications link.

## Why This Approach?

✅ **Simple** - Pure shell script, no external dependencies
✅ **Clean** - No `.background` folder, no `.DS_Store`, no hidden files
✅ **Fast** - Direct `hdiutil` command, no overhead
✅ **Reliable** - No AppleScript, no timing issues
✅ **Minimal** - Just the app and Applications link

## Usage

```bash
# Run from anywhere with path to .app
./tool/create_dmg_simple.sh [path_to_app]

# Examples:
./tool/create_dmg_simple.sh                                    # Default: build/OpenConverter.app
./tool/create_dmg_simple.sh build/OpenConverter.app            # Relative path
./tool/create_dmg_simple.sh /path/to/MyApp.app                 # Absolute path
./tool/create_dmg_simple.sh ../other/MyApp                     # Without .app extension

# Run from any directory
cd /tmp
/path/to/OpenConverter/tool/create_dmg_simple.sh /path/to/MyApp.app
```

**Features:**
- ✅ Accepts **absolute** or **relative** paths to `.app` file
- ✅ Automatically adds `.app` extension if not provided
- ✅ Creates DMG in the **same directory** as the app
- ✅ Can be run from **any directory**
- ✅ DMG name matches app name (e.g., `MyApp.app` → `MyApp.dmg`)

That's it! No dependencies to install, no configuration needed.

## How It Works

The script follows a simple 6-step process:

1. **Create staging folder** - Temporary directory for DMG contents
2. **Copy app** - Copy app to staging folder
3. **Bundle fix script** - Add `fix_gatekeeper.sh` inside the app bundle
4. **Add instructions** - Copy installation instructions to DMG
5. **Create Applications link** - Symlink to `/Applications`
6. **Create DMG** - Use `hdiutil` to create compressed DMG from staging folder

## What You Get

When users open the DMG, they see:
- `OpenConverter.app` - The application
- `Applications` - Link to Applications folder
- ` How to Install.txt` - **Clear installation instructions with copy-paste commands** ⭐

**Bonus:** The `fix_gatekeeper.sh` script is bundled inside the app at:
`OpenConverter.app/Contents/Resources/fix_gatekeeper.sh`

**No hidden files, no background, no custom icon - just clean and simple!**

## Gatekeeper Fix

macOS may prevent the app from opening with an "unidentified developer" error. We provide **two** solutions:

### Method 1: Simple Terminal Command (Recommended) ⭐

The installation instructions include a **clear, copy-paste command** that users can run in Terminal:

```bash
sudo xattr -r -d com.apple.quarantine /Applications/OpenConverter.app
```

**Why this approach?**
- ✅ **No Gatekeeper warnings** - Plain text instructions can't be quarantined
- ✅ **Simple and direct** - One command, copy-paste, done
- ✅ **Clear instructions** - Step-by-step guide in `📖 How to Install.txt`
- ✅ **Works reliably** - No executable files to be blocked
- ✅ **Professional** - Standard approach used by many macOS apps

**The instructions include:**
- How to open Terminal (with keyboard shortcut)
- The exact command to copy-paste
- What to expect (password prompt, no visible typing)
- Success confirmation

### Method 2: Bundled Script (Alternative)

A `fix_gatekeeper.sh` script is bundled inside the app at:
```
OpenConverter.app/Contents/Resources/fix_gatekeeper.sh
```

Users can run it from Terminal:
```bash
/Applications/OpenConverter.app/Contents/Resources/fix_gatekeeper.sh
```

This provides an interactive experience with colored output and validation.

### What Does This Do?

The quarantine attribute is set by macOS on apps downloaded from the internet. Removing it allows the app to run without Gatekeeper restrictions. This is safe and only affects OpenConverter.

## CI/CD Integration

The GitHub Actions workflow automatically:
1. Runs `create_dmg_simple.sh`
2. Creates `OpenConverter_macOS_aarch64.dmg`

See `.github/workflows/release.yaml` for details.

## Comparison with Other Approaches

| Aspect | This Approach | appdmg | Complex Shell Script |
|--------|--------------|---------|---------------------|
| **Dependencies** | None (just hdiutil) | Node.js, npm | hdiutil, osascript, SetFile |
| **Hidden Files** | None | `.background`, `.DS_Store` | `.background`, `.fseventsd`, `.DS_Store` |
| **Complexity** | ~65 lines bash | ~20 lines JSON + npm | ~200 lines bash + AppleScript |
| **Reliability** | ✅ Consistent | ✅ Consistent | ⚠️ AppleScript timing issues |
| **Speed** | ✅ Fast | Medium | Slow |

## Troubleshooting

### Permission denied

```bash
chmod +x tool/create_dmg_simple.sh
```

### App not found

Make sure OpenConverter.app exists in `build/` directory:

```bash
ls -la build/OpenConverter.app
```

## Example Output

```
Creating DMG (simple approach)...
Step 1: Creating staging folder...
Step 2: Copying app...
Step 3: Bundling Gatekeeper fix script...
  ✓ fix_gatekeeper.sh bundled in app
Step 4: Adding installation instructions...
Step 5: Creating Applications link...
Step 6: Creating DMG...
................................................
created: /path/to/OpenConverter.dmg

✓ DMG created successfully!
  File: OpenConverter.dmg
  Size: 48M
```

## Verification

To verify the DMG contents:

```bash
# Mount the DMG
open build/OpenConverter.dmg

# Check all files (including hidden)
ls -a "/Volumes/Install OpenConverter/"
# Output: .  ..  📖 How to Install.txt  Applications  OpenConverter.app

# Check visible files only
ls "/Volumes/Install OpenConverter/"
# Output: 📖 How to Install.txt  Applications  OpenConverter.app

# Verify fix script is bundled
ls -la "/Volumes/Install OpenConverter/OpenConverter.app/Contents/Resources/fix_gatekeeper.sh"
# Output: -rwxr-xr-x ... fix_gatekeeper.sh

# Test the fix script
"/Volumes/Install OpenConverter/OpenConverter.app/Contents/Resources/fix_gatekeeper.sh"
# Output: Shows Gatekeeper fix status
```

Perfect! No `.background`, no `.DS_Store`, no hidden files! ✅
