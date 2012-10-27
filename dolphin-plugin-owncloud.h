#ifndef DOLPHIN_PLUGIN_OWNCLOUD_H
#define DOLPHIN_PLUGIN_OWNCLOUD_H


#include <kversioncontrolplugin2.h>
#include <KDE/KFileItem>

class QTimer;
class KAction;

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

private:
    mutable QList<KUrl> m_currentUrls;;

    KAction* m_dummyAction;
};

#endif