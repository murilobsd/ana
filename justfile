#!/usr/bin/env -S just --justfile

_default:
  @just --list -u

# Format all files
fmt:
  cargo fmt --all
  taplo fmt
