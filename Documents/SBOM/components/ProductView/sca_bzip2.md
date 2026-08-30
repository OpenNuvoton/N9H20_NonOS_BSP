# bzip2 Component Declaration

## Component Information

- Name: bzip2
- SBOM Version: 1.0.2
- Upstream Distribution: bzip2-1.0.2
- Component Type: Library
- Scope: ProductView
- Integration: Vendored source
- Modification Status: Modified
- License: LicenseRef-bzip2-1.0.2
- Author and Supplier: Julian Seward

## Source Location

- `Loaders/SpiLoader_gzip/ziplib/bzlib.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib.h`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_private.h`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_decompress.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_huffman.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_crctable.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_randtable.c`

## Version Evidence

The vendored `bzlib.h` and `bzlib.c` state that the files are modified
versions from the bzip2-1.0.2 distribution.

The embedded library notice also identifies the bzip2/libbzip2 1.0
release family. The explicit upstream distribution statement is used
as the SBOM version evidence.

## Modification Evidence

The source files explicitly identify themselves as modified versions
of files from the bzip2-1.0.2 distribution.

The header also contains configuration changes for the U-Boot
environment, including:

- `BZ_NO_STDIO`
- `BZ_NO_COMPRESS`

These settings indicate an embedded decompression-oriented integration.

## License Evidence

The bzip2 license notice is contained in:

- `Loaders/SpiLoader_gzip/ziplib/bzlib.h`
- `Loaders/SpiLoader_gzip/ziplib/bzlib.c`

The license permits redistribution and use in source and binary forms,
with or without modification, subject to the stated conditions.

The SPDX license identifier used by the SBOM is:

- `LicenseRef-bzip2-1.0.2`

The SPDX identifier is the license identifier and does not represent
the component version.

## Usage

The component provides bzip2 decompression support for:

- `Loaders/SpiLoader_gzip`

The component is integrated with the compressed-image boot
implementation.

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/ProductView/sca_bzip2.json`
- Package URL: `pkg:generic/bzip2@1.0.2`
- Component Origin: Third-party
- Required Scope: ProductView

## Security Remediation

- CVE: CVE-2019-12900
- Status: Source backport applied; build, SAST, and negative-test verification pending
- Modified File: `Loaders/SpiLoader_gzip/ziplib/bzlib_decompress.c`
- Remediation: Reject `nSelectors` values greater than `BZ_MAX_SELECTORS` before accessing the fixed-size `selectorMtf[]` and `selector[]` arrays.
- Component Version: The upstream component identity remains bzip2 1.0.2.
- Default Runtime Linkage: Not demonstrated in the inspected GCC and KEIL project metadata.
- Distribution Assessment: The vendored source remains distributed with the BSP, so the source-level remediation is retained for future or customer integration.
- Verification Completed: Guard uniqueness, guard ordering, array-bound consistency, LF/no-BOM preservation, raw/filtered Git byte consistency, and canonical hash reproduction.
- Verification Pending: Target compilation, SAST, malformed-selector negative testing, and normal decompression regression testing.

## Review Result

- Classification: Third-Party OSS
- Status: Conditional; security verification pending

## Canonical Source Closure Hash

- Algorithm: SHA-256
- Value: `04c0dfd07929844a7155b382a202903c5e538e0e0e43fb259c554015b78654bd`
- Hash Method: Sorted UTF-8 normalized relative paths followed by exact file bytes
- Coverage: Seven bzip2 source and header files listed in this evidence
