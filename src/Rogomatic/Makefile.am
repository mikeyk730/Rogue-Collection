bin_PROGRAMS =  \
  player        \
	gene          \
	rgmplot       \
	histplot      \
  rogomatic

player_SOURCES = \
  globals.h      \
  install.h      \
  termtokens.h   \
  types.h        \
  arms.c         \
  command.c      \
  config.c       \
  database.c     \
  debug.c        \
  explore.c      \
  io.c           \
  learn.c        \
  ltm.c          \
  main.c         \
  mess.c         \
  monsters.c     \
  pack.c         \
  rand.c         \
  replay.c       \
  rooms.c        \
  scorefile.c    \
  search.c       \
  stats.c        \
  strategy.c     \
  survival.c     \
  tactics.c      \
  things.c       \
  titlepage.c    \
  utility.c      \
  worth.c        \
  getroguetoken.h\
  getroguetoken.c\
  debuglog.c

rogomatic_SOURCES = \
  globals.h         \
  types.h           \
  config.c          \
  debuglog.c        \
  findscore.c       \
  setup.c           \
  scorefile.c       \
  utility.c

noinst_PROGRAMS = \
  rgmplot         \
  datesub         \
  histplot        \
  gene

rgmplot_SOURCES = \
  globals.h       \
  types.h         \
  rgmplot.c       \
  utility.c

datesub_SOURCES = \
  datesub.c

histplot_SOURCES = \
  globals.h        \
  types.h          \
  histplot.c       \
  utility.c

gene_SOURCES = \
  globals.h    \
  types.h      \
  config.c     \
  debuglog.c   \
  gene.c       \
  rand.c       \
  learn.c      \
  stats.c      \
  utility.c

# special case for lex output
datesub.c: datesub.l
	lex -o $(builddir)/lex.yy.c $(srcdir)/datesub.l
	mv $(builddir)/lex.yy.c $(builddir)/datesub.c

CLEANFILES = $(builddir)/datesub.c

MAINTAINERCLEANFILES = config.h.in

