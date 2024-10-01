Argp
====

25.3.11.4 A Program Using Multiple Combined Argp Parsers

This program uses the same features as example 3, but has more options, and presents more structure in the ‘--help’ output. It also illustrates how you can ‘steal’ the remainder of the input arguments past a certain point for programs that accept a list of items. It also illustrates the key value ARGP_KEY_NO_ARGS, which is only given if no non-option arguments were supplied to the program. See Special Keys for Argp Parser Functions.

For structuring help output, two features are used: headers and a two part option string. The headers are entries in the options vector. See Specifying Options in an Argp Parser. The first four fields are zero. The two part documentation string are in the variable doc, which allows documentation both before and after the options. See Specifying Argp Parsers, the two parts of doc are separated by a vertical-tab character ('\v', or '\013'). By convention, the documentation before the options is a short string stating what the program does, and after any options it is longer, describing the behavior in more detail. All documentation strings are automatically filled for output, although newlines may be included to force a line break at a particular point. In addition, documentation strings are passed to the gettext function, for possible translation into the current locale. 

https://www.gnu.org/software/libc/manual/html_node/Argp-Example-4.html

`getopt`, `getopt_long`
=======================

https://linux.die.net/man/3/getopt_long

