# Berry

## Development

- A patch is required after cloning: https://github.com/juce-framework/JUCE/pull/967
  - `cd lib/JUCE && git apply ../../juce.patch`
- Currently, developement settings are only for macOS and VSCode.

```
code berry.code-workspace
```

| Command                     | VSCode |
| :-------------------------- | :----- |
| ./debug-with-plugin-host.sh | F5     |
| ./debug-build.sh            | F7     |
| ./release-build.sh          |        |
