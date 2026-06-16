#pragma once
#include "sv_qtcommon.h"
#include "Enum.h"

class EnumWidget : public QWidget
{
    Q_OBJECT
public:
    EnumWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        layout = new QHBoxLayout(this);
        initLayoutSpacing(layout);

        scrollArea = new HorizontalScrollAreaWidget(this);
        scrollArea->setFixedHeight(30);
        scrollArea->setMinimumWidth(50);

        layout->addWidget(scrollArea);

        setFixedHeight(30);
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
        auto btn = new QPushButton(name, this);
        //btn->setFixedSize(50, 25);

        // Set minimum size (100px)
    btn->setMinimumWidth(50);
    
    // Allow button to expand
    btn->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, 
                                      QSizePolicy::Fixed));

        //i assume buttons index will not change.
        int buttonIndex = enumButtonsCount();

        connect(btn, &QPushButton::clicked, [this, buttonIndex]()
        {
           SV_LOG("BUTTON CLICKED " + std::to_string(buttonIndex)); 
        });

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
private:
    QHBoxLayout*                layout          = nullptr;
    HorizontalScrollAreaWidget*     scrollArea  = nullptr;
};