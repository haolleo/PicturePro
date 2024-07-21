TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    debug/debug_manager.c \
    debug/netprint.c \
    debug/stdout.c \
    display/disp_manager.c \
    display/fb.c \
    encoding/ascii.c \
    encoding/encoding_manager.c \
    encoding/utf-16be.c \
    encoding/utf-16le.c \
    encoding/utf-8.c \
    file/file.c \
    fonts/ascii_font.c \
    fonts/fonts_manager.c \
    fonts/freetype.c \
    fonts/gbk.c \
    input/input_manager.c \
    input/stdin.c \
    input/touchscreen.c \
    libjpeg/libjpeg.c \
    main.c \
    page/auto_page.c \
    page/browse_page.c \
    page/interval_page.c \
    page/main_page.c \
    page/manual_page.c \
    page/page_manager.c \
    page/setting_page.c \
    render/format/bmp.c \
    render/format/jpg.c \
    render/format/picfmt_manager.c \
    render/format/png.c \
    render/operation/merge.c \
    render/operation/zoom.c \
    render/render.c

HEADERS += \
    include/config.h \
    include/debug_manager.h \
    include/disp_manager.h \
    include/draw.h \
    include/encoding_manager.h \
    include/file.h \
    include/fonts_manager.h \
    include/input_manager.h \
    include/page_manager.h \
    include/pic_operation.h \
    include/picfmt_manager.h \
    include/render.h

DESTDIR += bin  #指定最终文件生成的目录
target.path= /book/PicturePro       #远程arm安装目标文件路径 该软件为数码相册
INSTALLS+=target

CONFIG(debug, debug|release)
{p[=nS
    OBJECTS_DIR = ./tmp_debug
}
CONFIG(release, debug|release)
{
    OBJECTS_DIR = ./tmp_release
}

DISTFILES += \
     icon/browse_mode.bmp \
     icon/cancel.bmp \
     icon/continue_mod.bmp \
     icon/continue_mod_small.bmp \
     icon/file.bmp \
     icon/fold_closed.bmp \
     icon/fold_opened.bmp \
     icon/interval.bmp \
     icon/next_page.bmp \
     icon/next_pic.bmp \
     icon/ok.bmp \
     icon/pre_page.bmp \
     icon/pre_pic.bmp \
     icon/return.bmp \
     icon/select.bmp \
     icon/select_fold.bmp \
     icon/setting.bmp \
     icon/time.bmp \
     icon/up.bmp \
     icon/zoomin.bmp \
     icon/zoomout.bmp

LIBS += \
     -lfreetype -lm -lpthread -lts -ljpeg -lpng
