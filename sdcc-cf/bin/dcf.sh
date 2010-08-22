#!/bin/sh

# dcf.sh - Distributed Compile Farm Mediator
#
# Copyright (c) 2007-2010 Borut Razem <borut dot razem at siol dot net>
#
# This file is part of sdcc.
#
#  This software is provided 'as-is', without any express or implied
#  warranty.  In no event will the authors be held liable for any damages
#  arising from the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.

LOG_LINES=1000	# max number of lines in the log file
BWLIMIT=7	# bandwith limit for rsync

ETC_DIR=$HOME/etc
LOG_DIR=$HOME/log
LOCK_DIR=$HOME/lock

DCF_BUILDER_LIST_FILE=$ETC_DIR/dcf_list
DCF_LOG=$LOG_DIR/dcf.log
DCF_LOCK=$LOCK_DIR/dcf.lock

TREE_FILE=$HOME/tmp/tree.txt

mkdir -p $LOG_DIR $LOCK_DIR

WEBHOST=web.sourceforge.net
WEBUSER=sdcc-builder,sdcc
WEBHTDOCSDIR=/home/groups/s/sd/sdcc/htdocs

FRSHOST=frs.sourceforge.net
FRSUSER=$WEBUSER
FRSDIR=/home/frs/project/s/sd/sdcc/snapshot_builds


# substring
substr ()
# $1: string
# $2: offset
# $3: length
{
  if test "$3" = ""
  then
    echo "$1" | awk "{print substr(\$0, $2)}"
  else
    echo "$1" | awk "{print substr(\$0, $2, $3)}"
  fi
}


# file type
file_type ()
# $1: premissions string
{
  expr "$1" : "^\(.\)[-rwxXst]*$"
}


# generate file tree list
tree ()
# $1: user
# $2: host
# $3: root
# $4: subdir
{
  local old_ifs files line file type subdir

  files=$(echo "ls -lt" | sftp -b- "$1@$2:$3/$4" | sed -e '/^sftp> /d')
  old_ifs=$IFS
  IFS='
'
  for line in ${files}
  do
    # get file name from ls -l line
    # NOTE: this works only for file names without spaces!
    file=$(expr "${line}" : ".*[ ]\([^ ][^ ]*\)$")
    type=$(file_type $(substr "${line}" 0 8))
    case "${type}" in
    d)
      if test -z "$4"
      then
        subdir="${file}"
      else
        subdir="$4${file}"
      fi
      echo "${line}/$4"
      tree "$1" "$2" "$3" "${subdir}/"
      ;;
    [-l])
      echo "${line}"
      ;;
    esac
  done
  IFS="${old_ifs}"
}


# remove files & directories specified in arguments
# relays on find -depth
rm_list ()
{
  for file in $*
  do
    if test -d $file
    then
      rmdir $file
    else
      rm -f $file
    fi
  done
}


# list files & directories specified in arguments
# each file in a new line
list_files ()
{
  for file in $*
  do
    echo "  $file"
  done
}


# write the standard input to beginning of the log file
# and truncate it to $LOG_LINES lines
log_it ()
{
  local LOG=$(cat)

  if test -n "$LOG"
  then
    if test -e $DCF_LOG -a $(echo "$LOG" | wc -l) -lt $LOG_LINES
    then
      LOG=$(echo -e "$LOG\n" | cat - $DCF_LOG | head -n $LOG_LINES)
    fi
    echo "$LOG" > $DCF_LOG
  fi
}


# remove more than 7 files in dir from WEB server
rm_old_versions ()
{
  local i j k

  for i in htdocs/snapshots htdocs/regression_test_results
  do    
    for j in $(echo "ls -1t $i" | sftp -b- ${WEBUSER}@${WEBHOST} | sed -e '/^sftp> /d')
    do    
      for k in $(echo "ls -1t $j" | sftp -b- ${WEBUSER}@${WEBHOST} | sed -e '/^sftp> /d' | sed -e '1,7d')
      do
        if [ -n "$k" ]; then echo "removing $k"; echo "rm $k" | sftp -b- ${WEBUSER}@${WEBHOST}; fi
      done
    done
  done

  for k in $(echo "ls -1t htdocs/changelog_heads" | sftp -b- ${WEBUSER}@${WEBHOST} | sed -e '/^sftp> /d' | sed -e '1,7d')
  do
    if [ -n "$k" ]; then echo "removing $k"; echo "rm $k" | sftp -b- ${WEBUSER}@${WEBHOST}; fi
  done
}


# cleanup the lock file
cleanup ()
{
  rm -f $DCF_LOCK
}


# synchronise directory
sync_dir ()
# $1: source directory to sync
# $2: traget machine/directory to sync
{
  if test -d $1 && cd $1
  then
    FILE_LIST=$(find * -depth -print 2>/dev/null)
    if test -n "${FILE_LIST}"
    then
      echo "+++ start: $(date)"
      echo "=== files in $1:"
      list_files ${FILE_LIST}

      echo "=== rsyncing..."
      rsync $RSYNC_OPTS --relative --include='*.exe' -e ssh --size-only ${FILE_LIST} $2 2>&1 | grep -v -e "skipping directory"

      echo "=== removing..."
      rm_list ${FILE_LIST}

      echo "=== removing old versions..."
      rm_old_versions

      echo "--- end: $(date)"

      return 0
    fi
  fi

  return 1
}


{
  trap 'echo dcf.sh caught signal ; cleanup ; exit 1' 1 2 3 13 15

  if test -e ${DCF_BUILDER_LIST_FILE}
  then
    lockfile -r 0 ${DCF_LOCK} || exit 1

    test "${BWLIMIT}" != "" && RSYNC_OPTS="${RSYNC_OPTS} --bwlimit=${BWLIMIT}"

    rm -f ${TREE_FILE}

    {
      while read -r builder
      do
        export builder
        export TREE_FILE
        (
          builder=$(echo ${builder} | sed -e "s/^\(.*\)#.*$/\1/" -e "s/[ \t]*$//")
          if test ! -z "${builder}"
          then
            if sync_dir "/home/${builder}/htdocs/snapshots" ${FRSUSER}@${FRSHOST}:${FRSDIR}/
            then
              if test ! -e ${TREE_FILE}
              then
                tree ${FRSUSER} ${FRSHOST} ${FRSDIR} > ${TREE_FILE}
              fi
              sync_dir "/home/${builder}/htdocs" ${WEBUSER}@${WEBHOST}:${WEBHTDOCSDIR}/
            fi
          fi
        )
      done
    } < ${DCF_BUILDER_LIST_FILE}

    if test -e ${TREE_FILE}
    then
      pushd $(dirname ${TREE_FILE})
      rsync ${RSYNC_OPTS} --relative --include='*.exe' -e ssh --size-only $(basename ${TREE_FILE}) ${WEBUSER}@${WEBHOST}:${WEBHTDOCSDIR}/ 2>&1 | grep -v -e "skipping directory"
      popd
    fi
  fi

  cleanup
} 2>&1 | log_it

exit 0
