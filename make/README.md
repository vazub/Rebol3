# How to build Rebol 3

## Option 1: Using Siskin build tool

### Pre-requisites

1. Clone the Rebol 3 Repo if you haven't already: `git clone https://github.com/Oldes/Rebol3.git`
2. Download the [Siskin build tool](https://github.com/Siskin-framework/Builder/releases/latest) for your platform
3. Place the Siskin binary in the root of the repo (or into one of the `PATH` folders)

macOS users on 10.15+ may need to remove the quarantine setting from the Siskin builder tool after download using:
```
xattr -d -r com.apple.quarantine /path/to/Siskin
```

### Build walkthrough

1. Enter the `Rebol3` directory you cloned above: `cd ~/Source/Rebol3`
2. Start the build tool: `./Siskin make/rebol3.nest`
3. You should see an interactive CLI tool with a series of numbered options
4. Choose the package you would like to build or test by entering a number and pressing enter
5. Siskin will build Rebol and display detailed output of its process
6. Type `q` to exit Siskin

## Option 2: Using Zig compiler toolchain

> [!IMPORTANT]
> This build process is still in testing phase. It works overall, but is considered bleeding-edge and might require a bit more time to mature. Additionally, some build targets provided by the original flow are not supported yet.

### Pre-requisites

1. Get the latest Zig toolchain package. The easiest way is to download it [pre-compiled](https://ziglang.org/download/) for your system, unpack it and make sure the PATH is updated to find the `zig` binary.
2. (optional) If you want to contribute to the project development, get the latest [pre-compiled](https://github.com/Oldes/Rebol3/releases) `rebol` binary for your system as well, and put it in your PATH. This binary is needed to re-generate some files and headers for the build process itself, after changes to the source files take place.
2.1 The already pre-generated files in the repo are processed with `spec-x64-linux-musl.reb` file. You can find the supported `spec-*.reb` files in the `make` directory. To re-generate files and headers for your target, run the following command from there:

`rebol gen.r3 <your_target_specfile>`

### Build walkthrough

1. To build `rebol` binary, run the following command from your project root directory, replacing the values in brackets with your desired options (see next sections):

`zig build -Dtarget=<your_zig_target> -Doptimize=<your_optimization_preference>`

If you target *musl* libc ABI for Linux, you might additionally decide to build a fully statically-linked binary, by adding an additional option. Otherwise, *musl* libc will be linked dynamically by default:

`zig build -Dtarget=<your_zig_target> -Doptimize=<your_optimization_preference> -Dmusl-static=true`

> [!WARNING]
> Rebol project and community provide multiple downloadable extensions. Some of them are in form of shared C libraries, that are dynamically loaded at runtime, using a *dlopen* mechanism. If you statically link the `rebol` executable, you would not typically be able to load those extensions, and would rather have to compile them in when building Rebol itself, if so desired.

### Currently supported Zig build targets

- x86_64-linux-musl
- x86_64-linux-gnu
- x86_64-freebsd
- x86_64-netbsd
- aarch64-linux-musl
- aarch64-linux-gnu
- riscv64-linux-musl
- riscv64-linux-gnu

### Optimization options

These are standard Zig options, you can check out the details [here](https://ziglang.org/documentation/master/#Build-Mode):

- Debug
- ReleaseSafe
- ReleaseFast
- ReleaseSmall
