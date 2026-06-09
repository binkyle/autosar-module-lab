# CAN + PNC + ADAS Business Scenario Example

This directory contains a reference example of how `CommPolicyCoordinator`
can be configured for a CAN-based automotive project with Partial Network
Clustering (PNC) and ADAS-specific keep-alive requirements.

## Contents

- `CanAdasPnc_Cfg.c` — Example source IDs, timer config, action sets,
  policy rules, and channel configuration for the ADAS/PNC/CAN scenario.

## Key Concepts Demonstrated

- **Source IDs**: IGNITION, PNC_GLOBAL, PNC_ADAS, DIAG_KEEP, OTA_KEEP,
  LOCAL_KEEP, HOLD_VALID.
- **Source Timer**: PNC_GLOBAL and PNC_ADAS trigger a shared hold timer
  that keeps HOLD_VALID asserted for a configurable duration.
- **Action Sets**:
  - `AllFull` — all channels FULL_COMMUNICATION.
  - `AllNo` — all channels NO_COMMUNICATION, sleep candidate.
  - `AdasOnly` — only ADAS1 FULL, all others NO.
- **Policy Rules**:
  - IGNITION ON -> AllFull (priority 100).
  - PNC_GLOBAL + HOLD_VALID -> AllFull (priority 90).
  - PNC_ADAS + HOLD_VALID -> AdasOnly (priority 80).
  - DIAG / OTA / LOCAL keep -> AllFull (priority 85).
  - No active sources -> AllNo, sleep candidate (priority 0).

## Integration Notes

This is **not** the framework default configuration. It is provided as a
starting point for projects migrating from `CanCommCoordinator` or
integrating with Vector DaVinci / MICROSAR stacks.

Copy and adapt these definitions into your `project/vector/` or
`project/generic/` layer as needed.
