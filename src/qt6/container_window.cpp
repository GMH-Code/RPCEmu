/*
  RPCEmu - An Acorn system emulator

  This file Copyright (C) 2023 Gregory Maynard-Hoare

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <emscripten.h>
#include <QTimer>
#include <QDir>
#include <QFile>

#include "container_window.h"
#include "rpcemu.h"

/**
 * WebAssembly container window for the main application.
 *
 * This is necessary to allow Qt (WASM) to start up properly, and it also
 * handles setup of the filesystem before the main window is added.
 */
ContainerWindow::ContainerWindow(void (*main_init_callback)())
    : main_init_callback(main_init_callback)
{
	// Set window title
	setWindowTitle("RPCEmu");

	// Set up container for main window
	QTabWidget *central_widget = new QTabWidget; // Regular QWidget flickers when undersized
	main_layout_widget = new QBoxLayout(QBoxLayout::LeftToRight, central_widget);
	main_layout_widget->setContentsMargins(0, 0, 0, 0);
	start_label = new QLabel("RPCEmu-WASM is starting...");
	start_label->setObjectName(QString("InitLbl"));
	start_label->setStyleSheet("QLabel#InitLbl {color: white;}");
	main_layout_widget->addWidget(start_label);
	main_layout_widget->setAlignment(start_label, Qt::AlignCenter);
	central_widget->setObjectName(QString("Bkg"));
	central_widget->setStyleSheet("QWidget#Bkg {background-color: #222;}");
	setCentralWidget(central_widget);
	setCursor(Qt::WaitCursor);

	// Initialise the filesystem when there are no more Qt events
	load_timer = new QTimer(this);
	connect(load_timer, &QTimer::timeout, this, &ContainerWindow::load_timer_timeout);
	load_timer->setSingleShot(true);
	load_timer->start(0);

	// Check for filesystem initialisation (later)
	init_timer = new QTimer(this);
	connect(init_timer, &QTimer::timeout, this, &ContainerWindow::init_timer_timeout);
}

/**
 * Replace the 'loading' label with the emulator display
 */
void
ContainerWindow::set_contained_window(QMainWindow *main_window)
{
	start_label->hide();
	main_layout_widget->addWidget(main_window);
	main_layout_widget->setAlignment(main_window, Qt::AlignCenter);
}

/**
 * Check after one interval to ensure the inital container window has been
 * drawn, but before the emulator reads any data from MEMFS/IDBFS
 */
void
ContainerWindow::load_timer_timeout()
{
	rpclog("Requesting delayed async filesystem build...\n");

	EM_ASM(
		console.info("Mounting data folders...");
		FS.mkdir("/user");
		FS.mkdir("/hostfs");
		FS.mount(IDBFS, {}, "/user");
		FS.mount(IDBFS, {}, "/hostfs");
		console.info("Mounted.  Now loading data...");
		FS.syncfs(true, function (err) {
			if (err) {
				alert_msg = "Failed to load data: " + err;
				console.warn(alert_msg);
			} else {
				alert_msg = "Data folders mounted and loaded from browser database.";
				console.info(alert_msg);
			}

			FS.mkdir("/tmp/started");
		});
	);

	init_timer->start(100);
}

/**
 * Check at regular intervals to see if the filesystem has finished building
 */
void
ContainerWindow::init_timer_timeout()
{
	printf("Waiting for data load...\n");

	// Files and folders are restored asynchronously, but this folder is always
	// written last, after the restoration is complete.  By doing this, we
	// don't need a callback from async JavaScript, which in testing was not
	// always reliable.
	QDir started_dir("/tmp/started");

	if (!started_dir.exists())
		return;

	init_timer->stop();

	rpclog("Load complete.  Checking for presence of user data...\n");
	QFile test_file("/user/cmos.ram");

	if (test_file.exists()) {
		rpclog("User data present.\n");
	} else {
		rpclog("User data not present.  Preparing defaults...\n");
		copy_folder("/init", "/");
	}

	setCursor(Qt::ArrowCursor);
	main_init_callback();
}

/**
 * Recursively copy a folder from one path to another
 */
void
ContainerWindow::copy_folder(const QString& source_path, const QString& destination_path)
{
	QDir source_dir(source_path);
	QDir dest_dir(destination_path);

	if (!dest_dir.exists()) {
		dest_dir.mkpath(".");
	}

	QStringList source_files = source_dir.entryList(QDir::Files);

	for (const QString& source_file : source_files) {
		QString src_file_path = source_dir.filePath(source_file);
		QString dst_file_path = dest_dir.filePath(source_file);
		QFile::copy(src_file_path, dst_file_path);
	}

	QStringList sub_dirs = source_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	for (const QString& sub_dir : sub_dirs) {
		QString src_sub_dir = source_dir.filePath(sub_dir);
		QString dest_sub_dir = dest_dir.filePath(sub_dir);
		copy_folder(src_sub_dir, dest_sub_dir);
	}
}
