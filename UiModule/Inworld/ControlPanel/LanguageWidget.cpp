// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "LanguageWidget.h"
#include "Inworld/ControlPanelManager.h"

#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QDir>
#include <QTranslator>
#include <QApplication>
#include <QDebug>
#include <QListWidgetItem>
#include "MemoryLeakCheck.h"

namespace CoreUi
{
    LanguageWidget::LanguageWidget(QObject* settings_widget) : 
        QWidget(), translator_(0)
    {
        setupUi(this);
        connect(settings_widget, SIGNAL(SaveSettingsClicked()), this, SLOT(ExportSettings()));
        connect(settings_widget, SIGNAL(CancelClicked()), this, SLOT(CancelClicked()));
        QVBoxLayout *layout = this->findChild<QVBoxLayout *>("verticalLayout");
        QListWidget* lstWidget = this->findChild<QListWidget* >("listWidget");
        
   
        QStringList qmFiles = findQmFiles();
        
        for (int i = 0; i < qmFiles.size(); ++i) 
        {
         QString name = LanguageName(qmFiles[i]);
         QListWidgetItem* item = new QListWidgetItem(name, lstWidget);
         qmFileForCheckBoxMap.insert(item, qmFiles[i]);
        
        }
       connect(lstWidget, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(ItemPressed(QListWidgetItem*)));
       translator_ = new QTranslator();

    }

    void LanguageWidget::ExportSettings()
    {
        //todo
    }

    LanguageWidget::~LanguageWidget()
    {
        delete translator_;
        translator_ = 0;
    }

    void LanguageWidget::CancelClicked()
    {
        //todo
    }
    
    QStringList LanguageWidget::findQmFiles()
    {
         QDir dir("data/translations");
         QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files,
                                               QDir::Name);
         QMutableStringListIterator i(fileNames);
         while (i.hasNext()) {
             i.next();
             i.setValue(dir.filePath(i.value()));
         }
         return fileNames;
    }

    void LanguageWidget::ItemPressed(QListWidgetItem* item)
    {
        // Remove old
        QApplication::removeTranslator(translator_);

        QString file = qmFileForCheckBoxMap[item];
        if ( translator_->load(qmFileForCheckBoxMap[item]))
            QApplication::installTranslator(translator_); 
    }

  

    QString LanguageWidget::LanguageName(QString qmFile)
    {
        qmFile.chop(3);
        QString str = qmFile.right(2);

        QLocale loc(str);
        QString name = loc.languageToString(loc.language());
   
        switch ( loc.language())
        {
            case QLocale::Finnish:
            {
                name = "Suomi";
                break;
            }
            case QLocale::Swedish:
            {
                name = "Svenska";
                break;
            }

        }

        return name;

    }

}