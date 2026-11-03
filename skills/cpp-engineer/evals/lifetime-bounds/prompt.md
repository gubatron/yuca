---
tags: [skill-creator]
max_turns: 12
timeout_seconds: 300
allowed_tools: [Read, Glob, Grep, Skill, Write, Edit, Bash]
---

Review this C++11 library change. Return review.md with root causes, a minimal compatible correction, focused regression tests, and a short API contract. Do not modify any repository or claim unrun tests passed.

```cpp
struct Items {
    std::vector<int> data;
    std::vector<int>& values() { return data; }
    std::vector<int> slice(std::size_t start, std::size_t count) const noexcept {
        if (start + count > data.size()) count = data.size() - start;
        return {data.begin() + start, data.begin() + start + count};
    }
};
Items load();
void consume() { for (int v : load().values()) use(v); }
```
