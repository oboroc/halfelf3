.autodepend

#----------------------------------------------------------
# необходимо менять только 3 следующие строки
#----------------------------------------------------------
TARGET   = fdi.fmt
OBJFILES = fdi.obj filer.obj memory.obj

BCPATH = g:\

BCC = $(BCPATH)\bin\bcc32.exe
LINKER = $(BCPATH)\bin\tlink32.exe

CFLAGS =  -c -a1 -RT- -M- -x- -v- -I$(BCPATH)\include

LFLAGS = -L$(BCPATH)\lib -s -m -M -Tpd -aa -v-

# c0x32 - 32-bit console-mode EXE startup module
# c0w32 - GUI EXE startup module

ALLOBJ = $(OBJFILES)

ALLLIB = import32 cw32

all : $(TARGET)

$(TARGET) : $(OBJFILES)
        $(LINKER) $(LFLAGS) $(ALLOBJ), $(TARGET),, $(ALLLIB)

.cpp.obj:
        $(BCC) $(CFLAGS) $<
