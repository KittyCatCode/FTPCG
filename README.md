# FTPCG
Fast Two-Part Composite Generator v1.0, by ungato.

# How do I use it?
First, compile the two programs.

    cd ICFGen
    gcc lodepng.c main.c -o ../icfgen[.exe]
    cd ../CompGen
    gcc lodepng.c main.c -o ../compgen[.exe]

Instead of [.exe], put whatever extension executables are on your OS. For mac & linux, no extension necessary.

Now, use the first program, icfgen,  to make an icf (Intermediate Composite Format) file. This will take a while.

    icfgen frameFolder output.icf

The long part is finished, now let's use the second program to make some composites!

    compgen output.icf insertWidthHere output.png

Because compgen doesn't use the video's frames, instead using an icf file, it is super fast. This lets you try different widths without having to wait a long time.

# ICFViewer

ICFViewer.jar can be used instead of compgen. It works just as well, and has a nice UI that responds instantly when you change the width.

# What's in an ICF file?

It's just a list of colors.

    RGBRGBRG
    BRGBRGBR
    GBRGBRGB

and so on.
