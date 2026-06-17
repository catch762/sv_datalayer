#pragma once
#include "sv_qtcommon.h"
#include "Enum.h"
#include <QButtonGroup>
class EnumWidget : public QWidget
{
    Q_OBJECT
public:
    EnumWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        layout = new QHBoxLayout(this);
        initLayoutSpacing(layout);

        scrollArea = new HorizontalScrollAreaWidget(ScrollAreaContentHeight, this);
        //scrollArea->setFixedHeight(30);
        scrollArea->setMinimumWidth(50);

        

        layout->addWidget(scrollArea);

        enumButtonsGroup = new QButtonGroup(this);
        enumButtonsGroup->setExclusive(true);
        connect(enumButtonsGroup, &QButtonGroup::idClicked, [this](int btnIndex)
        {
            SV_LOG("BUTTON CLICKED " + std::to_string(btnIndex)); 
        });

        setMinimumWidth(50);
    }

    void setValue(Enum newValue)
    {
        enumValue = std::move(newValue);

        ensureThereIsExactlyThisCountOfEnumButtons(enumValue.entriesCount());

        updateEnumButtonsToMatchEnumValue();


        SV_LOG("OKKKK");
        emit valueChanged(enumValue);
    }

signals:
    void valueChanged(const Enum& value);

private:
    QPushButton* makeEnumButton(const QString& name = "")
    {
        //i assume buttons index will not change.
        const int buttonIndex = enumButtonsCount();

        auto btn = new QPushButton(name, this);
        btn->setCheckable(true);
        btn->setFixedHeight(ScrollAreaContentHeight);
        btn->setFlat(true);
        /*btn->setStyleSheet(
            "QPushButton {"
            "    border: none;"
            "    padding: 0px;"
            "    border-radius: 0px;"
            "    background-color: transparent;"
            "}"
            "QPushButton:hover {"
            "    background-color: #aaaaaa;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #888888;"
            "}"
        );*/

        static const QString stylePattern = 
            "QPushButton         {background-color: %1; border: none; border-radius: 0; padding: 0; margin: 0;}"
            "QPushButton:checked {background-color: %2; border: none; border-radius: 0; padding: 0; margin: 0;}"
            "QPushButton:hover:!checked   {background-color: %3; border: none; border-radius: 0; padding: 0; margin: 0;}";

        QColor stdColor = buttonIndex % 2 == 0 ? QColor(235,235,235) : QColor(229,229,229);
        QColor hoverColor = QColor(119, 184, 255);
        QColor selectColor = QColor(211, 228, 255);

        QString style = stylePattern.arg(stdColor.name(QColor::HexArgb))
                                    .arg(hoverColor.name(QColor::HexArgb))
                                    .arg(selectColor.name(QColor::HexArgb));
        btn->setStyleSheet(style);


        btn->setMinimumWidth(50);

        btn->setSizePolicy(QSizePolicy( QSizePolicy::Expanding, 
                                        QSizePolicy::Fixed ));

        

        enumButtonsGroup->addButton(btn, buttonIndex);

        

        return btn;
    }

    //they will not necessary be initialized to anything specific
    void ensureThereIsExactlyThisCountOfEnumButtons(const int requiredButtonsCount)
    {
        const int existingCount = enumButtonsCount();

        if (requiredButtonsCount > existingCount)
        {
            const int buttonsToAdd = requiredButtonsCount - existingCount;

            SV_WARN(std::format("adding {} btns", buttonsToAdd));

            for (int i = 0; i < buttonsToAdd; ++i)
            {
                scrollArea->getContentLayout()->addWidget(makeEnumButton());
            }
        }
        else if (existingCount > requiredButtonsCount)
        {
            const int buttonsToDelete = existingCount - requiredButtonsCount;

            SV_WARN(std::format("deleting {} btns", buttonsToDelete));

            deleteLastNItemsInLayout(scrollArea->getContentLayout(), buttonsToDelete);
        }
    }

    //assumes count of buttons matches count of entries already
    void updateEnumButtonsToMatchEnumValue()
    {
        SV_ASSERT(enumButtonsCount() == enumValue.entriesCount());

        for (int i = 0; i < enumButtonsCount(); ++i)
        {
            auto* btn = getEnumButton(i);
            if (!btn)
            {
                SV_ERROR(std::format("Couldnt find button at index {}, total {} buttons", i, enumButtonsCount()));
                continue;
            }

            auto* enumItem = enumValue.getEntryForIndex(i);
            if (!enumItem)
            {
                SV_ERROR(std::format("Couldnt find enum item at index {}, total {} buttons and {} enum items",
                     i, enumButtonsCount(), enumValue.entriesCount()));
                continue;
            }

            btn->setText(enumItem->name);
        }

        if(auto* btnThatShouldBeChecked = getEnumButton(enumValue.getCurrentIndex()))
        {
            btnThatShouldBeChecked->setChecked(true);
        }
        else SV_ERROR(std::format("Enum widget couldnt select button {}", enumValue.getCurrentIndex()));
    }

    int enumButtonsCount() const
    {
        return scrollArea->getContentLayout()->count();
    }

    QPushButton* getEnumButton(int index)
    {
        if (!isValidIndex(index, enumButtonsCount())) return nullptr;

        return dynamic_cast<QPushButton*>( scrollArea->getContentLayout()->itemAt(index)->widget() );
    }

private:
    Enum enumValue;
    QButtonGroup* enumButtonsGroup = nullptr; 
    const int ScrollAreaContentHeight = 22;
private:
    QHBoxLayout*                layout          = nullptr;
    HorizontalScrollAreaWidget*     scrollArea  = nullptr; //buttons for enum values live here

   
};