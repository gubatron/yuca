# C++ engineering skill smoke evaluations

## Initial run — 2026-11-03

Two fresh review-only runs per case: one with `cpp-engineer`, one baseline without it. Each configuration passed the four explicit assertions across `lifetime-bounds` and `hostile-topk`.

This is a **sanity check, not an effectiveness win**: the baseline also passed. Both identified lifetime/range/exception defects, preserved the requested language standard, rejected unsafe blanket compiler/container changes, and specified measured optimization and documented contracts without inventing execution evidence.

The with-skill lifetime review preserved temporary-call usability by returning owned containers from rvalues; the baseline instead rejected rvalue calls and adjusted the caller. Both are defensible contracts, with different compatibility tradeoffs. Human review should choose which behavior fits a real project.

Limitations: one run per case/configuration; static reviews rather than compiled repairs; no blinded human scoring; timing and token metrics were not exposed by the subagent tool and are unavailable. The assertions were non-discriminating on this sample. Do not generalize a 100% smoke score to implementation quality or productivity gains.

Generated outputs, grading evidence, comparison JSON/Markdown, and a human-review HTML page are kept under `build/skill-eval/`. Reproduce from the case prompts in fresh sessions; use skill-creator's aggregation/viewer tools. Before claiming effectiveness, add compiled repair tasks with seeded defects, actual sanitizer/test evidence, repeated independent trials, and human feedback. The skill's source digest/version is unchanged by these evaluation files.
