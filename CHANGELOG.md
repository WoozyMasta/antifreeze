# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog][],
and this project adheres to [Semantic Versioning][].

<!--
## Unreleased

### Added
### Changed
### Removed
-->

## [0.1.3][] - 2025-11-14

### Changed

* fixed regression where `cleanupBodiesTTL` could end up as `0` after
  updating from older configs, preventing dead infected cleanup
* preserved user-defined `cleanupBodiesTTL` on config schema upgrade
  (no longer overwritten with CE value)
* added sane fallback for `CleanupLifetimeDeadInfected` when CE global
  returns invalid or zero value

[0.1.3]: https://github.com/WoozyMasta/antifreeze/compare/0.1.2...0.1.3

## [0.1.2][] - 2025-11-09

### Added

* docs to workshop content
* license file to pbo

[0.1.2]: https://github.com/WoozyMasta/antifreeze/compare/0.1.1...0.1.2

## [0.1.1][] - 2025-11-09

### Changed

* removed not worked sound event limiter
* small refactor

[0.1.1]: https://github.com/WoozyMasta/antifreeze/compare/0.1.0...0.1.1

## [0.1.0][] - 2025-11-05

### Added

* First public release

[0.1.0]: https://github.com/WoozyMasta/antifreeze/tree/0.1.0

<!--links-->
[Keep a Changelog]: https://keepachangelog.com/en/1.1.0/
[Semantic Versioning]: https://semver.org/spec/v2.0.0.html
