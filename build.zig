const std = @import("std");
const builtin = @import("builtin");

pub fn build(b: *std.Build) !void {

    const optimize = b.standardOptimizeOption(.{});

    var target_query = b.standardTargetOptionsQueryOnly(.{});
    if (target_query.cpu_arch == .x86_64) {
        target_query.cpu_model = .{.explicit = &std.Target.x86.cpu.x86_64_v4}; // deflate requires AVX-512 feature
    }
    const target = b.resolveTargetQuery(target_query);
    const triple_name = try target_query.zigTriple(b.allocator);
    const bin_name = try std.mem.concat(b.allocator, u8, &[_][]const u8{ "rebol", "-", triple_name });
    defer b.allocator.free(bin_name);

    const musl_sta = b.option(bool, "musl-static", "Link musl libc statically (default: false)") orelse false;

    const rebol = b.addExecutable(.{
        .name = bin_name,
        .linkage = if (target.result.abi == .musl and musl_sta) .static else .dynamic,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
            .pic = true,
        }),
    });
    rebol.pie = true;
    rebol.addCSourceFiles(.{ .root = b.path("src"), .files = src, .flags = cflags });
    rebol.addIncludePath(b.path("src/include"));
    rebol.root_module.addCMacro("_FILE_OFFSET_BITS", "64");
    rebol.root_module.addCMacro("_LARGEFILE64_SOURCE", "");
    rebol.root_module.addCMacro("ENDIAN_LITTLE", "");
    rebol.root_module.addCMacro("REB_EXE", "");
    rebol.root_module.addCMacro("REBOL_OPTIONS_FILE", "\"gen-config.h\"");
    rebol.root_module.addCMacro("UNICODE", "");

    switch (target.result.os.tag) {
        .linux => {
            rebol.root_module.addCMacro("TO_LINUX", "");
        },
        .freebsd => {
            rebol.root_module.addCMacro("TO_FREEBSD", "");
            rebol.root_module.addCMacro("__BSD_VISIBLE", "1");
        },
        .netbsd => {
            rebol.root_module.addCMacro("TO_NETBSD", "");
        },
        else => {},
    }

    b.installArtifact(rebol);
}

const cflags: []const []const u8 = &.{
    "-Wl,-z,stack-size=4194304",
};

const src: []const []const u8 = &.{
    "core/a-constants.c",
    "core/a-globals.c",
    "core/a-lib.c",
    "core/b-boot.c",
    "core/b-init.c",
    "core/c-do.c",
    "core/c-error.c",
    "core/c-frame.c",
    "core/c-function.c",
    "core/c-handle.c",
    "core/c-port.c",
    "core/c-task.c",
    "core/c-word.c",
    "core/d-crash.c",
    "core/d-dump.c",
    "core/d-print.c",
    "core/f-blocks.c",
    "core/f-deci.c",
    "core/f-dtoa.c",
    "core/f-enbase.c",
    "core/f-extension.c",
    "core/f-int.c",
    "core/f-math.c",
    "core/f-modify.c",
    "core/f-qsort.c",
    "core/f-random.c",
    "core/f-round.c",
    "core/f-series.c",
    "core/f-stubs.c",
    "core/l-scan.c",
    "core/l-types.c",
    "core/m-gc.c",
    "core/m-pools.c",
    "core/m-series.c",
    "core/n-control.c",
    "core/n-crypt.c",
    "core/n-data.c",
    "core/n-hash.c",
    "core/n-io.c",
    "core/n-loop.c",
    "core/n-math.c",
    "core/n-sets.c",
    "core/n-strings.c",
    "core/n-system.c",
    "core/p-checksum.c",
    "core/p-console.c",
    "core/p-crypt.c",
    "core/p-dir.c",
    "core/p-dns.c",
    "core/p-event.c",
    "core/p-file.c",
    "core/p-net.c",
    "core/s-cases.c",
    "core/s-crc.c",
    "core/s-file.c",
    "core/s-find.c",
    "core/s-make.c",
    "core/s-mold.c",
    "core/s-ops.c",
    "core/s-trim.c",
    "core/s-unicode.c",
    "core/t-bitset.c",
    "core/t-block.c",
    "core/t-char.c",
    "core/t-datatype.c",
    "core/t-date.c",
    "core/t-decimal.c",
    "core/t-event.c",
    "core/t-function.c",
    "core/t-gob.c",
    "core/t-handle.c",
    "core/t-image.c",
    "core/t-integer.c",
    "core/t-logic.c",
    "core/t-map.c",
    "core/t-money.c",
    "core/t-none.c",
    "core/t-object.c",
    "core/t-pair.c",
    "core/t-port.c",
    "core/t-string.c",
    "core/t-struct.c",
    "core/t-time.c",
    "core/t-tuple.c",
    "core/t-typeset.c",
    "core/t-utype.c",
    "core/t-vector.c",
    "core/t-word.c",
    "core/u-bincode.c",
    "core/u-compress.c",
    "core/u-iconv.c",
    "core/u-mbedtls.c",
    "core/u-parse.c",
    "core/u-xxhash.c",
    "core/deflate/lib/adler32.c",
    "core/deflate/lib/crc32.c",
    "core/deflate/lib/deflate_compress.c",
    "core/deflate/lib/deflate_decompress.c",
    "core/deflate/lib/gzip_compress.c",
    "core/deflate/lib/gzip_decompress.c",
    "core/deflate/lib/utils.c",
    "core/deflate/lib/zlib_compress.c",
    "core/deflate/lib/zlib_decompress.c",
    "core/deflate/lib/arm/cpu_features_arm.c",
    "core/deflate/lib/x86/cpu_features.c",
    "core/mbedtls/aes.c",
    "core/mbedtls/aesni.c",
    "core/mbedtls/asn1parse.c",
    "core/mbedtls/asn1write.c",
    "core/mbedtls/bignum.c",
    "core/mbedtls/bignum_core.c",
    "core/mbedtls/cipher.c",
    "core/mbedtls/cipher_wrap.c",
    "core/mbedtls/constant_time.c",
    "core/mbedtls/ctr_drbg.c",
    "core/mbedtls/dhm.c",
    "core/mbedtls/ecdh.c",
    "core/mbedtls/ecdsa.c",
    "core/mbedtls/ecp.c",
    "core/mbedtls/ecp_curves.c",
    "core/mbedtls/entropy.c",
    "core/mbedtls/entropy_poll.c",
    "core/mbedtls/gcm.c",
    "core/mbedtls/md.c",
    "core/mbedtls/md5.c",
    "core/mbedtls/oid.c",
    "core/mbedtls/platform.c",
    "core/mbedtls/platform_util.c",
    "core/mbedtls/rsa.c",
    "core/mbedtls/rsa_alt_helpers.c",
    "core/mbedtls/sha1.c",
    "core/mbedtls/sha256.c",
    "core/mbedtls/sha512.c",
    "os/dev-dns.c",
    "os/dev-net.c",
    "os/host-args.c",
    "os/host-device.c",
    "os/host-stdio.c",
    "os/posix/dev-event.c",
    "os/posix/dev-file.c",
    "os/posix/dev-stdio.c",
    "os/posix/host-lib.c",
    "os/posix/host-readline.c",

    "os/host-main.c", // executable start
};
