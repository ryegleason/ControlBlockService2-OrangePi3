/**
 * (c) Copyright 2017  Florian Mueller (contact@petrockblock.com)
 * https://github.com/petrockblog/ControlBlock2
 *
 * Permission to use, copy, modify and distribute the program in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * This program is freeware for PERSONAL USE only. Commercial users must
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for the program or software derived from the program.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 */

#include <stdlib.h>
#include "fmt/format.h"
#include "PowerSwitch.h"
#include "Logger.h"

PowerSwitch::PowerSwitch(IDigitalIO& digitalIOReference, ShutdownActivated doShutdownValue) :
        doShutdown(doShutdownValue),
        isShutdownInitiatedValue(false),
        digitalIO(digitalIOReference)
{
    digitalIO.configureDevice(IDigitalIO::DIO_DEVICE_POWERSWITCH);

    ::gpiod::chip chip("gpiochip1");

    powerSwitchIn_port_ = std::make_shared<::gpiod::line>(chip.get_line(POWER_SWITCH_IN_GPIO));
    powerSwitchIn_port_->request({"gpiochip1", ::gpiod::line_request::DIRECTION_INPUT, 0}, 0);
    
	powerSwitchOut_port_ = std::make_shared<::gpiod::line>(chip.get_line(POWER_SWITCH_OUT_GPIO));
    setPowerSignal(PowerState::ON);

    Logger::logMessage(fmt::format("Created PowerSwitch. doShutdown: {}", doShutdownValue));
}

void PowerSwitch::update()
{
    if ((doShutdown == ShutdownActivated::ACTIVATED) && (getShutdownSignal() ==  ShutdownSignal::ACTIVATED)
            && (!isShutdownInitiatedValue)) {
        const std::string kShutdownCommand {"/etc/controlblockswitchoff.sh &"};
        Logger::logMessage("Initiating shutdown. Calling");
        system(kShutdownCommand.c_str());
        isShutdownInitiatedValue = true;
    }
}

bool PowerSwitch::isShutdownInitiated() const
{
    return isShutdownInitiatedValue;
}

void PowerSwitch::setPowerSignal(PowerState state)
{
    if (state == PowerState::OFF) {
        powerSwitchOut_port_->request({"gpiochip1", ::gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
        Logger::logMessage("Disabled power signal.");
    }
    else {
      powerSwitchOut_port_->request({"gpiochip1", ::gpiod::line_request::DIRECTION_OUTPUT, 0}, 1);
        Logger::logMessage("Enabled power signal.");
    }
}

PowerSwitch::ShutdownSignal PowerSwitch::getShutdownSignal()
{
    ShutdownSignal signal;
    if (!powerSwitchIn_port_->get_value()) {
        signal = ShutdownSignal::DEACTIVATED;
    }
    else {
        signal = ShutdownSignal::ACTIVATED;
    }
    return signal;
}
