# halfelf3 - ZX Spectrum plugins for Far Manager 3

Copyright (c) 1998-2000 HalfElf

Copyright (c) 2003 Александр Медведев, Старый Оскол

Copyright (c) 2020 halfelf3 team

Thanks to @RomanRom2 for pointing me to the source code
of the original halfelf plugins:
<http://zxsn.ru/halfelf/>.

## Potentially useful information

- <http://zxpress.ru/article.php?id=5075>
- <https://api.farmanager.com/ru/articles/escape.html>
- <https://github.com/MaxRusov/far-plugins>
- <https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp?view=vs-2019>
- <https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019>

## List of exported functions in two bundled plugins that use AnalyseW

| ArcLite          |TmpPanel          |HalfElf for Far 1.x |
|:-----------------|:-----------------|:-------------------|
|GetGlobalInfoW    |GetGlobalInfoW    |GetOpenPluginInfo?  |
|SetStartupInfoW   |SetStartupInfoW   |SetStartupInfo      |
|OpenW             |OpenW             |OpenPlugin          |
|AnalyseW          |AnalyseW          |OpenFilePlugin?     |
|ClosePanelW       |ClosePanelW       |ClosePlugin?        |
|ExitFARW          |ExitFARW          |ExitFAR             |
|GetFindDataW      |GetFindDataW      |GetFindData         |
|GetPluginInfoW    |GetPluginInfoW    |GetPluginInfo       |
|GetOpenPanelInfoW |GetOpenPanelInfoW | -                  |
|SetDirectoryW     |SetDirectoryW     |SetDirectory        |
|PutFilesW         |PutFilesW         |PutFiles            |
| -                |SetFindListW      | -                  |
| -                |ProcessPanelEventW|ProcessEvent?       |
|ProcessPanelInputW|ProcessPanelInputW|ProcessKey?         |
|ConfigureW        |ConfigureW        |Configure           |
|CloseAnalyseW     | -                | -                  |
|FreeFindDataW     | -                |FreeFindData        |
|GetFilesW         | -                |GetFiles            |
|DeleteFilesW      | -                |DeleteFiles         |
|MakeDirectoryW    | -                |MakeDirectory       |
|ProcessHostFileW  | -                |ProcessHostFile     |


| Far 1.x          | Far 3.x          |
|:----------------:|:----------------:|
|GetOpenPluginInfo |GetGlobalInfoW    |
|OpenFilePlugin    |AnalyseW          |
|ClosePlugin       |ClosePanelW       |
|ProcessEvent      |ProcessPanelEventW|
|ProcessKey        |ProcessPanelInputW|
