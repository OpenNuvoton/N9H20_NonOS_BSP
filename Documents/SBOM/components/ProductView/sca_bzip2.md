# bzip2 Component Declaration

## Component Information
- Name: bzip2
- SBOM Version: 1.0.2
- Upstream Distribution: bzip2-1.0.2
- Component Type: Library
- Scope: ProductView
- Integration: Modified vendored source
- License: LicenseRef-bzip2-1.0.2
- Author and Supplier: Julian Seward

## Source Location
- Loaders/SpiLoader_gzip/ziplib/bzlib.c
- Loaders/SpiLoader_gzip/ziplib/bzlib.h
- Loaders/SpiLoader_gzip/ziplib/bzlib_private.h
- Loaders/SpiLoader_gzip/ziplib/bzlib_decompress.c
- Loaders/SpiLoader_gzip/ziplib/bzlib_huffman.c
- Loaders/SpiLoader_gzip/ziplib/bzlib_crctable.c
- Loaders/SpiLoader_gzip/ziplib/bzlib_randtable.c

## Version and License Evidence
The vendored `bzlib.h` and `bzlib.c` identify modified files from the bzip2-1.0.2 distribution. The bzip2 license notice is present in `bzlib.h` and `bzlib.c`. The SBOM uses `LicenseRef-bzip2-1.0.2`.

## Integration and Build Scope
The vendored source contains embedded bzip2 decompression capability for compressed-image boot integration. Inclusion of the bzip2 runtime in the inspected default GCC and KEIL project configurations was not demonstrated. The observed default runtime decompression path uses zlib. The vendored bzip2 source remains part of the distributed ProductView and therefore remains in the Product SBOM.

## Vulnerability Technical Dispositions
- CVE-2019-12900: `patched-by-source-backport`; source remediation verified.
- CVE-2005-0953: `vulnerable-code-not-present`.
- CVE-2005-1260: `not-in-default-build`.
- CVE-2008-1372: `not-in-default-build`.
- CVE-2010-0405: `not-in-default-build`; source remediation unverified.
- CVE-2011-4089: `vulnerable-code-not-present`.

## Verification Status
- Technical review: Complete.
- Product Security approval: Pending.
- Target compilation: Pending unless supported by separate build evidence.
- SAST verification: Pending unless supported by a separate SAST report.
- Malformed-input negative testing: Pending unless supported by a separate test report.
- Normal decompression regression testing: Pending unless supported by a separate test report.

## Future Enablement Requirement
Any future enablement of the vendored bzlib sources requires renewed source-remediation, build-inclusion, SAST, and regression review before release.

## SBOM Mapping
- SCA Definition: Documents/SBOM/components/ProductView/sca_bzip2.json
- Evidence: Documents/SBOM/evidence/evidence_bzip2.md
- Package URL: pkg:generic/bzip2@1.0.2
- Component Origin: Third-party
- Required Scope: ProductView

## Canonical Source Closure Hash
- Algorithm: SHA-256
- Value: 04c0dfd07929844a7155b382a202903c5e538e0e0e43fb259c554015b78654bd
- Hash Method: Sorted UTF-8 normalized relative paths followed by exact file bytes
- Coverage: Seven bzip2 source and header files listed above

## Review Result
- Classification: Third-Party OSS
- Technical Review Status: Complete
- Product Security Approval: Pending
