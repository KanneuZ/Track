CPP    = cl.exe
CFLAGS = /EHsc /c /O2 /IC:\Users\Andrey\project\Libs\opencv\build\include
LINK   = link.exe
LIBS   = User32.lib Ole32.lib OleAut32.lib Strmiids.lib /LIBPATH:C:\Users\Andrey\project\\Libs\opencv\build\x64\vc16\lib opencv_world4100.lib 

all: main.exe

main.exe:	obj/main.obj
			taskkill /F /T /FI "IMAGENAME eq main.exe"
			$(LINK) obj/main.obj $(LIBS) 
			
obj/main.obj:	main.cpp
				$(CPP) $(CFLAGS) /Foobj/main.obj main.cpp

start:			main.exe
			taskkill /F /T /FI "IMAGENAME eq main.exe"
			start main.exe