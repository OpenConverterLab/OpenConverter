# OpenConverter Usage Guide for macOS Users

Thank you for downloading **OpenConverter**! Please follow the steps below to resolve any issues if the app does not open after transferring it out of the DMG file.

---

## Steps to Fix Opening Issues

1. **Download and Transfer the App:**
   - Download the OpenConverter DMG file.
   - Open the DMG and drag the **OpenConverter.app** to your preferred folder, such as the Applications folder.

2. **Resolve Gatekeeper Restrictions:**
   If macOS prevents the app from opening due to an unidentified developer error, you need to bypass Apple's Gatekeeper restrictions.

3. **Open Terminal:**
   - Open the **Terminal** app from your macOS utilities.

4. **Run the Following Commands:**

   - Remove the quarantine attribute:
     ```bash
     sudo xattr -r -d com.apple.quarantine OpenConverter.app
     ```
     This command removes the quarantine flag set by macOS for apps downloaded from the internet.

   - Re-sign the app:
     ```bash
     codesign --force --deep --sign - OpenConverter.app
     ```
     This command ensures that macOS recognizes the app as valid, even without a registered developer signature.

5. **Open the App:**
   - Double-click **OpenConverter.app** to launch it.
   - If macOS prompts for confirmation, click **Open**.

---

## Note
- These commands require administrative privileges. You will be asked to enter your macOS password when running `sudo`.
- If you experience any further issues, feel free to contact us through the project's repository or support channel.

Enjoy using **OpenConverter**! 🎉
