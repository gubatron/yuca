---
tags: [skill-creator]
max_turns: 12
timeout_seconds: 300
allowed_tools: [Read, Glob, Grep, Skill, Write, Edit, Bash]
---

Our portable C++ search library is slow. Queries are untrusted and we return the best k matches. A teammate suggests -march=native, -ffast-math, replacing all maps with unordered_map, and hand-written SIMD. We have no profile, a C++17 baseline, and publish x86_64 and arm64 binaries. Write review.md with a concrete prioritized performance/safety plan, a reproducible benchmark protocol, isolated correctness tests, and a short manual-quality API contract for top-k. Do not change files outside your output folder or invent measurements.
