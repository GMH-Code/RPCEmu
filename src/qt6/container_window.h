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
#ifndef CONTAINER_WINDOW_H
#define CONTAINER_WINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QLabel>

class ContainerWindow : public QMainWindow
{
	Q_OBJECT

public:
	ContainerWindow(void (*main_init_callback)());
	void set_contained_window(QMainWindow *pMainWin);

private slots:
	void load_timer_timeout();
	void init_timer_timeout();

private:
	QScrollArea *main_scroll_area;
	QLabel *start_label;
	QTimer *load_timer;
	QTimer *init_timer;
	void (*main_init_callback)();

	void copy_folder(const QString& source_path, const QString& destination_path);
};

#endif
