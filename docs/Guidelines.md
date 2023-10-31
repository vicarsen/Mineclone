# Guidelines Overview

The guidelines are made to have a properly defined system for writing the code so that it is easy to maintain in the future by contributors. They are meant to make it easy for a developer to browse through the code, use the code, and keep an organized and logical structure that is consistent throughout the whole project.
The guidelines are split into categories for ease of browsing through the document:

# Naming Schemes

Any project must have consistent naming schemes throughout its code, so that it is easy for developers to use code written by others, and not waste time looking through header files or LSP suggestions for the right variable, function, class, or namespace. The naming scheme is split into general rules, that apply to everything in the project, and specific rules, applied specifically to certain features. The general rules are as follows:
 1. Everything must have a descriptive and easy to read and remember name
 2. Abbreviations are strictly forbidden unless they are commonly used for some feature. Examples of allowed abbreviations: GUI for Graphical User Interface, AABB for axis aligned bounding box, idx for index, ptr for pointer, cnt for count, desc for description etc. Examples of forbidden abbreviations: FC for frustum culling, AO for ambient occlusion etc.)
 3. Abbreviations must be defaulted to uppercase if they are made up of the initials of the words in a phrase (GUI, AABB etc.), or defaulted to lowercase if they are more than just the first letter of a word (idx, ptr, cnt, desc etc.). Here "defaulted" means the casing can be overwritten if specified so in any specific naming rule

As for the specific rules, they are split up into categories based on the feature implemented:
 - [Namespaces](##namespaces-naming-scheme)
 - [Classes And Structs](##classes-and-structs-naming-scheme)
 - [Enums](##enums-naming-scheme)
 - [Functions](##functions-naming-scheme)
 - [Variables](##variables-naming-scheme)
 - [Macros](##macros-naming-scheme)

## Namespaces Naming Scheme

All namespaces' names must be in CamelCase, with the exception of the '__detail' namespace (see [Namespaces Structure](#namespaces-structure) bellow).

## Classes and Structs Naming Scheme

All classes' and structs' names must be in CamelCase.

## Enums Naming Scheme

All enums' names must be in CamelCase.

## Functions Naming Scheme

All functions' names must be in CamelCase.
Getters must be named 'GetProperty', and setters must be named 'SetProperty', where, depending on context, 'Property' is:
 - the name of the member variable operated on, if it operates on a member variable directly (Example: 'GetWidth' if it returns 'width')
 - the concatenation of the member variable and it's member operated on, if it operates on a member variable of a member variable (Example: 'GetTextureWidth' if it returns 'texture.width')
 - the concatenation of the member variable and the 'Property' of the member setter/getter operated on, if it operates on a getter/setter of a member variable (Example: 'GetTextureWidth' if it returns 'texture.GetWidth()')

## Variables Naming Scheme

All non-constant variables' names must be in snake_case.
All constant variables' names must be in SCREAMING_SNAKE_CASE.
Global variables' names must be preffixed with 'g_'.
Static member variables' names must be preffixed with 's_'.

## Macros Naming Scheme

All macros' names must be in SCREAMING_SNAKE_CASE.

# Notes
Parts of the code were written before these guidelines and need to be refactored.

