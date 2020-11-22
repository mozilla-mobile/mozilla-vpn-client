/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPMODEL_H
#define HELPMODEL_H

#include <QAbstractListModel>

class HelpModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(HelpModel)

public:
    enum HelpRoles {
        HelpEntryRole = Qt::UserRole + 1,
        HelpIdRole,
        HelpExternalLinkRole,
    };

    HelpModel();
    ~HelpModel();

    Q_INVOKABLE void open(int id);

    void forEach(std::function<void(const QString &name, int id)> &&callback);

    // QAbstractListModel methods

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &index, int role) const override;
};

#endif // HELPMODEL_H
