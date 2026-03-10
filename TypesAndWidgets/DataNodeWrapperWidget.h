#pragma once
#include "sv_qtcommon.h"

class DataNodeWrapperWidget : public QFrame
{
    Q_OBJECT
public:
    DataNodeWrapperWidget(  const QList<QWidget*>& contentWidgets = {},
                            const QString &name = {},
                            QWidget *parent = nullptr );

    void setExpanded(bool expanded);                        
private:
    void createAndInitTopStripe(const QString &name);
    void iterateContentWidgets(std::function<void(QWidget*)> visitor);
    
    void setContentWidgetsVisibleStatus(bool visible);

private:
    QVBoxLayout*    layout                              = nullptr;
    QWidget*            topStripe                       = nullptr;
    QPushButton*            stripeShowHideContentButton = nullptr;
    QLabel*                 stripeNameLabel             = nullptr;
    //                  >And all the content widgets will be added here, one by one after 'topStripe'
};