# http://doc.qt.io/qt-6/qmake-tutorial.html

CONFIG += debug_and_release


QT += core widgets gui multimedia
INCLUDEPATH += ../

# -Werror=switch
#	Ensures that using switch with enum requires every value to be handled
# -fno-common
#	Common symbols across object files will produce a link error
#	This is the default from GCC 10
#
QMAKE_CFLAGS   += -Werror=switch -fno-common -std=gnu17
QMAKE_CXXFLAGS += -Werror=switch -fno-common -std=gnu++17


HEADERS =	../superio.h \
		../cdrom-iso.h \
		../cmos.h \
		../cp15.h \
		../fdc.h \
		../hostfs.h \
		../hostfs_internal.h \
		../ide.h \
		../iomd.h \
		../keyboard.h \
		../mem.h \
		../sound.h \
		../vidc20.h \
		../arm_common.h \
		../arm.h \
		../disc.h \
		../disc_adf.h \
		../disc_hfe.h \
		../disc_mfm_common.h \
		main_window.h \
		configure_dialog.h \
		about_dialog.h \
		rpc-qt6.h \
		plt_sound.h

SOURCES =	../superio.c \
		../cdrom-iso.c \
		../cmos.c \
		../cp15.c \
		../fdc.c \
		../fpa.c \
		../hostfs.c \
		../ide.c \
		../iomd.c \
		../keyboard.c \
		../mem.c \
		../romload.c \
		../rpcemu.c \
		../sound.c \
		../vidc20.c \
		../podules.c \
		../podulerom.c \
		../icside.c \
		../rpc-machdep.c \
		../arm_common.c \
		../i8042.c \
		../disc.c \
		../disc_adf.c \
		../disc_hfe.c \
		../disc_mfm_common.c \
		settings.cpp \
		rpc-qt6.cpp \
		main_window.cpp \
		configure_dialog.cpp \
		about_dialog.cpp \
		plt_sound.cpp

# NAT Networking
linux | win32 | wasm {
	HEADERS +=	../network-nat.h \
			nat_edit_dialog.h \
			nat_list_dialog.h
	SOURCES += 	../network-nat.c \
			nat_edit_dialog.cpp \
			nat_list_dialog.cpp

	HEADERS += 	../slirp/bootp.h \
			../slirp/cutils.h \
			../slirp/debug.h \
			../slirp/if.h \
			../slirp/ip.h \
			../slirp/ip_icmp.h \
			../slirp/libslirp.h \
			../slirp/main.h \
			../slirp/mbuf.h \
			../slirp/misc.h \
			../slirp/sbuf.h \
			../slirp/slirp_config.h \
			../slirp/slirp.h \
			../slirp/socket.h \
			../slirp/tcp.h \
			../slirp/tcpip.h \
			../slirp/tcp_timer.h \
			../slirp/tcp_var.h \
			../slirp/tftp.h \
			../slirp/udp.h

	SOURCES +=	../slirp/bootp.c \
			../slirp/cksum.c \
			../slirp/cutils.c \
			../slirp/if.c \
			../slirp/ip_icmp.c \
			../slirp/ip_input.c \
			../slirp/ip_output.c \
			../slirp/mbuf.c \
			../slirp/misc.c \
			../slirp/sbuf.c \
			../slirp/slirp.c \
			../slirp/socket.c \
			../slirp/tcp_input.c \
			../slirp/tcp_output.c \
			../slirp/tcp_subr.c \
			../slirp/tcp_timer.c \
			../slirp/udp.c

	DEFINES += CONFIG_SLIRP

	# Libraries needed for NAT Networking
	win32 {
		LIBS += -liphlpapi -lws2_32
	}
}

RESOURCES =	icon.qrc

win32 { 
	SOURCES +=	../win/cdrom-ioctl.c \
			../win/network-win.c \
			../network.c \
			../hostfs-win.c \
			network_dialog.cpp \
			../win/tap-win32.c \
			../win/rpc-win.c \
			keyboard_win.c
	HEADERS +=	../network.h \
			network_dialog.h

	RC_ICONS = ../win/rpcemu.ico

	# Enable Data Execution Prevention (DEP)
	QMAKE_LFLAGS = -Wl,--nxcompat
}

linux {
	SOURCES +=	../cdrom-linuxioctl.c
}

linux | wasm {
	SOURCES +=	../network-linux.c \
			../network.c \
			network_dialog.cpp
	HEADERS +=	../network.h \
			network_dialog.h
}

unix | wasm {
	SOURCES +=	keyboard_x.c \
			../hostfs-unix.c \
			../rpc-linux.c
}

wasm {
	SOURCES +=	container_window.cpp
	HEADERS +=	container_window.h
}

wasm {
	QT_WASM_PTHREAD_POOL_SIZE = 3

	QMAKE_LFLAGS += -no-mimetype-database -lidbfs.js \
			--preload-file ../../roms/riscos@/roms/riscos \
			--preload-file ../../netroms@/netroms \
			--preload-file ../../poduleroms@/poduleroms \
			--preload-file ../../wasm/hostfs@/init/hostfs \
			--preload-file ../../wasm/cmos.ram@/init/user/cmos.ram \
			--preload-file ../../rpc.cfg@/init/user/rpc.cfg
}

# Place exes in top level directory
DESTDIR = ../..

CONFIG(dynarec) {
	SOURCES +=	../ArmDynarec.c
	HEADERS +=	../ArmDynarecOps.h \
			../codegen_x86_common.h

	contains(QMAKE_HOST.arch, x86_64):!win32: { # win32 always uses 32bit dynarec
		HEADERS +=	../codegen_amd64.h
		SOURCES +=	../codegen_amd64.c
	} else {
		HEADERS +=	../codegen_x86.h
		SOURCES +=	../codegen_x86.c
	}
	
	win32 {
		TARGET = RPCEmu-Recompiler
	} else {
		TARGET = rpcemu-recompiler
	}
} else {
	SOURCES +=	../arm.c \
			../codegen_null.c
	win32 {
		TARGET = RPCEmu-Interpreter
	} else {
		TARGET = rpcemu-interpreter
	}
}

# Big endian architectures
# need to find defines for sparc, arm be, mips be
contains(QMAKE_HOST.arch, ppc)|contains(QMAKE_HOST.arch, ppc64) {
	DEFINES += _RPCEMU_BIG_ENDIAN
}

CONFIG(debug, debug|release) {
	DEFINES += _DEBUG
	TARGET = $$join(TARGET, , , -debug)
}

LIBS +=
