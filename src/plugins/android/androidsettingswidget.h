/**************************************************************************
**
** Copyright (c) 2014 BogDan Vatra <bog_dan_ro@yahoo.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef ANDROIDSETTINGSWIDGET_H
#define ANDROIDSETTINGSWIDGET_H

#include "androidconfigurations.h"

#include <QList>
#include <QString>
#include <QWidget>
#include <QAbstractTableModel>

QT_BEGIN_NAMESPACE
class Ui_AndroidSettingsWidget;
QT_END_NAMESPACE

namespace Android {
namespace Internal {

class AvdModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    void setAvdList(const QVector<AndroidDeviceInfo> &list);
    QString avdName(const QModelIndex &index) const;

protected:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    QVector<AndroidDeviceInfo> m_list;
};

class AndroidSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    // Todo: This would be so much simpler if it just used Utils::PathChooser!!!
    AndroidSettingsWidget(QWidget *parent = 0);
    ~AndroidSettingsWidget();

    void saveSettings();

private slots:
    void sdkLocationEditingFinished();
    void ndkLocationEditingFinished();
    void searchForAnt(const Utils::FileName &location);
    void antLocationEditingFinished();
    void openJDKLocationEditingFinished();
    void browseSDKLocation();
    void browseNDKLocation();
    void browseAntLocation();
    void browseOpenJDKLocation();
    void openSDKDownloadUrl();
    void openNDKDownloadUrl();
    void openAntDownloadUrl();
    void openOpenJDKDownloadUrl();
    void addAVD();
    void removeAVD();
    void startAVD();
    void avdActivated(QModelIndex);
    void dataPartitionSizeEditingFinished();
    void manageAVD();
    void createKitToggled();

private:
    enum Mode { Sdk = 1, Ndk = 2, Java = 4, All = Sdk | Ndk | Java };
    enum State { NotSet = 0, Okay = 1, Error = 2 };
    void check(Mode mode);
    void applyToUi(Mode mode);
    bool sdkLocationIsValid() const;
    bool sdkPlatformToolsInstalled() const;

    State m_sdkState;
    State m_ndkState;
    QString m_ndkErrorMessage;
    int m_ndkCompilerCount;
    QString m_ndkMissingQtArchs;
    State m_javaState;

    Ui_AndroidSettingsWidget *m_ui;
    AndroidConfig m_androidConfig;
    AvdModel m_AVDModel;
};

} // namespace Internal
} // namespace Android

#endif // ANDROIDSETTINGSWIDGET_H
