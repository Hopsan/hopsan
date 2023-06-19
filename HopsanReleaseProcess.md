## Update documentation release notes and version number

- Create a release-2.XX.0 branch based on Hopsan/master
- Update the Hopsan-release-notes.txt document  
  Create a commit
- Update the doc/userHCOMScripting.dox
  - Open local dev build of the Hopsan code you are going to release,  
  In HCOM run: "help doxygen"  
  - Copy and replace the relevant section in the dox file.
  - Create a commit (if anything changed)
- Find places where version number shall be updated "git grep -E 2\\.[0-9]{1\,}\\.[0-9]":
  - HopsanCore/include/HopsanCoreVersion.h
  - makeDebRelease.sh
  - makeWindowsRelease.py
  - packaging/mac-app/build.sh  
  - Create a commit with the updated version numbers
- Sign the commits
- Push the release branch and make a pull request against Hopsan/master, wait until green in CI systems
- Once OK, locally merge the pull request into Hopsan/master using "git merge --ff-only" to preserve signatures  
  Do not use the merge button on GitHub, then signatures will be lost
- Push "git push Hopsan master"

## Tag release

- Create annotated tag: git tag -a v2.XX.0 -m "Release 2.XX.0"
- Push tag: git push Hopsan v2.XX.0

## Build Windows 64-bit release

- Use a clean disposable working copy of hopsan
- Fetch: "git fetch --all --prune"
- Checkout master branch: "git checkout master"
- Reset to correct tag: "git reset --hard v2.XX.0"
- Update submodules: "git submodule update --init"
- Ensure clean: "git clean -ffdx"
- Open a CMD prompt and go into the dependencies subdirectory
- Download all dependencies with: "download-dependencies.py --all"
- Build all dependencies using the setupAll.bat script
- Go to the root directory
- Run makeWindowsRelease.py
- Answer the questions (Choose NO for 32-bit, No need for MSVC binaries)
- Resulting packages will be in output64

## Build Windows 32-bit release (optional)

- Use a clean disposable working copy of hopsan (not same as for 64-bit)
- Fetch: "git fetch --all --prune"
- Checkout master branch: "git checkout master"
- Reset to correct tag: "git reset --hard v2.XX.0"
- Update submodules: "git submodule update --init"
- Ensure clean: "git clean -ffdx"
- Open a CMD prompt and go into the dependencies subdirectory
- In setHopsanBuildPaths.bat, Change hopsan_arch=x64 to hopsan_arch=x86
- Commit this change but DO NOT push it
- Download all dependencies with: "download-dependencies.py --all"
- Build all dependencies using the setupAll.bat script
- Go to the root directory
- Run makeWindowsRelease.py
- Answer the questions (Choose YES for 32-bit, No need for MSVC binaries)
- Resulting packages will be in output32


## Build Linux deb packages

- Use a clean disposable working copy of hopsan
- Fetch: "git fetch --all --prune"
- Checkout master branch: "git checkout master"
- Reset to correct tag: "git reset --hard v2.XX.0"
- Update submodules: "git submodule update --init"
- Ensure clean: "git clean -ffdx"
- Run ./makeDebRelease.sh
- Answer the questions
- Wait for the build to finish, verify that all packages were buildOK (shown at the end)
- The resulting debs can be found in ouput_deb/debs

## Generate signed checksums of binary packages

- Copy all release packages (both Windows and Linux) into the same directory
- Copy DebReadme.txt and Hopsan-release-notes.txt into the same directory
- Generate sha256 sums: "sha256sum * > hopsan-v2.XX.0-sha256sum.txt"
- Sign the shasum file: "gpg --detach-sign hopsan-v2.XX.0-sha256sum.txt"

## Create release on Github

- Create a draft release and choose the tag it should represent
- Copy from Hopsan-release-notes.txt to the release description
- Change the ----- Bug ----- sections to ### Bug etc.
- Upload the files (including shasums, signature and txt files)
- Publish the release

## Snap

You may need to install: "snap install snapcraft review-tools"

- Use a clean disposable hopsan clone
- Reset to correct tag: "git reset --hard v2.XX.0"
- Update submodules: "git submodule update --init"
- Ensure clean: "git clean -ffdx"
- Run ./makeSnapRelease.sh
- Answer the questions
- If you are on a desktop:
  - Run: "multipass delete snapcraft-hopsan"
  - Run: "multipass purge"
  - Run: "snapcraft snap"
- If you are inside a container:
  Run "snapcraft snap --use-lxd"
- Push release:
  "snapcraft upload --release=stable hopsan_VERSION_amd64.snap"

## Flatpak

- Fork flathub/com.github.hopsan.Hopsan.git
- Clone your fork
- Pull flathub/master (ensure you are up-to-date)
- Create a release branch based on master: "git checkout -b release-2.XX.0 flathub/master"
- Ensure no local modifications: "git status"
- From hopsan-code/dependencies run "./download-dependencies.py --output-flatpak"  
  You need to manually replace the dependencies text in com.github.hopsan.Hopsan.json.in (if any dependency has changed)
- Run makeFlatpakRelease.sh
- Answer questions
  - Base version: 2.XX.0 (should match tag, without initial v)
  - Revision    : The date and time stamp, see deb release packages, they have it in the file name, Ex: "20220711.1354"
  - Commit hash : Full commit hash (tagged commit)
- Commit the changes
- Sign the commit
- Push the release branch to your fork: "git push -u peter release-2.XX.0"
- Create a pull request against: flathub/master
- A test build should be started, if not call "bot, build" in the PR conversation
- Once the build is OK
  - You will get an email with a link to a test version, test it to make sure that it works (optional)
  - Merge do a local "git merge --ff-only" to master (to preserve signatures)
  - Push to flathub repo master branch: "git push flathub master"
  - The flatpak will automatically be published within a few hours
    You can also log in to the web interface to for earlier publication

## Update autoupdate feed

- Clone or pull hopsan-news
- Go into releases
- Run the add-new-release.sh script (point it to the directory with packages you previously uploaded)
- Verify the changes to hopsan-releases.xml
  One new block should have been added
- Add and Commit "git add -u; git commit -m "Release 2.XX.0"
- Sign the commit
- Push to github

## Update online documentation

- Clone Hopsan/documentation.git
- Remove all files except the .git sub-directory  
  Use the system default rm command (not git rm)
- Build the documentation in a clean hopsan working copy
- Copy all files from doc/html into the documentation working copy
- Add all changes: "git add ."
- Verify with "git status"
- Commit the changes
- Sign the commit
- Push to documentation repo: "git push Hopsan master"
