# Local Development Setup

- Keep the generated ZMK/Zephyr west workspace under `.local/zmk-workspace/` instead of the repository root. Run local builds from that directory and pass `-DZMK_CONFIG=/Users/benjervis/dev/corne-keymap/config` so the checked-in keymap repo stays clean.
- The local Zephyr SDK is installed at `/Users/benjervis/.local/share/zephyr-sdk/zephyr-sdk-0.16.9`; set `ZEPHYR_SDK_INSTALL_DIR` to that path for local `west build` commands. The original SDK archive is stored under `/Users/benjervis/.local/share/zephyr-sdk/archives/`.
- In the Codex sandbox, disable ccache for local ZMK builds with `CCACHE_DISABLE=1`; otherwise ccache may fail with `Operation not permitted` when it tries to use its default cache location.
- After moving the west workspace, pass `Zephyr_DIR=/Users/benjervis/dev/corne-keymap/.local/zmk-workspace/zephyr/share/zephyr-package/cmake` until Zephyr is re-exported from the new location.
