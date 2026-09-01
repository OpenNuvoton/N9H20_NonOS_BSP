# bzip2 Evidence

## Component Identification
- Component: bzip2
- SBOM Version: 1.0.2
- Upstream Distribution: bzip2-1.0.2
- Scope: ProductView
- Classification: Third-Party OSS
- Integration: Modified vendored source
- License: LicenseRef-bzip2-1.0.2

## Evidence Location
- Loaders/SpiLoader_gzip/ziplib/bzlib.c
- Loaders/SpiLoader_gzip/ziplib/bzlib.h
- Loaders/SpiLoader_gzip/ziplib/bzlib_private.h
- Loaders/SpiLoader_gzip/ziplib/bzlib_decompress.c
- Loaders/SpiLoader_gzip/ziplib/bzlib_huffman.c
- Loaders/SpiLoader_gzip/ziplib/bzlib_crctable.c
- Loaders/SpiLoader_gzip/ziplib/bzlib_randtable.c

## Version Evidence
The vendored `bzlib.h` and `bzlib.c` identify modified files from the bzip2-1.0.2 distribution. Historical 0.9.0 change-history entries are not used as the component version.

## Modification and License Evidence
The source is modified for embedded integration and defines `BZ_NO_STDIO` and `BZ_NO_COMPRESS`. The bzip2 license notice is contained in `bzlib.h` and `bzlib.c`. The SBOM license expression is `LicenseRef-bzip2-1.0.2`.

## Usage and Build-Scope Evidence
The distributed source contains bzip2 decompression capability intended for compressed-image boot integration. No external bzip2 API usage, exact default-project inclusion, or bzip2 build-metadata inclusion was demonstrated in the inspected GCC and KEIL configurations. The observed default runtime caller uses zlib APIs. Accordingly, source capability and distribution are recorded separately from default-build inclusion.

## Security Review
- CVE-2019-12900: `patched-by-source-backport`. The selector-count upper-bound guard is present before fixed-size selector array access. Source remediation is verified.
- CVE-2005-0953: `vulnerable-code-not-present`. The affected command-line permission-restoration path is absent.
- CVE-2005-1260: `not-in-default-build`. bzip2 runtime inclusion was not demonstrated in the inspected default project configurations.
- CVE-2008-1372: `not-in-default-build`. The affected bzip2 source path is not included in the inspected default project configurations.
- CVE-2010-0405: `not-in-default-build`. Source remediation remains unverified; future bzlib enablement requires renewed review.
- CVE-2011-4089: `vulnerable-code-not-present`. The affected `bzexe` tool and temporary-directory extraction path are absent.

## Verification Boundaries
Completed verification covers source presence, guard ordering, array-bound consistency, default-build scope, evidence integrity, and technical disposition review. Target compilation, SAST, malformed-input negative testing, and normal decompression regression testing remain pending unless supported by separate evidence.

## SHA-256 Evidence
- bzlib.c: 9A007D35527A6663C0DBD343027FFF0AA3674610412C5FB5501345C7BF896422
- bzlib.h: DF3875C706B51A9A4E920CF3CCD11CDDDD21C2FE8FDB7C7C699FB4374194084F
- bzlib_private.h: AEBE7F7B21682E230B932E7F0153F58E83C186EEF599C1291560B7D4C5847840
- bzlib_decompress.c: 52278F95DEA365839A7D5ECB2E4C48F276351AC21694E1CEED4743B34A9F66AB
- bzlib_huffman.c: C082F6FDAC11C40E9FF666D420B9E7FCA0D5D697E32FDB76E3EAA1A6E0150006
- bzlib_crctable.c: 98D9E2A621E6137F0F6C9B5CC16A2A4960094B4337958EDD4525DC8415B1B6AF
- bzlib_randtable.c: C49A46731E4A350D2A34C995CBF9C3344E3A3A2915AE3AB5E9DD119453F8B9DA

## SBOM Mapping
- SCA Definition: Documents/SBOM/components/ProductView/sca_bzip2.json
- Component Declaration: Documents/SBOM/components/ProductView/sca_bzip2.md
- Package URL: pkg:generic/bzip2@1.0.2

## Canonical Source Closure Hash
- Algorithm: SHA-256
- Value: 04c0dfd07929844a7155b382a202903c5e538e0e0e43fb259c554015b78654bd
- Hash Method: Sorted UTF-8 normalized relative paths followed by exact file bytes
- Coverage: Seven bzip2 source and header files listed in this evidence

## Review Result
- Technical Review Status: Complete
- Product Security Approval: Pending
- Release Publication: Blocked pending approval
