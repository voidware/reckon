################################################
# Make file for CASIO fx-9860G SDK Addin
#
############################
# Directory defines
TCDIR = I:\9860sdk\OS\SH
OSDIR = I:\9860sdk\OS
APPDIR = V:\sw\reckon
OUTDIR = V:\sw\reckon\Debug

################
# Main Defines
SH_EXEDIR=$(TCDIR)\bin

# Hitachi SH C/C++ Compiler02 phase
SHCC02_EXE=shc.exe
SHCC02_DEP="$(OSDIR)\FX\include\fxlib.h" "2d.h" "2di.h" "bcd.h" "bcdfloat.h" "bcdmath.h" "calc.h" "common.h" "complex.h" "dlist.h" \
      "eeval.h" "mi.h" "plot.h" "l3d\primitiv.h" "region.h" "solve.h" "symbol.h" "types.h" "complex2.h" "finance.h"

# Hitachi SH Assembler03 phase
SHASM03_EXE=asmsh.exe

# Hitachi OptLinker04 phase
SHLINK04_EXE=Optlnk.exe
SHLINK04_DEP="$(OSDIR)\FX\lib\fx9860G_library.lib"
SHLINK04_DEP2="$(OSDIR)\FX\lib\setup.obj"

#######################
# Files to build
FILE0=2d
FILESRC0="$(APPDIR)\$(FILE0).cpp"
FILEOBJ0="$(OUTDIR)\$(FILE0).obj"
FILE1=bcd
FILESRC1="$(APPDIR)\$(FILE1).cpp"
FILEOBJ1="$(OUTDIR)\$(FILE1).obj"
FILE2=bcdfloat
FILESRC2="$(APPDIR)\$(FILE2).cpp"
FILEOBJ2="$(OUTDIR)\$(FILE2).obj"
FILE3=bcdfloat2
FILESRC3="$(APPDIR)\$(FILE3).cpp"
FILEOBJ3="$(OUTDIR)\$(FILE3).obj"
FILE4=bcdmath
FILESRC4="$(APPDIR)\$(FILE4).cpp"
FILEOBJ4="$(OUTDIR)\$(FILE4).obj"
FILE5=calc
FILESRC5="$(APPDIR)\$(FILE5).cpp"
FILEOBJ5="$(OUTDIR)\$(FILE5).obj"
FILE6=clip
FILESRC6="$(APPDIR)\l3d\$(FILE6).cpp"
FILEOBJ6="$(OUTDIR)\$(FILE6).obj"
FILE7=l3api
FILESRC7="$(APPDIR)\l3d\$(FILE7).cpp"
FILEOBJ7="$(OUTDIR)\$(FILE7).obj"
FILE8=common
FILESRC8="$(APPDIR)\$(FILE8).c"
FILEOBJ8="$(OUTDIR)\$(FILE8).obj"
FILE9=cutils
FILESRC9="$(APPDIR)\$(FILE9).c"
FILEOBJ9="$(OUTDIR)\$(FILE9).obj"
FILE10=complex
FILESRC10="$(APPDIR)\$(FILE10).cpp"
FILEOBJ10="$(OUTDIR)\$(FILE10).obj"
FILE11=controller
FILESRC11="$(APPDIR)\l3d\$(FILE11).cpp"
FILEOBJ11="$(OUTDIR)\$(FILE11).obj"
FILE12=dlist
FILESRC12="$(APPDIR)\$(FILE12).cpp"
FILEOBJ12="$(OUTDIR)\$(FILE12).obj"
FILE13=drawpoly
FILESRC13="$(APPDIR)\l3d\$(FILE13).cpp"
FILEOBJ13="$(OUTDIR)\$(FILE13).obj"
FILE14=editor
FILESRC14="$(APPDIR)\l3d\$(FILE14).cpp"
FILEOBJ14="$(OUTDIR)\$(FILE14).obj"
FILE15=eeval
FILESRC15="$(APPDIR)\$(FILE15).cpp"
FILEOBJ15="$(OUTDIR)\$(FILE15).obj"
FILE16=Eval
FILESRC16="$(APPDIR)\$(FILE16).cpp"
FILEOBJ16="$(OUTDIR)\$(FILE16).obj"
FILE17=init
FILESRC17="$(APPDIR)\l3d\$(FILE17).cpp"
FILEOBJ17="$(OUTDIR)\$(FILE17).obj"
FILE18=mat
FILESRC18="$(APPDIR)\$(FILE18).cpp"
FILEOBJ18="$(OUTDIR)\$(FILE18).obj"
FILE19=matrix
FILESRC19="$(APPDIR)\l3d\$(FILE19).cpp"
FILEOBJ19="$(OUTDIR)\$(FILE19).obj"
FILE20=memory
FILESRC20="$(APPDIR)\l3d\$(FILE20).cpp"
FILEOBJ20="$(OUTDIR)\$(FILE20).obj"
FILE21=mi
FILESRC21="$(APPDIR)\$(FILE21).cpp"
FILEOBJ21="$(OUTDIR)\$(FILE21).obj"
FILE22=model
FILESRC22="$(APPDIR)\l3d\$(FILE22).cpp"
FILEOBJ22="$(OUTDIR)\$(FILE22).obj"
FILE23=os
FILESRC23="$(APPDIR)\l3d\$(FILE23).cpp"
FILEOBJ23="$(OUTDIR)\$(FILE23).obj"
FILE24=plot
FILESRC24="$(APPDIR)\$(FILE24).cpp"
FILEOBJ24="$(OUTDIR)\$(FILE24).obj"
FILE25=primitiv
FILESRC25="$(APPDIR)\l3d\$(FILE25).cpp"
FILEOBJ25="$(OUTDIR)\$(FILE25).obj"
FILE26=Reckon
FILESRC26="$(APPDIR)\$(FILE26).c"
FILEOBJ26="$(OUTDIR)\$(FILE26).obj"
FILE27=render
FILESRC27="$(APPDIR)\l3d\$(FILE27).cpp"
FILEOBJ27="$(OUTDIR)\$(FILE27).obj"
FILE28=rpn
FILESRC28="$(APPDIR)\$(FILE28).cpp"
FILEOBJ28="$(OUTDIR)\$(FILE28).obj"
FILE29=solve
FILESRC29="$(APPDIR)\$(FILE29).cpp"
FILEOBJ29="$(OUTDIR)\$(FILE29).obj"
FILE30=symbol
FILESRC30="$(APPDIR)\$(FILE30).cpp"
FILEOBJ30="$(OUTDIR)\$(FILE30).obj"
FILE31=triangle
FILESRC31="$(APPDIR)\l3d\$(FILE31).cpp"
FILEOBJ31="$(OUTDIR)\$(FILE31).obj"
FILE32=types
FILESRC32="$(APPDIR)\$(FILE32).cpp"
FILEOBJ32="$(OUTDIR)\$(FILE32).obj"
FILE33=utils
FILESRC33="$(APPDIR)\$(FILE33).src"
FILEOBJ33="$(OUTDIR)\$(FILE33).obj"
FILE34=view
FILESRC34="$(APPDIR)\l3d\$(FILE34).cpp"
FILEOBJ34="$(OUTDIR)\$(FILE34).obj"
FILE35=world
FILESRC35="$(APPDIR)\l3d\$(FILE35).cpp"
FILEOBJ35="$(OUTDIR)\$(FILE35).obj"
FILE36=plot3d
FILESRC36="$(APPDIR)\$(FILE36).cpp"
FILEOBJ36="$(OUTDIR)\$(FILE36).obj"
FILE37=complex2
FILESRC37="$(APPDIR)\$(FILE37).cpp"
FILEOBJ37="$(OUTDIR)\$(FILE37).obj"
FILE38=update3d
FILESRC38="$(APPDIR)\l3d\$(FILE38).cpp"
FILEOBJ38="$(OUTDIR)\$(FILE38).obj"
FILE39=qsort2
FILESRC39="$(APPDIR)\$(FILE39).c"
FILEOBJ39="$(OUTDIR)\$(FILE39).obj"
FILE40=finance
FILESRC40="$(APPDIR)\$(FILE40).cpp"
FILEOBJ40="$(OUTDIR)\$(FILE40).obj"
FILE41=big
FILESRC41="$(APPDIR)\$(FILE41).cpp"
FILEOBJ41="$(OUTDIR)\$(FILE41).obj"
FILE42=bigs
FILESRC42="$(APPDIR)\$(FILE42).cpp"
FILEOBJ42="$(OUTDIR)\$(FILE42).obj"
FILE43=int64
FILESRC43="$(APPDIR)\$(FILE43).cpp"
FILEOBJ43="$(OUTDIR)\$(FILE43).obj"
RFILE=FXADDINror
USERALLOBJ=$(FILEOBJ0) $(FILEOBJ1) $(FILEOBJ2) $(FILEOBJ3) $(FILEOBJ4) $(FILEOBJ5) $(FILEOBJ6) $(FILEOBJ7) $(FILEOBJ8) $(FILEOBJ9) \
      $(FILEOBJ10) $(FILEOBJ11) $(FILEOBJ12) $(FILEOBJ13) $(FILEOBJ14) $(FILEOBJ15) $(FILEOBJ16) $(FILEOBJ17) $(FILEOBJ18) $(FILEOBJ19) \
      $(FILEOBJ20) $(FILEOBJ21) $(FILEOBJ22) $(FILEOBJ23) $(FILEOBJ24) $(FILEOBJ25) $(FILEOBJ26) $(FILEOBJ27) $(FILEOBJ28) $(FILEOBJ29) \
      $(FILEOBJ30) $(FILEOBJ31) $(FILEOBJ32) $(FILEOBJ33) $(FILEOBJ34) $(FILEOBJ35) $(FILEOBJ36) $(FILEOBJ37) $(FILEOBJ38) $(FILEOBJ39) \
      $(FILEOBJ40) $(FILEOBJ41) $(FILEOBJ42) $(FILEOBJ43)

#######################
# nmake "all" statement

ALL:  SH_ENV \
	$(USERALLOBJ) \
	$(OUTDIR)\$(RFILE).bin \

####################
# Description blocks

!MESSAGE %3#C$z`&'0?
!MESSAGE
!MESSAGE Executing Hitachi SH C/C++ Compiler/Assembler phase
!MESSAGE

SH_ENV : 
	set SHC_INC=$(TCDIR)\include
	set PATH=$(TCDIR)\bin
	set SHC_LIB=$(TCDIR)\bin
	set SHC_TMP=$(OUTDIR)

$(FILEOBJ0) : $(FILESRC0) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ0)
-show=source
-listfile="$(OUTDIR)\$(FILE0).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC0)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ1) : $(FILESRC1) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ1)
-show=source
-listfile="$(OUTDIR)\$(FILE1).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC1)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ2) : $(FILESRC2) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ2)
-show=source
-listfile="$(OUTDIR)\$(FILE2).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC2)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ3) : $(FILESRC3) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ3)
-show=source
-listfile="$(OUTDIR)\$(FILE3).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC3)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ4) : $(FILESRC4) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ4)
-show=source
-listfile="$(OUTDIR)\$(FILE4).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC4)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ5) : $(FILESRC5) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ5)
-show=source
-listfile="$(OUTDIR)\$(FILE5).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC5)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ6) : $(FILESRC6) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ6)
-show=source
-listfile="$(OUTDIR)\$(FILE6).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC6)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ7) : $(FILESRC7) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ7)
-show=source
-listfile="$(OUTDIR)\$(FILE7).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC7)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ8) : $(FILESRC8) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ8)
-show=source
-listfile="$(OUTDIR)\$(FILE8).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC8)
-lang=c
-nologo
-debug
<<

$(FILEOBJ9) : $(FILESRC9) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ9)
-show=source
-listfile="$(OUTDIR)\$(FILE9).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC9)
-lang=c
-nologo
-debug
<<

$(FILEOBJ10) : $(FILESRC10) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ10)
-show=source
-listfile="$(OUTDIR)\$(FILE10).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC10)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ11) : $(FILESRC11) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ11)
-show=source
-listfile="$(OUTDIR)\$(FILE11).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC11)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ12) : $(FILESRC12) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ12)
-show=source
-listfile="$(OUTDIR)\$(FILE12).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC12)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ13) : $(FILESRC13) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ13)
-show=source
-listfile="$(OUTDIR)\$(FILE13).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC13)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ14) : $(FILESRC14) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ14)
-show=source
-listfile="$(OUTDIR)\$(FILE14).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC14)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ15) : $(FILESRC15) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ15)
-show=source
-listfile="$(OUTDIR)\$(FILE15).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC15)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ16) : $(FILESRC16) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ16)
-show=source
-listfile="$(OUTDIR)\$(FILE16).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC16)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ17) : $(FILESRC17) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ17)
-show=source
-listfile="$(OUTDIR)\$(FILE17).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC17)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ18) : $(FILESRC18) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ18)
-show=source
-listfile="$(OUTDIR)\$(FILE18).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC18)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ19) : $(FILESRC19) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ19)
-show=source
-listfile="$(OUTDIR)\$(FILE19).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC19)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ20) : $(FILESRC20) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ20)
-show=source
-listfile="$(OUTDIR)\$(FILE20).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC20)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ21) : $(FILESRC21) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ21)
-show=source
-listfile="$(OUTDIR)\$(FILE21).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC21)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ22) : $(FILESRC22) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ22)
-show=source
-listfile="$(OUTDIR)\$(FILE22).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC22)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ23) : $(FILESRC23) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ23)
-show=source
-listfile="$(OUTDIR)\$(FILE23).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC23)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ24) : $(FILESRC24) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ24)
-show=source
-listfile="$(OUTDIR)\$(FILE24).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC24)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ25) : $(FILESRC25) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ25)
-show=source
-listfile="$(OUTDIR)\$(FILE25).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC25)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ26) : $(FILESRC26) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ26)
-show=source
-listfile="$(OUTDIR)\$(FILE26).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC26)
-lang=c
-nologo
-debug
<<

$(FILEOBJ27) : $(FILESRC27) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ27)
-show=source
-listfile="$(OUTDIR)\$(FILE27).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC27)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ28) : $(FILESRC28) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ28)
-show=source
-listfile="$(OUTDIR)\$(FILE28).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC28)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ29) : $(FILESRC29) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ29)
-show=source
-listfile="$(OUTDIR)\$(FILE29).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC29)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ30) : $(FILESRC30) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ30)
-show=source
-listfile="$(OUTDIR)\$(FILE30).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC30)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ31) : $(FILESRC31) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ31)
-show=source
-listfile="$(OUTDIR)\$(FILE31).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC31)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ32) : $(FILESRC32) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ32)
-show=source
-listfile="$(OUTDIR)\$(FILE32).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC32)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ34) : $(FILESRC34) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ34)
-show=source
-listfile="$(OUTDIR)\$(FILE34).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC34)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ35) : $(FILESRC35) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ35)
-show=source
-listfile="$(OUTDIR)\$(FILE35).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC35)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ36) : $(FILESRC36) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ36)
-show=source
-listfile="$(OUTDIR)\$(FILE36).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC36)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ37) : $(FILESRC37) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ37)
-show=source
-listfile="$(OUTDIR)\$(FILE37).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC37)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ38) : $(FILESRC38) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ38)
-show=source
-listfile="$(OUTDIR)\$(FILE38).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC38)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ39) : $(FILESRC39) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ39)
-show=source
-listfile="$(OUTDIR)\$(FILE39).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC39)
-lang=c
-nologo
-debug
<<

$(FILEOBJ40) : $(FILESRC40) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ40)
-show=source
-listfile="$(OUTDIR)\$(FILE40).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC40)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ41) : $(FILESRC41) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ41)
-show=source
-listfile="$(OUTDIR)\$(FILE41).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC41)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ42) : $(FILESRC42) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ42)
-show=source
-listfile="$(OUTDIR)\$(FILE42).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC42)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ43) : $(FILESRC43) $(SHCC02_DEP)
	"$(SH_EXEDIR)\$(SHCC02_EXE)" -subcommand=<<
-cpu=sh3
-include="$(OSDIR)\FX\include","$(APPDIR)"
-objectfile=$(FILEOBJ43)
-show=source
-listfile="$(OUTDIR)\$(FILE43).lst"
-size
-noinline
-chgincpath
-errorpath
$(FILESRC43)
-lang=cpp
-nologo
-debug
<<

$(FILEOBJ33) : $(FILESRC33)
	"$(SH_EXEDIR)\$(SHASM03_EXE)" -subcommand=<<
$(FILESRC33)
-cpu=sh3
-endian=big
-round=zero
-denormalize=off
-include="$(APPDIR)"
-include="$(OSDIR)\FX\include"
-debug
-object=$(FILEOBJ33)
-literal=pool,branch,jump,return
-nologo
-chgincpath
-errorpath
<<

!MESSAGE
!MESSAGE Executing Hitachi OptLinker04 phase
!MESSAGE

"$(OUTDIR)\$(RFILE).bin" : $(USERALLOBJ) $(SHLINK04_DEP2) $(SHLINK04_DEP)
	"$(SH_EXEDIR)\$(SHLINK04_EXE)" -subcommand=<<
noprelink
sdebug
rom D=R
nomessage 
list "$(OUTDIR)\$(RFILE).map"
show symbol
nooptimize
start P_TOP,P,C,D,C$VTBL,C$INIT/0300200,B_BR_Size,B,R/08100000
fsymbol P
nologo
input $(USERALLOBJ)
input $(SHLINK04_DEP2)
library $(SHLINK04_DEP)
output "$(OUTDIR)\$(RFILE).abs"
-nomessage=1100
end
input "$(OUTDIR)\$(RFILE).abs"
form binary
output "$(OUTDIR)\$(RFILE).bin"
exit
<<

