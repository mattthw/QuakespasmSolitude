rem set PATH=C:/cygwin/usr/local/pspdev/bin;%PATH%
set PSPSDK=C:/cygwin/usr/local/pspdev
set PSP_MOUNT=/cygdrive/p
c:\cygwin\bin\bash -c 'export VS_PATH=`pwd`; /bin/bash --login -c "cd \"$VS_PATH\"; ./sed_make.sh %1 %2 %3 %4 %5 %6"'
