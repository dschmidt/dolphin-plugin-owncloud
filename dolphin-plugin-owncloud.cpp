#include "dolphin-plugin-owncloud.h"

#include <KDE/KAction>
#include <KDE/KFileItem>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KLocalizedString>

#include <QDebug>
#include <QTimer>
#include <QMessageBox>

K_PLUGIN_FACTORY(DolphinPluginOwnCloudFactory, registerPlugin<DolphinPluginOwnCloud>();)
K_EXPORT_PLUGIN(DolphinPluginOwnCloudFactory("dolphin-plugin-owncloud"))


DolphinPluginOwnCloud::DolphinPluginOwnCloud(QObject* parent, const QList< QVariant >& args)
    : KVersionControlPlugin2(parent)
{
    Q_UNUSED(args);
    qDebug() << Q_FUNC_INFO;

    m_dummyAction = new KAction(this);
    m_dummyAction->setIcon(KIcon("internet-web-browser"));
    m_dummyAction->setText(i18nc("@item:inmenu", "Do something not very useful..."));
    connect(m_dummyAction, SIGNAL(triggered()),
            this, SLOT(showStupidBox()));

}

DolphinPluginOwnCloud::~DolphinPluginOwnCloud()
{

}

bool DolphinPluginOwnCloud::beginRetrieval(const QString& directory)
{
    qDebug() << Q_FUNC_INFO;

    return true;
}

void DolphinPluginOwnCloud::endRetrieval()
{
    qDebug() << Q_FUNC_INFO;
}

QList< QAction* > DolphinPluginOwnCloud::actions(const KFileItemList& items) const
{
    qDebug() << Q_FUNC_INFO;
    m_currentUrls.clear();
    foreach(const KFileItem& item, items)
    {
        m_currentUrls.append(item.mostLocalUrl());
    }

    return (QList<QAction*>() << m_dummyAction);
}

KVersionControlPlugin2::ItemVersion DolphinPluginOwnCloud::itemVersion(const KFileItem& item) const
{
    qDebug() << Q_FUNC_INFO << item.localPath();

    // pretend everything is synced until we have an api for this
    return NormalVersion;
}

QString DolphinPluginOwnCloud::fileName() const
{
    const QLatin1String fileName(".csync_journal.db");
    qDebug() << Q_FUNC_INFO << fileName;
    return QLatin1String(fileName);
}

void DolphinPluginOwnCloud::showStupidBox() const
{
    QMessageBox::information(0, "Stupid Alert", "Something useful could happen instead with " + m_currentUrls.first().toLocalFile());
}
