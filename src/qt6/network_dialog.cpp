/*
  RPCEmu - An Acorn system emulator

  Copyright (C) 2016-2017 Matthew Howkins

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
#include <iostream>

#include <QMessageBox>

#include "main_window.h"
#include "network_dialog.h"

#include "network.h"

NetworkDialog::NetworkDialog(Emulator &emulator, Config *config_copy, QWidget *parent)
    : QDialog(parent),
	emulator(emulator),
	config_copy(config_copy)
{
	setWindowTitle("Configure RPCEmu Networking");

	// Create actions

	// Create widgets and layout
	net_off = new QRadioButton("Off");
	net_nat = new QRadioButton("Network Address Translation (NAT)");
	net_bridging = new QRadioButton("Ethernet Bridging");
	net_tunnelling = new QRadioButton("IP Tunnelling");

	bridge_label = new QLabel("Bridge Name");
	bridge_name = new QLineEdit(QString("rpcemu"));
	bridge_name->setMinimumWidth(192);
	bridge_hbox = new QHBoxLayout();
	bridge_hbox->insertSpacing(0, 48);
	bridge_hbox->addWidget(bridge_label);
	bridge_hbox->addWidget(bridge_name);

	tunnelling_label = new QLabel("IP Address");
	tunnelling_name = new QLineEdit(QString("172.31.0.1"));
	tunnelling_name->setMinimumWidth(192);
	tunnelling_hbox = new QHBoxLayout();
	tunnelling_hbox->insertSpacing(0, 48);
	tunnelling_hbox->addWidget(tunnelling_label);
	tunnelling_hbox->addWidget(tunnelling_name);

	// Create Buttons
	buttons_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);


	// Main layout
	vbox = new QVBoxLayout(this);
	vbox->addWidget(net_off);
	vbox->addWidget(net_nat);
	vbox->addWidget(net_bridging);
	vbox->addLayout(bridge_hbox);

	// IP Tunnelling is linux only
#if defined(Q_OS_LINUX)
	vbox->addWidget(net_tunnelling);
	vbox->addLayout(tunnelling_hbox);
#endif /* linux */

	vbox->addWidget(buttons_box);

	// Connect actions to widgets
	connect(net_off, &QRadioButton::clicked, this, &NetworkDialog::radio_clicked);
	connect(net_nat, &QRadioButton::clicked, this, &NetworkDialog::radio_clicked);
	connect(net_bridging, &QRadioButton::clicked, this, &NetworkDialog::radio_clicked);
	connect(net_tunnelling, &QRadioButton::clicked, this, &NetworkDialog::radio_clicked);

	connect(buttons_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

	connect(this, &QDialog::accepted, this, &NetworkDialog::dialog_accepted);
	connect(this, &QDialog::rejected, this, &NetworkDialog::dialog_rejected);

	// Set the values of the window to the config values
	applyConfig();

	// Remove resize on Dialog
	this->setFixedSize(this->sizeHint());
}

NetworkDialog::~NetworkDialog()
{
}

void
NetworkDialog::radio_clicked()
{
	if (net_bridging->isChecked()) {
		bridge_label->setEnabled(true);
		bridge_name->setEnabled(true);
	} else {
		bridge_label->setEnabled(false);
		bridge_name->setEnabled(false);
	}

	if (net_tunnelling->isChecked()) {
		tunnelling_label->setEnabled(true);
		tunnelling_name->setEnabled(true);
	} else {
		tunnelling_label->setEnabled(false);
		tunnelling_name->setEnabled(false);
	}
}

/**
 * User clicked OK on the Networking dialog box 
 */
void
NetworkDialog::dialog_accepted()
{
	QByteArray ba_bridgename, ba_ipaddress;
	char *bridgename, *ipaddress;
	NetworkType network_type = NetworkType_Off;

	// Take a copy of the existing config
	Config new_config;
	memcpy(&new_config, config_copy, sizeof(Config));

	// Fill in the choices from the dialog box
	if (net_off->isChecked()) {
		network_type = NetworkType_Off;
	} else if (net_nat->isChecked()) {
		network_type = NetworkType_NAT;
	} else if (net_bridging->isChecked()) {
		network_type = NetworkType_EthernetBridging;
	} else if (net_tunnelling->isChecked()) {
		network_type = NetworkType_IPTunnelling;
	}

	new_config.network_type = network_type;

	// Compare against existing config and see if it will cause a reset
	if (rpcemu_config_is_reset_required(&new_config, machine.model)) {
		int ret = MainWindow::reset_question(parentWidget());

		if (ret == QMessageBox::Cancel) {
			// Set the values in the dialog back to the current settings
			applyConfig();
			return;
		}
	}

	// By this point we either don't need to reset, or have the user's permission to reset

	// Update network config in emulator thread
	emit this->emulator.network_config_updated_signal(network_type,
	    bridge_name->text(), tunnelling_name->text());

	ba_bridgename = bridge_name->text().toUtf8();
	bridgename = ba_bridgename.data();

	ba_ipaddress = tunnelling_name->text().toUtf8();
	ipaddress = ba_ipaddress.data();

	// Apply configuration settings from Dialog to config_copy
	config_copy->network_type = network_type;
	if (config_copy->bridgename == NULL) {
		config_copy->bridgename = strdup(bridgename);
	} else if (strcmp(config_copy->bridgename, bridgename) != 0) {
		free(config_copy->bridgename);
		config_copy->bridgename = strdup(bridgename);
	}
	if (config_copy->ipaddress == NULL) {
		config_copy->ipaddress = strdup(ipaddress);
	} else if (strcmp(config_copy->ipaddress, ipaddress) != 0) {
		free(config_copy->ipaddress);
		config_copy->ipaddress = strdup(ipaddress);
	}
}

/**
 * User clicked cancel on the Networking dialog box 
 */
void
NetworkDialog::dialog_rejected()
{
	// Set the values in the dialog back to the current settings
	applyConfig();
}

/**
 * Set the values in the networking dialog box based on the current
 * values of the GUI config copy
 */
void
NetworkDialog::applyConfig()
{
//	if windows and iptunnelling, net = off

	// Select the correct radio button
	net_off->setChecked(false);
	net_nat->setChecked(false);
	net_bridging->setChecked(false);
	net_tunnelling->setChecked(false);
	switch (config_copy->network_type) {
	case NetworkType_Off:
		net_off->setChecked(true);
		break;
	case NetworkType_NAT:
		net_nat->setChecked(true);
		break;
	case NetworkType_EthernetBridging:
		net_bridging->setChecked(true);
		break;
	case NetworkType_IPTunnelling:
		net_tunnelling->setChecked(true);
		break;
	}

	// Use the helper function to grey out the boxes of unselected
	// network types
	radio_clicked();

	if(config_copy->bridgename && config_copy->bridgename[0] != '\0') {
		bridge_name->setText(config_copy->bridgename);
	}

	if(config_copy->ipaddress && config_copy->ipaddress[0] != '\0') {
		tunnelling_name->setText(config_copy->ipaddress);
	}
}
