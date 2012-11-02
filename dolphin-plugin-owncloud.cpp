#include "dolphin-plugin-owncloud.h"

#include <KDE/KAction>
#include <KDE/KFileItem>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KLocalizedString>
#include <KDE/KDebug>

#include <QTimer>
#include <QMessageBox>
#include <QLocalSocket>
#include <QThread>
#include <QFileInfo>
#include <QDir>

K_PLUGIN_FACTORY(DolphinPluginOwnCloudFactory, registerPlugin<DolphinPluginOwnCloud>();)
K_EXPORT_PLUGIN(DolphinPluginOwnCloudFactory("dolphin-plugin-owncloud"))


DolphinPluginOwnCloud::DolphinPluginOwnCloud(QObject* parent, const QList< QVariant >& args)
    : KVersionControlPlugin2(parent)
    , m_owncloudSocket(new QLocalSocket(this))
{
    Q_UNUSED(args);
    kDebug();

    m_publicShareLinkAction = new KAction(this);
    m_publicShareLinkAction->setIcon(KIcon(QLatin1String("internet-web-browser")));
    m_publicShareLinkAction->setText(i18nc("@item:inmenu", "Copy public link to clipboard"));

    connect(m_owncloudSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));

    m_socketPath = QDir::home().absolutePath().append(QLatin1String("/.local/share/data/ownCloud/socket"));
    kDebug() << "connect to server: " << m_socketPath;
    m_owncloudSocket->connectToServer(m_socketPath);
}

DolphinPluginOwnCloud::~DolphinPluginOwnCloud()
{

}

bool DolphinPluginOwnCloud::beginRetrieval(const QString& directory)
{
    kDebug();

    QLocalSocket* socket = new QLocalSocket();

    QString command(QLatin1String("RETRIEVE_FOLDER_STATUS:%1"));
    sendCommand(command.arg(directory), socket);
    socket->waitForReadyRead();

    while(socket->canReadLine())
    {
        QString line = QString::fromUtf8(socket->readLine().trimmed());
        QStringList splittedLine = line.split(QLatin1String(":"));

        if(splittedLine.count() != 3) continue;

        QString filePath = splittedLine.at(2);
        QString status = splittedLine.at(1);

        if(splittedLine.first().startsWith(QLatin1String("STATUS")))
        {
            QFileInfo fileInfo( filePath );
            if(fileInfo.exists())
            {
                m_status[filePath] = itemVersionForString(status);
            }
            else
                kDebug() << "received invalid file path";
        }
        else
        {
            kDebug() << "Hit race condition :\"";
            Q_ASSERT(false);
        }
    }

    socket->close();
    socket->deleteLater();

    return true;
}

void DolphinPluginOwnCloud::endRetrieval()
{
    kDebug();
    m_status.clear();
}

QList< QAction* > DolphinPluginOwnCloud::actions(const KFileItemList& items) const
{
    kDebug();
    m_currentUrls.clear();
    foreach(const KFileItem& item, items)
    {
        m_currentUrls.append(item.mostLocalUrl());
    }

    return (QList<QAction*>() << m_publicShareLinkAction);
}

KVersionControlPlugin2::ItemVersion DolphinPluginOwnCloud::itemVersion(const KFileItem& item) const
{
    return m_status.value(item.localPath(), UnversionedVersion);
}

QString DolphinPluginOwnCloud::fileName() const
{
    const QLatin1String fileName(".csync_journal.db");
    kDebug() << fileName;
    return fileName;
}

void DolphinPluginOwnCloud::sendCommand(const QString& command, QLocalSocket* socket) const
{
    kDebug() << command;

    if(!socket)
        socket = m_owncloudSocket;

    if(!socket->isOpen())
    {
        socket->connectToServer(m_socketPath);
        socket->waitForConnected();
    }

    QString localCommand = command;
    localCommand.append(QLatin1String("\n"));
    socket->write(localCommand.toUtf8());
}

void DolphinPluginOwnCloud::getPublicShareLink() const
{
    sendCommand(QString(QLatin1String("PUBLIC_SHARE_LINK:%1")).arg(m_currentUrls.first().toLocalFile()));
}


void DolphinPluginOwnCloud::onReadyRead()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    Q_ASSERT(socket);

    while(socket->canReadLine())
    {
        QString line = QString::fromUtf8(socket->readLine().trimmed());
        kDebug() << line;
        if(line.startsWith(QLatin1String("UPDATE_VIEW")))
        {
            kDebug() << "sync state changed, update view";
            emit itemVersionsChanged();
        }
    }
}


KVersionControlPlugin2::ItemVersion DolphinPluginOwnCloud::itemVersionForString(const QString& version) const
{
    if(version == QLatin1String("STATUS_NONE"))
    {
        return NormalVersion;
    }
    if(version == QLatin1String("STATUS_NEW"))
    {
        return AddedVersion;
    }
    if(version == QLatin1String("STATUS_EVAL"))
    {
        return LocallyModifiedVersion;
    }
    else if( version == QLatin1String("STATUS_STAT_ERROR") || version == QLatin1String("STATUS_ERROR"))
    {
        return UnversionedVersion;
    }
    else if(version == QLatin1String("STATUS_REMOVE") || version == QLatin1String("STATUS_UPDATED"))
    {
        kDebug() << version;
        return UnversionedVersion;
    }
    else if(version == QLatin1String("STATUS_CONFLICT"))
    {
        return ConflictingVersion;
    }
    else if(version == QLatin1String("STATUS_REMOVE"))
    {
        return RemovedVersion;
    }
    else if(version == QLatin1String("STATUS_IGNORE"))
    {
        return IgnoredVersion;
    }

    qWarning() << Q_FUNC_INFO << "unknown status: " << version;
    Q_ASSERT(false);
    return UnversionedVersion;
}
