DESTDIR=win-installer/ardesia_0.7-windows-1_i386

mkdir -p $DESTDIR

cp AUTHORS $DESTDIR
cp COPYING $DESTDIR
cp NEWS $DESTDIR
cp README $DESTDIR
cp win-installer/RUN $DESTDIR

mkdir -p $DESTDIR/bin

cp /mingw/bin/ardesia.exe $DESTDIR/bin
cp /mingw/bin/curtain.exe  $DESTDIR/bin
cp /mingw/bin/freetype6.dll $DESTDIR/bin
cp /mingw/bin/gspawn-win32-helper.exe $DESTDIR/bin
cp /mingw/bin/intl.dll $DESTDIR/bin
cp /mingw/bin/libatk-1.0-0.dll $DESTDIR/bin
cp /mingw/bin/libcairo-2.dll $DESTDIR/bin
cp /mingw/bin/libexpat-1.dll $DESTDIR/bin
cp /mingw/bin/libfontconfig-1.dll $DESTDIR/bin
cp /mingw/bin/libgdk-win32-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libgdk_pixbuf-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libgio-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libglib-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libgmodule-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libgobject-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libgsl-0.dll $DESTDIR/bin
cp /mingw/bin/libgslcblas-0.dll $DESTDIR/bin
cp /mingw/bin/libgthread-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libgtk-win32-2.0-0.dll $DESTDIR/bin
cp /mingw/bin/libpango-1.0-0.dll $DESTDIR/bin
cp /mingw/bin/libpangocairo-1.0-0.dll $DESTDIR/bin
cp /mingw/bin/libpangoft2-1.0-0.dll $DESTDIR/bin
cp /mingw/bin/libpangowin32-1.0-0.dll $DESTDIR/bin
cp /mingw/bin/libpng14-14.dll $DESTDIR/bin
cp /mingw/bin/spotlighter.exe $DESTDIR/bin
cp /mingw/bin/zlib1.dll $DESTDIR/bin

mkdir -p $DESTDIR/etc/gtk-2.0

cp /mingw/etc/gtk-2.0/gdk-pixbuf.loaders $DESTDIR/etc/gtk-2.0
cp /mingw/etc/gtk-2.0/gtkrc $DESTDIR/etc/gtk-2.0

mkdir -p $DESTDIR/lib/gtk-2.0/2.10.0/engines

cp /mingw/lib/gtk-2.0/2.10.0/engines/libpixmap.dll $DESTDIR/lib/gtk-2.0/2.10.0/engines
cp /mingw/lib/gtk-2.0/2.10.0/engines/libwimp.dll $DESTDIR/lib/gtk-2.0/2.10.0/engines

mkdir -p $DESTDIR/lib/gtk-2.0/2.10.0/loaders

cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-ani.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-bmp.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-gif.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-icns.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-ico.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-jpeg.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-pcx.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-png.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-pnm.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-ras.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-tga.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-tiff.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-wbmp.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-xbm.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders
cp /mingw/lib/gtk-2.0/2.10.0/loaders/libpixbufloader-xpm.dll $DESTDIR/lib/gtk-2.0/2.10.0/loaders

mkdir -p $DESTDIR/share/ardesia/scripts

cp /mingw/share/ardesia/scripts/screencast.bat $DESTDIR/share/ardesia/scripts

mkdir -p $DESTDIR/share/ardesia/ui

cp /mingw/share/ardesia/ui/annotation_window.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/ardesia.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/ardesia_horizontal.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/background_window.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/preference_dialog.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/project_dialog.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/text.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/text_window.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/vertical_bar.glade $DESTDIR/share/ardesia/ui

mkdir -p $DESTDIR/share/ardesia/ui/backgrounds

cp /mingw/share/ardesia/ui/backgrounds/blackboard.png $DESTDIR/share/ardesia/ui/backgrounds
cp /mingw/share/ardesia/ui/backgrounds/civil_society.png $DESTDIR/share/ardesia/ui/backgrounds
cp /mingw/share/ardesia/ui/backgrounds/musical_staff.png $DESTDIR/share/ardesia/ui/backgrounds
cp /mingw/share/ardesia/ui/backgrounds/notebook_paper.png $DESTDIR/share/ardesia/ui/backgrounds
cp /mingw/share/ardesia/ui/backgrounds/squared_blackboard.png $DESTDIR/share/ardesia/ui/backgrounds
cp /mingw/share/ardesia/ui/backgrounds/squared_paper.png $DESTDIR/share/ardesia/ui/backgrounds
cp /mingw/share/ardesia/ui/backgrounds/whiteboard.png $DESTDIR/share/ardesia/ui/backgrounds
cp /mingw/share/ardesia/ui/backgrounds/world_map.png $DESTDIR/share/ardesia/ui/backgrounds

cp /mingw/share/ardesia/ui/horizontal_bar.glade $DESTDIR/share/ardesia/ui
cp /mingw/share/ardesia/ui/info_dialog.glade $DESTDIR/share/ardesia/ui

mkdir -p $DESTDIR/share/ardesia/ui/pixmaps

cp /mingw/share/ardesia/ui/pixmaps/add-pdf.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/ardesia_logo.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/arrow.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/blue.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/eraser.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/filler.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/green.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/hand.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/hide.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/highlighter.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/lock.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/media-record.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/media-recorder-unavailable.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/medium.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/pencil.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/rectifier.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/red.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/rounder.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/thick.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/thin.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/unhide.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/unlock.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/white.png $DESTDIR/share/ardesia/ui/pixmaps
cp /mingw/share/ardesia/ui/pixmaps/yellow.png $DESTDIR/share/ardesia/ui/pixmaps

mkdir -p $DESTDIR/share/curtain/ui

cp /mingw/share/curtain/ui/curtain.png $DESTDIR/share/curtain/ui
cp /mingw/share/curtain/ui/curtain.ui $DESTDIR/share/curtain/ui

mkdir -p $DESTDIR/share/locale/es/LC_MESSAGES/

cp /mingw/share/locale/es/LC_MESSAGES/ardesia.mo $DESTDIR/share/locale/es/LC_MESSAGES/ardesia.mo

mkdir -p $DESTDIR/share/locale/fr/LC_MESSAGES/

cp /mingw/share/locale/fr/LC_MESSAGES/ardesia.mo $DESTDIR/share/locale/fr/LC_MESSAGES/ardesia.mo

mkdir -p $DESTDIR/share/locale/it/LC_MESSAGES/

cp /mingw/share/locale/it/LC_MESSAGES/ardesia.mo $DESTDIR/share/locale/it/LC_MESSAGES/ardesia.mo

mkdir -p $DESTDIR/share/spotlighter/ui/

cp /mingw/share/spotlighter/ui/spotlighter.ui $DESTDIR/share/spotlighter/ui/