# Ruby Hunter

[![Build status](https://img.shields.io/azure-devops/build/sipkab/033652c2-ba43-4422-9198-9073b1138eac/6/master)](https://dev.azure.com/sipkab/ruby-hunter/_build?definitionId=6)

Ruby Hunter is a retro-style miner game based on [Sapphire Yours (original homepage on Web Archive)](https://web.archive.org/web/20120125043954/http://members.aon.at/sapphire/index.htm). It is a 99.9% reimplementation of the gameplay while also adding new features such as level sharing and leaderboards.

Ruby Hunter was created in order to preserve the gameplay of Sapphire Yours as the originally released game wouldn't run properly on modern machines. The gameplay was reverse engineered from SY and (apart from a few exceptions) all the levels that were made for SY are compatible with Ruby Hunter.

The game is cross platform and available on: [Google Play (Android)](https://play.google.com/store/apps/details?id=com.bence.sipka.sapphireyours), [Steam (Windows, macOS)](https://store.steampowered.com/app/675870/Ruby_Hunter/), [Windows Store (UWP)](https://www.microsoft.com/en-us/p/ruby-hunter/9nblggh4wf2h). \
It can also be compiled for iOS, but it's no longer available on the App store due to the recurring developer fees. \
Executables for Linux can also be compiled with a little effort.

The development of this game was part of a learning process for me and the code that you encounter may not be of the highest quality. It was a project for me to experiment with game and graphics development while working with something I enjoy. In case you're interested in some of the interesting features or in what I've learned, read along.

<p align="center">
  <img width="240" src="https://sipkab.github.io/ruby-hunter/res/screenshots/bombodrome.png">
  <img width="240" src="https://sipkab.github.io/ruby-hunter/res/screenshots/emeraldcareII.png">
  <img width="240" src="https://sipkab.github.io/ruby-hunter/res/screenshots/lightmare.png">
  <img width="240" src="https://sipkab.github.io/ruby-hunter/res/screenshots/dynamicduo.png">
  <img width="240" src="https://sipkab.github.io/ruby-hunter/res/screenshots/entropy2.png">
  <img width="240" src="https://sipkab.github.io/ruby-hunter/res/screenshots/checkers.png">
</p>

## Table of Contents

  * [Game Features](#game-features)
    + [Gameplay](#gameplay)
    + [Level Editor](#level-editor)
    + [Community Hub](#community-hub)
      - [Leaderboards](#leaderboards)
    + [3D Graphics](#3d-graphics)
  * [Code Features](#code-features)
    + [Graphics Layer Hot Reloading (OpenGL <-> DirectX11)](#graphics-layer-hot-reloading-opengl---directx11)
    + [Cross Platform](#cross-platform)
    + [Shader Language](#shader-language)
    + [Audio Layer](#audio-layer)
    + [Fonts, Texture Atlases](#fonts-texture-atlases)
    + [Code Generation, Resource Processing](#code-generation-resource-processing)
      - [OpenGL Registry](#opengl-registry)
    + [Vector Class](#vector-class)
    + [Miscellaneous](#miscellaneous)
  * [What I've Learned](#what-ive-learned)
    + [Premature Optimization](#premature-optimization)
    + [Keep a Backup of Your Code](#keep-a-backup-of-your-code)
    + [Exception Handling](#exception-handling)
    + [Decide What You Want To Do](#decide-what-you-want-to-do)
  * [Why Release the Source Code?](#why-release-the-source-code)
  * [Building the Project](#building-the-project)
  * [License](#license)
  * [Special Thanks](#special-thanks)

## Game Features

### Gameplay

The gameplay of Ruby Hunter is 99.9% the same of Sapphire Yours. It was reverse engineered from the original game and was reimplemented from scratch in RH. 

*You might ask why only 99.9%?* \
Sapphire Yours had few minor gameplay bugs in it. These bugs were also exploited in some original levels. I did not make the effort to properly reproduce these bugs, because I don't really know how and why they were present. So instead of this, I probably introduced some bugs of my own, but I let you discover them.

One of the hardest thing during reverse engineering was is to find the random generator of Sapphire Yours. When a level is started, a random seed is generated for the game. This seed is used for the random events when the level is played, and in order to replicate the gameplay I needed to find the random generator in SY otherwise the demo sequences for the old levels wouldn't work. \
It took some time with OllyDbg, but [these 9 instructions](src/userapp/sapphire/level/SapphireRandom.h#L38) couldn't hide.

Testing the reverse engineered gameplay was easier, as I just had to re-run the existing demo sequences for the levels and check if they successfully complete them.

### Level Editor

One of the coolest features of Sapphire Yours was that it integrated a level editor in the game. This allowed the community to create and share levels thats why they become so many. \
I wanted to keep this feature so Ruby Hunter also integrates a level editor. You can also share them with others in the Community Hub.

For the game developers out there, implementing a level editor is not suprisingly hard. The core of it is just displaying a level in a still state, and creating different controls for the user to manipulate it.

### Community Hub

What's good of the level editing if you can't share them? The Community Hub is supposed to serve as a shared repository for levels and other gameplay data. Anybody can share their levels with others without having to go through file copying and stuf.

#### Leaderboards

The Community Hub also provides leaderboards for the players where they can compete for who solved a level the fastest, if the fewest steps, or collected the most gems. It can be quite interesting to see that someone managed to collect one or two more gems than the level requires, or can solve it a few steps faster.

You can also download the gameplay of other players on the leaderboard to see how they managed to solve that level in that specific way.

### 3D Graphics

Sapphire Yours originally came with 2D graphics. I was interested in making 3D graphics and dealing with 3D objects, so I made 3D graphics for Ruby Hunter. It's just the 2D level rendered with a little bit of depth. \
It's not very pretty, the animations are clunky, and the models aren't the best, but I think it's cool.

You can also switch to the 2D graphics of Sapphire Yours if you wish.

<p align="center">
  <img src="https://sipkab.github.io/ruby-hunter/res/sep_1.png">
</p>

## Code Features

Some cool things I've made with code.

### Graphics Layer Hot Reloading (OpenGL <-> DirectX11)

The whole rendering pipeline is made in a way that allows reloading resources on the fly. This also entails that the graphics engine can be swapped out any time during the game is running. I really disliked the idea that some games require, that you need to restart the game to switch between OpenGL and DirectX.

So in the end, the graphics framework allows you to switch rendering pipelines between frames. This is currently only makes sense on platforms where you have multiple available, that is Windows with OpenGL and DirectX. As Vulkan or Metal support was not added when the game was made, this feature is not prevalent on Android or Apple platforms. (However, completely doable.)

### Cross Platform

I believe cross platform applications should be the norm. That's why I developed the enclosing framework for the game to support multiple platforms. I needed to implement some basic functionality like file, network, and thread access for each platform, and was able to use them without having to worry. \
This approach also required to write glue code for each platform that is responsible for launching the application. Due to this fact, some Java, Objective C++ code is also present.

Another aspect of cross platform development is how you build your application. There are two ways:

1. Having multiple platform specific projects. One for Android, one for Windows, one for macOS, one for iOS, ... you see where this goes.
    * This often requires you to copy the files of your project to another computer for building.
2. Having a build system that is capable of building for multiple platforms.
    * This allows not littering the codebase with various .xcproj, .vcxproj, .sln, build.gradle, .project and other files.

After going with the 1st approach for a while, I ended up with the 2nd. \
With cross platform development, you most likely need at least two or maybe more machines for building. (Building for Apple platforms needs a Mac.) With this, copying the modified source files or just simply keeping the different project locations up to date is such a chore.

This was the second reason for creating a build tool alongside of the development of Ruby Hunter. (About the first reason later. TODO) \
With a proper build tool, you don't need to copy the files, the build tool will do it for you. I was using Eclipse for writing code, and I was glad that I no longer needed to open up Visual Studio, Xcode, or Android Studio to build for the target platforms. (Or even deal with their command line interface.) \
This greatly simplifies the development, testing, and release workflow for the project.

### Shader Language

In order to program the graphics pipeline, you need to write shaders. Shaders are little (or sometimes not so little) programs that run on the GPU. They are used to transform the input vertices, colors, textures, and whatsoever into the displayed picture.

Different graphics layers have different shader languages. For this particular project, you need 3: OpenGL, OpenGL ES, and DirectX. DirectX uses [HLSL](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-pguide) while OpenGL uses [GLSL](https://www.khronos.org/opengl/wiki/Core_Language_(GLSL)). There is also slight difference between the desktop GLSL and GLSL that is used on mobile devices (iOS, Android), that's why it's counted twice.

I didn't want to write the shaders for a single use-case three times, so I made an unified shader language that can be transpiled to the appropriate target during builds. Another advantage of this is that C++ classes can also be generated that allows easier loading and usage.

A very simple color shader is [here](gameres/shaders/simplecolorshader.usl).

### Audio Layer

Programming game audio was not fun. Dealing with lower level sound APIs is not easy as you need to keep the player fed with data even if you're doing something else in your game. You also need to take care not to block the sound rendering thread as well as ensure proper concurrent access to your buffers. \
This becomes a bit harder when creating a cross-platform application.

Ruby Hunter uses three different APIs on different platforms: OpenSL ES for Android, OpenAL on Apple platforms, XAudio2 on Windows. I've built an abstraction on top of them that works for now, but I won't consider it good. \
The sound effect files use the WAV format that was easy enough to write its own parser, but as uncompressed music files can become enormous, they needed to be compressed. The OGG format is used for for music files, and they are decompressed using [libogg and libvorbis](https://xiph.org/downloads/).

### Fonts, Texture Atlases

Text rendering is not straightforward when you're using OpenGL or DirectX. Without third party libraries, you don't really have access to opening TTF files and passing them directly to the rendering pipeline.

Ruby Hunter uses texture atlases for rendering text. The TTF file(s) are converted into a texture atlas and font descriptor during build time, that is loaded by the code and passed to the renderer as a sequence of quads. Ruby Hunter uses a monospace font (Consolas) in order to have a more *retro* style as most miner games were made decades ago, however, the font rendering should work with variable-width fonts as well. \
The conversion of TTF files are performed in Java during the build process.

Texture atlases are also used to compose multiple smaller images into a single larger image. This allows us not need to load multiple images, and requires less switching for the renderer. One example for this is the tiles and animations for the 2D graphics.

### Code Generation, Resource Processing

During build time, most of the game resources are processed and rewritten into a more efficient format so they can be interpreted easier during runtime. Small images are converted into texture atlases, font descriptors are generated, graphics shaders are transpiled, 3D object files are merged, C++ code are generated, and various smaller things are performed.

I believe that compile time verification is essential for software development as you can spot bugs much earlier in the process. I also think that if you can process your resources during build time and transform them into a format that can be loaded more efficiently during runtime, then you should do so. This allows faster startup, or just generally easier handling in code.

In the beginning, the code that performed the resource processing was just in a simple Java program. Later as it expanded for multiple features, it got hard to manage and extend. This is the first reason that a new build tool was developed alongside Ruby Hunter. A more generic approach was necessary to load the build tasks that were performed as part of the resource processing.

The build tool that was created later outgrew the project and became the [saker.build system](https://github.com/sakerbuild/saker.build). In hindsight its a bit too much, more on that in [Premature optimization](#premature-optimization)

#### OpenGL Registry

The project doesn't use any kind of third party OpenGL glue. (That is the code responsible for function lookup and providing headers for the OpenGL API.)

I've implemented a build task that loads the [OpenGL Registry](sipka.rubyhunter/render/resources/opengl_registry/opengl_registry.xml) XML file that contains the API specification. The XML is interpreted and appropriate headers and glue code is generated by the build task.

The registry is available here: [KhronosGroup/OpenGL-Registry/xml](https://github.com/KhronosGroup/OpenGL-Registry/tree/master/xml)

### Vector Class

Something that I've really liked in graphics shader languages is how you can access the fields of a vector in different orders:

```glsl
vec3 v;
vec3 rotated = v.zxy;
vec3 reds = v.rrr;
```

I wanted to replicate this in C++, and the [Vector](src/rhfw/framework/geometry/Vector.h) class was the result. It may still be incomplete, but allows the same as above:

```cpp
Vector3F v;
Vector3F rotated = v.zxy();
Vector3F reds = v.rrr();
```

You can also have different components in the vectors:

```cpp
Vector3<int> v;
Vector3<Vector3<float>> vv;
```

The accessor functions (`zxy()`, `rrr()`, etc...) don't actually return copies of the vector, but only an indexing view to it. The `Vector` type can also have any fixed dimension size and doesn't have internal loops, but any internal iteration is done by template metaprogramming. In theory most of these could be optimized away, but I'm not entirely sure about it. I think it's cool nonetheless.

### Miscellaneous

Some minor features.

* The game supports gamepads via DirectInput and XInput.
* There's a [scroll and fling gesture detector](src/rhfw/framework/io/touch/gesture/scroll/ScrollGestureDetector.cpp) class. It's nothing extra, but required me to calculate some derivatives to make the animations smooth when reapplying velocity. I think this was the first real world usage of derivatives outside of university.
* The UI uses length as a metric to create measurements on screen instead of pixels. The UI layout is programmed and rendered without many helper classes. The drawing code isn't pretty.
* The game supports keymapping, so you can remap your inputs. This was relatively easy to implement and basically just a lookup table that you put in front of your keyboard events.
* The project supports hijacking the `new` operator and reports it to you if you attempt to free some memory twice, or leave some memory region unallocated.
* The code uses no STL.

<p align="center">
  <img src="https://sipkab.github.io/ruby-hunter/res/sep_2.png">
</p>

## What I've Learned

I used Ruby Hunter to learn developing software. Here's my conclusions.

### Premature Optimization

You may've heard the phrase *'Premature optimization is the root of all evil'*. It's also the root of unnecessary complex code architecture and too much code generation. When I started the project I was obsessed with eliminating single `if` conditions, virtual function calls, and generating as much code as I can during build time so it can be optimized away.

**This was completely unnecessary.**

The code architecture became complex, sometimes hard to follow, and relied on code generation to work. I've written most of the code during 2015-2017, and now that I have to read it again (2020), it is not easy to navigate. Modifying some parts became harder and there are unnecessary indirections between header files. Sometimes I have this *'What was I thinking?'* moments.

My advice is that you should write the code first that works. After it works, then you can take a look at optimizations, but don't make hasty decisions based on assumptions. Virtual function calls and `if` conditions rarely going to be the bottleneck.

### Keep a Backup of Your Code

In the summmer of 2019 my SSD gave up on me. I've had some of my code on it including Ruby Hunter among many more important files. If you have even a single bit that you don't want to lose, **make a backup now**.

Luckily for me, a specialist was able to restore my files and nothing of importance was lost (but it did cost a lot). Some files had minor binary artifacts in them, so if you see some non readable data or errors in resources, this is the cause (please file an issue.).

### Exception Handling

**Handle the exceptions.** I've written most of the code by assuming the [Happy path](https://en.wikipedia.org/wiki/Happy_path). Not handling exceptions could result in your program crashing, although an exception may be perfectly valid scenario.

In particular, I don't check the exceptions or error codes of most file operations, rendering call results, or dynamic function lookups. Don't be like me, handle those exceptions, and display them to the user in some way. It is much easier to fix an error based on a message, rather than just the application crashing.

### Decide What You Want To Do

When you embark on a project, decide up front what is it that you want to accomplish. It is generally unnecessary to write every last part of your application yourself. E.g. If you want to create a game, you probably don't want to deal with the way how OpenGL is loaded by your game. Set the goals for your project, and work with the abstractions that leave out unnecessary low-level details. It's okay to use third party products or libraries when reimplementing them wouldn't contribute a proportionate amount towards your goal.

In my case I mostly wanted to learn and explore various platforms and rendering APIs. On the way I decided that I want to create Ruby Hunter based on Sapphire Yours. Reimplemented abstractions on top of lower level APIs were part of my learning process. I was curious about how these components work together and whether a cross-platform common layer can be build on top of them. \
I did not want to deal with the details of loading images or audio files myself, so using libpng and libogg was a suitable solution for me.

<p align="center">
  <img src="https://sipkab.github.io/ruby-hunter/res/sep_3.png">
</p>

## Why Release the Source Code?

I decided to release the source for Ruby Hunter and to make it free on Steam as well for two reasons.

1. The game was made to preserve the gameplay of Sapphire Yours. This just doesn't work if the code is closed or the game simply isn't free. I believe that the game is awesome and hope to bring joy to anyone who plays it. I personally really enjoy the Leaderboards on which players can compete against each other for better scores.
2. I wish to shine some light on my other project that is the [saker.build system](https://github.com/sakerbuild/saker.build). It was initially developed alongside of Ruby Hunter but became a much bigger project with some pioneer features unmatched by existing tools. Ruby Hunter is also a test project for saker.build to verify that it is indeed capable of manging a cross-platform project.

For the future, I don't indend to make major improvements for the game, mostly smaller bugfixes. I have some minor features that I wish to implement over time, but don't expect anything major. \
If anybody wishes to contribute, have any questions, or just want to say hello, please [file an issue](https://github.com/Sipkab/ruby-hunter/issues/new) and I'll be happy to respond.

<p align="center">
  <img src="https://sipkab.github.io/ruby-hunter/res/sep_4.png">
</p>

## Building the Project

Ruby Hunter uses the [saker.build system](https://github.com/sakerbuild/saker.build) to build its code for any of the target platforms. The clean building consists of two steps:

1. The build tasks that help the resource processing and code generation needs to be installed.
2. The game can be build.

First of all, you need JDK 8+ and to [install the build system](https://saker.build/saker.build/doc/installation.html). (Command line or plugin.)

1. Run the `install` target in `sipka.rubyhunter/saker.build`: 
    ```
     java -jar saker.build.jar -bd build install sipka.rubyhunter/saker.build
    ```
2. Run the `export` target for your platform in `saker.build`: 
    ```
     java -jar saker.build.jar -bd build export_win32
    ```

If you build for Android you should use the `debug_export_android` that signs the APK with a debug key. You can use the `adb` tool to install it on your device. \
When building for UWP, you can use the `run_winstore` target to build the game and also add it to your PC in development mode. The target will also launch the game as well.

You can always refer to the [azure-pipelines.yml](azure-pipelines.yml) file that configures the [CI builds on Azure](https://dev.azure.com/sipkab/ruby-hunter/_build?definitionId=6). (It's not more complicated than what is above.)

Please note that the game that you build manually won't be able to connect to the community server.

## License

**Note** that only the source code for the project is licensed under *GNU General Public License v3.0 only* ([`GPL-3.0-only`](https://spdx.org/licenses/GPL-3.0-only.html)).

The files that have the extension .c, .cpp, .m, .mm, .java , .build, .lang are considered to be part of the source code of Ruby Hunter.

All rights are reserved for the resource files. (This is due to the fact that I don't have the rights to relicense some of the resources.)

## Special Thanks

My special thanks go to the original developer(s) of Sapphire Yours, Reinhard Grafl who as the copyright holder allowed Ruby Hunter to exist, Jürgen Wallner who made the original soundtrack, and to everyone who made levels for the original game. \
The people mentioned above don't endorse Ruby Hunter or is not affiliated with it in any way.






