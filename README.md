# hongen

`hongen` is a simple watchdog for Windows.

Usage:

```
$ hongen <command-line>
```

`hongen` will start the command specified in the command line and will keep the child process running forever.
The nice thing about it is that it will create a [Job Object](http://msdn.microsoft.com/en-us/library/windows/desktop/ms684161(v=vs.85).aspx)for the child process and all its decendents. When the child process exits (crashes), it hongen will close the job object and subsequently
will cause all the decendents to be killed as well.

We use `hongen` as part of [anode](http://anodejs.org) as the root of our worker roles in Azure.

## Samples

```
$ hongen %ProgramFiles(x86)%\nodejs\node.exe c:\server.js
```

Will spawn the node program `c:\server.js` and will keep it alive forever.
Note that you can use environment variable expansions (e.g. %ProgramFiles(x86)% resolves to where the 32-bit program files are).

```
$ hongen %COMSPEC% /c c:\some\batch.cmd
```

 > Note that you should use `%COMSPEC% /C` (resolves to where `cmd.exe` is) if you want to run a batch file.

## License

MIT

## Author

[Elad Ben-Israel](http://eladb.github.com)

