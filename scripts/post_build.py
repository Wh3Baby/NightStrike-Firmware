#!/usr/bin/env python3
"""
Post-build script for NightStrike Firmware
Handles binary merging, size reporting, etc.
"""

Import("env")

def post_build(source, target, env):
    """Post-build hook"""
    print("[Post-Build] NightStrike Firmware post-build script")
    # TODO: Merge binaries, generate reports, etc.

env.AddPostAction("buildprog", post_build)

