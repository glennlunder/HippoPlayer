# HippoPlayer

HippoPlayer is a music player for Mac, Windows and Linux that mostly focuses on music made for systems such as Amiga, C64, and gaming systems. While HippoPlayer will support MP3, FLAC, etc it's not the main target.

## Motivation

The original HippoPlayer is something I used a lot when I had my Amiga. Over many years I tried various players and while some of them hold up none of them has been what I been looking for.
Before starting this project I looked around quite a bit for alternatives and nothing really fit the bill.

Closest is likely [XMPlay](https://www.xmplay.com/), but that's for Windows only and closed source which isn't what I want either but feature wise it's fairly good.
I started this project roughly 7 years ago. I did some work in a private repository and had something very basic up and running. Then other things happened and the project has been on ICE for quite many years but I always had it back in my head that I wanted to come back to it.

The aim I have with HippoPlayer is "a modern music player for your oldsk00l needs" This means it should feel nice and great to use with features that you can expect from a modern player but allow you to play older formats in the best way possible.

## Moving forward

The project has now been started up again and it's now useable but still very limited. The screenshot include here is a just for **testing**. This is **not** the final design of the UI (but it's nicer to have something useable instead of cmdline only :) A redesign of the UI is in the works.

![Screenshot](/bin/hippo_screenshot_new.png)

There are many plans for HippoPlayer moving forward:

* Very rich support for many formats. The current format list is available here https://github.com/emoon/HippoPlayer/issues/13
* Big focus on visualizing data including (but not limited to): tracker view, hw state for emulated hardware, samples, oscilloscope, piano view and more.
* A modern UI that is easy to use but still full of features.
* Deep integration with existing websites that provides content that HippoPlayer can use and/or show as part of playing music such as: Demozoo, SceneSat and Scenemusic/Nectarine. This list will certainly grow with customized integration for many of these.
* Easy access to music. Possibility to index and auto download music from from various (free) sources of music (such as [modland](ftp.modland.com)) and more.
* More to come here!

Early builds of HippoPlayer for Mac and Windows can be found [here](http://hippoplayer.s3.eu-west-3.amazonaws.com/list.html?sort=lastmod&sortdir=desc)

# Building the code

[![Build status](https://ci.appveyor.com/api/projects/status/q8b0avg6trjk8xv1?svg=true)](https://ci.appveyor.com/project/emoon/hippoplayer)
[![Build Status](https://travis-ci.org/emoon/HippoPlayer.svg?branch=master)](https://travis-ci.org/emoon/HippoPlayer)

HippoPlayer is being written in [Rust](https://www.rust-lang.org) and uses Qt for the UI. The UI bindings are being (semi) auto generated by a tool called `ui_gen` This tool is only needed if you are going to change/add functionality to the UI but isn't required to build the code.
All plugins uses a C API which means that plugins may be written in C/C++ also (which most currently are as they use existing player libs)

Rust 1.20+ Latest stable version of **Rust** (1.20+) needs to be present on the system. We recommend using [rustup](https://www.rustup.rs/) to install and manage your Rust toolchain(s). If you already have rustup installed but aren't on the latest stable Rust, you can simply run `rustup update`. When installing using rustup for the first time simply select the defaults (option 1).
Qt needs to be installed on your system and then an environment variable needs to be assigned to `QT5` such as `QT5=/path/to/Qt/5.10.0/clang_64` You can download Qt from https://www.qt.io/ which has a free version for open source development.

### Mac

Run `scripts/mac_build_debug.sh` and if everything went ok run `t2-output/macosx-clang-debug-default/hippo_player`

### Windows

Run `scripts\win64_build_debug.cmd` and run the resulting executable in `t2-output\win64-msvc-debug-default\hippo_player`

### Linux

Please note that these instructions are for apt-get based distributions, e.g. Debian, Ubuntu and derivatives. We'll add additional instructions for Arch, Solus and so forth as we go along.

The following packages needs to be installed: qt57base mesa-common-dev libasound2-dev alsa-utils alsa-oss
Using apt-get the do

```
sudo apt-get install qt57base mesa-common-dev libasound2-dev alsa-utils alsa-oss
```

Then these steps to install Tundra:

```
git clone --recursive https://github.com/deplinenoise/tundra.git
cd tundra
make
sudo make install
```

Now enter the HippoPlayer directory and do

```
tundra2 -v linux-gcc-debug
```

Then run HippoPlayer with

```
t2-ouput/linux-gcc-debug-default/hippo_player
```

## Acknowledgement

The name HippoPlayer is reused with permission from Kari-Pekka Koljonen which made the original HippoPlayer for Amiga. The original code can be found [here](https://github.com/koobo/HippoPlayer)

## License

Licensed under either of

 * Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE) or http://www.apache.org/licenses/LICENSE-2.0)
 * MIT license ([LICENSE-MIT](LICENSE-MIT) or http://opensource.org/licenses/MIT)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, as defined in the Apache-2.0 license, shall be dual licensed as above, without any additional terms or conditions.

## Exception to the licence

Notice that each plugin may be under a different licence given the origin of the original code. Please check this first if you have concerns regarding this.


