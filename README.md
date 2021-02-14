# zerofile

Simple command line tool that fill a file with binary zeroes, then removes it.

The file is filled 4096 bytes at the time (blocksize), and halves it when disk space is to small for 4096 bytes. So next time it's 2048 and so on, until it's less than 256 when it will remove the tempfile and exit.
The reason for halving is to really fill up all the free space with zeroes. If the selected block size is larger than the file system block size the halving is not really usefull,

## Author
zerofile originated from [https://github.com/nollan/zerofile](https://github.com/nollan/zerofile). I have the written consent to use the name 'zerofile' and thank Patric Fors for his efforts. zerofile was originally written in 'Go' and I converted it here to 'C'. The functionality is 100% the same, with the exception of the new test script.

## Building
gcc -o zerofile zerofile.c

## Example
```
$ zerofile
Using temp file: '0slask0.zro'
Written: 40300699648 bytes
Duration: 7m41.89093482s ; Performance: 87251549.195 bytes/sec
Removing temp-file
```

Why filling empty space with zeroes?

* Deleting a file just throws away the location to it, not the content.
* When making an image of the filesystem, to keep the size down.

## Options

No arguments needed, but if you want to create the file on a different place than current directory put path and filename as an argument (`zerofile some/other/file.tmp`).

The filename of the tempfile is by default `0slask0.zro`, use an argument to select your own name (`zerofile whatever.000`).

And use `-quiet` to skip the progress report on written bytes, a bit quicker this way.

Set a specific block size with `-blocksize`, it's 4096 by default.

## Performance

Stats produced are quick and dirty, don't use them as a hard disk performance checker. :)


## License
See [LICENSE](LICENSE)

Note: As this pertains to item #3, I have been granted permission by the original author of zerofile to use the name zerofile and thank him profusely as this is what I believe GitHub is all about. See: https://github.com/nollan/zerofile/issues/1
