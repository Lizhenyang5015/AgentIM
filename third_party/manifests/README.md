# Dependency manifests

Every dependency must have a manifest that records at least:

- name and exact version or commit ID;
- binary provenance, integrity validation, and archive location;
- license and redistribution obligations;
- supported Windows and macOS architectures;
- exported CMake package or target names;
- runtime files and final-package collection rules.

The current manifests cover Qt Base and CEF. Add equivalent manifests before
introducing OpenSSL, SQLite, Protobuf, Boost, or any other dependency.

`cef.json` is the canonical record for the pinned CEF version, official archive
source, per-platform SHA-1 values, staged roots, wrapper policy, and runtime
inventory. Human-readable documentation should link to it instead of copying
version or checksum values.
