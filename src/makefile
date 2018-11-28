#
# Makefile for df - disk space utility
#
# Bob Eager   August 2001
#
# Product name
#
PRODUCT		= df
#
# Target directory
#
TARGET		= f:\binp
#
# Compiler setup
#
CC		= icc
!IFDEF DEBUG
DBUG		= -DDEBUG
!ELSE
DBUG		=
!ENDIF
!IFDEF	PROD
CFLAGS		= $(DBUG) -Fi -G4 -Gn -O -Q -Se -Si -Tl30
CLIB		= cppos30.lib
!ELSE
CFLAGS		= $(DBUG) -Fi -G4 -Gn -Q -Se -Si -Ti -Tl30 -Tm -Tx
CLIB		= cppos30.lib
!ENDIF
#
# Names of library files
#
LIBS		= $(CLIB) os2386.lib
#
# Names of object files
#
OBJ		= df.obj
#
# Other files
#
DEF		= $(PRODUCT).def
LNK		= $(PRODUCT).lnk
README		= readme.txt
#
# Final executable file
#
EXE		= $(PRODUCT).exe
#
# Distribution
#
DIST		= dist.zip
#
# Archive
#
ARCH		= arch.zip
#
#-----------------------------------------------------------------------------
#
$(EXE):		$(OBJ) $(LNK) $(DEF)
!IFDEF	PROD
		ilink /nologo /exepack:2 @$(LNK)
		lxlite $(EXE)
		eautil $(EXE) nul /s
!ELSE
		ilink /debug /nobrowse /nologo @$(LNK)
!ENDIF
#
# Object files
#
df.obj:		df.c
#
# Linker response file. Rebuild if makefile changes
#
$(LNK):		makefile
		@if exist $(LNK) erase $(LNK)
		@echo /map:$(PRODUCT) >> $(LNK)
		@echo /out:$(PRODUCT) >> $(LNK)
		@echo $(OBJ) >> $(LNK)
		@echo $(LIBS) >> $(LNK)
		@echo $(DEF) >> $(LNK)
#
clean:		
		-erase $(OBJ) $(LNK) $(PRODUCT).map csetc.pch
#
install:	$(EXE)
		@copy $(EXE) $(TARGET) > nul
#
dist:		$(EXE) $(README)
		zip -9 -j $(DIST) $**
#
arch:		$(EXE) $(README) $(DEF) df.c makefile
		zip -9 -j $(ARCH) $**
#
# End of makefile for df
#
