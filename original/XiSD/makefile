.autodepend

#----------------------------------------------------------
# необходимо менять только 3 следующие строки
#----------------------------------------------------------
TARGET   = XiSD.dll
OBJFILES = debug.obj XiSD.obj FmtReader.obj tools.obj iSDOS_tools.obj manager_tools.obj manager.obj iterator.obj manager_get_files.obj memory.obj manager_delete_files.obj registry.obj manager_make_folder.obj manager_put_files.obj manager_process_key.obj

BCPATH = g:\

BCC = $(BCPATH)\bin\bcc32.exe
LINKER = $(BCPATH)\bin\tlink32.exe

CFLAGS =  -c -a1 -RT- -M- -x- -v- -I$(BCPATH)\include

LFLAGS = -L$(BCPATH)\lib -s -m -M -Tpd -aa -v-


ALLOBJ = $(OBJFILES)

ALLLIB = import32 cw32

all : $(TARGET)

$(TARGET) : $(OBJFILES)
        $(LINKER) @&&|
 $(LFLAGS) +
$(ALLOBJ)
$(TARGET),
$(ALLLIB)
|
.cpp.obj:
        $(BCC) $(CFLAGS) $<
