# Build Instructions
> *If you are experienced with building software libraries and using version control, then you may want to read the Advanced section first, which describes alternative ways of building G3D to reduce your initial download size or deploy precompiled G3D binaries across multiple developers.*

Use `buildg3d help` for information on the build system

## Local
- Use: You're developing local to the same machine that you're building G3D on.
- You might want to single-step from your code into G3D source code when debugging.
- You want fast incremental builds of G3D when you update to get the latest patches.
- Set your include and data directories to the source tree as described below and use `buildg3d local` to compile the library

## Distrib
- Use: You're building the library for distribution as a binary to other developers, such as students in a university course.
- Set your include and data directories to the source tree as described below and use `buildg3d distrib` to compile the library. Have your other developers set their environment as described in the **Dependent Developers** section.

---

## Source Code
You can [browse the repository](https://sourceforge.net/p/g3d/code/HEAD/tree/) from the top of tree or revision history.

### Full Library

You can check out the entire library (10 GB) anonymously using:
```bash
svn co svn://svn.code.sf.net/p/g3d/code/ g3d
```

...or you can check out just the subdirectories that you need. For example, `svn://svn.code.sf.net/p/g3d/code/G3D10` is just the library source code, without the large asset packs.

If you use the `https://` protocol and receive errors such as:
```
svn: E175002: Unexpected HTTP status 504 'Gateway Time-out'
```
just run:
```bash
svn cleanup
svn update
```
to continue. This should not happen with the `svn://` protocol.

For reference when performing a partial checkout, the top-level directory structure is:

```
***********************************************************************
*   g3d                                                               *
*    |                                                                *
*    +-- G3D10                                                        *
*    |                                                                *
*    +-- data10                                                       * 
*    |     |                                                          * 
*    |     +-- common                                                 * 
*    |     +-- game                                                   * 
*    |     '-- research                                               * 
*    |                                                                *
*    '-- www                                                          *
***********************************************************************
```

### Minimum Download

G3D includes common asset files for making games, demos, and research projects, as well as for its own sample programs and automated tests. The asset packs are large, so they are in a separate subdirectory tree from the source code.

The minimal installation (about 300 MB) to be able to run the sample programs is:
```bash
svn co --depth immediates https://svn.code.sf.net/p/g3d/code/ g3d
svn up --set-depth immediates g3d/data10
svn up --set-depth infinity g3d/G3D10
svn up --set-depth infinity g3d/data10/common
```
Later, if you want the additional data packs, run:
```bash
svn up --set-depth infinity data10/research
svn up --set-depth infinity data10/game
```

---

## macOS

> **Note:** Because macOS video drivers are produced by Apple as part of the operating system and Apple has deprecated open graphics standards, they lag behind the features on Windows by several years. The G3D build on macOS uses legacy support to emulate these features for you.

### Requirements

1. OS X 10.12.2 or later
2. [Python 3.10.4](https://www.python.org/downloads/release/python-3104/)
3. LLVM 8.0.0 and clang++, from [Xcode](https://developer.apple.com/xcode/)
4. [yasm 1.3](http://www.macports.org/ports.php?by=name&substr=yasm) via MacPorts
5. [Exuberant-ctags](http://www.macports.org/ports.php?by=library&substr=ctags) via MacPorts
6. A directory for G3D, such as `~/g3d`, and set a `g3d` environment variable to its path.
7. Consider joining the [g3d-users](https://groups.google.com/forum/#!forum/g3d-users) Google group to be notified of new features and receive support.

### Configure & Build

```bash
export g3d=### your choice! Frequently: /Users/$USER/g3d ###
export G3D10DATA=$g3d/G3D10/data-files:$g3d/data10/common:$g3d/data10/game:$g3d/data10/research
export INCLUDE=$g3d/G3D10/external/glew.lib/include:$g3d/G3D10/external/glfw.lib/include:$g3d/G3D10/external/civetweb.lib/include:$g3d/G3D10/external/qrencode.lib/include:$g3d/G3D10/external/zlib.lib/include:$g3d/G3D10/external/physx/include:$g3d/G3D10/G3D-base.lib/include:$g3d/G3D10/G3D-gfx.lib/include:$g3d/G3D10/G3D-app.lib/include:$g3d/G3D10/external/tbb/include:$g3d/G3D10/external/glslang/glslang:$INCLUDE
export LIBRARY=$g3d/G3D10/build/lib:$g3d/G3D10/build/bin:$LIBRARY
export PATH=$PATH:$g3d/G3D10/bin:$g3d/G3D10/build/bin
```

1. Add the above environment variable assignments to your `~/.bashrc` file.
2. Ensure that your `~/.bash_profile` contains the line `source ~/.bashrc`.
3. Check out the source tree with:
   ```bash
   svn co svn://svn.code.sf.net/p/g3d/code/ $g3d
   ```
4. In the `$g3d/G3D10` directory, run:
   ```bash
   ./buildg3d lib data tools
   ```
5. For more details on creating your own programs, see this [Guide](https://casual-effects.com/g3d/G3D10/build/manual/guidenewproject.html).
Once you've built G3D, compile any of the sample programs or your own by running `icompile` in the project directory. The `icompile` script is included with G3D, so you don't need to write Makefiles unless you want to.

---

## Linux

### Arch Linux

Install the following packages using pacman:

```bash
sudo pacman -Syu subversion clang doxygen xorg-server base-devel gtk3 universal-ctags yasm libzip freeglut lldb
```

_Listing: Arch Linux packages required for building G3D._

---

### NixOS

```nix
{
  environment.systemPackages = with pkgs; [
    subversion
    clang
    doxygen
    xorg.xorgproto
    gtk3
    universal-ctags
    yasm
    libzip
    freeglut
    lldb
  ];
}
```

_Listing: NixOS packages required for building G3D._

### Steps to Configure & Build

1. **Install Required Packages:**  
   Use the above command (or your distribution's equivalent) to install the necessary packages.

2. **Set Environment Variables:**  
   Add the environment variable assignments from the listing above to your `~/.bashrc` file and ensure your `~/.bash_profile` sources `~/.bashrc`.

3. **Checkout Source Tree:**  
   ```bash
   svn co svn://svn.code.sf.net/p/g3d/code/ $g3d
   ```

4. **Build G3D:**  
   ```bash
   ./buildg3d lib data tools
   ```

5. **New Project Guide:**  
   For information on creating your own programs that use G3D, refer to the [G3D New Project Guide](https://casual-effects.com/g3d/G3D10/build/manual/guidenewproject.html).

## Sample Programs
Run from the sample project directory (can use `icompile`).
