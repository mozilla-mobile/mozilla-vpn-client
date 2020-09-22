#ifndef LOCALIZER_H
#define LOCALIZER_H

#include <QAbstractListModel>
#include <QTranslator>

class Localizer final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString systemLanguage READ systemLanguage CONSTANT)
    Q_PROPERTY(QString systemLocalizedLanguage READ systemLocalizedLanguage CONSTANT)
    Q_PROPERTY(QString code READ code NOTIFY languageChanged)

public:
    enum ServerCountryRoles {
        LanguageRole = Qt::UserRole + 1,
        LocalizedLanguageRole,
        CodeRole,
    };

    Localizer() = default;
    ~Localizer() = default;

    void initialize(const QString &code);

    Q_INVOKABLE void setLanguage(const QString &code);

    QString code() const { return m_code; }

    QString systemLanguage() const { return languageName(""); }

    QString systemLocalizedLanguage() const { return localizedLanguageName(""); }

    // QAbstractListModel methods

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    void loadLanguage(const QString &code);

    QString languageName(const QString &code) const;
    QString localizedLanguageName(const QString &code) const;

signals:
    void languageChanged(const QString &code);

private:
    QTranslator m_translator;

    QString m_code;
    QStringList m_languages;
};

#endif // LOCALIZER_H
