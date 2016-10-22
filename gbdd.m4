# Configure paths for GBDD
# Stolen from glib 

dnl AM_PATH_GBDD([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GBDD, and define GBDD_CFLAGS and GBDD_LIBS
dnl
AC_DEFUN(AM_PATH_GBDD,
[dnl 
dnl Get the cflags and libraries from the gbdd-config script
dnl
AC_ARG_WITH(gbdd-prefix,[  --with-gbdd-prefix=PFX   Prefix where GBDD is installed (optional)],
            gbdd_config_prefix="$withval", gbdd_config_prefix="")
AC_ARG_WITH(gbdd-exec-prefix,[  --with-gbdd-exec-prefix=PFX Exec prefix where GBDD is installed (optional)],
            gbdd_config_exec_prefix="$withval", gbdd_config_exec_prefix="")

  if test x$gbdd_config_exec_prefix != x ; then
     gbdd_config_args="$gbdd_config_args --exec-prefix=$gbdd_config_exec_prefix"
     if test x${GBDD_CONFIG+set} != xset ; then
        GBDD_CONFIG=$gbdd_config_exec_prefix/bin/gbdd-config
     fi
  fi
  if test x$gbdd_config_prefix != x ; then
     gbdd_config_args="$gbdd_config_args --prefix=$gbdd_config_prefix"
     if test x${GBDD_CONFIG+set} != xset ; then
        GBDD_CONFIG=$gbdd_config_prefix/bin/gbdd-config
     fi
  fi

  AC_PATH_PROG(GBDD_CONFIG, gbdd-config, no)
  min_gbdd_version=ifelse([$1], ,0.1,$1)
  AC_MSG_CHECKING(for GBDD - version >= $min_gbdd_version)
  no_gbdd=""
  if test "$GBDD_CONFIG" = "no" ; then
    no_gbdd=yes
  else
    GBDD_CFLAGS=`$GBDD_CONFIG $gbdd_config_args --cflags`
    GBDD_LIBS=`$GBDD_CONFIG $gbdd_config_args --libs`
    gbdd_config_major_version=`$GBDD_CONFIG $gbdd_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gbdd_config_minor_version=`$GBDD_CONFIG $gbdd_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gbdd_config_micro_version=`$GBDD_CONFIG $gbdd_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  fi
  if test "x$no_gbdd" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)

     GBDD_CFLAGS=""
     GBDD_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_SUBST(GBDD_CFLAGS)
  AC_SUBST(GBDD_LIBS)
  rm -f conf.gbddtest
])
