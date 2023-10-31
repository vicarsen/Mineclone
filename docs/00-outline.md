# Introduction

The project is mostly an educational adventure for the sole developer of the game. If it eventually becomes managed by any sort of team, then its purpose, intended use, target audience, and scope will be reestablished to reflect what that team can achieve.
The game is intended to be some sort of game engine for voxel based games, but relatively limiting, to keep the size of the project manageable. It should allow its users to create games with premade packs of features, make their own pack of features, and play games made by themselves or other users, similarly to games like Roblox, but with the sandbox and graphical style of Minecraft. The target audience is split into 3 categories: game composers - those who create games by putting together premade features with very basic visual scripting (UE5 blueprint-like or Scratch style), game developers - those who actually code the premade features, and create the assets required, and players - those who only play the games created by composers. Game composers can be any demographic, with or without experience in programming, so the composing experience must be as intuitive and as documented as possible. Game developers are mostly professional programmers, hobby programmers, with prior coding experience, to whom only the API should be documented and explained. Players are not restricted to any demographic, and can be of any age, depending on the game played (further age restriction requirements will be detailed later).

# General description

The game engine must provide a framework for creating packs of features, and some vanilla packs. It must handle rendering, input and output, AI models and interactions, particle systems, debugging and profiling, game events, and other systems necessary for any game engine. It must specialize in voxel based worlds, and provide the tools necessary for creating, and managing such a world in realtime.

# Software features and requirements

## Functional requirements

 1. Game developer requirements:
   - a high-level rendering API, with little to no performance cost for such an architecture compared to pure low-level implementation
   - a low-level rendering API with zero cost compared to pure low-level implementation, for cases not covered by the high-level API, or for unexpected performance deficits of the high-level API
   - a high-level events API, with little to no performance cost for such an architecture compared to pure low-level implementation
   - a high-level world generation API, for managing voxel worlds, with good memory management, performance, and with seemless transitions between parts of the world when it comes to travelling through it
   - a high-level AI API, for creating NPCs, and entities, with at least semi-realistic behavior, with features like custom verbal interaction, pathfinding, resource management etc. available
 2. Game composer requirements:
   - a friendly interface for putting together games with feature packs
   - a friendly interface for defining scripts that combine such feature packs using graphical blocks in the style of UE5 blueprints, or Scratch blocks
   - a world composer where it is possible to use features from feature packs, and scripts coded to put together levels, characters, buildings, worlds, procedurally generated world systems etc.
 3. Player requirements:
   - a hub for finding games to play
   - progress statistics or rewards for progressing through different games
   - seemless game launching from the hub to the game screen, with background updates, downloads, and configuration

## Internal requirements

### APIs and libraries required

 - programming language: C++ 20
 - utilities: C++ STD library, possible migration to own implementation of some of the tools in the future
 - graphics API: OpenGL, possible migration to Vulkan in the future
 - window library: GLFW, possible migration to own implementation in the future
 - maths library: GLM
 - logging library: spdlog
 - GUI library: ImGUI, ImPlot
 - Sound library: OpenAL
 - AI library: TBD (possibly OpenAI API for NPC verbal interactions; more research needed)

### Performance requirements

 - world generation and rendering must allow for large open worlds to be playable on low-spec computers
 - player to world interaction must be instantaneous (unless otherwise specifically required by the respective feature), with no lag spikes or performance issues on such interactions
 - decent loading times, but large increases in loading time are acceptable if they lead to decent in-game performance benefits
 - game feature / game compilation in debug mode fast, to allow for quick testing, while in release mode slow, with as many optimizations as possible

### Quality requirements

 - readability: the code must be easily readable, with comments where clarifications are needed
 - maintainability: the code must be easily maintainable, respecting principles like RAII, DRY, design by contract etc., and have easy to use profiling and debugging tools
 - usability: the code must provide an easy to use API for developers
 - consistency: the code must have consistent versioning, with non backwards compatible changes resulting in major version changes
 - flexibility: the cost must cover as many use cases as possible, and if not, provide the tools necessary to implement the features required for those specific use cases

