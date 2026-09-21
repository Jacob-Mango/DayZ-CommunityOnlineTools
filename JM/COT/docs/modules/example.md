---
name: Example Module
description: Reference implementation and starter template for developers creating custom COT modules
type: project
---

## Overview

`JMExampleModule` serves as an official reference implementation and boilerplate template for mod developers building new modules for Community Online Tools. It demonstrates how to inherit from `JMRenderableModuleBase`, register RPCs, setup permissions, create forms, and handle client-server communication.

## Key Files

- `Scripts/5_Mission/CommunityOnlineTools/modules/Example/JMExampleModule.c` — Example module class implementing boilerplate callbacks
- `Scripts/5_Mission/CommunityOnlineTools/modules/Example/JMExampleForm.c` — Example UI form demonstrating layout format, widget binding, and button callbacks

## Compilation Note

`JMExampleModule` is wrapped inside `#ifdef DIAG` in `JMModuleConstructor.c`. It is compiled only during diagnostic/development builds and is not registered in standard production builds.

## Developer Guide

To build a custom module using `JMExampleModule` as a blueprint:
1. Refer to [../systems/module-creation.md](../systems/module-creation.md) for step-by-step module creation guidelines.
2. Inherit your module class from `JMRenderableModuleBase` (for modules with UI) or `JMModuleBase` (for headless background modules).
3. Register your module class in a modded `JMModuleConstructor.RegisterModules`.
4. Define your module's RPC enum offset in your code or modded `RPC.c`.
