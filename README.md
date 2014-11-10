zeitkatze
=========

Zeitkatze is time cat -- literally

What's the simplest stopwatch in a linux console?
Well, just type `time cat` to start, and push ctrl-c to stop.
Zeitkatze is the same thing, with just a little extra.

It continuously updates the elapsed time display, you can hit ctrl-c once to display a split time,
and if you hit ctrl-d, (or ctrl-c twice in quick succession), zeitkatze prints the total time and
exits.

And it has colors! Enabled by default, you can disable them with the environment variable
`ZEITKATZE_COLOR`. Just set it to `0` (`export ZEITKATZE_COLOR=0` does that, or add
`ZEITKATZE_COLOR=0` to your shell rc). There's even a command line switch to override it:
`--color` or `-c` to enable colors, `--no-color` or `-n` to disable.
