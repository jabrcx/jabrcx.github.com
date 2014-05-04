---
layout: post
title: "how to save jobs when you kill storage"
date: 2014-05-04 09:34:13 -0400
comments: true
categories: 
---

It's a common predicament for admins of dynamic systems that also run long jobs --
you need to take down some storage, but there are processes using it that you really don't want to kill.
Here's a way to save those jobs by live-migrating them to new storage.
In short, it's another case in which gdb let's you do the sysadmin equivalent of [changing the tires while driving down the road](https://www.youtube.com/watch?v=MQm5BnhTBEQ).

Do the following for each process id `PID` involved in each job:


### 1: Attach with the GNU debugger

``` bash attach to the process
$ gdb -p PID
```

This will also pause the job.


### 2: Note all the open files

Now list all open files of the process:

``` bash list open files
$ lsof -p PID
```

Search this for the filesystem path that you're decommissioning, and note the file descriptor (`FD` column) of every file that needs to move, including the current working directory (`FD`=`cwd`), if applicable.


### 3: Flush data

Back in the gdb session, flush all the file descriptors so that no in-core data is lost; for each numeric file descriptor `FILE_DESCRIPTOR` run:

``` c flush in-core data
(gdb) call fsync(FILE_DESCRIPTOR)
```

Flush OS file system buffers, too; in a shell, run:

``` bash
$ sync
```


### 4: Copy the files

Create a directory for the process on the new storage, and copy all affected files from the old storage to the new storage.
Note their paths.



### 5: Change the process's current working directory

If the process's current working directory is on the affected storage, change it to the new storage; in the gdb session, run:

``` c change current working directory
(gdb) call chdir("/PATH/TO/NEW/CURRENT/WORKING/DIRECTORY")
```


### 6: Change each file descriptor to point to its new file

For each pair of numeric file descriptor `FILE_DESCRIPTOR` and new storage path `/PATH/TO/NEW/FILE`, run the following in the gdb session:

``` c swap open files
#note the file descriptor
set var $oldfd = FILE_DESCRIPTOR
#get the access mode and flags of the original file
call fcntl($oldfd, 3)
#open the new file, with those flags
call open("/PATH/TO/NEW/FILE", $)
#note the new file's file descriptor
set var $newfd = $
#get the offset in the original file
call lseek($oldfd, 0, 1)
#set the offset to the identical position in the new file
call lseek($newfd, $, 0)
#copy the new file descriptor to the old one
call dup2($newfd, $oldfd)
#close the new file descriptor
call close($newfd)
```

Some constants are used above:

* The `3` in `fcntl($oldfd, 3)` is `F_GETFL`, from /usr/include/bits/fcntl.h
* The `1` in `lseek($oldfd, 0, 1)` is `SEEK_CUR`, from /usr/include/fcntl.h
* The `0` in `lseek($newfd, $, 0)` is `SEEK_SET`, from /usr/include/fcntl.h


### 7: Check

Now when you look at the open files:

``` bash
$ lsof -p PID
```

you should see all files paths have changed from the old storage to the new storage.

That's it!

You can quit gdb, which will resume the process:

``` c
(gdb) quit
```


### Notes

The job could have absolute paths stored in memory, and after resuming may try to open files on the old storage.
If you can, setup up a symbolic link or some other trick to redirect it to the new storage.

If you're keeping the same path (e.g. upgrading storage or just taking a downtime), you can just switch the job to a temporary filesystem, swap out the primary storage, and re-switch back the storage to the original location.
Then there are no concerns about the process trying to use files in the "old" path, since it will be the same.

Beware of processes that have open network connections and could hit TCP timeouts if the switch takes too long.
