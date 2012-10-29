#ifndef DOLPHIN_PLUGIN_OWNCLOUD_H
#define DOLPHIN_PLUGIN_OWNCLOUD_H


#include <kversioncontrolplugin2.h>
#include <KDE/KFileItem>

class QTimer;
class KAction;
class QLocalSocket;

class DolphinPluginOwnCloud : public KVersionControlPlugin2
{
    Q_OBJECT

public:
    DolphinPluginOwnCloud(QObject* parent, const QList<QVariant>& args);
    virtual ~DolphinPluginOwnCloud();

    bool beginRetrieval(const QString& directory);
    void endRetrieval();
    QList<QAction*> actions(const KFileItemList& items) const;
    ItemVersion itemVersion(const KFileItem& item) const;
    QString fileName() const;

private slots:
    void showStupidBox() const;
    void sendCommand(const QString& command, QLocalSocket* socket = 0) const;

private:
    ItemVersion itemVersionForString(const QString& version) const;

private:
    mutable QList<KUrl> m_currentUrls;

    KAction* m_dummyAction;
    QLocalSocket* m_owncloudSocket;
    QMap<QString, ItemVersion> m_status;
};

#endif