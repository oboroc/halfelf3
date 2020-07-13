# xFar3 - ZX Spectrum emulators plugins for Far Manager 3

Copyright (c) 1998-2000 HalfElf
Copyright (c) 2003 Александр Медведев, Старый Оскол
Copyright (c) 2020 xFar3 team

Thanks to RomanRom2 for pointing me to the source code
of the original xFar plugins:
<http://zxsn.ru/halfelf/>.

I had a problem, building "Hello, World!" example from source with
SDK headers worked, but produced a DLL with no exported functions.
@wilcey on [Far Manager Telegram group](https://t.me/FarManager) advised me to use
[Base64 plugin](https://github.com/Aleksoid1978/Base64) as an example.
Through this example I learned I can use `def` to define exported functions
in Visual Studio project.

To do it, first you have to create a `def` file.
I created plugin.def in the same folder as `vcxproj`
file with the following content:

```
EXPORTS
	GetGlobalInfoW
	SetStartupInfoW
	GetPluginInfoW
	OpenW
```

Then add it to project under `Resource Files`.

Then right-click on project, click on `Properties`.
Switch `Configuration` to `All Configurations`and `Platform` to `All Platforms`.
Now click on `Linker` and then `Input`.
In `Module Definition File`, type in the file name `plugin.def`.
If you recompile the project and check the dll for exported functions, you'll see them now.
More information on `def` file format is available
[here](https://docs.microsoft.com/en-us/cpp/build/exporting-from-a-dll-using-def-files?view=vs-2019)

## Potentially useful information

- <http://zxpress.ru/article.php?id=5075>
- <https://api.farmanager.com/ru/articles/escape.html>
- <https://github.com/MaxRusov/far-plugins>
- <https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp?view=vs-2019>
- <https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019>
