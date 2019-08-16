/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "desktopdevice.h"
#include "desktopdeviceprocess.h"
#include "deviceprocesslist.h"
#include "localprocesslist.h"
#include "desktopprocesssignaloperation.h"

#include <coreplugin/fileutils.h>

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/runcontrol.h>

#include <ssh/sshconnection.h>

#include <utils/environment.h>
#include <utils/hostosinfo.h>
#include <utils/portlist.h>
#include <utils/stringutils.h>
#include <utils/url.h>

#include <QCoreApplication>

using namespace ProjectExplorer::Constants;
using namespace Utils;

namespace ProjectExplorer {

DesktopDevice::DesktopDevice()
{
    setupId(IDevice::AutoDetected, DESKTOP_DEVICE_ID);
    setType(DESKTOP_DEVICE_TYPE);
    setDefaultDisplayName(QCoreApplication::translate("ProjectExplorer::DesktopDevice",
                                                      "Local PC"));
    setDisplayType(QCoreApplication::translate("ProjectExplorer::DesktopDevice", "Desktop"));

    setDeviceState(IDevice::DeviceStateUnknown);
    setMachineType(IDevice::Hardware);
    setOsType(HostOsInfo::hostOs());

    const QString portRange =
            QString::fromLatin1("%1-%2").arg(DESKTOP_PORT_START).arg(DESKTOP_PORT_END);
    setFreePorts(Utils::PortList::fromString(portRange));
    setOpenTerminal([](const Utils::Environment &env, const QString &workingDir) {
        Core::FileUtils::openTerminal(workingDir, env);
    });
}

IDevice::DeviceInfo DesktopDevice::deviceInformation() const
{
    return DeviceInfo();
}

IDeviceWidget *DesktopDevice::createWidget()
{
    return nullptr;
    // DesktopDeviceConfigurationWidget currently has just one editable field viz. free ports.
    // Querying for an available port is quite straightforward. Having a field for the port
    // range can be confusing to the user. Hence, disabling the widget for now.
}

bool DesktopDevice::canAutoDetectPorts() const
{
    return true;
}

bool DesktopDevice::canCreateProcessModel() const
{
    return true;
}

DeviceProcessList *DesktopDevice::createProcessListModel(QObject *parent) const
{
    return new Internal::LocalProcessList(sharedFromThis(), parent);
}

DeviceProcess *DesktopDevice::createProcess(QObject *parent) const
{
    return new Internal::DesktopDeviceProcess(sharedFromThis(), parent);
}

DeviceProcessSignalOperation::Ptr DesktopDevice::signalOperation() const
{
    return DeviceProcessSignalOperation::Ptr(new DesktopProcessSignalOperation());
}

class DesktopDeviceEnvironmentFetcher : public DeviceEnvironmentFetcher
{
public:
    DesktopDeviceEnvironmentFetcher() = default;

    void start() override
    {
        emit finished(Utils::Environment::systemEnvironment(), true);
    }
};

DeviceEnvironmentFetcher::Ptr DesktopDevice::environmentFetcher() const
{
    return DeviceEnvironmentFetcher::Ptr(new DesktopDeviceEnvironmentFetcher());
}

class DesktopPortsGatheringMethod : public PortsGatheringMethod
{
    Runnable runnable(QAbstractSocket::NetworkLayerProtocol protocol) const override
    {
        // We might encounter the situation that protocol is given IPv6
        // but the consumer of the free port information decides to open
        // an IPv4(only) port. As a result the next IPv6 scan will
        // report the port again as open (in IPv6 namespace), while the
        // same port in IPv4 namespace might still be blocked, and
        // re-use of this port fails.
        // GDBserver behaves exactly like this.

        Q_UNUSED(protocol)

        Runnable runnable;
        if (HostOsInfo::isWindowsHost() || HostOsInfo::isMacHost()) {
            runnable.executable = FilePath::fromString("netstat");
            runnable.commandLineArguments =  "-a -n";
        } else if (HostOsInfo::isLinuxHost()) {
            runnable.executable = FilePath::fromString("/bin/sh");
            runnable.commandLineArguments = "-c 'cat /proc/net/tcp*'";
        }
        return runnable;
    }

    QList<Utils::Port> usedPorts(const QByteArray &output) const override
    {
        QList<Utils::Port> ports;
        const QList<QByteArray> lines = output.split('\n');
        for (const QByteArray &line : lines) {
            const Port port(Utils::parseUsedPortFromNetstatOutput(line));
            if (port.isValid() && !ports.contains(port))
                ports.append(port);
        }
        return ports;
    }
};

PortsGatheringMethod::Ptr DesktopDevice::portsGatheringMethod() const
{
    return DesktopPortsGatheringMethod::Ptr(new DesktopPortsGatheringMethod);
}

QUrl DesktopDevice::toolControlChannel(const ControlChannelHint &) const
{
    QUrl url;
    url.setScheme(Utils::urlTcpScheme());
    url.setHost("localhost");
    return url;
}

} // namespace ProjectExplorer
