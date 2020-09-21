#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QAbstractListModel>
#include <QTranslator>

class Localizer final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ServerCountryRoles {
        LanguageRole = Qt::UserRole + 1,
        CodeRole,
    };

    Localizer() = default;
    ~Localizer() = default;

    void initialize(const QString& code);

    Q_INVOKABLE void setLanguage(const QString &code);

    // QAbstractListModel methods

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
  void loadLanguage(const QString& code);

signals:
    void languageChanged(const QString& code);

private:
    QTranslator m_translator;

    QStringList m_languages;
};

#endif // LOCALIZER_H
